#include "sifs-internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// can't write to an uninitialised pointer (e.g. in strcpy())
int find_parent_blockID(const char *volumename, const char *pathname, int nblocks, int blocksize)
{
    char path_name[SIFS_MAX_NAME_LENGTH];
    //char *path_name = malloc(strlen(pathname) + 1); 
    strcpy(path_name, pathname);
    FILE *fp = fopen(volumename, "r+");
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT), SEEK_SET);

    char *path;
    int parent_blockID = 0;
    SIFS_DIRBLOCK parent_buffer;
    int child_blockID;
    SIFS_DIRBLOCK child_buffer;
    char delimiter[] = "/"; //check which direction

    path = strtok(path_name, delimiter);
    

    do
    {
        fread(&parent_buffer, sizeof(parent_buffer), 1, fp);
        for (int i = 0; i < SIFS_MAX_ENTRIES; i++)
        {
            child_blockID = parent_buffer.entries[i].blockID;
            fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + child_blockID*blocksize, SEEK_SET);
            fread(&child_buffer, sizeof(child_buffer), 1, fp);
            if (strcmp(child_buffer.name, path) == 0) 
            {
                parent_blockID = child_blockID;
                fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + parent_blockID*blocksize, SEEK_SET);
                memset(&parent_buffer, 0, sizeof(parent_buffer));
                break;
            }    
        }
    }
    while ((path = strtok(NULL, delimiter)) != NULL);

    // search failed, does not exist 
    return parent_blockID;
}

