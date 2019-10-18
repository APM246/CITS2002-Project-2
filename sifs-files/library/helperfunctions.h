#include "sifs-internal.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>

//                                  
// ----------------------------------------------------------------------- MACROS 

#define fseek_to_blockID(ID)  fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + ID*blocksize, SEEK_SET) 
#define NO_CONTIGUOUS_BLOCKS -1 // No contiguous blocks of memory found to store data blocks 
#define NO_SUCH_BLOCKID -1 // There is no directory or file entry with that pathname 
#define NO_SUCH_FILENAME -1 // There is no file entry with that file name 
#define MEMORY_ALLOCATION_FAILED -2 // NULL pointer returned from malloc()
#define max(x,y) (x) < (y) ? (y) : (x) // returns maximum of two values 

//                          HELPER FUNCTIONS CATEGORISED 

// ----------------------------------------------------------------------- VOLUME HEADER INFO

extern void get_volume_header_info(const char *volumename, size_t *blocksize, uint32_t *nblocks);

// ----------------------------------------------------------------------- CHANGING BITMAP

extern int change_bitmap(const char *volumename, char type, SIFS_BLOCKID *blockID, uint32_t nblocks);

// ----------------------------------------------------------------------- PATHNAME 

extern char *find_name(const char *pathname);
extern int get_number_of_slashes(const char* pathname);

// ----------------------------------------------------------------------- blockID

extern int find_parent_blockID(const char *volumename, const char *pathname, uint32_t nblocks, size_t blocksize);
extern int find_blockID(const char *volumename, const char *pathname, uint32_t nblocks, size_t blocksize);

// ----------------------------------------------------------------------- FILEBLOCK

extern void sort_filenames(FILE *fp, char *filename, SIFS_FILEBLOCK *fileblock, SIFS_BLOCKID blockID, uint32_t nblocks, size_t blocksize);
extern int find_fileindex(SIFS_FILEBLOCK *fileblock, char *name);

// ----------------------------------------------------------------------- ERROR CHECKING

extern bool check_valid_volume(const char *volumename);
