#include "helperfunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// get information about a requested directory
int SIFS_dirinfo(const char *volumename, const char *pathname,
                 char ***entrynames, uint32_t *nentries, time_t *modtime)
{
    int blocksize, nblocks;
    get_volume_header_info(volumename, &blocksize, &nblocks);
    int parent_blockID = find_parent_blockID(volumename, pathname, nblocks, blocksize);
    printf("\n%i\n", parent_blockID);
    FILE *fp = fopen(volumename, "r+");
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + parent_blockID*blocksize, SEEK_SET);
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
        fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + entry_ID*blocksize, SEEK_SET);
        
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
