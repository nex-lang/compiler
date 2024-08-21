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
        size_t ty_size;
        
        enum SymbolType {
            SYMBOL_VARIABLE,
            SYMBOL_FUNCTION,
            SYMBOL_STRUCT,
            SYMBOL_CLASS,
            SYMBOL_ENUM,
            SYMBOL_MEP,
            SYMBOL_MODULE,
            SYMBOL_ATTR,
            SYMBOL_ERR,
            SYMBOL_UNRE
        } type;
        
        union {
            ASTN_VariableDecl var;
            ASTN_FunctionDecl fn;
            ASTN_ClassDecl clas;
            ASTN_StructDecl stru;
            ASTN_EnumDecl enu;
            ASTN_ErrDecl err;
            ASTN_AttributeDecl attr;
            ASTN_Module mod;
            ASTN_DataTypeSpecifier param;
        } data;

        struct {
            struct Symbol** borrower_list;
            size_t borrower_size;
        } life;

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
Symbol* symbol_init(char* id, unsigned int type, unsigned int scope, unsigned int nest, uint8_t decl_line, uint8_t decl_col, size_t ty_size);
Symbol* symtbl_lookup(SymTable* table, char* id, uint64_t scope, uint8_t scope_offset, uint64_t recent_root);
Symbol* symtbl_slookup(SymTable* table, int id);

int32_t symtbl_hash(const char* key, unsigned int scope);
void symtbl_borrowsym(SymTable* table, Symbol* symbol, Symbol* borrower);

#endif // SYMTBL_H
