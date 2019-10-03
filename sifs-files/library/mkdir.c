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
    sizeof jump = sizeof SIFS_VOLUME_HEADER + sizeof 
    fseek(fp, , SEEK_CUR)

        
    //printf("\n%s\n", new_dir.name);

    return 0;
}

