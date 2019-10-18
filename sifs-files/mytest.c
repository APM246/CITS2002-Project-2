#include <stdio.h>
#include <stdlib.h>
#include "sifs.h"
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

char *writefile(char *pathname, size_t *size)
{
    FILE *fp = fopen(pathname, "r");
    struct stat buf;
    stat(pathname, &buf);
    *size = buf.st_size;
    char *buffer = malloc(*size);
    fread(buffer, *size, 1, fp);
    return buffer;
}

void free_entrynames(char **entrynames, uint32_t nentries)
{
    if(entrynames != NULL) {
        for(int e=0 ; e<nentries ; ++e) {
            free(entrynames[e]);
        }
        free(entrynames);
    }
}

int main(int argc, char *argv[])
{
    char *volumename = "project testing/propertest";

    if (access(volumename, F_OK) == 0)
    {
        remove(volumename);
    }

    size_t blocksize = 2500;
    uint32_t nblocks = 270;
    SIFS_mkvolume(volumename, blocksize, nblocks);

    char names[20][100] = {{"/dir1"}, {"/dir1/subdir1"}, {"/dir2"}, {"/dir1/subdir2"}, {"/dir2/wow"}, 
    {"/diar3"}, {"/diar5"}, {"/diar6"}, {"/diar4"}, {"/diar4/three"}};

    for (int i = 0; i < 10; i++)
    {
        SIFS_mkdir(volumename, names[i]);
    }

    SIFS_rmdir(volumename, "/diar3");
    SIFS_rmdir(volumename, "/diar5");

    size_t size;
    char *filename = "tocopy/random2.txt";
    char *data = writefile(filename, &size);
    SIFS_writefile(volumename, "/diar4/three/test.txt", data, size); 
    free(data);

    //SIFS_mkdir(volumename, "/diar4/one"); SIFS_perror(NULL);
    SIFS_mkdir(volumename, "/dir2/wow1");
    SIFS_mkdir(volumename, "/diar4/one1");
    SIFS_mkdir(volumename, "/diar7");
    SIFS_mkdir(volumename, "/diar7/deep");
    filename = "tocopy/long.docx";
    data = writefile(filename, &size);
    SIFS_writefile(volumename, "/diar7/deep/super.txt", data, size);
    free(data);

    filename = "tocopy/random2.txt";
    data = writefile(filename, &size);
    SIFS_writefile(volumename, "/diar7/deep/test1.txt", data, size);
    free(data);

    SIFS_mkdir(volumename, "/dir6");
    filename = "tocopy/random2.txt";
    data = writefile(filename, &size);
    SIFS_writefile(volumename, "/diar6/wtf", data, size);
    free(data);

    filename = "tocopy/optimised.gif";
    data = writefile(filename, &size);
    SIFS_writefile(volumename, "/dir2/woo.c", data, size);
    free(data);

    SIFS_rmdir(volumename, "/dir2/wow1");
    SIFS_mkdir(volumename, "dir6/please");


    printf("\n\nexpected:\nsuper.txt\ntest1.txt\n-----------------------\n");
    char        **entrynames;
    uint32_t    nentries;
    time_t      modtime;
    SIFS_dirinfo(volumename, "//diar7//deep", &entrynames, &nentries, &modtime); 
    for(int e=0 ; e<nentries ; ++e) {
        printf("%s\n", entrynames[e]); 
    }
    free_entrynames(entrynames, nentries);
    printf("\n\nexpected:\nwow\nwoo.c\n-----------------------\n");

    SIFS_dirinfo(volumename, "/dir2", &entrynames, &nentries, &modtime);
    for(int e=0 ; e<nentries ; ++e) {
        printf("%s\n", entrynames[e]); 
    }
    free_entrynames(entrynames, nentries);

    size_t length;
    printf("\n\nexpected: 139389\n-------------------------\n");
    SIFS_fileinfo(volumename, "///diar7////deep//super.txt", &length, &modtime);
    printf("modtime: %slength:%lu\n\n", ctime(&modtime), length);

    remove("project testing/shouldbeunicorn.gif");
    void *buffer;
    size_t nbytes;
    SIFS_readfile(volumename, "/dir2/woo.c", &buffer, &nbytes);
    FILE *fp = fopen("project testing/shouldbeunicorn.gif", "w");
    fwrite(buffer, nbytes, 1, fp);
    free(buffer);
    fclose(fp);

    data = writefile("tocopy/random2.txt", &size);
    SIFS_writefile(volumename, "dir6///////////////supersuperlongname", data, size);

    SIFS_rmfile(volumename, "/diar7/deep///test1.txt");
    printf("\nCheck visual. Order should be test.txt, wtf (index 1) and supersuperlongname (index 2).\n\n");
    char *nullptr = NULL;

    void *buffer1;
    size_t nbytes1;
    //SIFS_writefile(volumename, "/diar4//cool")
    //SIFS_mkdir(volumename, "/diar4/two2");

    // ERROR CHECKING
    SIFS_perror(NULL);
    SIFS_writefile("wrooong", "/dir6/interesting.txt", data, size); SIFS_perror("No such volume:");
    SIFS_mkdir("project testing/propertest", nullptr); SIFS_perror("Invalid argument");
    SIFS_fileinfo("project testing/propertest", "diar7/deep/doesnexist.txt", &length, &modtime); SIFS_perror("No such file or directory entry");
    SIFS_mkdir(volumename, "/dir2//wo7/nope"); SIFS_perror("Invalid argument");
    SIFS_writefile(volumename, "diar4/three/test.txt", data, size); SIFS_perror("Volume, file or directory already exists");
    SIFS_rmdir(volumename, "////dir6"); SIFS_perror("Directory is not empty");
    SIFS_rmfile(volumename, "/dir1/subdir2"); SIFS_perror("Not a file");
    SIFS_dirinfo(volumename, "/dir2/woo.c", &entrynames, &nentries, &modtime); SIFS_perror("Not a directory");
    SIFS_readfile("sample volumes/photos", "/dir2/woo.c", &buffer1, &nbytes1); SIFS_perror("Not a volume");
    data = writefile("tocopy/random2.txt", &size);
    SIFS_writefile("project testing/shouldbeunicorn.gif", "/dir2/woo.c", data, size); SIFS_perror("Not a volume");

    free(data);
    return 0;
}
