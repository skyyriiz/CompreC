//
// Created by oui on 21/06/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <zip.h>
#include <string.h>
#include <libgen.h>


#include "interface.h"
#include "manage.h"

void printMenu() {
    printf("\nWhat do you want to do with this file?\n");
    printf("1. Delete the file\n");
    printf("2. Rename the file\n");
    printf("3. Leave the program\n");
    printf("Choose : ");
}

void printMainMenu(){
    printf("\nWhat do you want to do?\n");
    printf("1. Extract\n");
    printf("2. Include a file\n");
    printf("3: Select a file\n");
    printf("4. Leave the program\n");
    printf("Choose : ");
}

void menu(char *zip) {
    struct zip *archive;
    int err;
    char new_name[256];

    int choice_main;
    int choice_extract;
    char *fileToAdd;
    char file_to_modify[256];
    int exitMenu = 0;


    while(exitMenu == 0) {
        printf("\n\n");
        archive = zip_open(zip, 0, &err);
        if (archive == NULL) {
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

        printf("\n");
        printMainMenu();
        scanf("%d", &choice_main);
        switch (choice_main) {
            case 1:
                printf("Do you want to extract the zip file? (No -> 0 / Yes -> 1)\n");
                scanf("%d", &choice_extract);

                char password[256];
                int choice_password;
                printf("There is password in the zip file? (No -> 0 / Yes -> 1)\n");
                scanf("%d", &choice_password);

                if (choice_extract == 1 && choice_password == 1) {
                    printf("Write the password: ");
                    scanf("%s", password);
                    extractArchive(zip, password);
                } else if (choice_extract == 1 && choice_password == 0) {
                    extractArchive(zip, password);
                } else if (choice_extract == 0 && choice_password == 0) {
                    printf("OK");
                } else {
                    printf("Wrong entry.");
                }
                break;
            case 2:
                printf("Enter the path of the file that you want to add (+ the name of the file in it)\n");
                scanf("%s", fileToAdd);
                includeFileToZip(zip, fileToAdd, basename(fileToAdd));
                break;
            case 3:
                printf("Enter the name of the file to modify: ");
                scanf("%s", file_to_modify);

                int file_index = -1;

                for (i = 0; i < num_files; i++) {
                    const char *file_name = zip_get_name(archive, i, 0);
                    if (strcmp(file_name, file_to_modify) == 0) {
                        file_index = i;
                        break;
                    }
                }

                if (file_index == -1) {
                    printf("Le fichier spécifié n'a pas été trouvé dans le fichier ZIP.\n");
                    zip_close(archive);
                    exit(EXIT_FAILURE);
                }

                int choice = 0;
                while (choice != 3) {
                    printMenu();
                    scanf("%d", &choice);

                    switch (choice) {
                        case 1:
                            if (zip_delete(archive, file_index) == -1) {
                                printf("Impossible to delete the file.\n");
                            } else {
                                char new_name[256];
                                printf("The file just get deleted.\n");
                            }
                            break;

                        case 2:
                            printf("Entrer the new file name : ");
                            scanf("%s", new_name);
                            if (zip_rename(archive, file_index, new_name) == -1) {
                                printf("Impossible to rename the file.\n");
                            } else {
                                printf("The file just get renamed.\n");
                            }
                            break;

                        case 3:
                            printf("Done.\n");
                            break;

                        default:
                            printf("Wrong choice, retry.\n");
                            break;
                    }
                    break;
                    case 4:
                        printf("Done.\n");
                        exitMenu = 1;
                    break;

                    default:
                        printf("Wrong choice, retry.\n");
                    break;
                }
                zip_close(archive);
        }
    }
}
