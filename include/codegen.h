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

typedef struct ASM_CharSymbol {
    char* label;
    char value;
    struct ASM_CharSymbol* next;
} ASM_CharSymbol;

typedef struct StringLiteralManager {
    ASM_StringSymbol* head;
    int counter;
} StringLiteralManager;

typedef struct CharLiteralManager {
    ASM_CharSymbol* head;
    int counter;
} CharLiteralManager;

typedef struct StackVar {
    size_t offset;
    size_t size;
    uint32_t id;
} StackVar;

typedef struct StackVars {
    size_t size;
    size_t ac_size;
    StackVar** vars;
} StackVars;

typedef struct CS_StackVars {
    size_t size;
    char** vars;
    uint32_t* id;


    size_t char_size;
    char** char_vars;
    uint32_t* char_id;


} CS_StackVars;

typedef struct Generator {
    FILE* fp;

    StringLiteralManager* str_literals;
    CharLiteralManager* char_literals;

    StackVars cur_variables;
    CS_StackVars cur_csvariables;
} Generator;

typedef struct arthdata {
    char* reg;
    char* sireg;
    char* size; 
} arthdata;


Generator* gen_init(char* filename);
void gen_free(Generator* gen);

unsigned long hash_string(const char *str);

void double_to_ieee_hex(double value, char* hex_str);
void float_to_ieee_hex(float value, char* hex_str);
arthdata get_arth_regsize(size_t sz);


void gen_print_prep(Generator* gen, size_t size, size_t offset);
bool gen_for_char_str(Generator* gen, uint32_t iden);

void stackvar_push(Generator* gen, size_t offset, uint32_t id, size_t size);
void csstackvar_push(Generator* gen, char* str, bool is_char, uint32_t iden);
void handle_literal_agn(Generator* gen, ASTN_VariableDecl decl);

unsigned long gen_str_symb(ASM_StringSymbol** head, const char* str, int* counter, bool is_def, uint32_t dest_hash);
unsigned long gen_char_symb(ASM_CharSymbol** head, char ch, int* counter, bool is_def, uint32_t dest_hash);

void gen_string_lits(FILE* fp, ASM_StringSymbol* head);
void gen_char_lits(FILE* fp, ASM_CharSymbol* head);

void gen_variable(Generator* gen, size_t size, uint32_t id);

void gen_stmt(AST_Node* statement, Generator* gen);
void gen_assgn(AST_Node* stm, Generator* gen);
void generate_program(AST_Node* node, Generator* gen);
void generate_data(Generator* gen);

void GEN(AST_Node *root);

#endif /* CODEGEN_H */
