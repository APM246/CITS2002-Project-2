#include <stdio.h>
#include <stdlib.h>
#include "sifs.h"

int main(int argc, char *argv[])
{
    if (argc != 3) 
    {
        printf("\n\nWrong command-line arguments\n");
        return 1;
    }

    if (SIFS_rmdir(argv[1], argv[2]) != 0)
    {
        SIFS_perror(NULL);
        return 1;
    }

    return 0;
}