#include "helperfunctions.h"

// read the contents of an existing file from an existing volume
int SIFS_readfile(const char *volumename, const char *pathname,
		  void **data, size_t *nbytes)
{
   if (volumename == NULL || pathname == NULL)
    {
        SIFS_errno = SIFS_EINVAL;
        return 1;
    }

    if (!check_valid_volume(volumename))
    {
        return 1;
    }

    // ROOT IS NOT A FILE
    if (strlen(pathname) == 1 && *pathname == '/')
    {
        SIFS_errno = SIFS_ENOTFILE;
        return 1;
    }

    // ACCESS VOLUME HEADER INFORMATION
    size_t blocksize;
    uint32_t nblocks;
    SIFS_BLOCKID blockID;
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

    // ACQUIRE FIRSTBLOCKID
    fseek_to_blockID(blockID);
    SIFS_FILEBLOCK fileblock;
    fread(&fileblock, sizeof(SIFS_FILEBLOCK), 1, fp);
    int firstblockID = fileblock.firstblockID;
    *nbytes = fileblock.length;

    // ALLOCATE MEMORY FOR BUFFER AND COPY INTO
    char *buffer = malloc(*nbytes);
    fseek_to_blockID(firstblockID);
    fread(buffer, *nbytes, 1, fp);

    // ASSIGN TO DATA pointer
    *data = buffer;

    fclose(fp);
    return 0;
}
