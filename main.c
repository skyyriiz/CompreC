#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zip.h>

#include "manage.h"
#include "crack.h"
#include "interface.h"

int main(int argc, char *argv[]) {
    int opt;
    char fileToZip[256] = "";

    while ((opt = getopt(argc, argv, "ho:bdpeicf:")) != -1) {
        char* optarg_value = optarg;

        switch (opt) {
            case 'h':
                printFile("help.txt");
                break;
            case 'o':
                /*if (optarg != NULL) {
                    char* archiveName = optarg;
                    printArchiveContent(archiveName);
                } else {
                    fprintf(stderr, "Veuillez spécifier un nom d'archive.\n");
                }*/
                break;
            case 'b':
                printf("b");
                break;
            case 'd':
                printf("d");
                break;
            case 'p':
                printf("p");
                break;
            case 'e':
                if (optind < argc) {
                    extractArchive(argv[optind]);
                } else {
                    fprintf(stderr, "Veuillez spécifier un fichier ou un dossier à extraire.\n");
                }
                break;

            case 'i':
                printf("i");
                break;
            case 'c':
                createZip("test", "archive");
                break;
            case 'f':
                printf("f");
                break;
            default:
                printf("h");
                return 0;
        }
    }

    return 0;
}