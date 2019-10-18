#include "helperfunctions.h"

// remove an existing file from an existing volume
int SIFS_rmfile(const char *volumename, const char *pathname)
{
   if (volumename == NULL || pathname == NULL)
    {
        SIFS_errno = SIFS_EINVAL;
        return 1;
    }

    if (!check_valid_volume(volumename))
    {
        return 1;
    }

     // THROW ERROR IF USER TRIES TO DELETE ROOT DIRECTORY
    if (strlen(pathname) == 1 && *pathname == '/')
    {
        SIFS_errno = SIFS_ENOTFILE;
        return 1;
    }

    // OBTAIN INFORMATION 
    FILE *fp = fopen(volumename, "r+");
    uint32_t nblocks;
    size_t blocksize;
    SIFS_BLOCKID blockID;
    get_volume_header_info(volumename, &blocksize, &nblocks);
    if ((blockID = find_blockID(volumename, pathname, nblocks, blocksize)) == NO_SUCH_BLOCKID)
    {
        SIFS_errno = SIFS_ENOENT;
        return 1;
    } 
    char *name = find_name(pathname); 
    fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);

    // READ BITMAP
    char bitmap[nblocks];
    fread(bitmap, sizeof(bitmap), 1, fp);
    SIFS_BIT type = bitmap[blockID];

    // THROW ERROR IF PATHNAME IS NOT A FILE
    if (type != SIFS_FILE)
    {
        SIFS_errno = SIFS_ENOTFILE; // The block is a directory block
        return 1;
    }

    // OBTAIN INFO ABOUT NENTRIES 
    SIFS_BLOCKID parent_blockID = find_parent_blockID(volumename, pathname, nblocks, blocksize); 
    SIFS_DIRBLOCK dirblock;
    fseek_to_blockID(parent_blockID);
    fread(&dirblock, sizeof(SIFS_DIRBLOCK), 1, fp);
    int nentries = dirblock.nentries; 

    // UPDATE FILEBLOCK (EITHER REMOVE IF LAST ENTRY OR UPDATE VALUES)
    fseek_to_blockID(blockID);
    SIFS_FILEBLOCK fileblock;
    fread(&fileblock, sizeof(SIFS_FILEBLOCK), 1, fp); 
    uint32_t fileindex; 
    if ((fileindex = find_fileindex(&fileblock, name)) == NO_SUCH_FILENAME)
    {
        SIFS_errno = SIFS_ENOENT; // LAST CHECK FOR WHETHER PATHNAME IS INVALID
        return 1;
    }
    if (fileblock.nfiles == 1)
    {        
        // REMOVE DATABLOCK(S) FROM VOLUME 
        SIFS_BLOCKID firstblockID = fileblock.firstblockID; 
        fseek_to_blockID(firstblockID);
        char zeroes[fileblock.length];
        memset(zeroes, 0, sizeof(zeroes));
        fwrite(zeroes, sizeof(zeroes), 1, fp); 

        //UPDATE BITMAP AND WRITE TO VOLUME 
        bitmap[blockID] = SIFS_UNUSED; // UPDATE FILEBLOCK BIT
        uint32_t n_datablocks = ceil(((double) fileblock.length)/blocksize); 
        for (int i = firstblockID; i < firstblockID + n_datablocks; i++)
        {
            bitmap[i] = SIFS_UNUSED; //UPDATE DATABLOCKS BITS
        }
        fseek(fp, sizeof(SIFS_VOLUME_HEADER), SEEK_SET);
        fwrite(bitmap, nblocks*sizeof(SIFS_BIT), 1, fp);

        // REMOVE FILEBLOCK FROM VOLUME 
        memset(&fileblock, 0, sizeof(SIFS_FILEBLOCK));
    }
    else
    {
        // SORT FILENAMES ARRAY (shuffle down)
        sort_filenames(fp, name, &fileblock, blockID, nblocks, blocksize);
        fileblock.nfiles--;
    }
    
    // WRITE FILE BLOCK BACK TO VOLUME
    fseek_to_blockID(blockID);
    fwrite(&fileblock, sizeof(SIFS_FILEBLOCK), 1, fp);

    //UPDATE PARENT DIRECTORY 
    fseek_to_blockID(parent_blockID);
    fread(&dirblock, sizeof(SIFS_DIRBLOCK), 1, fp);
    for (int i = 0; i < nentries; i++)   
    {
        if (dirblock.entries[i].blockID == blockID && dirblock.entries[i].fileindex == fileindex)
        {
            // shuffle entries in entries array 1 spot down
            while (i < nentries - 1)
            { 
                dirblock.entries[i].blockID = dirblock.entries[i+1].blockID;
                dirblock.entries[i].fileindex = dirblock.entries[i+1].fileindex;
                i++;
            }

            break;
        }
    }
    dirblock.entries[nentries - 1].blockID = 0;   //update last spot that was shuffled down
    dirblock.entries[nentries - 1].fileindex = 0;
    dirblock.modtime = time(NULL);
    dirblock.nentries--;

    // WRITE BACK TO VOLUME 
    fseek_to_blockID(parent_blockID);
    fwrite(&dirblock, sizeof(SIFS_DIRBLOCK), 1, fp);

    fclose(fp);
    return 0;
}
