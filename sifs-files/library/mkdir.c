#include "helperfunctions.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>

// make a new directory within an existing volume
int SIFS_mkdir(const char *volumename, const char *dirname)
{
    if (access(volumename, F_OK) != 0)
    {
        SIFS_errno	= SIFS_ENOVOL;
        return 1;
    }

    // CREATE THE NEW DIRECTORY BLOCK
    SIFS_DIRBLOCK new_dir; 
    strcpy(new_dir.name, dirname);
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
        
    // need helper function that breaks down pathname (removes / and shows where entries array is stored)
    // maybe readbackwards and stop at first '/'. Pointer to parent directory, keep decrementing address
    // until another '/' is reached
    /* look at parent directory's parent directory's entries array in SIFS_dirblock and
    find blockID whose corresponding name is the parent's directory. Then use that blockID to find
    its block and update nentries and its own entries array. */

    size_t jump = sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT); 
    fseek(fp, jump, SEEK_SET);
    SIFS_DIRBLOCK dir;
    fread(&dir, sizeof(SIFS_DIRBLOCK), 1, fp);
    dir.entries[dir.nentries].blockID = blockID;
    dir.nentries++;
    fseek(fp, jump, SEEK_SET);
    fwrite(&dir, sizeof dir, 1, fp);
    fclose(fp);

    //throw error if directory already exists 

    return 0; 
}