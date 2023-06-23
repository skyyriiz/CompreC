#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zip.h>
#include <libgen.h>

#include "manage.h"


int includeFileToZip(const char* zip_file, const char* file, const char* end_filename) {
    struct zip* archive = zip_open(zip_file, ZIP_CREATE, NULL);
    if (!archive) {
        return -1;
    }

    struct zip_source* source = zip_source_file(archive, file, 0, -1);
    if (!source) {
        zip_close(archive);
        return -1;
    }

    const char* file_path = end_filename;
    int index = zip_name_locate(archive, file_path, 0);
    if (index >= 0) {
        zip_delete(archive, index);
    }

    int result = zip_add(archive, file_path, source);
    zip_close(archive);
    return result;
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