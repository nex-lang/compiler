#ifndef GEN_H
#define GEN_H

#include "sao.h"
#include "parser.h"
#include "ast.h"
#include "token.h"
#include "io.h"

#include "gen/stack.h"
#include "gen/var.h"
#include "gen/utils.h"
#include "gen/arth.h"

#include <string.h>

typedef struct Generator {
    FILE* fp;
    SymTable* tbl;
} Generator;


Generator* gen_init(char* filename);
void gen_free(Generator* gen);

// void gen_variable(Generator* gen, size_t size, uint32_t id);
void gen_stmt(AST_Node* statement, Generator* gen);
void generate_program(AST_Node* node, Generator* gen);

void GEN(AST_Node *root, char* name, SymTable* tbl);

#endif // GEN_H
