#ifndef GEN_STACK_H
#define GEN_STACK_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


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

unsigned long hash_string(const char *str);

unsigned long gen_str_symb(ASM_StringSymbol** head, const char* str, int* counter, bool is_def, uint32_t dest_hash);
unsigned long gen_char_symb(ASM_CharSymbol** head, char ch, int* counter, bool is_def, uint32_t dest_hash);

void gen_string_lits(FILE* fp, ASM_StringSymbol* head);
void gen_char_lits(FILE* fp, ASM_CharSymbol* head);

void stackvar_push(StackVars* cur_variables, size_t offset, uint32_t id, size_t size);
void csstackvar_push(CS_StackVars* cur_csvariables, char* str, bool is_char, uint32_t iden);


#endif // GEN_STACK_H