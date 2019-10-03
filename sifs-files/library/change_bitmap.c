#include <stdio.h>
#include <stdlib.h>
#include "sifs-internal.h" 

// WILL NEED TO CHANGE AS BITMAP IS NOT ALWAYS APPENDED TO. SOMETIMES BLOCKS WILL BECOME UNUSED
// THUS CAN FILL UP 'u' BIT IN THE MIDDLE OF THE BITMAP
// USE FOR LOOP AND STOP AT FIRST INSTANCE OF 'u'
void change_bitmap(const char *volumename, char SIFS_BIT)
{
    FILE *fp = fopen(volumename, "r+");
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + 1, SEEK_SET);
    fwrite(&SIFS_BIT, 1, 1, fp);
    fclose(fp);
}