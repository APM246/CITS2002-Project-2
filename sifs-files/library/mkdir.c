#include "helperfunctions.h"

// make a new directory within an existing volume
int SIFS_mkdir(const char *volumename, const char *pathname)
{
    // NO SUCH VOLUME 
    if (access(volumename, F_OK) != 0)
    {
        SIFS_errno	= SIFS_ENOVOL;
        return 1;
    }

    // EXTRACT DIRECTORY NAME 
    char *directory_name = find_name(pathname);
    
    // THROW ERROR IF NAME IS TOO LONG OR '/' PATHNAME PROVIDED (USER TRIES TO CREATE "ANOTHER" ROOT)
    if ((strlen(directory_name) + 1) > SIFS_MAX_NAME_LENGTH || (strlen(pathname) == 1 && *pathname == '/'))
    {
        SIFS_errno = SIFS_EINVAL;
        return 1;
    } 
    
    //obtain information about nblocks and blocksize
    int blocksize, nblocks;
    get_volume_header_info(volumename, &blocksize, &nblocks);

    // CHECK IF PATHNAME IS VALID
    FILE *fp = fopen(volumename, "r+");
    int parent_blockID = find_parent_blockID(volumename, pathname, nblocks, blocksize);
    if (parent_blockID == NO_SUCH_BLOCKID)
    {
        SIFS_errno = SIFS_EINVAL;
        return 1;
    }
    else if (parent_blockID == MEMORY_ALLOCATION_FAILED)
    {
        SIFS_errno = SIFS_ENOMEM;
        return 1;
    }

    // DIRECTORY WITH THAT NAME ALREADY EXISTS 
    if (find_blockID(volumename, pathname, nblocks, blocksize) != NO_SUCH_BLOCKID)
    {
        SIFS_errno = SIFS_EEXIST;
        return 1;
    }
    
    // CHECK IF PARENT BLOCK HAS NO SPACE LEFT FOR ENTRIES 
    fseek_to_blockID(parent_blockID);
    SIFS_DIRBLOCK dir;
    fread(&dir, sizeof(SIFS_DIRBLOCK), 1, fp);
    if (dir.nentries == SIFS_MAX_ENTRIES)
    {
        SIFS_errno = SIFS_EMAXENTRY;
        return 1;
    }
    
    // CREATE THE NEW DIRECTORY BLOCK
    SIFS_DIRBLOCK new_dir; 
    strcpy(new_dir.name, directory_name);
    new_dir.modtime = time(NULL);
    new_dir.nentries = 0;

    // CHANGE FROM 'u' TO 'd'. THROW ERROR IF NOT ENOUGH SPACE
    int block_ID;
    if (change_bitmap(volumename, SIFS_DIR, &block_ID, nblocks) != 0)
    {
        SIFS_errno = SIFS_ENOSPC;
        return 1;
    }
    
    // WRITE DIRECTORY BLOCK TO VOLUME 
    fseek_to_blockID(block_ID);
    fwrite(&new_dir, sizeof new_dir, 1, fp);

    // UPDATE MODTIME, ENTRIES AND NENTRIES OF PARENT DIRECTORY 
    dir.entries[dir.nentries].blockID = block_ID; 
    dir.nentries++; 
    dir.modtime = time(NULL);
    fseek_to_blockID(parent_blockID);
    fwrite(&dir, sizeof dir, 1, fp);
    fclose(fp); 

    return 0; 
}
