//
// Created by oui on 21/06/23.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zip.h>

#include "crack.h"

#define MAX_PASSWORD_LENGTH 50

int extractZIPWithBruteForce(const char* zipFileName, const char* dictionaryFileName) {
    struct zip *zipFile;
    struct zip_file *file;
    char password[MAX_PASSWORD_LENGTH];

    // Open the zip
    zipFile = zip_open(zipFileName, ZIP_CHECKCONS, NULL);
    if (!zipFile) {
        printf("Impossible d'ouvrir le fichier ZIP.\n");
        return -1;
    }

    // Open the dic
    FILE* dictionaryFile = fopen(dictionaryFileName, "r");
    if (!dictionaryFile) {
        printf("Impossible d'ouvrir le dictionnaire de mots de passe.\n");
        zip_close(zipFile);
        return -1;
    }

    // Browse the dictionnary
    while (fgets(password, MAX_PASSWORD_LENGTH, dictionaryFile) != NULL) {
        // Supprimer le saut de ligne à la fin du mot de passe
        password[strcspn(password, "\n")] = 0;

        // Try to open the file with password
        if (zip_set_default_password(zipFile, password) == 0) {
            // Browse the zip file
            struct zip_stat fileStat;
            for (int i = 0; zip_stat_index(zipFile, i, 0, &fileStat) == 0; i++) {
                // Open the actual file
                file = zip_fopen_index(zipFile, i, 0);
                if (!file) {
                    printf("Impossible d'ouvrir le fichier dans le ZIP.\n");
                    continue;
                }

                // Extract the content of the file
                const char* fileName = strrchr(fileStat.name, '/');
                if (fileName == NULL) {
                    fileName = fileStat.name;
                } else {
                    fileName++;
                }

                FILE* outputFile = fopen(fileName, "wb");
                if (!outputFile) {
                    //printf("Impossible de créer le fichier de sortie.\n");
                    zip_fclose(file);
                    continue;
                }

                char buffer[1024];
                int bytesRead;
                while ((bytesRead = zip_fread(file, buffer, sizeof(buffer))) > 0) {
                    fwrite(buffer, 1, bytesRead, outputFile);
                }

                fclose(outputFile);
                zip_fclose(file);
            }

            // If password found, stop
            break;
        }
    }

    // Close dic n zip
    fclose(dictionaryFile);
    zip_close(zipFile);

    return 0;
}