#include "gen/import.h"
#include "gen/func.h"

void gen_import(ASTN_ImportDecl imp, Generator* gen) {
    if (imp.modules.size == 0) {
        return;
    }

    for (size_t i = 0; i < imp.modules.size; i++) {
        gen_module(imp.modules.items[i], gen);
    }
}

void gen_module(ASTN_Module* mod, Generator* gen) {
    Symbol* sym = NULL;
    printf("!! %s\n", mod->module);
    sym = symtbl_slookup(gen->tbl, symtbl_hash(mod->module, 0));


    if (sym == NULL) {
        return;
    }


    switch (sym->data.type) {
        case SYMBOL_FUNCTION:
            gen_fnd(sym->data.data.fn, gen);
            break;
        default:
            break;
    }
}