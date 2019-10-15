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

    void *buffer;
    size_t nbytes;

    if (SIFS_readfile(argv[1], argv[2], &buffer, &nbytes) != 0) 
    {
        SIFS_perror(NULL);
        return 1;
    } 

    //printf("\n%s\n\n", (char *) buffer);
    FILE *fp = fopen(argv[3], "w");
    fwrite(buffer, nbytes, 1, fp);
    free(buffer);
    fclose(fp);
    return 0;
}
