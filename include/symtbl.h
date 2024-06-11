#ifndef SYMTBL_H
#define SYMTBL_H

#include "token.h"
#include "ast.h"

#include <inttypes.h>
#include <stdlib.h>

typedef struct Symbol {
    struct {
        int32_t id; 
        unsigned int scope, nest;
        uint8_t mem_type, mem_mod, mem_sto, access_type;
        
        enum SymbolType {
            SYMBOL_VARIABLE,
            SYMBOL_FUNCTION,
            SYMBOL_STRUCT,
            SYMBOL_CLASS,
            SYMBOL_ENUM,
            SYMBOL_MEP,
            SYMBOL_MODULE,
            SYMBOL_ATTR,
            SYMBOL_ERR
        } type;
        
        AST_Node* data;

        struct {
            struct Symbol** borrower_list;
            size_t borrower_size;
        } life; // borrow logs

        int decl_line;
        int decl_col;
    } data;
    struct Symbol* next;
} Symbol;

typedef struct SymTable {
    Symbol* symbol;
} SymTable;


SymTable* symtbl_init();
void symtbl_free(SymTable* table);
Symbol* symbol_init(char* id, unsigned int type, unsigned int scope, unsigned int nest, uint8_t mem_type, 
    uint8_t mem_mod, uint8_t mem_sto, uint8_t  access_type, uint8_t decl_line, uint8_t decl_col);

Symbol* symtbl_lookup(SymTable* table, char* id,  unsigned int scope, uint8_t scope_offset);

int32_t symtbl_hash(const char* key, unsigned int scope);
void symtbl_borrowsym(SymTable* table, Symbol* symbol, Symbol* borrower);

#endif // SYMTBL_H