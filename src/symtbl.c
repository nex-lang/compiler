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
    uint8_t mem_mod, uint8_t mem_sto, uint8_t  access_type, uint8_t decl_line, uint8_t decl_col, size_t ty_size) {
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
    symb->data.life.borrower_list = malloc(1 * sizeof(Symbol*));

    symb->data.decl_line = decl_line;
    symb->data.decl_col = decl_col;

    symb->data.ty_size = ty_size;

    symb->next = NULL;


    return symb;
}


Symbol* symtbl_lookup(SymTable* table, char* id, uint64_t scope, uint8_t scope_offset, uint64_t recent_root) { 
    uint32_t hash_id = symtbl_hash((const char*)id, scope);
    Symbol* current = table->symbol;

    while (current != NULL) {
        if (current->data.id == hash_id) {
            return current;
        }
        current = current->next;
    }

    for (unsigned int i = 1; i <= scope_offset && scope >= i; ++i) {
        current = table->symbol;
        hash_id = symtbl_hash((const char*)id, scope - i);
        while (current != NULL) {
            if (current->data.id == hash_id) {
                return current;
            }
            current = current->next;
        }
    }

    current = table->symbol;
    hash_id = symtbl_hash((const char*)id, recent_root);
    while (current != NULL) {
        if (current->data.id == hash_id) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

Symbol* symtbl_slookup(SymTable* table, int id) {
    Symbol* current = table->symbol;

    while (current != NULL) {
        if (current->data.id == id) {
            return current;
        }
        current = current->next;
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