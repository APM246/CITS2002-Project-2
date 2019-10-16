#include "helperfunctions.h"

// get information about a requested file
int SIFS_fileinfo(const char *volumename, const char *pathname,
		  size_t *length, time_t *modtime)
{
    // NO SUCH VOLUME 
    if (access(volumename, F_OK) != 0)
    {
        SIFS_errno	= SIFS_ENOVOL;
        return 1;
    }

    // ROOT IS NOT A FILE
    if (strlen(pathname) == 1 && *pathname == '/')
    {
        SIFS_errno = SIFS_ENOTFILE;
        return 1;
    }

    int blocksize, nblocks, blockID;
    get_volume_header_info(volumename, &blocksize, &nblocks); 

     // NO SUCH FILE EXISTS 
    if ((blockID = find_blockID(volumename, pathname, nblocks, blocksize)) == NO_SUCH_BLOCKID) 
    {
        SIFS_errno = SIFS_ENOENT;
        return 1;
    }

    FILE *fp = fopen(volumename, "r+");
    char bitmap[nblocks];
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    fread(bitmap, sizeof(bitmap), 1, fp);

    // NOT A FILE
    if ((bitmap[blockID] != SIFS_FILE))
    {
        SIFS_errno = SIFS_ENOTFILE;
        return 1;
    }

    fseek_to_blockID(blockID);
    SIFS_FILEBLOCK fileblock;
    fread(&fileblock, sizeof(SIFS_FILEBLOCK), 1, fp);
    *length = fileblock.length;
    *modtime = fileblock.modtime;

    fclose(fp);
    return 0;
}
