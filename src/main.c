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
    char password[256];

    //const char path[256] = "/home/mike/Documents/GitHub/CompreC/dick.txt";

    while ((opt = getopt_long(argc, argv, "hf:b:d:p:e:o:i:c:r:", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'h':
                // Display help menu
                printFile("help.txt");
                break;
            case 'o':
                // Open the user interface
                menu(optarg);
                break;
            case 'b':
                // WIP bruteforce
                printf("b");
                break;
            case 'd':
                // WIP password cracking with dictionary
                if (extractZIPWithBruteForce(optarg, file_name) == -1) {
                    printf("Brute Force attack failed\n");
                    return -1;
                } else {
                    printf("Extraction done \n");
                }
                break;
            case 'p':
                // Store password given by the user
                strcpy(password, optarg);
                break;
            case 'e':
                // Extract archive in the current directory with password if one given
                extractArchive(optarg, password);
                break;
            case 'i':
                // Include a file or directory in a given zip archive
                includeElementToZip(optarg, file_name, basename(file_name), "");
                break;
            case 'c':
                // Create a zip archive from a file or folder
                createZip(optarg, file_name);
                break;
            case 'f':
                // Store file's name given by the user
                strcpy(file_name, optarg);
                break;
            case 'r':
                // Remove a file or folder and its elements in a zip archive
                removeElementFromArchive(optarg, file_name);
                break;
            default:
                // Display help menu
                printFile("help.txt");
                return 0;
        }
    }
        return 0;
    }


