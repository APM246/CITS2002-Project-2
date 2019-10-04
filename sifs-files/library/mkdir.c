#include "sifs-internal.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

    // CHANGE FROM 'u' TO 'd'
    int blockID;
    if (change_bitmap(volumename, SIFS_DIR, &blockID) != 0)
    {
        SIFS_errno = SIFS_EMAXENTRY;
        return 1;
    }
    
    FILE *fp = fopen(volumename, "r+");
    // for all 3 types of block since size is fixed for blocks (internal fragmentation)) until 'u' found
    // since size of bitmap not known, use SEEK_END and go backwards until first 'u' found
    //sizeof jump = sizeof SIFS_VOLUME_HEADER + sizeof 
    
    fseek(fp, -1024*(100-blockID), SEEK_END);
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

