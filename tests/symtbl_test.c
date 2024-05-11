#include <criterion/criterion.h>

#include "symtbl.h"

TestSuite(symtbl);

Test(symtbl, init) {
    SymTable* table = symtbl_init();

    cr_assert_not_null(table,
        "symtbl: symtable struct shouldn't be null");
    cr_assert_null(table->symbol,
        "symtbl: symtable symbol initialized incorrectly");

    symtbl_free(table);
}

Test(symtbl, insert_symbol) {
    SymTable* table = symtbl_init();

    Symbol* symbol = symbol_init("is_alive", 1, 0, TOK_BOOL, 0, TOK_VAR, TOK_PUB, 23, 1);
    
    cr_assert_eq(symbol->data.id, -607172399,
        "symtbl: symbol id was initialized incorrectly: expected: -607172399 found: %d", symbol->data.id);
    cr_assert_eq(symbol->data.scope, 1,
        "symtbl: symbol scope was initialized incorrectly: expected: 1 found: %d", symbol->data.id);
    cr_assert_eq(symbol->data.nest, 0,
        "symtbl: symbol nesting level was initialized incorrectly: expected: 0 found: %d", symbol->data.id);
    cr_assert_eq(symbol->data.mem_type, TOK_BOOL,
        "symtbl: symbol memory type was initialized incorrectly: expected: %d found: %d", TOK_BOOL, symbol->data.mem_type);
    cr_assert_eq(symbol->data.mem_mod, 0,
        "symtbl: symbol memory modifier was initialized incorrectly: expected: 0 found: %d", symbol->data.mem_mod);
    cr_assert_eq(symbol->data.mem_sto, TOK_VAR,
        "symtbl: symbol memory storage specifier was initialized incorrectly: expected: %d found: %d", TOK_VAR, symbol->data.mem_type);
    cr_assert_eq(symbol->data.access_type, TOK_PUB,
        "symtbl: symbol memory access specifier was initialized incorrectly: expected: %d found: %d", TOK_PUB, symbol->data.access_type);
    cr_assert_eq(symbol->data.decl_line, 23,
        "symtbl: symbol line was initialized incorrectly: expected: 23 found: %d", symbol->data.decl_line);
    cr_assert_eq(symbol->data.decl_col, 1,
        "symtbl: symbol column was initialized incorrectly: expected: 1 found: %d", symbol->data.decl_col);

    Symbol* symbol2 = symbol_init("hp", 1, 0, TOK_INT, TOK_SHORT, TOK_VAR, TOK_PUB, 98, 23);

    symtbl_insert(table, symbol);
    symtbl_insert(table, symbol2);

    cr_assert_eq(table->symbol, symbol,
        "symtbl: symbol not insterted propperly into table");
    cr_assert_eq(table->symbol->next, symbol2,
        "symtbl: symbol's next symbol not insterted propperly into table");

    symtbl_free(table);
}

Test(symtbl, lookup_symbol) {
    SymTable* table = symtbl_init();

    char* iden_arr[] = {"is_alive", "hp", "ammo", "grav", "posx", "posy", "cur_sprite"};
    uint8_t scope_arr[] = {4, 4, 4, 4, 0, 4, 4};
    uint8_t nest_arr[] = {1, 2, 1, 2, 0, 2, 3};
    uint8_t type_arr[] = {TOK_BOOL, TOK_UINT, TOK_UINT, TOK_FLOAT, TOK_INT, TOK_INT, TOK_STRING};
    uint8_t mod_arr[] = {0, TOK_S_SHORT, TOK_SHORT, 0, TOK_SHORT, TOK_SHORT, 0};
    uint8_t sto_arr[] = {TOK_VAR, TOK_VAR, TOK_VAR, TOK_CONST, TOK_VAR, TOK_VAR, TOK_VAR};
    uint8_t access_arr[] = {TOK_PUB, TOK_PUB, TOK_PUB, TOK_GLOB, TOK_PUB, TOK_PUB, TOK_PUB};
    uint8_t line_arr[] = {12, 23, 34, 45, 56, 67, 78};
    
    for (int i = 0; i < 7; i++) {
        symtbl_insert(
            table, 
            symbol_init(iden_arr[i], scope_arr[i], nest_arr[i], type_arr[i],
            mod_arr[i], sto_arr[i], access_arr[i], line_arr[i], 1)
        );
    }    

    Symbol* symbol = symtbl_lookup(table, "grav", 0);

    cr_assert_not_null(symbol,
        "symtbl: symbol lookup failed");
    cr_assert_eq(symbol->data.mem_type, TOK_FLOAT,
        "symtbl: symbol lookup found unexpected type: expected: %d found: %d", TOK_FLOAT, symbol->data.mem_type);

    symtbl_free(table);
}

Test(symtbl, borrow_symbol) {
    SymTable* table = symtbl_init();

    Symbol* symbol = symbol_init("is_alive", 0, 0, TOK_BOOL, 0, TOK_VAR, TOK_PUB, 23, 1);
    Symbol* borrower = symbol_init("game_loop", 0, 0, TOK_INT, TOK_SHORT, TOK_VAR, TOK_PUB, 98, 23);

    symtbl_insert(table, symbol);
    symtbl_insert(table, borrower);

    symtbl_borrowsym(table, symbol, borrower);

    cr_assert_eq(symbol->data.life.borrower_size, 1,
        "symtbl: borrow symbol did not update borrower_size correctly: expected: 1 found: %zu", symbol->data.life.borrower_size);
    cr_assert_eq(symbol->data.life.borrower_list[0], borrower,
        "symtbl: borrow symbol did not update borrower_list proopperly");

    symtbl_free(table);
}