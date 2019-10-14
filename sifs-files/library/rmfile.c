#include "helperfunctions.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// remove an existing file from an existing volume
int SIFS_rmfile(const char *volumename, const char *pathname)
{
    // NO SUCH VOLUME 
    if (access(volumename, F_OK) != 0)
    {
        SIFS_errno	= SIFS_ENOVOL;
        return 1;
    }

     // THROW ERROR IF USER TRIES TO DELETE ROOT DIRECTORY
    if (strlen(pathname) == 1 && *pathname == '/')
    {
        SIFS_errno = SIFS_ENOTDIR;
        return 1;
    }

    FILE *fp = fopen(volumename, "r+");
    int nblocks, blocksize;
    get_volume_header_info(volumename, &blocksize, &nblocks);
    int blockID = find_blockID(volumename, pathname, nblocks, blocksize); 
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    char bitmap[nblocks];
    fread(bitmap, sizeof(bitmap), 1, fp);
    SIFS_BIT type = bitmap[blockID];

    // THROW ERROR IF PATHNAME IS NOT A FILE
    if (type != SIFS_FILE)
    {
        if (blockID == -1) SIFS_errno = SIFS_ENOENT;
        else SIFS_errno = SIFS_ENOTFILE; // The block is a file or data block
        return 1;
    }

    // OBTAIN INFO ABOUT NENTRIES 
    int parent_blockID = find_parent_blockID(volumename, pathname, nblocks, blocksize); 
    SIFS_DIRBLOCK dirblock;
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + parent_blockID*blocksize, SEEK_SET);
    fread(&dirblock, sizeof(SIFS_DIRBLOCK), 1, fp);
    int nentries = dirblock.nentries;

    // UPDATE FILEBLOCK (EITHER REMOVE IF LAST ENTRY OR UPDATE VALUES)
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + blockID*blocksize, SEEK_SET);
    SIFS_FILEBLOCK fileblock;
    fread(&fileblock, sizeof(SIFS_FILEBLOCK), 1, fp);
    if (fileblock.nfiles == 1)
    {        
        // REMOVE DATABLOCK(S) FROM VOLUME 
        SIFS_BLOCKID firstblockID = fileblock.firstblockID; 
        fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + firstblockID*blocksize, SEEK_SET);
        memset(fp, 0, fileblock.length);

        //UPDATE BITMAP AND WRITE TO VOLUME 
        bitmap[blockID] = SIFS_UNUSED; // UPDATE FILEBLOCK BIT
        uint32_t n_datablocks = ceil(((double) fileblock.length)/blocksize); 
        for (int i = firstblockID; i < firstblockID + n_datablocks; i++)
        {
            bitmap[i] = SIFS_UNUSED; //UPDATE DATABLOCKS BITS
        }
        fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
        fwrite(bitmap, nblocks*sizeof(SIFS_BIT), 1, fp);

        // REMOVE FILEBLOCK FROM VOLUME 
        memset(&fileblock, 0, sizeof(SIFS_FILEBLOCK));
    }
    else
    {
        char *name = fileblock.filenames[get_fileindex(fp, parent_blockID, blockID, nentries, nblocks, blocksize)];
        // SORT FILENAMES ARRAY (shuffle down)
        sort_filenames(fp, name, &fileblock);
        fileblock.nfiles--;
    }
    
    // WRITE FILE BLOCK BACK TO VOLUME
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + blockID*blocksize, SEEK_SET);
    fwrite(&fileblock, sizeof(SIFS_FILEBLOCK), 1, fp);

    //UPDATE PARENT DIRECTORY 
    for (int i = 0; i < nentries; i++)   
    {
        if (dirblock.entries[i].blockID == blockID)
        {
            // shuffle entries in entries array 1 spot down
            while (i < nentries - 1)
            { 
                dirblock.entries[i].blockID = dirblock.entries[i+1].blockID;
                dirblock.entries[i].fileindex = dirblock.entries[i+1].fileindex;
                i++;
            }

            break;
        }
    }
    dirblock.entries[nentries - 1].blockID = 0;   //update last spot that was shuffled down
    dirblock.entries[nentries - 1].fileindex = 0;
    dirblock.modtime = time(NULL);
    dirblock.nentries--;

    // WRITE BACK TO VOLUME 
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + parent_blockID*blocksize, SEEK_SET);
    fwrite(&dirblock, sizeof(SIFS_DIRBLOCK), 1, fp);

    fclose(fp);
    return 0;
}
