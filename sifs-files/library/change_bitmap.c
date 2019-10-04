#include <stdio.h>
#include <stdlib.h>
#include "sifs-internal.h" 

// WILL NEED TO CHANGE AS BITMAP IS NOT ALWAYS APPENDED TO. SOMETIMES BLOCKS WILL BECOME UNUSED
// THUS CAN FILL UP 'u' BIT IN THE MIDDLE OF THE BITMAP
// USE FOR LOOP AND STOP AT FIRST INSTANCE OF 'u'
// OR fseek by 1 char each time, then fread() to see if char is 'u'
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