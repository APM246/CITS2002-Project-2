#include <stdio.h>
#include <stdlib.h>
#include "sifs.h"

int main(int argc, char *argv[])
{
    if (SIFS_rmdir(argv[1], argv[2]) != 0)
    {
        printf("\nWhoops\n");
        perror(argv[0]);
        return 1;
    }

    return 0;
}