#include "helperfunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#define NO_CONTIGUOUS_BLOCKS -1

// find contiguous blocks to store contents of files - returns first blockID of data blocks 
int find_contiguous_blocks(size_t nbytes, size_t blocksize, int nblocks, const char *volumename, int *nblocks_needed)
{
    *nblocks_needed = ceil(((double) nbytes)/blocksize); 
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

    // ACCESS VOLUME INFORMATION
    int nblocks, blocksize, fileblockID, firstblockID, nblocks_needed;
    get_volume_header_info(volumename, &blocksize, &nblocks);

    // CHECK VALIDITY OF PATHNAME  
    char *start_of_pathname = extract_start_of_pathname(pathname);
    if (find_blockID(volumename, start_of_pathname, nblocks, blocksize) == -1)
    {
        SIFS_errno = SIFS_EINVAL;
        return 1;
    }

    // DETERMINE IF FILE IS IDENTICAL TO PRE-EXISTING FILE
    FILE *fp = fopen(volumename, "r+");
    unsigned char MD5buffer[MD5_BYTELEN];
    MD5_buffer(data, nbytes, MD5buffer);
    char bitmap[nblocks];
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    fread(bitmap, sizeof(bitmap), 1, fp);
    bool isIdentical = false;
    for (int i = 0; i < nblocks; i++)
    {
        if (bitmap[i] == SIFS_FILE)
        {
            SIFS_FILEBLOCK fb;
            fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + i*blocksize, SEEK_SET);
            fread(&fb, sizeof(SIFS_FILEBLOCK), 1, fp);
            if (memcmp(fb.md5, MD5buffer, MD5_BYTELEN) == 0)
            {
                isIdentical = true;
                fileblockID = i;
                break;
            }
        }
    }

    // CHANGE BITMAP TO ADD FILE BLOCK IF FILE IS NOT IDENTICAL 
    if (!isIdentical)
    {
        change_bitmap(volumename, SIFS_FILE, &fileblockID, nblocks);
        firstblockID = find_contiguous_blocks(nbytes, blocksize, nblocks, volumename, &nblocks_needed);

        if (firstblockID == NO_CONTIGUOUS_BLOCKS)
        {
            SIFS_errno = SIFS_ENOSPC;
            // REVERT CHANGE IN BITMAP (REMOVE FILEBLOCK) OR CHANGE BITMAP LATER (MAKE SURE TO SKIP PAST
            // FIRST EMPTY BLOCK IN find_contiguous_blocks() (reserve for fileblock))
            return 1;
        }
    }

    char *file_name = find_name(pathname);

    // INITIALISE FILEBLOCK OR UPDATE PRE-EXISTING FILEBLOCK IF FILE IS IDENTICAL
    SIFS_FILEBLOCK fileblock;
    if (!isIdentical)
    {
        memset(&fileblock, 0, sizeof(fileblock));
        strcpy(fileblock.filenames[0], file_name);
        fileblock.firstblockID = firstblockID;
        fileblock.length = nbytes;
        fileblock.modtime = time(NULL); 
        fileblock.nfiles = 1;
        memcpy(&fileblock.md5, MD5buffer, MD5_BYTELEN); 

        // WRITE THE ACTUAL FILE TO THE VOLUME
        fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + firstblockID*blocksize, SEEK_SET);
        fwrite(data, nbytes, 1, fp);

        // UPDATE BITMAP
        fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
        fread(bitmap, sizeof(bitmap), 1, fp);
        for (int i = firstblockID; i < firstblockID + nblocks_needed; i++)
        {
            bitmap[i] = SIFS_DATABLOCK;
        }
        fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
        fwrite(bitmap, sizeof(bitmap), 1, fp);
    }
    
    else
    {
        fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + fileblockID*blocksize, SEEK_SET);
        fread(&fileblock, sizeof(SIFS_FILEBLOCK), 1, fp);
        strcpy(fileblock.filenames[fileblock.nfiles], file_name); 
        fileblock.nfiles++; 
    }
    
    // WRITE THE FILEBLOCK TO THE VOLUME 
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + fileblockID*blocksize, SEEK_SET);
    fwrite(&fileblock, sizeof(fileblock), 1, fp);

    // UPDATE PARENT DIRECTORY - MODTIME, NENTRIES AND ENTRIES ARRAY
    int parent_blockID = find_parent_blockID(volumename, pathname, nblocks, blocksize);
    size_t jump = sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + parent_blockID*blocksize;
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
