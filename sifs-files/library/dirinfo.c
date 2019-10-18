#include "helperfunctions.h"

// get information about a requested directory
int SIFS_dirinfo(const char *volumename, const char *pathname,
                 char ***entrynames, uint32_t *nentries, time_t *modtime)
{
    // CHECK NULL ARGUMENTS
    if (volumename == NULL || pathname == NULL)
    {
        SIFS_errno = SIFS_EINVAL;
        return 1;
    } 

    if (!check_valid_volume(volumename))
    {
        return 1;
    }

    size_t blocksize;
    uint32_t nblocks;
    SIFS_BLOCKID blockID;
    get_volume_header_info(volumename, &blocksize, &nblocks); 

    // REQUESTING INFO OF ROOT DIRECTORY
    if (strlen(pathname) == 1 && *pathname == '/')
    {
        blockID = 0;
    }

    // NO SUCH DIRECTORY EXISTS 
    else if ((blockID = find_blockID(volumename, pathname, nblocks, blocksize)) == NO_SUCH_BLOCKID) 
    {
        SIFS_errno = SIFS_ENOENT;
        return 1;
    }

    FILE *fp = fopen(volumename, "r+");
    char bitmap[nblocks];
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    fread(bitmap, sizeof(bitmap), 1, fp);

    // NOT A DIRECTORY
    if (bitmap[blockID] != SIFS_DIR)
    {
        SIFS_errno = SIFS_ENOTDIR;
        return 1;
    }

    fseek_to_blockID(blockID);
    SIFS_DIRBLOCK dirblock;
    fread(&dirblock, sizeof(SIFS_DIRBLOCK), 1, fp);
    *nentries = dirblock.nentries;
    *modtime = dirblock.modtime;

    char **local_entrynames = NULL;
    local_entrynames = malloc(sizeof(char *)*(*nentries));
    for (int i = 0; i < *nentries; i++)
    {
        local_entrynames[i] = malloc(SIFS_MAX_NAME_LENGTH);
    }

    for (int i = 0; i < *nentries; i++)
    {
        int entry_ID = dirblock.entries[i].blockID;
        fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
        char bitmap[nblocks]; 
        fread(bitmap, sizeof(bitmap), 1, fp);
        SIFS_BIT bit = bitmap[entry_ID];
        fseek_to_blockID(entry_ID);
        
        if (bit == SIFS_DIR)
        {
            SIFS_DIRBLOCK entry_dirblock;
            fread(&entry_dirblock, sizeof(entry_dirblock), 1, fp);
            strcpy(local_entrynames[i], entry_dirblock.name);
        }
        else if (bit == SIFS_FILE)
        {
            int file_index = dirblock.entries[i].fileindex;
            SIFS_FILEBLOCK entry_fileblock;
            fread(&entry_fileblock, sizeof(entry_fileblock), 1, fp);
            strcpy(local_entrynames[i], entry_fileblock.filenames[file_index]);
        }
    }

    *entrynames = local_entrynames;
    fclose(fp);
    return 0;
}
