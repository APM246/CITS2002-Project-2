#include "helperfunctions.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// remove an existing file from an existing volume
int SIFS_rmfile(const char *volumename, const char *pathname)
{
    // NO SUCH VOLUME 
    if (access(volumename, F_OK) != 0)
    {
        SIFS_errno	= SIFS_ENOVOL;
        return 1;
    }

     // THROW ERROR IF USER TRIES TO DELETE ROOT DIRECTORY
    if (strlen(pathname) == 1 && *pathname == '/')
    {
        SIFS_errno = SIFS_ENOTDIR;
        return 1;
    }

    FILE *fp = fopen(volumename, "r+");
    int nblocks, blocksize;
    get_volume_header_info(volumename, &blocksize, &nblocks);
    int block_ID = find_blockID(volumename, pathname, nblocks, blocksize); 
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    char bitmap[nblocks];
    fread(bitmap, sizeof(bitmap), 1, fp);
    SIFS_BIT type = bitmap[block_ID];

    // THROW ERROR IF PATHNAME IS NOT A FILE
    if (type != SIFS_FILE)
    {
        if (block_ID == -1) SIFS_errno = SIFS_ENOENT;
        else SIFS_errno = SIFS_ENOTFILE; // The block is a file or data block
        return 1;
    }

    // CHECK IF REMOVING LAST ENTRY OF A PARTICULAR FILEBLOCK
    //int parent_blockID = find_parent_blockID(volumename, pathname, nblocks, blocksize);
    

    fclose(fp);
    return 0;
}
