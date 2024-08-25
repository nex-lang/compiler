#include "gen.h"
#include "io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION "beta.0.1"

void print_status(const char* message) {
    printf("[NEX]: %s\n", message);
}

void print_usage(const char* program_name) {
    printf("Usage: %s [options] <source files>\n", program_name);
    printf("Options:\n");
    printf("  -W<warning>          Enable specific warnings (e.g.,all,extra,unused,deprecated)\n");
    printf("  -O<level>            Set optimization level (0-3)\n");
    printf("  -o <file>            Set output file name\n");
    printf("  -h, --help           Show this help message\n");
    printf("  -v, --version        Show current version\n");
}

int main(int argc, char* argv[]) {
    Warnings warnings = {0};
    uint8_t optimization_level = 0;
    char* output_file = NULL;
    uint32_t source_files_count = 0;
    char** source_files = NULL;
    LibraryList* lib_list = malloc(sizeof(LibraryList));
    LibraryList* pclib_list = malloc(sizeof(LibraryList));

    parse_library_args(argc, argv, lib_list);
    parse_pclibrary_args(argc, argv, lib_list);

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-W", 2) == 0) {
            if (strlen(argv[i]) > 2) {
                if (strcmp(argv[i] + 2, "all") == 0) {
                    warnings.all = 1;
                } else if (strcmp(argv[i] + 2, "extra") == 0) {
                    warnings.extra = 1;
                } else if (strcmp(argv[i] + 2, "unused") == 0) {
                    warnings.unused = 1;
                } else if (strcmp(argv[i] + 2, "deprecated") == 0) {
                    warnings.deprecated = 1;
                } else {
                    print_status("ERROR: INVALID WARNING OPTION");
                    return 1;
                }
            } else {
                print_status("ERROR: MISSING WARNING OPTION AFTER -W");
                return 1;
            }
        } else if (strncmp(argv[i], "-O", 2) == 0) {
            if (strlen(argv[i]) > 2) {
                optimization_level = atoi(argv[i] + 2);
                if (optimization_level < 0 || optimization_level > 3) {
                    print_status("ERROR: INVALID OPTIMIZATION LEVEL");
                    return 1;
                }
            } else {
                print_status("ERROR: MISSING OPTIMIZATION LEVEL AFTER -O");
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
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("Running version %s\n", VERSION);
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

    Parser* parser;
    SAO* sao;

    AST_Node** ast_list = malloc(source_files_count * sizeof(AST_Node));
    SymTable** symtbl_list = malloc(source_files_count * sizeof(SymTable));
    Lexer** lexer_list = malloc(source_files_count * sizeof(Lexer));
    char** *flag_list = malloc((lib_list->count + pclib_list->count + STD_LIBS) * sizeof(char) * source_files_count);
    uint8_t* flag_sz = malloc(sizeof(uint8_t) * source_files_count);


    for (int i = 0; i < source_files_count; i++) {
        parser = parser_init(source_files[i],
                                    source_files, lib_list, pclib_list, i, source_files_count,
                                    NEX_WARNINGS, warnings,
                                    NEX_OPTIMIZATION, optimization_level,
                                    0);

        if (parser == NULL) {
            print_status("ERROR: FAILED TO INITIALIZE PARSER");
            return 1;
        }

        parser_parse(parser);
        
        ast_list[i] = parser->root;
        symtbl_list[i] = parser->tbl;
        lexer_list[i] = parser->lexer;

        PRINT_AST_NODE(parser->root, 0);
    }

    for (int i = 0; i < source_files_count; i++) {
        sao = sao_init(ast_list, lexer_list, symtbl_list, source_files, source_files_count, i);
        sao_analyze(sao, sao->roots[i], sao->tbls[i]);
        
        flag_list[i] = sao->flags;
        flag_sz[i] = sao->flag_no;

        Symbol* sym = sao->tbls[sao->cur]->symbol;   
        PRINT_SYMB_TBL(sym);

        sym = sao->tbls[sao->cur]->symbol; 

        while (sym->next != NULL) {
            if (sym->data.type == SYMBOL_UNRE) {
                REPORT_ERROR(sao->lexers[sao->cur], "U_RESOLVED_SYM");
            }

            sym = sym->next;
        }
    }

    char* flags;

    for (int i = 0; i < source_files_count; i++) {
        char *extension = strstr(source_files[i], ".nex");
    
        if (extension != NULL && strcmp(extension, ".nex") == 0) {
            *extension = '\0';
        }

        extension = strstr(source_files[i], ".nx");
        if (extension != NULL && strcmp(extension, ".nx") == 0) {
            *extension = '\0';
        }

        flags = malloc(sizeof(char) * flag_sz[i] * 14);

        for (size_t j = 0; j < flag_sz[i]; j++) {
            sprintf(flags, "-l%s", flag_list[i][j]);
        }

        GEN(parser->root, source_files[i], parser->tbl);
        if (flag_sz[i] == 0) {
            EXEC("mlinr x86 %s.inr", source_files[i]); 
            free(flags);   
        } else {
            EXEC("mlinr x86 %s.inr %s", source_files[i], flags); 
            free(flags);   
        }
    }
         
    for (size_t i = 0; i < lib_list->count; i++) {
        free(lib_list->libraries[i].name);
    }   


    parser_free(parser);
    
    free(lib_list->libraries);
    free(source_files);
    
    print_status("PROGRAM GENERATED SUCCESSFULLY");

    return 0;
}