//
// Source with all functions used to manage user interface
//
#include <stdio.h>
#include <stdlib.h>
#include <zip.h>
#include <string.h>
#include <libgen.h>


#include "interface.h"
#include "manage.h"
#include "crack.h"

void printMenu() {
    // Program usage menu
    printf("\nWhat do you want to do with this file?\n");
    printf("1. Delete the file\n");
    printf("2. Rename the file\n");
    printf("3. Extract the file\n");
    printf("4. Leave the program\n");
    printf("Choose : ");
}

void printMainMenu(){
    // Main menu
    printf("\nWhat do you want to do?\n");
    printf("1. Extract\n");
    printf("2. Include a file\n");
    printf("3. Select a file\n");
    printf("4. Leave the program\n");
    printf("Choose : ");
}

void menu(char *zip) {
    // Start menu
    struct zip *archive;
    int err;
    char new_name[256];

    int choice_main;
    int choice_extract;
    char fileToAdd[256];
    char file_to_modify[256];
    int exitMenu = 0;

    const char path[256] = "/home/mike/Documents/GitHub/CompreC/dick.txt";

    while(exitMenu == 0) {
        // Main loop
        printf("\n\n");
        archive = zip_open(zip, 0, &err);
        if (archive == NULL) {
            // If the archive couldn't be opened
            printf("Impossible to open the ZIP file: %s\n", zip_strerror(archive));
            exit(EXIT_FAILURE);
        }

        int num_files = zip_get_num_files(archive);
        int i;

        for (i = 0; i < num_files; i++) {
            // Print each file and folder in the archive
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
                    if (extractZIPWithBruteForce(zip, path) == -1) {
                        printf("Brute Force attack failed\n");
                    } else {
                        printf("Extraction done \n");
                    }
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

                int choiceDirectory;
                printf("Where do you want to save in the archive? (In the root directory -> 0 / Other -> 1)\n");
                scanf("%d", &choiceDirectory);

                if(choiceDirectory == 0){
                    includeElementToZip(zip, fileToAdd, basename(fileToAdd), "");
                } else if (choiceDirectory == 1){
                    char pathFinal[256];
                    printf("What is the path where you want to put your file in this archive?\n");
                    scanf("%s", pathFinal);

                    int choiceName;
                    printf("Do you want to change the name of the file? (No -> 0 / Yes -> 1\n");
                    scanf("%d", &choiceName);

                    if(choiceName == 0){
                        includeElementToZip(zip, fileToAdd, basename(fileToAdd), pathFinal);
                    } else if(choiceName == 1){
                        char addNewName[256];
                        printf("Give his new name.\n");
                        scanf("%s", addNewName);
                        includeElementToZip(zip, fileToAdd, addNewName, pathFinal);
                    } else {
                        printf("Wrong choice, try again.\n");
                    }
                } else {
                    printf("Wrong choice, try again.\n");
                }
                break;
            case 3:
                printf("Enter the name of the file to modify: ");
                scanf("%s", file_to_modify);

                int file_index = -1;

                for (i = 0; i < num_files; i++) {
                    // Find the index of the wanted file
                    const char *file_name = zip_get_name(archive, i, 0);
                    if (strcmp(file_name, file_to_modify) == 0) {
                        file_index = i;
                        break;
                    }
                }

                if (file_index == -1) {
                    printf("The file is not found\n");
                    zip_close(archive);
                    exit(EXIT_FAILURE);
                }

                int choice = 0;
                while (choice != 3) {
                    printf("%s", file_to_modify);
                    printMenu();
                    scanf("%d", &choice);
                    struct zip_file *file = zip_fopen_index(archive, file_index, 0);
                    int file_index = -1;

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
                            if (file == NULL) {
                                printf("Impossible to open the zip file.\n");
                            } else {
                                char new_file_name[512];
                                snprintf(new_file_name, sizeof(new_file_name), "./%s", file_to_modify);
                                FILE *output_file = fopen(new_file_name, "wb");
                                if (output_file == NULL) {
                                    printf("Impossible to create te file.\n");
                                } else {
                                    char buffer[1024];
                                    int num_bytes;
                                    while ((num_bytes = zip_fread(file, buffer, sizeof(buffer))) > 0) {
                                        fwrite(buffer, 1, num_bytes, output_file);
                                    }
                                    fclose(output_file);
                                    printf("The file get extracted.\n");
                                }
                                zip_fclose(file);
                            }
                            break;

                        case 4:
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
