#ifndef GEN_H
#define GEN_H

#include "sao.h"

#include "gen/stack.h"
#include "gen/arth.h"

#include <string.h>

typedef struct Generator {
    FILE* fp;

    StringLiteralManager* str_literals;
    CharLiteralManager* char_literals;

    StackVars cur_variables;
    CS_StackVars cur_csvariables;
} Generator;


Generator* gen_init(char* filename);
void gen_free(Generator* gen);

void gen_print_prep(Generator* gen, size_t size, size_t offset);
bool gen_for_char_str(Generator* gen, uint32_t iden);

// void gen_variable(Generator* gen, size_t size, uint32_t id);
void gen_stmt(AST_Node* statement, Generator* gen);
void generate_program(AST_Node* node, Generator* gen);
void generate_data(Generator* gen);

void GEN(AST_Node *root);

#endif // GEN_H
