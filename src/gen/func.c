#include "gen/func.h"

#include "gen.h"

void gen_fn(ASTN_FunctionDecl fn, Generator* gen) {
    char* ret_type = shortkw(fn.data_type_specifier.data.prim);

    if (ret_type == NULL) {
        return;
        // support type inference later
    }

    WO(gen->fp, 0, "define %s @%s (", ret_type, get_fid(fn.identifier));

    Symbol* sym = NULL;
    for (size_t i = 0; i < fn.parameters->size; i++) {
        if (i != fn.parameters->size - 1) {
            sym = symtbl_slookup(gen->tbl, fn.parameters->parameter[i]->identifier);
            WO(gen->fp, 0, "%s %%%s, ", shortkw(sym->data.data.param.data.prim), get_fid(sym->data.id));
        } else {
            sym = symtbl_slookup(gen->tbl, fn.parameters->parameter[i]->identifier);
            WO(gen->fp, 0, "%s %%%s) {\n", shortkw(sym->data.data.param.data.prim), get_fid(sym->data.id));
        }
    }

    for (size_t i = 0; i < fn.statements->size; i++) {
        gen_stmt(fn.statements->statement[i], gen);
    }

    WO(gen->fp, 0, "}\n");
}

void gen_call(ASTN_Call call, FILE* fp, SymTable* tbl) {
    Symbol* sym = symtbl_slookup(tbl, call.identifier);


    if (sym == NULL) {
        return;
    }

    printf("-> %i\n", call.type);
    switch (call.type) {
        case CALL_FN:
            // sym->data.data.fn
            WO(fp, 1, "call @%s\n", get_fid(call.identifier));
            break;
        case CALL_CLASS:
            // call.identifiers __init__ method 
            break;
        case CALL_STRUCT:
            break;
    }
}