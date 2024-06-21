#ifndef CODEGEN_H
#define CODEGEN_H

#include "sao.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ASM_StringSymbol {
    char *label;
    char *value;
    struct ASM_StringSymbol *next;
} ASM_StringSymbol;

unsigned long hash_string(const char *str);
unsigned long write_string_symb(ASM_StringSymbol **head, const char *str, int *counter);
void generate_code_for_statement(AST_Node* statement, FILE *fp, int *counter, ASM_StringSymbol **head);
void generate_code_for_ast(AST_Node *node, FILE *fp, int *counter, ASM_StringSymbol **head);
void write_string_literals(FILE *fp, ASM_StringSymbol *head);

void GEN(AST_Node *root);

#endif /* CODEGEN_H */
