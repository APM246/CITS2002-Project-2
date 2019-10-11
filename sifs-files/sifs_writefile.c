#include <stdio.h>
#include <stdlib.h>
#include "sifs.h"

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("\nIncorrect number of arguments\n");
        return 1;
    }


    FILE *fp = fopen(argv[3], "r");
    char buffer[1000]; //change - use stat to determine size 
    fread(buffer, sizeof(buffer), 1, fp);
    SIFS_writefile(argv[1], argv[2], buffer, sizeof(buffer));

    return 0;
}