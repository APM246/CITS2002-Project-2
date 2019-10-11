#include "sifs-internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//  EXTRACT NAME (REMOVE SUPER DIRECTORIES FROM NAME)
char *find_name(const char *pathname)
{
    char *directory_name;
    if ((directory_name = strrchr(pathname, '/')) != NULL)
    {
        directory_name++; // move one char past '/'
    }
    else
    {
        directory_name = malloc(SIFS_MAX_NAME_LENGTH);
        strcpy(directory_name, pathname);
    }
    return directory_name;
}

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

// can't write to an uninitialised pointer (e.g. in strcpy()) but other functions don't need to malloc (e.g strrchr, pointer points to already
// allocated memory or handles malloc() itself) (exception initialise to NULL?)
// weirdly a double pointer is used for the address of a single pointer, triple pointer for double pointer, etc.
int find_parent_blockID(const char *volumename, const char *pathname, int nblocks, int blocksize)
{
    int max_iterations;
    if ((max_iterations = get_number_of_slashes(pathname)) == 0) return 0;
    char *path_name = malloc(SIFS_MAX_NAME_LENGTH); 
    strcpy(path_name, pathname);
    if (*pathname == '/')
    {
        if (strlen(pathname) == 1) return 0; // pathname provided is the root directory 
        else path_name++; //skip past first '/'
    }

    FILE *fp = fopen(volumename, "r+");
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT), SEEK_SET);

    char *path;
    int parent_blockID = 0;
    SIFS_DIRBLOCK parent_buffer;
    int child_blockID;
    SIFS_DIRBLOCK child_buffer;
    char delimiter[] = "/"; 
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

    return parent_blockID;
}

// read bitmap through this function as well, add extra paramter 
int find_blockID(const char *volumename, const char *pathname, int nblocks, int blocksize)
{
    char *directory_name = find_name(pathname);
    int parent_blockID = find_parent_blockID(volumename, pathname, nblocks, blocksize);
    FILE *fp = fopen(volumename, "r+");
    fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + parent_blockID*blocksize, SEEK_SET);
    SIFS_DIRBLOCK parent_dirblock; 
    fread(&parent_dirblock, sizeof(parent_dirblock), 1, fp);
    int nentries = parent_dirblock.nentries;

    for (int i = 0; i < nentries; i++)
    {
        int entry_blockID = parent_dirblock.entries[i].blockID;
        fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
        char bitmap[nblocks]; 
        fread(bitmap, sizeof(bitmap), 1, fp);
        SIFS_BIT type = bitmap[entry_blockID];
        fseek(fp, sizeof(SIFS_VOLUME_HEADER) + nblocks*sizeof(SIFS_BIT) + entry_blockID*blocksize, SEEK_SET);
        
        if (type == SIFS_DIR)
        {
            SIFS_DIRBLOCK entry_dirblock;
            fread(&entry_dirblock, sizeof(entry_dirblock), 1, fp);
            if (strcmp(entry_dirblock.name, directory_name) == 0) 
            {
                
                return entry_blockID;
            }
        }

        else if (type == SIFS_FILE)
        {
            SIFS_FILEBLOCK entry_file;
            int index = parent_dirblock.entries[i].fileindex;
            fread(&entry_file, sizeof(entry_file), 1, fp);
            if (strcmp(entry_file.filenames[index], directory_name) == 0)
            {
                // add bitmap parameter assignment via pointer 
                return entry_blockID;
            }
        }
    }

    return -1;
}