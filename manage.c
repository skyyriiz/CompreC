#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zip.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

#include "manage.h"

void printFile(char *filename) {
    FILE *fptr;
    char c;
    fptr = fopen(filename, "r");
    if (fptr == NULL) {
        printf("Cannot open file \n");
        exit(0);
    }
    c = fgetc(fptr);
    while (c != EOF) {
        printf("%c", c);
        c = fgetc(fptr);
    }

    fclose(fptr);

}


void createZip(const char* path, const char* archiveName) {
    // ajoute l'extension .zip si elle n'est pas déjà présente
    char zipExtension[5] = ".zip";
    char fullArchiveName[strlen(archiveName) + strlen(zipExtension) + 1];
    strcpy(fullArchiveName, archiveName);
    if (strcmp(fullArchiveName + strlen(fullArchiveName) - strlen(zipExtension), zipExtension) != 0) {
        strcat(fullArchiveName, zipExtension);
    }

    struct zip *zipfile;
    int err = 0;

    // Ouvre le fichier zip en mode création
    if ((zipfile = zip_open(fullArchiveName, ZIP_CREATE | ZIP_EXCL, &err)) == NULL) {
        zip_error_t error;
        zip_error_init_with_code(&error, err);
        fprintf(stderr, "Erreur lors de la création de l'archive : %s\n", zip_error_strerror(&error));
        zip_error_fini(&error);
        return;
    }

    // vérifie si le chemin donné est un fichier ou un dossier
    int isDirectory = 0;
    if (access(path, F_OK) == 0) {
        // Le chemin existe
        if (access(path, R_OK) == 0) {
            // Le chemin est accessible en lecture
            if (access(path, X_OK) == 0) {
                // Le chemin est exécutable =  probablement un dossier
                isDirectory = 1;
            }
        } else {
            fprintf(stderr, "Impossible d'accéder au fichier ou au dossier : %s\n", path);
            zip_close(zipfile);
            return;
        }
    } else {
        fprintf(stderr, "Le fichier ou le dossier n'existe pas : %s\n", path);
        zip_close(zipfile);
        return;
    }

    // Compression d'un fichier
    if (!isDirectory) {
        zip_source_t *source = zip_source_file(zipfile, path, 0, -1);
        if (source == NULL) {
            fprintf(stderr, "Erreur lors de l'ajout du fichier à l'archive : %s\n", path);
            zip_close(zipfile);
            return;
        }

        const char *filename = strrchr(path, '/');
        if (filename == NULL) {
            filename = strrchr(path, '\\');
        }
        if (filename == NULL) {
            filename = path;
        } else {
            filename++;  // Ignore le séparateur de dossier
        }

        if (zip_add(zipfile, filename, source) < 0) {
            fprintf(stderr, "Erreur lors de l'ajout du fichier à l'archive : %s\n", path);
            zip_source_free(source);
            zip_close(zipfile);
            return;
        }
    }

        // Compression d'un dossier
    else {
        DIR *dir = opendir(path);
        if (dir == NULL) {
            fprintf(stderr, "Erreur lors de l'ouverture du dossier : %s\n", path);
            zip_close(zipfile);
            return;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char entryPath[PATH_MAX];
                snprintf(entryPath, sizeof(entryPath), "%s/%s", path, entry->d_name);

                if (!createZipEntry(zipfile, entryPath, entry->d_name)) {
                    fprintf(stderr, "Erreur lors de l'ajout de l'entrée au dossier : %s\n", entryPath);
                    zip_close(zipfile);
                    closedir(dir);
                    return;
                }
            }
        }

        closedir(dir);
    }

    // Ferme l'archive
    if (zip_close(zipfile) < 0) {
        fprintf(stderr, "Erreur lors de la fermeture de l'archive : %s\n", zip_strerror(zipfile));
    } else {
        printf("Archive créée avec succès.\n");
    }
}

int createZipEntry(struct zip *zipfile, const char *entryPath, const char *entryName) {
    struct zip_source *source = zip_source_file(zipfile, entryPath, 0, -1);
    if (source == NULL) {
        return 0;
    }

    if (zip_add(zipfile, entryName, source) < 0) {
        zip_source_free(source);
        return 0;
    }

    return 1;
}

void extractArchive(const char* archiveName, const char *password) {
    struct zip *zipfile;
    zipfile = zip_open(archiveName, 0, NULL);
    if (zipfile == NULL) {
        fprintf(stderr, "Impossible d'ouvrir l'archive '%s'\n", archiveName);
        return;
    }

    int numFiles = zip_get_num_entries(zipfile, 0);
    if (numFiles == -1) {
        fprintf(stderr, "Erreur lors de la récupération du nombre de fichiers dans l'archive\n");
        zip_close(zipfile);

    }

    for (int i = 0; i < numFiles; i++) {
        struct zip_stat fileStat;
        if (zip_stat_index(zipfile, i, 0, &fileStat) == -1) {
            fprintf(stderr, "Impossible de récupérer les informations sur le fichier %d dans l'archive\n", i);
            continue;
        }

        printf("Extraction du fichier %d : %s\n", i, fileStat.name);

        zip_file_t *file;
        if(strcmp(password, "")){
            file = zip_fopen_index_encrypted(zipfile, i, 0, password);
        }else{
            printf("Aucun mot de passe renseigné\n");
            file = zip_fopen_index(zipfile, i, 0);
        }

        if (file == NULL) {
            fprintf(stderr, "Erreur lors de l'ouverture du fichier %d dans l'archive\n", i);
            continue;
        }

        char buffer[1024];
        ssize_t bytesRead;
        FILE *outputFile = fopen(fileStat.name, "wb");
        if (outputFile == NULL) {
            fprintf(stderr, "Erreur lors de la création du fichier %s\n", fileStat.name);
            zip_fclose(file);
            continue;
        }

        while ((bytesRead = zip_fread(file, buffer, sizeof(buffer))) > 0) {
            fwrite(buffer, 1, bytesRead, outputFile);
        }

        fclose(outputFile);
        zip_fclose(file);
    }

    zip_close(zipfile);
}

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

/*
void printArchiveContent(const char* archiveName) {
    struct zip *zipfile;
    int err = 0;

    // Ouvre le fichier zip en mode lecture
    if ((zipfile = zip_open(archiveName, ZIP_RDONLY, &err)) == NULL) {
        zip_error_t error;
        zip_error_init_with_code(&error, err);
        fprintf(stderr, "Erreur lors de l'ouverture de l'archive : %s\n", zip_error_strerror(&error));
        zip_error_fini(&error);
        return;
    }

    // Parcours tous les fichiers/dossiers de l'archive
    int numEntries = zip_get_num_entries(zipfile, 0);
    printf("Contenu de l'archive '%s':\n", archiveName);
    for (int i = 0; i < numEntries; i++) {
        const char* entryName = zip_get_name(zipfile, i, 0);
        printf("%d - %s\n", i+1, entryName);
    }

    // Ferme l'archive
    if (zip_close(zipfile) < 0) {
        fprintf(stderr, "Erreur lors de la fermeture de l'archive : %s\n", zip_strerror(zipfile));
    }
}
*/
