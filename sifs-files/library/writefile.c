#include "helperfunctions.h"

// find contiguous blocks to store contents of files - returns first blockID of data blocks 
int find_contiguous_blocks(size_t nbytes, size_t blocksize, int nblocks, const char *volumename, int *nblocks_needed)
{
    *nblocks_needed = ceil(((double) nbytes)/blocksize); 
    FILE *fp = fopen(volumename, "r"); 
    char bitmap[nblocks];
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    fread(bitmap, sizeof(bitmap), 1, fp);

    if (nblocks < *nblocks_needed + 1 + 1)
    {
        return NO_CONTIGUOUS_BLOCKS;
    }
    
    bool isReserved = true; //reserve for fileblock
    for (int i = 0; i < nblocks; i++)
    {
        for (int j = 0; j < *nblocks_needed; j++)
        {
            // FIRST UNUSED BLOCK THAT IS ENCOUNTERED MUST BE RESERVED FOR FILEBLOCK
            if (bitmap[i] == SIFS_UNUSED && isReserved)
            {
                isReserved = false;
                break;
            }

            if (j == *nblocks_needed - 1 && bitmap[i+j] == SIFS_UNUSED) 
            {
                return i;
            }
            else if (bitmap[i+j] != SIFS_UNUSED) 
            {
                break;
            }
        }
    }

    return NO_CONTIGUOUS_BLOCKS;
}

// add a copy of a new file to an existing volume
int SIFS_writefile(const char *volumename, const char *pathname,
		   void *data, size_t nbytes)
{
    if (volumename == NULL || pathname == NULL || data == NULL || nbytes <= 0)
    {
        SIFS_errno = SIFS_EINVAL;
        return 1;
    }

    if (!check_valid_volume(volumename))
    {
        return 1;
    }   

    // ACCESS VOLUME INFORMATION
    int nblocks, blocksize, fileblockID, firstblockID, nblocks_needed;
    get_volume_header_info(volumename, &blocksize, &nblocks);

     // THROW ERROR IF NAME IS TOO LONG OR FILENAME PROVIDED IS JUST "/"
    if ((strlen(find_name(pathname)) + 1) > SIFS_MAX_NAME_LENGTH || (strlen(pathname) == 1 && *pathname == '/'))
    {
        SIFS_errno = SIFS_EINVAL;
        return 1;
    } 

    // CHECK IF PATHNAME IS VALID 
    int parent_blockID = find_parent_blockID(volumename, pathname, nblocks, blocksize);
    if (parent_blockID == NO_SUCH_BLOCKID)
    {
        SIFS_errno = SIFS_EINVAL;
        return 1;
    }
    else if (parent_blockID == MEMORY_ALLOCATION_FAILED)
    {
        SIFS_errno = SIFS_ENOMEM;
        return 1;
    }

    // FILE WITH THE PROVIDED NAME ALREADY EXISTS 
    if (find_blockID(volumename, pathname, nblocks, blocksize) != NO_SUCH_BLOCKID)
    {
        SIFS_errno = SIFS_EEXIST;
        return 1;
    }

    // CHECK IF PARENT BLOCK HAS NO SPACE LEFT FOR ENTRIES
    FILE *fp = fopen(volumename, "r+");
    fseek_to_blockID(parent_blockID);
    SIFS_DIRBLOCK dir;
    fread(&dir, sizeof(SIFS_DIRBLOCK), 1, fp);
    if (dir.nentries == SIFS_MAX_ENTRIES)
    {
        SIFS_errno = SIFS_EMAXENTRY;
        return 1;
    }

    // DETERMINE IF FILE IS IDENTICAL TO PRE-EXISTING FILE
    unsigned char MD5buffer[MD5_BYTELEN];
    MD5_buffer(data, nbytes, MD5buffer);
    char bitmap[nblocks];
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
    fread(bitmap, sizeof(bitmap), 1, fp);
    bool isIdentical = false;
    for (int i = 0; i < nblocks; i++)
    {
        if (bitmap[i] == SIFS_FILE)
        {
            SIFS_FILEBLOCK fb;
            fseek_to_blockID(i);
            fread(&fb, sizeof(SIFS_FILEBLOCK), 1, fp);
            if (memcmp(fb.md5, MD5buffer, MD5_BYTELEN) == 0)
            {
                isIdentical = true;
                fileblockID = i;
                break;
            }
        }
    }

    // CHANGE BITMAP TO ADD NEW FILE BLOCK IF FILE IS NOT IDENTICAL 
    if (!isIdentical)
    {
        firstblockID = find_contiguous_blocks(nbytes, blocksize, nblocks, volumename, &nblocks_needed);

        if (firstblockID == NO_CONTIGUOUS_BLOCKS)
        {
            SIFS_errno = SIFS_ENOSPC;
            return 1;
        }

        change_bitmap(volumename, SIFS_FILE, &fileblockID, nblocks); //add 'f' to bitmap
    }

    char *file_name = find_name(pathname); 

    // INITIALISE FILEBLOCK OR UPDATE PRE-EXISTING FILEBLOCK IF FILE IS IDENTICAL
    SIFS_FILEBLOCK fileblock;
    if (!isIdentical)
    {
        memset(&fileblock, 0, sizeof(fileblock));
        strcpy(fileblock.filenames[0], file_name);
        fileblock.firstblockID = firstblockID;
        fileblock.length = nbytes;
        fileblock.modtime = time(NULL); 
        fileblock.nfiles = 1;
        memcpy(&fileblock.md5, MD5buffer, MD5_BYTELEN); 

        // WRITE THE ACTUAL FILE (DATA BLOCKS) TO THE VOLUME
        fseek_to_blockID(firstblockID);
        fwrite(data, nbytes, 1, fp);

        // UPDATE BITMAP
        fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
        fread(bitmap, sizeof(bitmap), 1, fp);
        for (int i = firstblockID; i < firstblockID + nblocks_needed; i++)
        {
            bitmap[i] = SIFS_DATABLOCK;
        }
        fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
        fwrite(bitmap, sizeof(bitmap), 1, fp);
    }
    
    else
    {
        fseek_to_blockID(fileblockID);
        fread(&fileblock, sizeof(SIFS_FILEBLOCK), 1, fp);

        // NO SPACE LEFT IN FILEBLOCK (FILENAMES ARRAY)
        if (fileblock.nfiles == SIFS_MAX_ENTRIES)
        {
            SIFS_errno = SIFS_ENOSPC;
            return 1;
        }
        strcpy(fileblock.filenames[fileblock.nfiles], file_name); 
        fileblock.nfiles++; 
    }
    
    // WRITE THE FILEBLOCK TO THE VOLUME 
    fseek_to_blockID(fileblockID);
    fwrite(&fileblock, sizeof(fileblock), 1, fp);

    // UPDATE PARENT DIRECTORY - MODTIME, NENTRIES AND ENTRIES ARRAY 
    fseek_to_blockID(parent_blockID);
    SIFS_DIRBLOCK dirblock;
    fread(&dirblock, sizeof(SIFS_DIRBLOCK), 1, fp);
    dirblock.modtime = time(NULL);
    dirblock.entries[dirblock.nentries].blockID = fileblockID;
    dirblock.entries[dirblock.nentries].fileindex = fileblock.nfiles - 1; 
    dirblock.nentries++;
    
    // WRITE PARENT DIRECTORY TO VOLUME
    fseek_to_blockID(parent_blockID);
    fwrite(&dirblock, sizeof(dirblock), 1, fp);

    fclose(fp);
    return 0;
}
