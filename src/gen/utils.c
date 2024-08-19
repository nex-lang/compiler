#include "gen/utils.h"

#include <stdio.h>

#include "symtbl.h"

char* shortkw(uint8_t kw) {

    switch (kw) {
        case TOK_L_SSINT: return "i8";
        case TOK_L_SINT: return "i16";
        case TOK_L_INT: return "i32";
        case TOK_L_LINT: return "i64";
        case TOK_L_SSUINT: case TOK_L_CHAR: return "u8";
        case TOK_L_SUINT: return "u16";
        case TOK_L_UINT: return "u32";
        case TOK_L_LUINT: return "u64";
        case TOK_TRUE: case TOK_FALSE: return "i1";       
        case TOK_L_LLUINT: return "f32";
        case TOK_L_FLOAT: return "f64";


        default: break;
    }

    return NULL;
}

char* glval(AST_Node* expr) {
    if ((uint8_t)expr->data.expr.type != (uint8_t)EXPR_LITERAL) {
        return NULL;
    }

    char* str = (char*)malloc(255 * sizeof(char)); 

    switch (expr->data.expr.data.literal.type) {
        case TOK_L_SSINT:
        case TOK_L_SINT:
        case TOK_L_INT:
        case TOK_L_LINT:
            sprintf(str, "%ld", expr->data.expr.data.literal.value.int_.norm);
            return str;
            break;
        case TOK_L_LLINT:
            break;
        case TOK_L_SSUINT:
        case TOK_L_SUINT:
        case TOK_L_UINT:
        case TOK_L_LUINT:
            sprintf(str, "%lu", expr->data.expr.data.literal.value.uint.norm);
            return str;
            break;
        case TOK_L_LLUINT:
            break;
        case TOK_L_FLOAT:
            sprintf(str, "%f", expr->data.expr.data.literal.value.float_.bit32);
            return str;
            break;
        case TOK_L_DOUBLE:
            sprintf(str, "%lf", expr->data.expr.data.literal.value.float_.bit64);
            return str;
            break;
        case TOK_L_CHAR:
            sprintf(str, "%i", expr->data.expr.data.literal.value.character);
            return str;
            break;
        case TOK_L_STRING:
            break;
        case TOK_TRUE:
            return "1";
            break;
        case TOK_FALSE:
            return "0";
            break;
        case TOK_L_SIZE:
            sprintf(str, "%zu", expr->data.expr.data.literal.value.size);
            return str;
            break;
        default: break;
    }

    return NULL;
}

int16_t gltype(AST_Node* expr) {
    if (expr->type != EXPR) {
        return -1;
    }

    if (expr->data.expr.type != EXPR_LITERAL) {
        return -1;
    } 

    return expr->data.expr.data.literal.type;
}

char* glepval(ASTN_Expression* expr) {
    if ((uint8_t)expr->type != (uint8_t)EXPR_LITERAL) {
        return NULL;
    }

    char* str = (char*)malloc(255 * sizeof(char)); 

    switch (expr->data.literal.type) {
        case TOK_L_SSINT:
        case TOK_L_SINT:
        case TOK_L_INT:
        case TOK_L_LINT:
            sprintf(str, "%ld", expr->data.literal.value.int_.norm);
            return str;
            break;
        case TOK_L_LLINT:
            break;
        case TOK_L_SSUINT:
        case TOK_L_SUINT:
        case TOK_L_UINT:
        case TOK_L_LUINT:
            sprintf(str, "%lu", expr->data.literal.value.uint.norm);
            return str;
            break;
        case TOK_L_LLUINT:
            break;
        case TOK_L_FLOAT:
            sprintf(str, "%f", expr->data.literal.value.float_.bit32);
            return str;
            break;
        case TOK_L_DOUBLE:
            sprintf(str, "%lf", expr->data.literal.value.float_.bit64);
            return str;
            break;
        case TOK_L_CHAR:
            sprintf(str, "%i", expr->data.literal.value.character);
            return str;
            break;
        case TOK_L_STRING:
            break;
        case TOK_TRUE:
            return "1";
            break;
        case TOK_FALSE:
            return "0";
            break;
        case TOK_L_SIZE:
            sprintf(str, "%zu", expr->data.literal.value.size);
            return str;
            break;
        default: break;
    }

    return NULL;
}

char* get_fid(int iden) {
    char* res = malloc(sizeof(char) * 10);
    sprintf(res, "x%x", iden);

    return res;
}

char* get_fidtyvar(int iden, SymTable* tbl) {
    Symbol* sym = symtbl_slookup(tbl, iden);
    if (sym == NULL) {
        return NULL;
    }

    return shortkw(sym->data.data.var.data_type_specifier.data.prim);
}


char* get_id(ASTN_Litlist iden) {
    if (iden.mult.size != 0) {
        return NULL;
    }

    char* res = malloc(sizeof(char) * 10);
    sprintf(res, "x%x", iden.sg);

    return res;
}


char** get_ids(ASTN_Litlist iden) {

}