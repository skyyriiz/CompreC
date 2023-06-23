//
// Created by oui on 21/06/23.
//

#ifndef C_MANAGE_H
#define C_MANAGE_H

void createZip(const char* path, const char* archiveName);
int createZipEntry(struct zip *zipfile, const char *entryPath, const char *entryName);
void printArchiveContent(const char* archiveName);
void printFile(char *filename);
void extractArchive(const char* archiveName);
int openZipWithPassword(const char* archiveName, const char* password);
void openZip(const char* fileToOpen);
void compressFolder(const char* folderName);
void addDirectoryToZip(struct zip *archive, const char *directoryPath, const char *parentPath);
int includeFileToZip(const char* zip_file, const char* file, const char* end_filename);

#endif //C_MANAGE_H
