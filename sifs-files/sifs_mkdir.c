#include <stdio.h>
#include <stdlib.h>
#include "sifs.h"

int main(int argc, char *argv[])
{
    if (SIFS_mkdir(argv[1], argv[2]) != 0) 
    {
        SIFS_perror(NULL);
        return 1;
    }

    return 0;
}