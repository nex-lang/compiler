#ifndef SAO_H
#define SAO_H

#include "parser.h"

typedef struct SAO {
    AST_Node** roots;
    Lexer** lexers;
    SymTable** tbls;
    char** files;
    char** flags;
    size_t flag_no;

    uint32_t count;
    uint32_t cur;
} SAO;

#define STD_LIBS 2

SAO* sao_init(AST_Node** roots, Lexer** lexers, SymTable** tables, char** files, uint32_t count, uint32_t cur);

void sao_analyze(SAO* sao, AST_Node* node, SymTable* tbl);

bool sao_stms(SAO* sao, AST_Node* node, SymTable* tbl);
bool sao_call(SAO* sao, ASTN_Statement stm, SymTable* tbl);
bool sao_import(SAO* sao, AST_Node* node, SymTable* tbl);

bool param_check();
bool resolve_sym(SymTable* src, SymTable* dest, ASTN_Modules mods);
bool resolve_std_sym(SAO* sao, char* sub, ASTN_Modules mods);
void fi_resolve_sym(SymTable* src, SymTable* dest);
int get_source_file_index(char** source_files, uint32_t source_files_count, char* filename);

SymTable* std_io();
SymTable* std_math();

void symtbl_rinsert(SymTable* tbl, Symbol* symbol, char* raw_symb);

#endif // SAO_H 
// build/nex examples/ret0.nex examples/vec.nex