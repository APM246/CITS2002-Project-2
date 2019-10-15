#include "sifs-internal.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>

//                                  
// ----------------------------------------------------------------------- MACROS 

//#define fileseek(nblocks, blocksize, blockID) fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + blockID*blocksize, SEEK_SET); 


//                          HELPER FUNCTIONS CATEGORISED 

// ----------------------------------------------------------------------- VOLUME HEADER INFO

extern void get_volume_header_info(const char *volumename, int *blocksize, int *nblocks);

// ----------------------------------------------------------------------- CHANGING BITMAP

extern int change_bitmap(const char *volumename, char type, int *blockID, int nblocks);

// ----------------------------------------------------------------------- PATHNAME 

extern char *find_name(const char *pathname);
extern char *extract_start_of_pathname(const char *pathname);
extern int get_number_of_slashes(const char* pathname);

// ----------------------------------------------------------------------- blockID

extern int find_parent_blockID(const char *volumename, const char *pathname, int nblocks, int blocksize);
extern int find_blockID(const char *volumename, const char *pathname, int nblocks, int blocksize);

// ----------------------------------------------------------------------- FILEBLOCK

extern void sort_filenames(FILE *fp, char *filename, SIFS_FILEBLOCK *fileblock, SIFS_BLOCKID blockID, uint32_t nblocks, size_t blocksize);
extern int find_fileindex(SIFS_FILEBLOCK *fileblock, char *name);
