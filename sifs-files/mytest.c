#include <stdio.h>
#include <stdlib.h>
#include <sifs.h>

int main(int argc, char *argv[])
{
    char *volumename = "sample volumes/propertest";
    size_t blocksize = 2500;
    uint32_t nblocks = 132;
    SIFS_mkvolume(volumename, blocksize, nblocks);

    SIFS_mkdir(volumename, )
    


    //SIFS_writefile(volumename, "/subdir1/photo1", )   

    return 0;
}
