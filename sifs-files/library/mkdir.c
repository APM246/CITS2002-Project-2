#include "helperfunctions.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>

// make a new directory within an existing volume
int SIFS_mkdir(const char *volumename, const char *pathname)
{
    if (access(volumename, F_OK) != 0)
    {
        SIFS_errno	= SIFS_ENOVOL;
        return 1;
    }

    // EXTRACT DIRECTORY NAME 
    char *directory_name;
    if ((directory_name = strrchr(pathname, '/')) != NULL)
    {
        directory_name++; // move one char past '/'
    }
    else
    {
        directory_name = malloc(SIFS_MAX_NAME_LENGTH + 1);
        strcpy(directory_name, pathname);
    }
    
    // THROW ERROR IF NAME IS TOO LONG 
    if ((strlen(directory_name) + 1) > SIFS_MAX_NAME_LENGTH)
    {
        SIFS_errno = SIFS_EINVAL;
        return 1;
    } 

    // CREATE THE NEW DIRECTORY BLOCK
    SIFS_DIRBLOCK new_dir; 
    strcpy(new_dir.name, directory_name);   
    new_dir.modtime = time(NULL);
    new_dir.nentries = 0;

    //obtain information about nblocks and blocksize
    FILE *fp = fopen(volumename, "r+");
    char buffer[sizeof(SIFS_VOLUME_HEADER)];
    fread(buffer, sizeof(buffer), 1, fp);
    int nblocks = ((SIFS_VOLUME_HEADER *) buffer)->nblocks; // data type of int ok?
    size_t blocksize = ((SIFS_VOLUME_HEADER *) buffer)->blocksize;

    // CHANGE FROM 'u' TO 'd'
    int blockID;
    if (change_bitmap(volumename, SIFS_DIR, &blockID, nblocks) != 0)
    {
        SIFS_errno = SIFS_EMAXENTRY;
        return 1;
    }
    
    fseek(fp, -blocksize*(nblocks-blockID), SEEK_END);
    fwrite(&new_dir, sizeof new_dir, 1, fp);

    // UPDATE ENTRIES AND NENTRIES OF PARENT DIRECTORY 
    int parent_blockID = find_parent_blockID(volumename, pathname, nblocks, blocksize);
    size_t jump = sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + parent_blockID*blocksize; // use macro
    fseek(fp, jump, SEEK_SET);
    SIFS_DIRBLOCK dir;
    fread(&dir, sizeof(SIFS_DIRBLOCK), 1, fp);
    dir.entries[dir.nentries].blockID = blockID;
    dir.nentries++;
    fseek(fp, jump, SEEK_SET);
    fwrite(&dir, sizeof dir, 1, fp);
    fclose(fp);

    //throw error if directory already exists
    // find_parent_blockID needs to throw errors, etc.  

    return 0; 
}