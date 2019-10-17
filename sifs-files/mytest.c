#include <stdio.h>
#include <stdlib.h>
#include "sifs.h"
#include <unistd.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
    char *volumename = "project testing/propertest";

    if (access(volumename, F_OK) == 0)
    {
        remove(volumename);
    }

    size_t blocksize = 2500;
    uint32_t nblocks = 132;
    SIFS_mkvolume(volumename, blocksize, nblocks);

    char names[20][100] = {{"/dir1"}, {"/dir1/subdir1"}, {"dir2"}, {"/dir1/subdir2"}, {"/dir2/wow"}, 
    {"diar3"}, {"diar6"}, {"/diar4"}, {"/diar4/three"}};

    for (int i = 0; i < 9; i++)
    {
        SIFS_mkdir(volumename, names[i]);
    }

    


    //SIFS_writefile(volumename, "/subdir1/photo1", )   

    return 0;
}
