#include "sifs-internal.h"
#include <stdio.h>

extern void get_volume_header_info(const char *volumename, int *blocksize, int *nblocks);
extern int change_bitmap(const char *volumename, char type, int *blockID, int nblocks);
extern int get_number_of_slashes(const char* pathname);
extern int find_parent_blockID(const char *volumename, const char *pathname, int nblocks, int blocksize);
extern int find_blockID(const char *volumename, const char *pathname, int nblocks, int blocksize);
extern char *find_name(const char *pathname);
extern char *extract_start_of_pathname(const char *pathname);
extern void sort_filenames(FILE *fp, char *filename, SIFS_FILEBLOCK *fileblock, SIFS_BLOCKID blockID, uint32_t nblocks, size_t blocksize);
extern int find_fileindex(SIFS_FILEBLOCK *fileblock, char *name);