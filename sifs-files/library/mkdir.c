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
    // CLEAN UP REPEATED USE OF FOPEN() (IN CHANGE_BITMAP AND FOLLOWING CODE)
    change_bitmap(volumename, SIFS_DIR);
    
    FILE *fp = fopen(volumename, "r+");
    // for generalised calculation of jump, look at bitmap and 
    // add bytes for each bit (use max size of block
    // for all 3 types of block since size is fixed for blocks (internal fragmentation)) until 'u' found
    // since size of bitmap not known, use SEEK_END and go backwards until first 'u' found
    //sizeof jump = sizeof SIFS_VOLUME_HEADER + sizeof 
    
    fseek(fp, -1024*99, SEEK_END);

    // NEED TO ASK HELP2002 WHETHER NEED TO FORMAT IN CHAR ARRAY BEFORE FWRITE()
    // OR JUST IMMEDIATELY PASS ADDRESS OF STRUCT 
    /*char block[SIFS_MIN_BLOCKSIZE];
    memset(block, 0, sizeof block);
    memcpy(block, &new_dir, sizeof new_dir);
    fwrite(block, sizeof block, 1, fp);*/

    fwrite(&new_dir, sizeof new_dir, 1, fp);
        
    // ALSO: need to update entries[MAX_SIFS_ENTRIES] array (of parent directory)
    // need helper function that breaks down pathname (removes / and shows where entries array is stored)
    // maybe readbackwards and stop at first '/'. Pointer to parent directory, keep decrementing address
    // until another '/' is reached

    // modify nentries as well (below)
    size_t jump = sizeof(SIFS_VOLUME_HEADER) + 100 + sizeof(SIFS_MAX_NAME_LENGTH) + sizeof(time_t);
    // + sizeof(uint32_t)
    fseek(fp, jump, SEEK_SET);
    char read[1];
    fread(read, 1, 1, fp);
    //printf("\n%i\n", atoi(read));
    read[0]++;
    fseek(fp, -1, SEEK_CUR);
    fwrite(read, 1, 1, fp);

    return 0; 
}

