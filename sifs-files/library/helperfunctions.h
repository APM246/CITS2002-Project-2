#include "sifs-internal.h"

// WHEN A BLOCK GOES FROM UNUSED TO USED, AT LEAST ONE OF THESE FUNCTIONS WILL BE CALLED
extern int change_bitmap(const char *volumename, char SIFS_BIT, int *blockID, int nblocks);
extern int find_parent_blockID(char *volumename, char *pathname, int nblocks);
