#ifndef CODEGEN_H
#define CODEGEN_H

#include "sao.h"

#include <stdio.h>
#include <stdlib.h>

void GEN(AST_Node *root);
void generate_code_for_ast(AST_Node *node, FILE *fp);
void generate_code_for_statement(AST_Node *statement, FILE *fp);

#endif /* CODEGEN_H */
