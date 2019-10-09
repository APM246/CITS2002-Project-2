#include "sifs-internal.h"
#include <stdio.h>
#include <stdlib.h>

void get_volume_header_info(const char *volumename, int *blocksize, uint32_t *nblocks)
{
    FILE *fp = fopen(volumename, "r+");
    char buffer[sizeof(SIFS_VOLUME_HEADER)];
    fread(buffer, sizeof(buffer), 1, fp);
    *nblocks = ((SIFS_VOLUME_HEADER *) buffer)->nblocks; // data type of int ok?
    *blocksize = ((SIFS_VOLUME_HEADER *) buffer)->blocksize;
}

