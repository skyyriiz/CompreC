#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zip.h>

#include "manage.h"
#include "crack.h"
#include "interface.h"



int main(int argc, char *argv[]){
    int opt;
    while ((opt = getopt(argc, argv, "ho:bdpeic:")) != -1){
        switch (opt) {
            case 'h':
                printFile("help.txt");
                break;
            case 'o':
                //printf("o");
                printf("%s", optarg);
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
            case 'i':
                printf("i");
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