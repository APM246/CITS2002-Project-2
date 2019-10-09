#include "sifs-internal.h"

extern void get_volume_header_info(const char *volumename, int *blocksize, uint32_t *nblocks);
extern int change_bitmap(const char *volumename, char type, SIFS_BLOCKID *blockID, uint32_t nblocks);
extern int get_number_of_slashes(const char* pathname);
extern SIFS_BLOCKID find_parent_blockID(const char *volumename, const char *pathname, uint32_t nblocks, int blocksize);
extern SIFS_BLOCKID find_blockID(const char *volumename, const char *pathname, uint32_t nblocks, int blocksize);
extern char *find_name(const char *pathname);
