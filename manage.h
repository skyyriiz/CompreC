//
// Created by oui on 21/06/23.
//

#ifndef C_MANAGE_H
#define C_MANAGE_H

int createZip(char *path);
void printFile(char *filename);
int includeFileToZip(const char* zip_file, const char* file, const char* end_filename);

#endif //C_MANAGE_H
