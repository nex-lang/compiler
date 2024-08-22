#include "sao.h"


void SAO(AST_Node** roots, SymTable** tbls, char** files, uint32_t count, uint32_t cur) {
    trav(roots, tbls, files, count, roots[cur], tbls[cur]);
    // optimize and analyze
}

void trav(AST_Node** roots, SymTable** tbls, char** files, uint32_t count, AST_Node* node, SymTable* tbl) {
    if (node->next != NULL) {

        if (node->type == STMT) {
            switch (node->data.stm.type) {
            case STMT_IMPORT_DECL:
                int16_t idx = get_source_file_index(files, count, node->data.stm.data.import_decl.source->module);

                if (idx == -1) {
                    exit(-1);
                }

                resolve_sym(tbls[idx], tbl, node->data.stm.data.import_decl.modules);
                break;            
            default:
                break;
            }
        }

        trav(roots, tbls, files, count, node->next, tbl);
    }
    
    // if (node->left != NULL) {
        // trav(node->left);
    // }
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

int get_source_file_index(char** source_files, uint32_t source_files_count, char* filename) {
    
    for (uint32_t i = 0; i < source_files_count; i++) {
        if (strcmp(source_files[i], filename) == 0) {
            return i;
        }
    }
    return -1;
}