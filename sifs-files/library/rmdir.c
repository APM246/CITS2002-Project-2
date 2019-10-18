#include "helperfunctions.h"

// remove an existing directory from an existing volume
int SIFS_rmdir(const char *volumename, const char *pathname)
{
    if (volumename == NULL || pathname == NULL)
    {
        SIFS_errno = SIFS_EINVAL;
        return 1;
    }

    if (!check_valid_volume(volumename))
    {
        return 1;
    }
    // THROW ERROR IF USER TRIES TO DELETE ROOT DIRECTORY
    if (strlen(pathname) == 1 && *pathname == '/')
    {
        SIFS_errno = SIFS_EINVAL;
        return 1;
    }

    FILE *fp = fopen(volumename, "r+");
    uint32_t nblocks;
    size_t blocksize;
    int block_ID;
    get_volume_header_info(volumename, &blocksize, &nblocks);

    // NO SUCH DIRECTORY EXISTS 
    if ((block_ID = find_blockID(volumename, pathname, nblocks, blocksize)) == NO_SUCH_BLOCKID) 
    {
        SIFS_errno = SIFS_ENOENT;
        return 1;
    }

    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    char bitmap[nblocks];
    fread(bitmap, sizeof(bitmap), 1, fp);
    SIFS_BIT type = bitmap[block_ID];

    // THROW ERROR IF PATHNAME IS NOT A DIRECTORY
    if (type != SIFS_DIR)
    {
        SIFS_errno = SIFS_ENOTDIR; // The block is a file block
        return 1;
    }

    fseek_to_blockID(block_ID);
    SIFS_DIRBLOCK dirblock;
    fread(&dirblock, sizeof(dirblock), 1, fp);

    // THROW ERROR IF DIRECTORY IS NOT EMPTY 
    if (dirblock.nentries != 0)
    {
        SIFS_errno = SIFS_ENOTEMPTY;
        return 1;
    }

    // change bitmap - REMOVE FIRST TWO LINES (?)
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    fread(bitmap, sizeof(bitmap), 1, fp);
    bitmap[block_ID] = SIFS_UNUSED;
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    fwrite(bitmap, sizeof(bitmap), 1, fp);
    
    // clear directory block from volume 
    fseek_to_blockID(block_ID);
    memset(&dirblock, 0, sizeof(dirblock)); 
    fwrite(&dirblock, sizeof(dirblock), 1, fp);

    // update 3 fields of parent directory 
    SIFS_DIRBLOCK parentblock;
    int parent_blockID = find_parent_blockID(volumename, pathname, nblocks, blocksize); 
    fseek_to_blockID(parent_blockID);
    fread(&parentblock, sizeof(parentblock), 1, fp);
    int nentries = parentblock.nentries;
    for (int i = 0; i < nentries; i++)  
    {
        if (parentblock.entries[i].blockID == block_ID)
        {
            // shuffle entries in entries array 1 spot down
            while (i < nentries - 1)
            { 
                parentblock.entries[i].blockID = parentblock.entries[i+1].blockID;
                parentblock.entries[i].fileindex = parentblock.entries[i+1].fileindex;
                i++;
            }

            break;
        }
    }
    parentblock.entries[nentries - 1].blockID = 0;   //update last spot that was shuffled down
    parentblock.entries[nentries - 1].fileindex = 0;
    parentblock.nentries--;
    parentblock.modtime = time(NULL);
    fseek_to_blockID(parent_blockID);
    fwrite(&parentblock, sizeof(parentblock), 1, fp);

    fclose(fp);
    return 0;
}
