//
// Source with all functions used to manage zip archives
//

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
    // Print the file's content
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
    // Add .zip extension if not already
    char zipExtension[5] = ".zip";
    char fullArchiveName[strlen(archiveName) + strlen(zipExtension) + 1];
    strcpy(fullArchiveName, archiveName);
    if (strcmp(fullArchiveName + strlen(fullArchiveName) - strlen(zipExtension), zipExtension) != 0) {
        strcat(fullArchiveName, zipExtension);
    }

    struct zip *zipfile;
    int err = 0;

    // Open zip archive with creation flag
    if ((zipfile = zip_open(fullArchiveName, ZIP_CREATE | ZIP_EXCL, &err)) == NULL) {
        zip_error_t error;
        zip_error_init_with_code(&error, err);
        fprintf(stderr, "Error creating archive: %s\n", zip_error_strerror(&error));
        zip_error_fini(&error);
        return;
    }

    // Check if path given is a file or folder
    int isDirectory = 0;
    if (access(path, F_OK) == 0) {
        // Path exists
        if (access(path, R_OK) == 0) {
            // Path is read accessible
            if (access(path, X_OK) == 0) {
                // Path is executable, and so is a folder
                isDirectory = 1;
            }
        } else {
            fprintf(stderr, "Unable to access file or folder: %s\n", path);
            zip_close(zipfile);
            return;
        }
    } else {
        fprintf(stderr, "File or folder does not exist: %s\n", path);
        zip_close(zipfile);
        return;
    }

    // File compression
    if (!isDirectory) {
        zip_source_t *source = zip_source_file(zipfile, path, 0, -1);
        if (source == NULL) {
            fprintf(stderr, "Error adding file to archive: %s\n", path);
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
            filename++;  // Ignore folder separator
        }

        if (zip_add(zipfile, filename, source) < 0) {
            fprintf(stderr, "Error adding file to archive: %s\n", path);
            zip_source_free(source);
            zip_close(zipfile);
            return;
        }
    }

    // Folder compression
    else {
        DIR *dir = opendir(path);
        if (dir == NULL) {
            fprintf(stderr, "Error opening file: %s\n", path);
            zip_close(zipfile);
            return;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char entryPath[PATH_MAX];
                snprintf(entryPath, sizeof(entryPath), "%s/%s", path, entry->d_name);

                if (!createZipEntry(zipfile, entryPath, entry->d_name)) {
                    fprintf(stderr, "Error adding entry to folder: %s\n", entryPath);
                    zip_close(zipfile);
                    closedir(dir);
                    return;
                }
            }
        }

        closedir(dir);
    }

    // Close the archive
    if (zip_close(zipfile) < 0) {
        fprintf(stderr, "Error closing archive: %s\n", zip_strerror(zipfile));
    } else {
        printf("Archive successfully created.\n");
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
    // Opening zip file
    zipfile = zip_open(archiveName, 0, NULL);
    if (zipfile == NULL) {
        fprintf(stderr, "Cannot open archive '%s'\n", archiveName);
        return;
    }

    // Retrieving number of files in archive
    int numFiles = zip_get_num_entries(zipfile, 0);
    if (numFiles == -1) {
        fprintf(stderr, "Error retrieving number of files in archive\n");
        zip_close(zipfile);

    }

    // For each file in the archive
    for (int i = 0; i < numFiles; i++) {
        struct zip_stat fileStat;
        // Retrieve file information
        if (zip_stat_index(zipfile, i, 0, &fileStat) == -1) {
            fprintf(stderr, "Unable to retrieve %d file information from archive\n", i);
            continue;
        }

        printf("Extracting file %d : %s\n", i, fileStat.name);

        zip_file_t *file;
        // Use the password if one given
        if(strcmp(password, "")){
            file = zip_fopen_index_encrypted(zipfile, i, 0, password);
        }else{
            printf("oui\n");
            file = zip_fopen_index(zipfile, i, 0);
        }

        if (file == NULL) {
            // If the file couldn't be opened
            fprintf(stderr, "Error opening %d file in archive\n", i);
            continue;
        }

        // Create the file in user's directory
        char buffer[1024];
        ssize_t bytesRead;
        FILE *outputFile = fopen(fileStat.name, "wb");
        if (outputFile == NULL) {
            fprintf(stderr, "Error creating %s file\n", fileStat.name);
            zip_fclose(file);
            continue;
        }

        // Write the whole file content
        while ((bytesRead = zip_fread(file, buffer, sizeof(buffer))) > 0) {
            fwrite(buffer, 1, bytesRead, outputFile);
        }

        fclose(outputFile);
        zip_fclose(file);
    }

    zip_close(zipfile);
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
                zip_close(archive);
                printf("Error: Failed to remove the file %ld from the archive.\n", filesToRemove[i]);
                free(filesToRemove);
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

int includeElementToZip(const char* zip_file, const char* path, const char* entry_name, const char* dest) {
    struct zip* archive = zip_open(zip_file, ZIP_CREATE, NULL);
    if (!archive) {
        return -1;
    }

    struct stat file_info;
    if (stat(path, &file_info) != 0) {
        zip_close(archive);
        return -1;
    }

    if (S_ISDIR(file_info.st_mode)) {
        // If the input is a directory, include its contents recursively
        DIR* directory = opendir(path);
        if (!directory) {
            zip_close(archive);
            return -1;
        }

        struct dirent* entry;
        while ((entry = readdir(directory)) != NULL) {
            // Skip "." and ".." directories
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            // Construct the full path of the entry
            char entry_path[PATH_MAX];
            snprintf(entry_path, sizeof(entry_path), "%s/%s", path, entry->d_name);

            // Construct the relative path within the zip
            char zip_path[PATH_MAX];
            snprintf(zip_path, sizeof(zip_path), "%s/%s", dest, entry->d_name);

            struct stat entry_info;
            if (stat(entry_path, &entry_info) != 0) {
                closedir(directory);
                zip_close(archive);
                return -1;
            }

            if (S_ISDIR(entry_info.st_mode)) {
                // If the entry is a directory, include its contents recursively
                int result = includeElementToZip(zip_file, entry_path, zip_path, dest);
                if (result != 0) {
                    closedir(directory);
                    zip_close(archive);
                    return -1;
                }
            } else {
                // If the entry is a file, include it directly in the zip
                struct zip_source* source = zip_source_file(archive, entry_path, 0, -1);
                if (!source) {
                    closedir(directory);
                    zip_close(archive);
                    return -1;
                }

                int index = zip_name_locate(archive, zip_path, 0);
                if (index >= 0) {
                    zip_delete(archive, index);
                }

                int result = zip_add(archive, zip_path, source);
                if (result != 0) {
                    closedir(directory);
                    zip_close(archive);
                    return -1;
                }
            }
        }

        closedir(directory);
    } else {
        // If the input is a file, include it directly in the zip
        struct zip_source* source = zip_source_file(archive, path, 0, -1);
        if (!source) {
            zip_close(archive);
            return -1;
        }

        int index = zip_name_locate(archive, entry_name, 0);
        if (index >= 0) {
            zip_delete(archive, index);
        }

        char zip_path[PATH_MAX];
        snprintf(zip_path, sizeof(zip_path), "%s/%s", dest, entry_name);

        int result = zip_add(archive, zip_path, source);
        if (result != 0) {
            zip_close(archive);
            return -1;
        }
    }

    zip_close(archive);
    return 0;
}
