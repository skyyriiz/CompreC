#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zip.h>
#include <libgen.h>

#include "manage.h"

void includeFileToZip(char *path_to_file, char *path_to_zip){
    int err = 0;
    struct zip *f_zip = zip_open(path_to_zip, ZIP_CHECKCONS, &err);
    struct zip_source * n_zip = zip_source_file(f_zip,path_to_file, (off_t)0, (off_t)0);
    zip_add(f_zip, basename(path_to_zip), n_zip);
    zip_close(f_zip);
    zip_source_free(n_zip);
}

void printFile(char *filename){
    FILE *fptr;
    char c;
    fptr = fopen(filename, "r");
    if (fptr == NULL)
    {
        printf("Cannot open file \n");
        exit(0);
    }
    c = fgetc(fptr);
    while (c != EOF)
    {
        printf ("%c", c);
        c = fgetc(fptr);
    }

    fclose(fptr);

}

int createZip(char *path){
    int error = 0;
    struct zip *archive = zip_open(path, ZIP_CREATE, &error);
    if(error){
        printf("could not open or create archive\n");
        return -1;
    }
    zip_close(archive);
    printf("Archive created\n");
    return 0;
}