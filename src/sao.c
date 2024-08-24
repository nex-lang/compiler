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
    if (node->next != NULL) {
        if (node->type == STMT) {

            switch (node->data.stm.type) {
            case STMT_IMPORT_DECL:
                int16_t idx = -1;
                if (node->data.stm.data.import_decl.type == IMP_LOCAL) {
                    idx = get_source_file_index(sao->files, sao->count, node->data.stm.data.import_decl.source->module);

                    if (idx == -1) {
                        exit(-1);
                    }

                    resolve_sym(sao->tbls[idx], tbl, node->data.stm.data.import_decl.modules);
                } else if (node->data.stm.data.import_decl.type == IMP_LOCALF) {                    
                    idx = get_source_file_index(sao->files, sao->count, node->data.stm.data.import_decl.modules.items[0]->module);
                    
                    if (idx == -1) {
                        exit(-1);
                    }

                    fi_resolve_sym(sao->tbls[idx], tbl);
                } else if (node->data.stm.data.import_decl.type == IMP_STD) {
                    resolve_std_sym(sao, node->data.stm.data.import_decl.source->module, node->data.stm.data.import_decl.modules);
                }
                break;            
            default:
                break;
            }
        }

        sao_analyze(sao, node->next, tbl);
    }
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
    char* libs[2] = {"io", "math"};

    for (size_t i = 0; i < 2; i++) {
        if (strcmp(libs[i], sub) == 0) {
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