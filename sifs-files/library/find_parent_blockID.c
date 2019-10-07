#include "sifs-internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int find_parent_blockID(char *volumename, char *pathname, int nblocks)
{
    FILE *fp = fopen(volumename, "r+");
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT), SEEK_SET);
    char *path = strtok(pathname, "/");
    SIFS_DIRBLOCK buffer;
    while ()
    {
        int temp_blockID;
        fread(&buffer, sizeof(buffer), 1, fp);
        for (int i = 0; i < SIFS_MAX_ENTRIES; i++)
        {
            temp_blockID = buffer.entries[i].blockID;
            //fread that blockID and check its name to see if it matches. 
        }

    }


    return 0;
}

