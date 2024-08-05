#include "gen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_status(const char* message) {
    printf("[NEX]: %s\n", message);
}

void print_usage(const char* program_name) {
    printf("Usage: %s [options] <source files>\n", program_name);
    printf("Options:\n");
    printf("  -W <option>   Enable specific warnings (e.g., all, extra)\n");
    printf("  -O <level>    Set optimization level (0-3)\n");
    printf("  -o <file>     Set output file name\n");
    printf("  -h            Show this help message\n");
}

int main(int argc, char* argv[]) {
    int enable_all_warnings = 0;
    int enable_extra_warnings = 0;
    int optimization_level = 0;
    char* output_file = NULL;
    int source_files_count = 0;
    char** source_files = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-W") == 0) {
            if (i + 1 < argc) {
                if (strcmp(argv[i + 1], "all") == 0) {
                    enable_all_warnings = 1;
                } else if (strcmp(argv[i + 1], "extra") == 0) {
                    enable_extra_warnings = 1;
                } else {
                    print_status("ERROR: INVALID WARNING OPTION");
                    return 1;
                }
                i++;
            } else {
                print_status("ERROR: MISSING ARGUMENT FOR -W");
                return 1;
            }
        } else if (strcmp(argv[i], "-O") == 0) {
            if (i + 1 < argc) {
                optimization_level = atoi(argv[++i]);
                if (optimization_level < 0 || optimization_level > 3) {
                    print_status("ERROR: INVALID OPTIMIZATION LEVEL");
                    return 1;
                }
            } else {
                print_status("ERROR: MISSING ARGUMENT FOR -O");
                return 1;
            }
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                print_status("ERROR: MISSING ARGUMENT FOR -o");
                return 1;
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            source_files_count++;
            source_files = realloc(source_files, source_files_count * sizeof(char*));
            if (source_files == NULL) {
                print_status("ERROR: MEMORY ALLOCATION FAILED");
                return 1;
            }
            source_files[source_files_count - 1] = argv[i];
        }
    }

    if (source_files_count == 0) {
        print_status("ERROR: NO INPUT FILES SPECIFIED. Use -h for more information");
        return 1;
    }

    for (int i = 0; i < source_files_count; i++) {
        Parser* parser = parser_init(source_files[i]);
        if (parser == NULL) {
            print_status("ERROR: FAILED TO INITIALIZE PARSER");
            return 1;
        }

        // parser->enable_all_warnings = enable_all_warnings;
        // parser->enable_extra_warnings = enable_extra_warnings;
        // parser->optimization_level = optimization_level;
        // parser->output_file = output_file;

        parser_parse(parser);

        SAO(parser->root);

        // GEN(parser);

        parser_free(parser);
    }

    free(source_files);

    print_status("PROGRAM GENERATED SUCCESSFULLY");

    return 0;
}
