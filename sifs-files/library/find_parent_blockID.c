#include "sifs-internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_number_of_slashes(const char* pathname)
{
    char *path_name = malloc(sizeof(pathname));
    strcpy(path_name, pathname);
    int number = 0;
    char delimiter[] = "/";
    if (strcmp(strtok(path_name, delimiter), pathname) == 0) return 0;

    while (strtok(NULL, delimiter) != NULL) number++;
    free(path_name);
    
    return number;
}

// can't write to an uninitialised pointer (e.g. in strcpy())
// path and child_buffer.name don't ever agree unless entry in root
// function to find number of slashes 
int find_parent_blockID(const char *volumename, const char *pathname, int nblocks, int blocksize)
{
    char path_name[SIFS_MAX_NAME_LENGTH];
    //char *path_name = malloc(strlen(pathname) + 1); 
    strcpy(path_name, pathname);
    FILE *fp = fopen(volumename, "r+");
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT), SEEK_SET);

    char *path;
    int parent_blockID = 0;
    SIFS_DIRBLOCK parent_buffer;
    int child_blockID;
    SIFS_DIRBLOCK child_buffer;
    char delimiter[] = "/"; //check which direction
    int max_iterations = get_number_of_slashes(pathname);
    int n_iterations = 0;

    path = strtok(path_name, delimiter);

    do
    {
        fread(&parent_buffer, sizeof(parent_buffer), 1, fp);
        for (int i = 0; i < SIFS_MAX_ENTRIES; i++)
        {
            child_blockID = parent_buffer.entries[i].blockID;
            fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + child_blockID*blocksize, SEEK_SET);
            memset(&child_buffer, 0, sizeof(child_buffer));
            fread(&child_buffer, sizeof(child_buffer), 1, fp);
            if (strcmp(child_buffer.name, path) == 0) 
            {
                parent_blockID = child_blockID;
                memset(&parent_buffer, 0, sizeof(parent_buffer));
                fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + parent_blockID*blocksize, SEEK_SET);
                n_iterations++;
                break;
            }    
        }
    }
    while ((path = strtok(NULL, delimiter)) != NULL && n_iterations < max_iterations);

    // search failed, does not exist 
    return parent_blockID;
}