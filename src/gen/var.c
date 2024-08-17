#include "gen/var.h"
#include "gen/utils.h"

#include "io.h"

void gen_var_decl(ASTN_VariableDecl decl, FILE* fp) {
    /*
    %id =  alloca <type> [, <num elements>] [, align <alignment>]
    */

    char* alloca = malloc(sizeof(char) * 100);


    if (decl.data_type_specifier.data.prim == -1) {
        return;
    }

    if (decl.data_type_specifier.is_arr) {
        char* sz_kw;
        if (decl.data_type_specifier.arr <= UINT8_MAX) {
            sz_kw = "u8";
        } else if (decl.data_type_specifier.arr <= UINT16_MAX) {
            sz_kw = "u16";
        } else if (decl.data_type_specifier.arr <= UINT32_MAX) {
            sz_kw = "u32";
        }

        sprintf(alloca, "%s, %s %zu", shortkw(decl.data_type_specifier.data.prim), sz_kw, decl.data_type_specifier.arr);
    } else {
        alloca = shortkw(decl.data_type_specifier.data.prim);
    }
    

    char* id = get_id(decl.iden);

    if (id != NULL) {
        WO(fp, 1, "%%%s = alloca %s\n", id, alloca); 
    }

}