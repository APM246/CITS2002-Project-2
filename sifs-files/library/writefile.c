#include "helperfunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#define NO_CONTIGUOUS_BLOCKS -1

// find contiguous blocks to store contents of files - returns first blockID of data blocks 
int find_contiguous_blocks(size_t nbytes, size_t blocksize, int nblocks, const char *volumename, int *nblocks_needed)
{
    *nblocks_needed = ceil(((double) nbytes)/blocksize); printf("\nnbytes: %lu, blocksize: %lu, needed: %i\n", nbytes, blocksize, *nblocks_needed);
    FILE *fp = fopen(volumename, "r"); 
    char bitmap[nblocks];
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    fread(bitmap, sizeof(bitmap), 1, fp);

    if (nblocks < *nblocks_needed + 1 + 1)
    {
        return NO_CONTIGUOUS_BLOCKS;
    }
    
    for (int i = 0; i < nblocks; i++)
    {
        for (int j = 0; j < *nblocks_needed; j++)
        {
            if (j == *nblocks_needed - 1 && bitmap[i+j] == SIFS_UNUSED) return i;
            else if (bitmap[i+j] != SIFS_UNUSED) break;
        }
    }

    return NO_CONTIGUOUS_BLOCKS;
}


// add a copy of a new file to an existing volume
int SIFS_writefile(const char *volumename, const char *pathname,
		   void *data, size_t nbytes)
{
    // NO SUCH VOLUME 
    if (access(volumename, F_OK) != 0)
    {
        SIFS_errno	= SIFS_ENOVOL;
        return 1;
    }    

    // check directory validity 

    // ACCESS VOLUME INFORMATION
    int nblocks, blocksize, fileblockID, firstblockID, nblocks_needed;
    get_volume_header_info(volumename, &blocksize, &nblocks);

    // CHANGE BITMAP TO ADD FILE BLOCK
    change_bitmap(volumename, SIFS_FILE, &fileblockID, nblocks);

    firstblockID = find_contiguous_blocks(nbytes, blocksize, nblocks, volumename, &nblocks_needed);

    if (firstblockID == NO_CONTIGUOUS_BLOCKS)
    {
        SIFS_errno = SIFS_ENOSPC;
        return 1;
    }

    printf("\nfirstID: %i\n", firstblockID);
    char *file_name = find_name(pathname);

    // INITIALISE FILEBLOCK
    SIFS_FILEBLOCK fileblock;
    memset(&fileblock, 0, sizeof(fileblock));
    strcpy(fileblock.filenames[0], file_name); // need generalisation
    fileblock.firstblockID = firstblockID;
    fileblock.length = nbytes;
    fileblock.modtime = time(NULL);
    fileblock.nfiles++;
    memcpy(&fileblock.md5, MD5_file(file_name), MD5_STRLEN + 1); //printf("\n%s, strlen: %lu\n", fileblock.md5, strlen(MD5_file(file_name)));

    // WRITE THE FILEBLOCK TO THE VOLUME 
    FILE *fp = fopen(volumename, "r+");
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + fileblockID*blocksize, SEEK_SET);
    fwrite(&fileblock, sizeof(fileblock), 1, fp);

    // WRITE THE ACTUAL FILE TO THE VOLUME
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + firstblockID*blocksize, SEEK_SET);
    fwrite(data, nbytes, 1, fp);

    // UPDATE BITMAP
    char bitmap[nblocks];
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    fread(bitmap, sizeof(bitmap), 1, fp);
    for (int i = firstblockID; i < firstblockID + nblocks_needed; i++)
    {
        bitmap[i] = SIFS_DATABLOCK;
    }
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    fwrite(bitmap, sizeof(bitmap), 1, fp);

    // UPDATE PARENT DIRECTORY - MODTIME, NENTRIES AND ENTRIES ARRAY
    int parent_blockID = find_parent_blockID(volumename, pathname, nblocks, blocksize);
    size_t jump = sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + parent_blockID*blocksize; // use macro
    fseek(fp, jump, SEEK_SET);
    SIFS_DIRBLOCK dirblock;
    fread(&dirblock, sizeof(SIFS_DIRBLOCK), 1, fp);
    dirblock.modtime = time(NULL);
    dirblock.entries[dirblock.nentries].blockID = fileblockID;
    dirblock.entries[dirblock.nentries].fileindex = fileblock.nfiles - 1;
    dirblock.nentries++;
    
    // WRITE PARENT DIRECTORY TO VOLUME
    fseek(fp, jump, SEEK_SET);
    fwrite(&dirblock, sizeof(dirblock), 1, fp);

    fclose(fp);
    return 0;
}
