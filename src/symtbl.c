#include "symtbl.h"

#include <stdio.h>
#include <string.h>

SymTable* symtbl_init() {
    SymTable* table = calloc(1, sizeof(SymTable));

    table->symbol = NULL;

    return table;
}

void symtbl_free(SymTable* table) {
    if (!table) {
        return;
    }

    Symbol* current = table->symbol;
    Symbol* next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    free(table);
}

Symbol* symbol_init(char* id, unsigned int type, unsigned int scope, unsigned int nest, uint8_t mem_type, 
    uint8_t mem_mod, uint8_t mem_sto, uint8_t  access_type, uint8_t decl_line, uint8_t decl_col) {
    Symbol* symb = calloc(1, sizeof(Symbol));

    symb->data.id = symtbl_hash((const char*)id, scope);
    symb->data.scope = scope;
    symb->data.nest = nest;
    symb->data.type = type;

    symb->data.mem_type = mem_type;
    symb->data.mem_mod = mem_mod;
    symb->data.mem_sto = mem_sto;
    symb->data.access_type = access_type;

    symb->data.life.borrower_size = 0;
    symb->data.life.borrower_list = malloc(symb->data.life.borrower_size * sizeof(Symbol*));

    symb->data.decl_line = decl_line;
    symb->data.decl_col = decl_col;

    symb->next = NULL;

    return symb;
}


Symbol* symtbl_lookup(SymTable* table, char* id, unsigned int scope, uint8_t scope_offset) {
    Symbol* current = table->symbol;

    while (current != NULL) {
        if (current->data.id == symtbl_hash((const char*)id, scope)) {
            return current;
        }
        current = current->next;
    }

    if (scope_offset > 0 && scope > 0) {
        Symbol* peek = symtbl_lookup(table, id, scope - 1, scope_offset);
        if (peek != NULL && peek->data.nest >= 2) {
            return peek;
        }
    }

    return NULL;
}



int32_t symtbl_hash(const char* key, unsigned int scope) {
    int32_t hash_val = 5381;

    while (*key) {
        hash_val = ((hash_val << 5) + hash_val) + *key++;
    }

    hash_val = ((hash_val << 5) + hash_val) + scope;

    return hash_val;
}


void symtbl_borrowsym(SymTable* table, Symbol* symbol, Symbol* borrower) {
    if (!table || !symbol) {
        return;
    }

    table->symbol->data.life.borrower_list = realloc(table->symbol->data.life.borrower_list, (table->symbol->data.life.borrower_size + 1) * sizeof(Symbol*));

    table->symbol->data.life.borrower_list[table->symbol->data.life.borrower_size] = borrower;

    table->symbol->data.life.borrower_size += 1;
}