#include <stdio.h>
#include <stdlib.h>
#include "sifs.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("\nIncorrect number of arguments\n");
        return 1;
    }

    
    FILE *fp = fopen(argv[3], "r");
    struct stat buf;
    stat(argv[3], &buf);
    int size = buf.st_size;
    char buffer[size]; 
    fread(buffer, size, 1, fp);
    if (SIFS_writefile(argv[1], argv[2], buffer, size) != 0)
    {
        SIFS_perror(NULL);
    }

    return 0;
}
