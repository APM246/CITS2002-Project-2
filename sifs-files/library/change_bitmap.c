#include <stdio.h>
#include <stdlib.h>
#include "sifs-internal.h" 

// add 1 more parameter: file pointer, avoids repetition. 
int change_bitmap(const char *volumename, char type, SIFS_BLOCKID *blockID, int nblocks)
{
    FILE *fp = fopen(volumename, "r+");
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    SIFS_BIT bit;

    for (int i = 0; i < nblocks; i++)
    {
        fread(&bit, sizeof(char), 1, fp);
        if (bit == SIFS_UNUSED) 
        {
            *blockID = i;
            fseek(fp, -1, SEEK_CUR);
            fwrite(&type, sizeof(char), 1, fp);
            fclose(fp);
            return 0;
        }
    }

    return 1;
}