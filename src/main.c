#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>


void print_status(const char* message) {
    printf("[NEX]: %s\n", message);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_status("ERROR: NO INPUT FILE SPECIFIED");
        return 1;
    }

    Parser* parser = parser_init((char*)argv[1]);

    parser_parse(parser);

    SAO(parser->root);

    GEN(parser->root);

    char nasm_cmd[100];
    sprintf(nasm_cmd, "nasm -f elf64 %s.asm -o %s.o", "prog", "prog");
    if (system(nasm_cmd) != 0) {
        parser_free(parser);
        return 1;
    }

    char ld_cmd[100];
    sprintf(ld_cmd, "ld %s.o -o %s", "prog", "prog");
    if (system(ld_cmd) != 0) {
        parser_free(parser);
        return 1;
    }

    remove("prog.o");

    parser_free(parser);

    print_status("PROGRAM GENERATED SUCCESSFULLY");

    return 0;
}
