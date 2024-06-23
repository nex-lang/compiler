#ifndef CODEGEN_H
#define CODEGEN_H

#include "sao.h"
#include "stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ASM_StringSymbol {
    char *label;
    char *value;
    struct ASM_StringSymbol *next;
} ASM_StringSymbol;

typedef struct ASM_VarSymbol {
    char *label;
    char *value;
    struct ASM_VarSymbol *next;
} ASM_VarSymbol;

unsigned long hash_string(const char *str);
unsigned long write_string_symb(ASM_StringSymbol **head, const char *str, int *counter);
unsigned long write_var_symb(ASM_VarSymbol **head, uint32_t id, const char *value, int *counter);
void generate_code_for_statement(AST_Node* statement, FILE *fp, int *counter, int* var_counter, ASM_StringSymbol **head, ASM_VarSymbol **var_head, Stack *var_stack);
void generate_code_for_ast(AST_Node *node, FILE *fp, int *counter, int* var_counter, ASM_StringSymbol **head, ASM_VarSymbol **var_head, Stack *var_stack);
void write_string_literals(FILE *fp, ASM_StringSymbol *head);
void write_var_decl(FILE *fp, ASM_VarSymbol *head);

void GEN(AST_Node *root);

#endif /* CODEGEN_H */
