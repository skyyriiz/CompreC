//
// Created by oui on 21/06/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <zip.h>


#include "interface.h"

void menu(char *zip) {
    struct zip *archive;
    int err;

    archive = zip_open(zip, 0, &err);
    if (archive == NULL){
        printf("Impossible to open the ZIP file: %s\n", zip_strerror(archive));
        exit(EXIT_FAILURE);
    }

    int num_files = zip_get_num_files(archive);
    int i;

    for (i = 0; i < num_files; i++) {
        const char *file_name = zip_get_name(archive, i, 0);
        printf("%d ", i);
        printf("- Fichier : %s\n", file_name);
    }

    

    zip_close(archive);
}
