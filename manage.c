#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zip.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <archive.h>
#include <archive_entry.h>

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
    // Ajoute l'extension .zip si elle n'est pas déjà présente
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

    // Vérifie si le chemin donné est un fichier ou un dossier
    int isDirectory = 0;
    if (access(path, F_OK) == 0) {
        // Le chemin existe
        if (access(path, R_OK) == 0) {
            // Le chemin est accessible en lecture
            if (access(path, X_OK) == 0) {
                // Le chemin est exécutable, donc probablement un dossier
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