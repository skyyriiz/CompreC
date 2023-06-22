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



int main(int argc, char *argv[]){
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
            {"create",     required_argument, 0,  'c' }

    };
    int long_index =0;
    char file_name[64];
    while ((opt = getopt_long(argc, argv, "hf:b:d:p:e:o:i:c:", long_options, &long_index)) != -1){
        switch (opt) {
            case 'h':
                printFile("help.txt");
                break;
            case 'f':
                strcpy(file_name, optarg);
                break;
            case 'b':
                printf("b");
                break;
            case 'd':
                printf("d");
                break;
            case 'p':
                printf("p");
                break;
            case 'e':
                printf("e");
                break;
            case 'o':
                printf("%s", optarg);
                break;
            case 'i':
                includeFileToZip(file_name, optarg);
                break;
            case 'c':
                createZip(optarg);
                break;
            default:
                printf("h");
                return 0;
        }
    }
    return 0;
}