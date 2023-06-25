#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zip.h>
#include <getopt.h>
#include <libgen.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "manage.h"
#include "crack.h"
#include "interface.h"

static int extraireFichier(int id, struct zip *f_zip, const char *repSortie, const char* motDePasse){
    struct zip_stat file_stat;
    struct zip_file *file_zip=NULL;
    zip_stat_index(f_zip, id, 0, &file_stat);

    /* 4. pour connaître la taille du fichier et ainsi pouvoir le lire en entier*/
    if(zip_stat(f_zip, file_stat.name, 0, &file_stat) == -1)
    {
        printf("%s\n", zip_strerror(f_zip));
        return -1;
    }

    printf("taille du fichier %s : %d \n", file_stat.name, (int)(file_stat.size*sizeof(char)+1));


    /* 5. on ouvre le fichier archivé */
    if(motDePasse==NULL)
    {
        /* sans mot de passe */
        file_zip=zip_fopen(f_zip, file_stat.name, ZIP_FL_UNCHANGED);
    }
    else
    {
        /* avec mot de passe */
        file_zip=zip_fopen_encrypted(f_zip, file_stat.name, ZIP_FL_UNCHANGED, motDePasse);
    }
    if(!file_zip)
    {
        printf("%s\n", zip_strerror(f_zip));
        return -1;
    }

    char *str=NULL;
    str = malloc((size_t)(file_stat.size+1));
    memset(str, 0, (size_t)(file_stat.size+1));
    if(str == NULL)
    {
        printf("Erreur d'allocation mémoire\n");
        return -1;
    }


    /* 6. on lit le fichier archivé */
    if(zip_fread(file_zip, str, (size_t)(file_stat.size)) != file_stat.size)
    {
        printf("%s\n", zip_strerror(f_zip));
        free(str);

        zip_fclose(file_zip);
        file_zip = NULL;
        return -1;
    }


    /* 7. on l'écrit en sortie en mode binaire */
    FILE *fOut = NULL;

    if(repSortie != NULL) /* si on a indiqué un répertoire de sortie */
    {


        char *repOut= malloc(FILENAME_MAX * sizeof(char));
        if(repOut == NULL)
        {
            printf("Erreur d'allocation mémoire\n");
            free(str);

            zip_fclose(file_zip);
            file_zip = NULL;
            return -1;
        }

        /* on test les '/' pour ne pas avoir affaire avec des "foo//bar/fichier" ou autres */
        memset(repOut, 0, FILENAME_MAX * sizeof(char));
        if( (repSortie[strlen(repSortie)-1]=='/') && (file_stat.name[0]=='/'))
        {
            strncpy(repOut, repSortie, strlen(repSortie)-2);
            strcat(repOut, file_stat.name);
        }
        else if( (repSortie[strlen(repSortie)-1]=='/') || (file_stat.name[0]=='/'))
            sprintf(repOut, "%s%s", repSortie, file_stat.name);
        else
            sprintf(repOut, "%s/%s", repSortie, file_stat.name);



        /* Avant d'extraire le fichier il faut créer les répertoires nécessaires le cas échéant */
        //printf("repOut: %s\n", strcat(strcat(dirname(repOut), "/"), basename(repOut)));
        printf("repOut: %s\n", repOut);
        //printf("repOut: %s\n", dirname(repOut) basename(repOut));
        mkdir(repOut, 0777);

        /* si c'est autre chose qu'un dossier on l'écrit */
        if(repOut[strlen(repOut)-1]!='/')
        {
            if((fOut = fopen(repOut, "wb"))==NULL)
            {
                printf("Erreur à la création du fichier %s\n", repOut);
                free(str);

                free(repOut);

                zip_fclose(file_zip);
                file_zip = NULL;
                return -1;
            }

            /* on écrit le fichier */
            fwrite(str, sizeof(char), (size_t)file_stat.size, fOut);
            fclose(fOut);
            fOut = NULL;
        }

        zip_fclose(file_zip);
        file_zip = NULL;
        free(str);
        free(repOut);



    }
    else /* sinon on utilise le répertoire d'où est lancée la commande */
    {


        /* Avant d'extraire le fichier il faut créer les répertoires nécessaires le cas échéant */
        printf("file_stat.name: %s\n", file_stat.name);
        mkdir(file_stat.name, 0777);



        /* si c'est autre chose qu'un dossier on l'écrit */
        if(file_stat.name[strlen(file_stat.name)-1] != '/' )
        {
            rmdir(file_stat.name);
            if((fOut = fopen(file_stat.name, "wb"))==NULL)
            {
                printf("Erreur à la création du fichier %s\n", file_stat.name);

                zip_fclose(file_zip);
                file_zip = NULL;
                free(str);

                return -1;

            }
            /* on écrit le fichier */
            fwrite(str, sizeof(char), (size_t)file_stat.size, fOut);

            free(str);
            fclose(fOut);
            fOut = NULL;
            zip_fclose(file_zip);
            file_zip = NULL;

        }
    }

    //free(str);
    file_zip = NULL;
    return 0;
}

static int decompresserFichierZip(const char* fichierZip, const char* repSortie, const char* motDePasse){
    int err=0;
    struct zip *f_zip=NULL;

    char buf_erreur[128];
    /* 1. Ouverture de l'archive */
    f_zip = zip_open(fichierZip, ZIP_CHECKCONS, &err);
    /* s'il y a des erreurs */
    if(err != ZIP_ER_OK)
    {
        zip_error_to_str(buf_erreur, sizeof buf_erreur, err, errno);
        printf("Error %d : %s\n",err, buf_erreur);
        return -1;
    }
    /* si le fichier zip n'est pas ouvert */
    if(f_zip==NULL)
    {
        printf("Erreur à l'ouverture du fichier %s\n", fichierZip);
        return -1;
    }


    /* 2. on récupère le nombre de fichiers dans l'archive zip */
    int count = zip_get_num_files(f_zip);
    if(count==-1)
    {
        printf("Erreur à l'ouverture du fichier %s\n", fichierZip);
        zip_close(f_zip);
        f_zip = NULL;
        return -1;
    }

    printf("Nombre de fichiers dans l'archive : %d\n", count);

    int i;

    /* 3. on lit tous les fichiers */
    for(i=0; i<count; i++)
    {
        /* on lance la fonction qui extraira le fichier en position "i" dans l'archive Zip */
        if(extraireFichier(i, f_zip, repSortie, motDePasse)==-1)
        {
            printf("Erreur à l'extraction du fichier %s\n", zip_get_name(f_zip, i, ZIP_FL_UNCHANGED));
            zip_close(f_zip);
            f_zip = NULL;
            return -1;
        }
    }

    /* lecture terminée, fermeture de l'archive */
    zip_close(f_zip);
    f_zip = NULL;


    return 0;
}




int main(int argc, char *argv[]) {
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
    int long_index=0;
    char file_name[128];
    char fileToZip[256] = "";
    char password[256];


    while ((opt = getopt_long(argc, argv, "hf:b:d:p:e:o:i:c:", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'h':
                printFile("help.txt");
                break;
            case 'o':
                /*if (optarg != NULL) {
                    char* archiveName = optarg;
                    printArchiveContent(archiveName);
                } else {
                    fprintf(stderr, "Veuillez spécifier un nom d'archive.\n");
                }*/
                break;
            case 'b':
                printf("b");
                break;
            case 'd':
                printf("d");
                break;
            case 'p':
                strcpy(password, optarg);
                break;
            case 'e':
                //extractArchive(optarg, password);
                decompresserFichierZip(optarg, "a", "oui");
                break;
            case 'i':
                includeFileToZip(optarg, file_name, basename(file_name));
                break;
            case 'c':
                createZip(optarg, file_name);
                break;
            case 'f':
                strcpy(file_name, optarg);
                break;
            default:
                printf("h\n");
                return 0;
        }
    }
        return 0;
    }


