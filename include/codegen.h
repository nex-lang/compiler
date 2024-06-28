#ifndef CODEGEN_H
#define CODEGEN_H

#include "sao.h"
#include "stack.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ASM_StringSymbol {
    char* label;
    char* value;
    struct ASM_StringSymbol* next;
} ASM_StringSymbol;

typedef struct StringLiteralManager {
    ASM_StringSymbol* head;
    int counter;
} StringLiteralManager;

typedef struct Generator {
    StringLiteralManager* str_literals;

    struct {size_t size; struct { size_t size; size_t offset; uint32_t id; }* vars; } cur_variables;
    FILE* fp;
} Generator;


Generator* gen_init(char* filename);

unsigned long hash_string(const char *str);

void handle_literal_agn(Generator* gen, ASTN_Expression* variable_decl);
unsigned long gen_str_symb(ASM_StringSymbol **head, const char *str, int *counter);
void gen_string_lits(FILE* fp, ASM_StringSymbol* head);

void gen_variable(Generator* gen, size_t size, uint32_t id);

void gen_stmt(AST_Node* statement, Generator* gen);
void generate(AST_Node* node, Generator* gen);
void GEN(AST_Node *root);

#endif /* CODEGEN_H */
