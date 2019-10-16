#include "sifs-internal.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>

//                                  
// ----------------------------------------------------------------------- MACROS 

#define blockID_offset(blockID, nblocks, blocksize)  sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + blockID*blocksize 
#define NO_CONTIGUOUS_BLOCKS -1 // No contiguous blocks of memory found to store data blocks 
#define NO_SUCH_BLOCKID -1 // There is no directory or file entry with that pathname 
#define NO_SUCH_FILENAME -1 // There is no file entry with that file name 
#define MEMORY_ALLOCATION_FAILED -2 // NULL pointer returned from malloc()

//                          HELPER FUNCTIONS CATEGORISED 

// ----------------------------------------------------------------------- VOLUME HEADER INFO

extern void get_volume_header_info(const char *volumename, int *blocksize, int *nblocks);

// ----------------------------------------------------------------------- CHANGING BITMAP

extern int change_bitmap(const char *volumename, char type, int *blockID, int nblocks);

// ----------------------------------------------------------------------- PATHNAME 

extern char *find_name(const char *pathname);
extern int get_number_of_slashes(const char* pathname);

// ----------------------------------------------------------------------- blockID

extern int find_parent_blockID(const char *volumename, const char *pathname, int nblocks, int blocksize);
extern int find_blockID(const char *volumename, const char *pathname, int nblocks, int blocksize);

// ----------------------------------------------------------------------- FILEBLOCK

extern void sort_filenames(FILE *fp, char *filename, SIFS_FILEBLOCK *fileblock, SIFS_BLOCKID blockID, uint32_t nblocks, size_t blocksize);
extern int find_fileindex(SIFS_FILEBLOCK *fileblock, char *name);
