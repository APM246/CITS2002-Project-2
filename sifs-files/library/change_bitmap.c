#include <stdio.h>
#include <stdlib.h>
#include "sifs-internal.h" 

// add 1 more parameter: file pointer, avoids repetition. 
int change_bitmap(const char *volumename, char SIFS_BIT, int *blockID, int nblocks)
{
    FILE *fp = fopen(volumename, "r+");
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    char bit;

    for (int i = 0; i < nblocks; i++)
    {
        fread(&bit, sizeof(char), 1, fp);
        if (bit == 'u') 
        {
            *blockID = i;
            fseek(fp, -1, SEEK_CUR);
            fwrite(&SIFS_BIT, sizeof(char), 1, fp);
            fclose(fp);
            return 0;
        }
    }

    return 1;
}