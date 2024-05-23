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

    symb->data.id = symtbl_hash((const char*)id);
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

void symtbl_insert(SymTable* table, Symbol* symbol) {
    if (!table) {
        exit(EXIT_FAILURE);
        return;
    }

    if (table->symbol == NULL) {
        table->symbol = symbol;
        return;
    }

    Symbol* current = table->symbol;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = symbol;
}

Symbol* symtbl_lookup(SymTable* table, char* id, uint8_t scope_offset) {
    Symbol* current = table->symbol;

    while (current != NULL) {
        if (current->data.id == symtbl_hash((const char*)id)) {
            return current;
        }
        current = current->next;
    }

    if (current != NULL && current->data.nest >= 2) {
        unsigned int k = (scope_offset > 0) ? scope_offset : current->data.nest;
        Symbol* peek = symtbl_lookup(table, id, k - 1);

        while (k >= 2 && peek != NULL) {
            return peek;
        }
    }

    return NULL;
}


uint32_t symtbl_hash(const char* key) {
    uint32_t hash_val = 5381;

    while (*key) {
        hash_val = ((hash_val << 5) + hash_val) + *key++;
    }

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