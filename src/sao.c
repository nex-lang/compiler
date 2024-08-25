#include "sao.h"


SAO* sao_init(AST_Node** roots, Lexer** lexers, SymTable** tables, char** files, uint32_t count, uint32_t cur) {
    SAO* sao = malloc(sizeof(SAO));

    sao->roots = roots;
    sao->tbls = tables;
    sao->lexers = lexers;
    sao->flags = malloc(sizeof(char));
    sao->flag_no = 0;

    sao->files = files;
    sao->count = count;
    sao->cur = cur;

    return sao;
}


void sao_analyze(SAO* sao, AST_Node* node, SymTable* tbl) {
    if (node == NULL) {
        return;
    }

    if (node->type == STMT && node->data.stm.type == STMT_IMPORT_DECL) {
        sao_import(sao, node, tbl);
    } else if (node->type == MEP) {
        for (size_t i = 0; i < node->data.mep.statements->size; i++) {
            sao_stms(sao, node->data.mep.statements->statement[i], tbl);
        }
    }




    if (node->next != NULL) {
        sao_analyze(sao, node->next, tbl);
    }
}

bool sao_stms(SAO* sao, AST_Node* node, SymTable* tbl) {
    switch (node->data.stm.type) {
        case STMT_CALL:
            sao_call(sao, node->data.stm, tbl);
            break;    
        default:
            break;
    }
}

bool sao_call(SAO* sao, ASTN_Statement stm, SymTable* tbl) {
    if (stm.data.call.type != CALL_FN) {
        return false;
    }

    Symbol* sym = symtbl_slookup(tbl, stm.data.call.identifier);

    if (sym != NULL && sym->data.type != SYMBOL_FUNCTION) {
        return false;
    }

    if (stm.data.call.params->size != sym->data.data.fn.parameters->size) {
        REPORT_ERROR(sao->lexers[sao->cur], "U_IPARAMS", sym->data.data.fn.parameters->size, stm.data.call.params->size);
        return false;
    }    

    for (size_t i = 0; i < sym->data.data.fn.parameters->size; i++) {
        if (stm.data.call.params->parameter[i]->data.expr.data.literal.type
            != sym->data.data.fn.parameters->parameter[i]->data_type_specifier.data.prim) {
        REPORT_ERROR(sao->lexers[sao->cur], "E_VPARAMS");
        }
    }
}

bool sao_import(SAO* sao, AST_Node* node, SymTable* tbl) {
    int16_t idx = -1;

    if (node->data.stm.data.import_decl.type == IMP_LOCAL) {
        idx = get_source_file_index(sao->files, sao->count, node->data.stm.data.import_decl.source->module);

        if (idx == -1) {
            exit(-1);
        }

        resolve_sym(sao->tbls[idx], tbl, node->data.stm.data.import_decl.modules);
        return true;
    } else if (node->data.stm.data.import_decl.type == IMP_LOCALF) {                    
        idx = get_source_file_index(sao->files, sao->count, node->data.stm.data.import_decl.modules.items[0]->module);
        
        if (idx == -1) {
            exit(-1);
        }

        fi_resolve_sym(sao->tbls[idx], tbl);
        return true;
    } else if (node->data.stm.data.import_decl.type == IMP_STD) {
        resolve_std_sym(sao, node->data.stm.data.import_decl.source->module, node->data.stm.data.import_decl.modules);
        return true;
    }

    return false;
}

bool resolve_sym(SymTable* src, SymTable* dest, ASTN_Modules mods) {
    Symbol* sym = malloc(sizeof(Symbol));
    Symbol* sym2 = malloc(sizeof(Symbol));


    for (size_t i = 0; i < mods.size; i++) {
        sym = symtbl_lookup(src, mods.items[i]->module, 0, 0, 0);
        sym2 = symtbl_lookup(dest, mods.items[i]->module, 0, 0, 0);  

        if (sym2 == NULL) {
            // WTF HOW!
            return false;
        }

        if (sym == NULL) {
            // attempt to import non existing modules 
            return false;
        }

        sym2->data.type = sym->data.type;
        sym2->data.ty_size = sym->data.ty_size;
        sym2->data.data = sym->data.data;
    }

    return true;
}

bool resolve_std_sym(SAO* sao, char* sub, ASTN_Modules mods) {
    char* libs[STD_LIBS] = {"io", "math"};
    SymTable* libtbl[STD_LIBS] = {std_io(), std_math()};

    for (size_t i = 0; i < STD_LIBS; i++) {
        if (strcmp(libs[i], sub) == 0) {
            resolve_sym(libtbl[i], sao->tbls[sao->cur], mods);
            sao->flags = realloc(sao->flags, sizeof(char) * (sao->flag_no + 1));
            sao->flags[sao->flag_no] = libs[i];
            sao->flag_no += 1;
            return true;
        }
    } 

    return false;
}

void fi_resolve_sym(SymTable* src, SymTable* dest) {
    Symbol* sym = malloc(sizeof(Symbol));
    Symbol* sym2 = malloc(sizeof(Symbol));
    
    sym2 = dest->symbol;

    while (sym2->next != NULL) {
        if (sym2->data.type == SYMBOL_UNRE) {
            sym = symtbl_slookup(src, sym2->data.id);
            
            
            if (sym != NULL) {
                sym2->data = sym->data;
            }
        }

        sym2 = sym2->next;
    }

}

int get_source_file_index(char** source_files, uint32_t source_files_count, char* filename) {
    
    for (uint32_t i = 0; i < source_files_count; i++) {
        if (strcmp(source_files[i], filename) == 0) {
            return i;
        }
    }
    return -1;
}

SymTable* std_io() {
    SymTable* tbl = symtbl_init();

    /* temporary until i figure out libs: fn puts => (str: __buf);  */
    Symbol* sym = symbol_init("puts", SYMBOL_FUNCTION, 0, 0, 0, 0, 0);
    sym->data.data.fn.parameters = malloc(sizeof(ASTN_Parameters));
    sym->data.data.fn.parameters->parameter = malloc(sizeof(ASTN_Parameter));
    sym->data.data.fn.parameters->size = 1;

    ASTN_Parameter* puts_param = malloc(sizeof(ASTN_Parameter));
    Symbol* param_sym = symbol_init("__buf", SYMBOL_VARIABLE, 1, 0, 0, 0, 0);
    puts_param->identifier = param_sym->data.id;
    puts_param->data_type_specifier.data.prim = TOK_L_STRING;
    sym->data.data.fn.parameters->parameter[0] = puts_param;

    symtbl_rinsert(tbl, sym, "puts");
    symtbl_rinsert(tbl, param_sym, "puts");

    return tbl;
}


SymTable* std_math() {
    SymTable* tbl = symtbl_init();

    symtbl_rinsert(tbl, symbol_init("gcd", SYMBOL_FUNCTION, 0, 0, 0, 0, 0), "gcd");
    symtbl_rinsert(tbl, symbol_init("lcm", SYMBOL_FUNCTION, 0, 0, 0, 0, 0), "lcm");

    return tbl;
}

void symtbl_rinsert(SymTable* tbl, Symbol* symbol, char* raw_symb) {
    if (!tbl) {
        exit(EXIT_FAILURE); 
        return;
    }

    Symbol* cur = tbl->symbol;

    Symbol* checks;
    checks = tbl->symbol;

    while (checks != NULL) {
        if (checks->data.id == symbol->data.id) {
            return; 
        }
        checks = checks->next;
    }


    if (tbl->symbol == NULL) {        
        tbl->symbol = symbol;
        return;
    }

    Symbol* current = tbl->symbol;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = symbol;
}