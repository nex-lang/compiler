#ifndef IO_H
#define IO_H

#include <stdlib.h>

typedef struct Library {
    char* name;
    float version;
} Library;

typedef struct LibraryList {
    Library* libraries;
    size_t count;
} LibraryList;

typedef struct LocalFiles {
    char** src;
    size_t count;
} LocalFiles;

#define WO(fp, indent, format, ...) \
    do { \
        for (int i = 0; i < (indent); i++) fprintf((fp), "\t"); \
        fprintf((fp), (format), ##__VA_ARGS__); \
    } while (0)

#define EXEC(cmd, ...) \
    do { \
        char command[256]; \
        snprintf(command, sizeof(command), (cmd), ##__VA_ARGS__); \
        printf("Executing: %s\n", command); \
        int ret = system(command); \
        if (ret != 0) { \
            fprintf(stderr, "Error executing command: %s\n", command); \
        } \
    } while (0)

char* io_load_file(char* filename);
void parse_library_args(int argc, char* argv[], LibraryList* lib_list);
void parse_pclibrary_args(int argc, char* argv[], LibraryList* lib_list);

#endif // IO_H