#include "helperfunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//int find_entry_index

// remove an existing directory from an existing volume
int SIFS_rmdir(const char *volumename, const char *pathname)
{
    FILE *fp = fopen(volumename, "r+");
    int nblocks, blocksize;
    get_volume_header_info(volumename, &blocksize, &nblocks);
    SIFS_BLOCKID blockID = find_blockID(volumename, pathname, nblocks, blocksize);
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    char bitmap[nblocks];
    fread(bitmap, sizeof(bitmap), 1, fp);
    SIFS_BIT type = bitmap[blockID];

    // THROW ERROR IF PATHNAME IS NOT A DIRECTORY
    if (type != SIFS_DIR)
    {
        SIFS_errno = SIFS_ENOTDIR;
        return 1;
    }

    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + blockID*blocksize, SEEK_SET);
    SIFS_DIRBLOCK dirblock;
    fread(&dirblock, sizeof(dirblock), 1, fp);

    // THROW ERROR IF DIRECTORY IS NOT EMPTY 
    if (dirblock.nentries != 0)
    {
        SIFS_errno = SIFS_ENOTEMPTY;
        return 1;
    }

    // change bitmap
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    fread(bitmap, sizeof(bitmap), 1, fp);
    bitmap[blockID] = SIFS_UNUSED;
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    fwrite(bitmap, sizeof(bitmap), 1, fp);
    
    // clear memory
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + blockID*blocksize, SEEK_SET);
    memset(&dirblock, 0, sizeof(dirblock));
    fwrite(&dirblock, sizeof(dirblock), 1, fp);

    // update 3 fields of parent directory 
    SIFS_DIRBLOCK parentblock;
    SIFS_BLOCKID parent_blockID = find_parent_blockID(volumename, pathname, nblocks, blocksize);
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + parent_blockID*blocksize, SEEK_SET);
    fread(&parentblock, sizeof(parentblock), 1, fp);
    parentblock.nentries--;
    parentblock.modtime = time(NULL);

    /*for (int i = 0; i < SIFS_MAX_ENTRIES; i++)
    {
        //if ()
    }*/
    // TEMPORARY JUST TO TEST (should be done in for loop above)
    parentblock.entries[0].blockID = 0;
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + parent_blockID*blocksize, SEEK_SET);
    fwrite(&parentblock, sizeof(parentblock), 1, fp);

    fclose(fp);
    return 0;
}

