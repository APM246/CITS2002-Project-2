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
    // data type of int ok?
    int nblocks = ((SIFS_VOLUME_HEADER *) buffer)->nblocks;
    size_t blocksize = ((SIFS_VOLUME_HEADER *) buffer)->blocksize;

    // CHANGE FROM 'u' TO 'd'
    int blockID;
    if (change_bitmap(volumename, SIFS_DIR, &blockID, nblocks) != 0)
    {
        SIFS_errno = SIFS_EMAXENTRY;
        return 1;
    }
    
    printf("\nnblocks: %i, blocksize: %lu, blockID: %i\n", nblocks, blocksize, blockID);
    //FILE *fp = fopen(volumename, "r+");
    fseek(fp, -blocksize*(nblocks-blockID), SEEK_END);
    fwrite(&new_dir, sizeof new_dir, 1, fp);
        
    // ALSO: need to update entries[MAX_SIFS_ENTRIES] array (of parent directory)
    // need helper function that breaks down pathname (removes / and shows where entries array is stored)
    // maybe readbackwards and stop at first '/'. Pointer to parent directory, keep decrementing address
    // until another '/' is reached
    // modify nentries as well (below)

    size_t jump = sizeof(SIFS_VOLUME_HEADER) + 100 + 1024; //+ 1024n where n is number of blocks away
    fseek(fp, jump, SEEK_SET);
    char read[1024];
    fread(&read, sizeof(read), 1, fp);
    SIFS_DIRBLOCK *ptr = (SIFS_DIRBLOCK *) read;
    printf("\n%s\n", ptr->name);
    //read++;
    //fseek(fp, -sizeof(read), SEEK_CUR);
    //fwrite(&read, sizeof(read), 1, fp);

    return 0; 
}

