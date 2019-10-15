#include <stdio.h>
#include <stdlib.h>
#include "sifs.h"
#include <time.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        //printf("Incorrect number of arguments");
        fprintf(stderr, "Incorrect number of arguments");
        return 1;
    }

    size_t length;
    time_t modtime;

    if (SIFS_fileinfo(argv[1], argv[2], &length, &modtime) != 0) 
    {
        SIFS_perror(NULL);
        return 1;
    }

    printf("\nlength: %lu\n", length);
    printf("modtime: %s\n", ctime(&modtime));

    return 0;
}
