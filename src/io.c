#include "io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* io_load_file(char* filename) {
    /*
    reads provided filename (from the directory where executable was ran / command was called)
    return: processed buffer: char* including all contents of provided filename
    */
    
    char* buffer = 0;
    long long length;
    FILE* f = fopen(filename, "rb");
    
    if (!f) {
        exit(EXIT_FAILURE);
        return NULL; 
    }

    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);

    buffer = malloc(sizeof(char) * (length + 1));

    if (!buffer) {
        fclose(f);
        exit(EXIT_FAILURE);
        return NULL;
    }
    
    size_t len =  fread(buffer, 1, length, f);
    fclose(f);

    buffer[len] = '\0';

    return buffer;
}

void parse_library_args(int argc, char* argv[], LibraryList* lib_list) {
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-lib", 4) == 0) {
            if (strlen(argv[i]) > 5) {
                char* libs = argv[i] + 4;
                char* token = strtok(libs, " ");
                while (token != NULL) {
                    lib_list->libraries = realloc(lib_list->libraries, (lib_list->count + 1) * sizeof(Library));
                    if (lib_list->libraries == NULL) {
                        print_status("ERROR: MEMORY ALLOCATION FAILED");
                        exit(1);
                    }
                    char* equal_sign = strchr(token, '=');
                    if (equal_sign != NULL) {
                        *equal_sign = '\0';
                        char* version_str = equal_sign + 1;
                        lib_list->libraries[lib_list->count].name = strdup(libs);
                        lib_list->libraries[lib_list->count].version = strtof(version_str, NULL);
                        lib_list->count++;
                    }
                    token = strtok(NULL, " ");
                }
            } else {
                print_status("ERROR: MISSING LIBRARY ARGUMENT AFTER -lib");
                exit(1);
            }
        }
    }
}


void parse_pclibrary_args(int argc, char* argv[], LibraryList* lib_list) {
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-plib", 4) == 0) {
            if (strlen(argv[i]) > 5) {
                char* libs = argv[i] + 4;
                char* token = strtok(libs, " ");
                while (token != NULL) {
                    lib_list->libraries = realloc(lib_list->libraries, (lib_list->count + 1) * sizeof(Library));
                    if (lib_list->libraries == NULL) {
                        print_status("ERROR: MEMORY ALLOCATION FAILED");
                        exit(1);
                    }
                    char* equal_sign = strchr(token, '=');
                    if (equal_sign != NULL) {
                        *equal_sign = '\0';
                        char* version_str = equal_sign + 1;
                        lib_list->libraries[lib_list->count].name = strdup(libs);
                        lib_list->libraries[lib_list->count].version = strtof(version_str, NULL);
                        lib_list->count++;
                    }
                    token = strtok(NULL, " ");
                }
            } else {
                print_status("ERROR: MISSING LIBRARY ARGUMENT AFTER -plib");
                exit(1);
            }
        }
    }
}

