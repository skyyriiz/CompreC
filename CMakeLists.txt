cmake_minimum_required(VERSION 3.25)
project(CompreC)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_PREFIX_PATH "/home/kali/CLionProjects/CompreC/libs")
include_directories(.)
add_executable(CompreC
        compile.sh
        crack.c
        crack.h
        help.txt
        install.sh
        interface.c
        interface.h
        main.c
        manage.c
        manage.h)


# chemin vers le répertoire contenant les fichiers de la bibliothèque libzip
set(LIBZIP_DIR ${/home/kali/CLionProjects/CompreC/libs/libzip-1.9.2})
# ajoute le répertoire contenant les fichiers de la bibliothèque libzip
include_directories(${LIBZIP_DIR})
# ajoute la bibliothèque libzip
find_library(LIBZIP_LIBRARY zip ${LIBZIP_DIR}/lib)
# ajoute la bibliothèque à l'exécutable
target_link_libraries(program ${LIBZIP_LIBRARY})