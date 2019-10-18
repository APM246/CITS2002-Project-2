#include "helperfunctions.h"

// -----------------------------------------------------------------------

void get_volume_header_info(const char *volumename, int *blocksize, int *nblocks)
{
    FILE *fp = fopen(volumename, "r+");
    char buffer[sizeof(SIFS_VOLUME_HEADER)];
    fread(buffer, sizeof(buffer), 1, fp);
    *nblocks = ((SIFS_VOLUME_HEADER *) buffer)->nblocks; 
    *blocksize = ((SIFS_VOLUME_HEADER *) buffer)->blocksize;
    fclose(fp);
}

// -----------------------------------------------------------------------

int change_bitmap(const char *volumename, char type, int *blockID, int nblocks)
{
    FILE *fp = fopen(volumename, "r+");
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    SIFS_BIT bit;

    for (int i = 0; i < nblocks; i++)
    {
        fread(&bit, sizeof(char), 1, fp);
        if (bit == SIFS_UNUSED) 
        {
            *blockID = i;
            fseek(fp, -sizeof(SIFS_BIT), SEEK_CUR);
            fwrite(&type, sizeof(char), 1, fp);
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return 1;
}

// -------------------------------------------- PATHNAME 

//  EXTRACT NAME (REMOVE SUPER DIRECTORIES FROM NAME)
char *find_name(const char *pathname)
{
    char *directory_name;
    if ((directory_name = strrchr(pathname, '/')) != NULL)
    {
        directory_name++; // move one char past '/'
        return directory_name;
    }
    else
    {
        return (char *) pathname; //return pathname since it doesn't have any slashes
        // still satisfies "const" property 
    }
}

// RETURNS 0 IF ARGUMENT IS "/NAME. FUNCTION IS DESIGNED TO BE USED WITH NON-ROOT ENTRIES  
int get_number_of_slashes(const char* pathname)
{
    char *path_name = malloc(strlen(pathname) + 1); 
    if (path_name == NULL)
    {
        return MEMORY_ALLOCATION_FAILED;
    }
    strcpy(path_name, pathname);
    int number = 0;
    char delimiter[] = "/";
    if (strcmp(strtok(path_name, delimiter), pathname) == 0) 
    {
        free(path_name);
        return 0;
    }

    while (strtok(NULL, delimiter) != NULL) number++; 
    free(path_name); 
    return number;
}

// --------------------------------------------------------- blockID 

int find_parent_blockID(const char *volumename, const char *pathname, int nblocks, int blocksize)
{
    int max_iterations = get_number_of_slashes(pathname);
    if (max_iterations == 0) 
    {
        return 0;
    }
    else if (max_iterations == MEMORY_ALLOCATION_FAILED) 
    {
        return NO_SUCH_BLOCKID;
    }
    
    if (strlen(pathname) == 1 && pathname[0] == '/') 
    {
        return 0; // pathname provided is the root directory 
    }

    char *path_name = malloc(strlen(pathname) + 1);
    strcpy(path_name, pathname);

    FILE *fp = fopen(volumename, "r+");
    // FSEEK TO ROOT DIRECTORY TO BEGIN WITH 
    fseek_to_blockID(0);

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
        uint32_t nentries = max(parent_buffer.nentries, 1); // ensures nentries is not 0
        for (int i = 0; i < nentries; i++) 
        {
            child_blockID = parent_buffer.entries[i].blockID;
            fseek_to_blockID(child_blockID);
            memset(&child_buffer, 0, sizeof(child_buffer));
            fread(&child_buffer, sizeof(child_buffer), 1, fp);
            if (strcmp(child_buffer.name, path) == 0) 
            {
                parent_blockID = child_blockID;
                memset(&parent_buffer, 0, sizeof(parent_buffer));
                fseek_to_blockID(parent_blockID);
                n_iterations++;
                break;
            }

            //  PATHNAME COMPONENT DOESN'T EXIST 
            if (i == nentries - 1) 
            {
                free(path_name);
                return NO_SUCH_BLOCKID;
            }
        }
    }
    while ((path = strtok(NULL, delimiter)) != NULL && n_iterations < max_iterations);

    free(path_name);
    fclose(fp);
    return parent_blockID; 
}

// read bitmap through this function as well, add extra paramter 
int find_blockID(const char *volumename, const char *pathname, int nblocks, int blocksize)
{
    char *directory_name = find_name(pathname);
    int parent_blockID;
    if ((parent_blockID = find_parent_blockID(volumename, pathname, nblocks, blocksize)) == NO_SUCH_BLOCKID)
    {
        return NO_SUCH_BLOCKID;
    } 
    FILE *fp = fopen(volumename, "r+");
    fseek_to_blockID(parent_blockID);
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
        fseek_to_blockID(entry_blockID);
        
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
                return entry_blockID;
            }
        }
    }

    fclose(fp);
    return NO_SUCH_BLOCKID;
}

// -------------------------------------------------------- FILEBLOCK 

// FINDS FILEINDEX OF A PARTICULAR FILE 
int find_fileindex(SIFS_FILEBLOCK *fileblock, char *name)
{
    uint32_t nfiles = fileblock->nfiles;

    for (int i = 0; i < nfiles; i++)
    {
        if (strcmp(fileblock->filenames[i], name) == 0)
        {
            return i;
        }
    }

    return NO_SUCH_FILENAME;  // check return value in caller 
}

/* 
    WHEN THE FILENAMES ARRAY IS BEING SHUFFLED DOWN AFTER RMFILE() IS CALLED (TO ELIMINATE GAPS), 
    THE FILEINDEX VALUES OF VARIOUS FILES MUST BE DECREMENTED (ONLY THE ONES BEING SHUFFLED DOWN)
    E.G. IF NAME2 REMOVED FROM {NAME1, NAME2, NAME3} --> {NAME1, NAME3, ..}
    THE ENTRY IN THE ENTRIES ARRAY WHICH CORRESPONDS TO NAME3 MUST HAVE ITS FILEINDEX VALUE
    DECREMENTED 
*/
void decrement_fileindex(FILE *fp, int fileindex, SIFS_BLOCKID blockID, uint32_t nblocks, size_t blocksize)
{
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    char bitmap[nblocks];
    fread(bitmap, nblocks, 1, fp);

    for (int i = 0; i < nblocks; i++)
    {
        if (bitmap[i] == SIFS_DIR)
        {
            SIFS_DIRBLOCK dirblock;
            fseek_to_blockID(i);
            fread(&dirblock, sizeof(SIFS_DIRBLOCK), 1, fp);
            uint32_t nentries = dirblock.nentries;

            for (int j = 0; j < nentries; j++)
            {
                if (dirblock.entries[j].blockID == blockID && dirblock.entries[j].fileindex > fileindex)
                {
                    dirblock.entries[j].fileindex--;
                }
            }

            // WRITE BACK TO VOLUME 
            fseek_to_blockID(i);
            fwrite(&dirblock, sizeof(SIFS_DIRBLOCK), 1, fp);
        }
    }
}

void sort_filenames(FILE *fp, char *filename, SIFS_FILEBLOCK *fileblock, SIFS_BLOCKID blockID, uint32_t nblocks, size_t blocksize)
{
    uint32_t nfiles = fileblock->nfiles; //value hasn't been decremented yet 

    for (int i = 0; i < nfiles; i++)
    {
        if (strcmp(fileblock->filenames[i], filename) == 0)
        {
            decrement_fileindex(fp, i, blockID, nblocks, blocksize);
            while (i < nfiles - 1)
            {
                strcpy(fileblock->filenames[i], fileblock->filenames[i+1]);
                i++;
            }
            break;
        }
    }

    // update last spot
    strcpy(fileblock->filenames[nfiles - 1], "");
}

// ------------------------------------------------------------------- ERROR CHECKING

bool check_valid_volume(const char *volumename)
{
    // NO SUCH VOLUME 
    if (access(volumename, F_OK) != 0)
    {
        SIFS_errno	= SIFS_ENOVOL;
        return false;
    }

    FILE *fp = fopen(volumename, "r+");
    if (fp == NULL)
    {
        SIFS_errno = SIFS_ENOTVOL;
        return false;
    }

    // ENSURE FILE'S SIZE IS VALID (FIRST TEST)
    struct stat buf;
    stat(volumename, &buf);
    int size = buf.st_size;

    if (size > sizeof(SIFS_VOLUME_HEADER))
    {
        // CHECK THAT FILE IS A VALID VOLUME BY EXAMINING BLOCKSIZE (SECOND TEST)
        SIFS_VOLUME_HEADER header;
        fread(&header, sizeof(SIFS_VOLUME_HEADER), 1, fp);
        if (header.blocksize > 1023)
        {
            fclose(fp);
            return true;
        }
    }

    fclose(fp);
    SIFS_errno = SIFS_ENOTVOL;
    return false;   
}
