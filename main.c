#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zip.h>
#include <getopt.h>
#include <libgen.h>

#include "manage.h"
#include "crack.h"
#include "interface.h"

int removeElementFromArchive(const char* archivePath, const char* elementToRemove) {
    // Open the archive
    zip_t* archive = zip_open(archivePath, ZIP_CHECKCONS, NULL);
    if (!archive) {
        // Handle archive opening error
        printf("Error: Failed to open the archive.\n");
        return -1;
    }

    // Check if the element is a directory
    const char* slash = "/";
    size_t elementLength = strlen(elementToRemove);
    if (strncmp(&elementToRemove[elementLength - 1], slash, 1) == 0) {
        // The element is a directory

        // Remove files from the directory in the archive
        size_t prefixLength = elementLength;
        if (elementLength == 1) {
            prefixLength = 0;
        }

        // Get the list of files inside the directory
        zip_int64_t numFiles = zip_get_num_entries(archive, ZIP_FL_UNCHANGED);
        zip_int64_t* filesToRemove = (zip_int64_t*)malloc(sizeof(zip_int64_t) * numFiles);
        int numFilesToRemove = 0;

        for (zip_int64_t i = 0; i < numFiles; ++i) {
            const char* filename = zip_get_name(archive, i, ZIP_FL_UNCHANGED);
            if (!filename) {
                // Handle filename retrieval error
                free(filesToRemove);
                zip_close(archive);
                printf("Error: Failed to retrieve the filename.\n");
                return -1;
            }

            if (strncmp(elementToRemove, filename, prefixLength) == 0) {
                filesToRemove[numFilesToRemove++] = i;
            }
        }

        // Remove files in reverse order
        for (int i = numFilesToRemove - 1; i >= 0; --i) {
            if (zip_delete(archive, filesToRemove[i]) != 0) {
                // Handle file deletion error
                free(filesToRemove);
                zip_close(archive);
                printf("Error: Failed to remove the file %ld from the archive.\n", filesToRemove[i]);
                return -1;
            }
        }

        free(filesToRemove);
    } else {
        // The element is a file

        // Check if the file exists in the archive
        zip_int64_t index = zip_name_locate(archive, elementToRemove, ZIP_FL_NOCASE);
        if (index < 0) {
            // Handle element not found error
            zip_close(archive);
            printf("Error: Failed to find the element %s in the archive.\n", elementToRemove);
            return -1;
        }

        // Remove the file from the archive
        if (zip_delete(archive, index) != 0) {
            // Handle file deletion error
            zip_close(archive);
            printf("Error: Failed to remove the file %s from the archive.\n", elementToRemove);
            return -1;
        }
    }

    // Close the archive
    zip_close(archive);

    return 0;
}

int main(int argc, char *argv[]) {
    int opt;
    static struct option long_options[] = {
            {"help",       no_argument,       0,  'h' },
            {"file",       required_argument, 0,  'f' },
            {"bruteforce", required_argument, 0,  'b' },
            {"dictionary", required_argument, 0,  'd' },
            {"password",   required_argument, 0,  'p' },
            {"extract",    required_argument, 0,  'e' },
            {"open",       required_argument, 0,  'o' },
            {"include",    required_argument, 0,  'i' },
            {"create",     required_argument, 0,  'c' },
            {"remove",     required_argument, 0,  'r' }

    };
    int long_index=0;
    char file_name[128];
    char fileToZip[256] = "";
    char password[256];


    while ((opt = getopt_long(argc, argv, "hf:b:d:p:e:o:i:c:r:", long_options, &long_index)) != -1) {
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
                strcpy(password, optarg);
                break;
            case 'e':
                extractArchive(optarg, password);
                break;
            case 'i':
                includeFileToZip(optarg, file_name, basename(file_name));
                break;
            case 'c':
                createZip(optarg, file_name);
                break;
            case 'f':
                strcpy(file_name, optarg);
                break;
            case 'r':
                removeElementFromArchive(optarg, file_name);
                break;
            default:
                printf("h\n");
                return 0;
        }
    }
        return 0;
    }


