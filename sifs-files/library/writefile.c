#include "helperfunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

// find contiguous blocks to store contents of files 
void find_contiguous_blocks(size_t nbytes, size_t blocksize, int nblocks)
{
    int nblocks_needed = ceil(nbytes/blocksize);
    FILE *fp = 
    
    for (int i = 0; i < nblocks; i++)
    {
        
    }
}


// add a copy of a new file to an existing volume
int SIFS_writefile(const char *volumename, const char *pathname,
		   void *data, size_t nbytes)
{
    // NO SUCH VOLUME 
    if (access(volumename, F_OK) != 0)
    {
        SIFS_errno	= SIFS_ENOVOL;
        return 1;
    }    

    int nblocks, blocksize, blockID;
    get_volume_header_info(volumename, &blocksize, &nblocks);
    change_bitmap(volumename, SIFS_FILE, &blockID, nblocks);

    return 0;
}
