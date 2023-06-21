#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zip.h>

#include "manage.h"

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