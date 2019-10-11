#include <stdio.h>
#include <stdlib.h>
#include "sifs.h"
#include <string.h>

char *find__name(const char *pathname)
{
    char *directory_name;
    if ((directory_name = strrchr(pathname, '/')) != NULL)
    {
        directory_name++; // move one char past '/'
    }
    else
    {
        directory_name = malloc(32);
        strcpy(directory_name, pathname);
    }
    return directory_name;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("\nIncorrect number of arguments\n");
        return 1;
    }

    
    FILE *fp = fopen(find__name(argv[2]), "r");
    char buffer[100]; //change - use stat to determine size 
    fread(buffer, sizeof(buffer), 1, fp);
    SIFS_writefile(argv[1], argv[2], buffer, sizeof(buffer));

    return 0;
}