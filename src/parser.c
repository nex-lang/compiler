#include "parser.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

Parser* parser_init(char* filename, char** srcs, LibraryList* lib_list, ...) {
    Parser* parser = calloc(1, sizeof(Parser));

    parser->lexer = lexer_init(filename);
    parser->cur = lexer_next_token(parser->lexer);
    parser->tbl = symtbl_init();
    parser->tree = ast_init(ROOT);
    parser->root = parser->tree;

    parser->highest_scope = 0;
    parser->recent_root = 0;
    parser->scope = 0;
    parser->nest = 0;

    parser->lib_list = lib_list;
    parser->source_list = srcs;

    memset(&parser->warnings, 0, sizeof(Warnings));

    va_list args;
    va_start(args, lib_list);

    Flags option;
    while ((option = va_arg(args, Flags)) != 0) {
        switch (option) {
            case NEX_WARNINGS:
                parser->warnings = va_arg(args, Warnings);
                break;
            case NEX_OPTIMIZATION:
                parser->optimization = va_arg(args, int);
                break;
            default: break;
        }
    }

    va_end(args);

    return (Parser*)parser;
}

void parser_free(Parser* parser) {
    /*
    De-initializes provided parser
    */

    if (!parser) {
        return;
    }

    lexer_free(parser->lexer);
    token_free(parser->cur);

    PRINT_AST_NODE(parser->root, 0);

    ast_free(parser->tree);
    ast_free(parser->root);

    Symbol* cur = parser->tbl->symbol;
    PRINT_SYMB_TBL(cur);

    symtbl_free(parser->tbl);
    parser = NULL;
}

bool parser_expectsq(Parser* parser, ...) {
    va_list args;
    va_start(args, parser);

    Token *cur_token;
    while ((cur_token = va_arg(args, Token *)) != NULL) {
        parser_consume(parser);

        if (parser->cur != cur_token) {
            va_end(args);
            return false;
        }
    }

    va_end(args);
    return true;
}


bool parser_expect(Parser* parser, uint8_t expected) {
    if (parser->cur->type != expected) {
        return false;            
    }

    parser_consume(parser);
    return true;
}

bool parser_expect_spec_value(Parser* parser, uint8_t expected_t, char* expected_c) {
    if ((parser->cur->type != expected_t) || strcmp(parser->cur->value, expected_c) != 0) {
        return false;            
    }

    parser_consume(parser);
    return true;
}

void parser_consume(Parser* parser) {
    if (parser->cur->type == TOK_EOF) {
        exit(0);
    }

    token_free(parser->cur);

    parser->cur = lexer_next_token(parser->lexer);
}

void parser_parse(Parser* parser) {

    while (parser->cur->type != TOK_EOF) {
        AST_Node* n = parser_parse_typestart(parser);
        if (n != NULL) {
            parser->tree->right = n;
        } else {
            switch (parser->cur->type) {
                case TOK_ATHER:
                    parser_consume(parser);
                    if (parser->cur->type != TOK_IMPORT) {
                        REPORT_ERROR(parser->lexer, "E_DECLS_AF_ATHER");
                        break;
                    }
                    parser->tree->right = parser_parse_import(parser);
                    break;
                case TOK_COLON:
                    parser->tree->right = parser_parse_mep_decl(parser);
                    break;
                case TOK_FN:
                    parser->tree->right = parser_parse_function_decl(parser);
                    break;
                case TOK_IDEN:  
                    parser->tree->right = parser_parse_expr(parser, 0);
                    break;
                case TOK_ATTR:
                    parser->tree->right = parser_parse_attr_decl(parser);                
                    break;
                case TOK_CLASS:                
                    parser->tree->right = parser_parse_class_decl(parser);
                    break;
                case TOK_ERR: 
                    parser->tree->right = parser_parse_err_decl(parser);
                    break;
                case TOK_ENUM:
                    parser->tree->right = parser_parse_enum_decl(parser);
                    break;
                case TOK_STRUCT:
                    parser->tree->right = parser_parse_struct_decl(parser);
                    break;
                default:
                    break;
            }

        }
        

        if (!(parser->tree && parser->tree->right)) {
            break;
        }

        AST_Node* tmp = parser->tree->right; 

        parser->tree = tmp;
    }


    return;
}


/* int parse_stospec(Parser* parser, bool expect_further) {
    if (!(parser->cur->type == TOK_VAR || parser->cur->type == TOK_MUT || parser->cur->type == TOK_CONST)) {
        return NULL;
    }

    if (!expect_further) {
        return parser->cur->type;
    }

    if (parser_expect(parser, TOK_COLON)) {
        parser_parse_var        
    }



}

int parse_accspec(Parser* parser, bool expect_further); */


ASTN_Literal parser_parse_literal(Parser* parser) {
    ASTN_Literal lit;
    char *endptr;

    if (!IS_LITERAL(parser->cur->type)) {
        lit.type = -1;
        return lit;
    }

    lit.type = parser->cur->type;

    switch (lit.type) {
        case TOK_L_SSINT:
        case TOK_L_SINT:
        case TOK_L_INT:
        case TOK_L_LINT:
            lit.value.int_.norm = (int64_t)strtol(parser->cur->value, &endptr, 10);
            break;
        case TOK_L_LLINT:
            int128_t i128; strtoint128(parser->cur->value, i128);

            lit.value.int_.bit128.low = i128.low;
            lit.value.int_.bit128.high = i128.high;
            break;
        case TOK_L_SSUINT:
        case TOK_L_SUINT:
        case TOK_L_UINT:
        case TOK_L_LUINT:
            lit.value.uint.norm = (uint64_t)strtoul(parser->cur->value, &endptr, 10);
            break;
        case TOK_L_LLUINT:
            uint128_t u128; strtouint128(parser->cur->value, u128);

            lit.value.uint.bit128.low = u128.low;
            lit.value.uint.bit128.high = u128.high;
            break;
        case TOK_L_FLOAT:
            lit.value.float_.bit32 = strtof(parser->cur->value, &endptr);
            break;
        case TOK_L_DOUBLE:
            lit.value.float_.bit64 = strtod(parser->cur->value, &endptr);
            break;
        case TOK_L_CHAR:
            lit.value.character = (parser->cur->value[0] != '\0') ? parser->cur->value[0] : '\0';
            break;
        case TOK_L_STRING:
            lit.value.string = parser->cur->value;
            break;
        case TOK_TRUE:
            lit.value.boolean = 1;
            break;
        case TOK_FALSE:
            lit.value.boolean = 0;
            break;
        case TOK_L_SIZE:
            lit.value.size = (size_t)strtoull(parser->cur->value, &endptr, 10);
            break;
        default: lit.type = -1; return lit; break;
    }


    parser_consume(parser);
    return lit;
}


ASTN_DataTypeSpecifier parser_parse_dt_spec(Parser* parser) {
    ASTN_DataTypeSpecifier dts;
    dts.data.prim = 0;
    dts.is_arr = false;
    char* iden;
    char *endptr;
    int int_dts = 0;


    while (parser->cur->type == TOK_IDEN) {
        iden = parser->cur->value;
        dts.data.prim = -1;

        if (parser->cur->type == TOK_PERIOD) {
            parser_consume(parser); 
            if (parser->cur->type == TOK_IDEN) {
                parser_consume(parser);
            } else {
                REPORT_ERROR(parser->lexer, "E_EXPECT_IDEN", parser->cur->value);
                dts.data.prim = -1;
                return dts;
            }
        }

        if (parser->cur->type != TOK_PERIOD) {
            iden = parser->cur->value;
            break;
        }
    }

    if (dts.data.prim == -1) {
        Symbol* symb = symtbl_lookup(parser->tbl, (const char*)iden, 0, 0,  parser->recent_root);

        if (symb == NULL) {
            REPORT_ERROR(parser->lexer, "U_UOUDTY", parser->cur->value);
            dts.data.prim = -1;
            return dts;
        }

        if (!(symb->data.type == SYMBOL_CLASS || symb->data.type == SYMBOL_STRUCT || symb->data.type == SYMBOL_ENUM)) {
            REPORT_ERROR(parser->lexer, "U_UOUDTY", parser->cur->value);
            dts.data.prim = -1;
            return dts;
        }


        dts.data.cust_type = symb->data.ty_size; 
        parser_consume(parser);
    }


    if (parser_expect(parser, TOK_S_SHORT)) {
        int_dts = 8;
    } else if (parser_expect(parser, TOK_SHORT)) {
        int_dts = 16;
    } else if (parser_expect(parser, TOK_LONG)) {
        int_dts += 64;
    } else if (parser_expect(parser, TOK_L_LONG)) {
        int_dts += 128;
    }

    if (parser_expect(parser, TOK_INT)) {
        int_dts = (int_dts == 0) ? 32 : int_dts;
    } else if (parser_expect(parser, TOK_UINT)) {
        int_dts += 1;
        int_dts = (int_dts == 1) ? 33 : int_dts;
    }

    switch (int_dts) {
        case 8: dts.data.prim = TOK_L_SSINT; break;
        case 16: dts.data.prim = TOK_L_SINT; break;
        case 32: dts.data.prim = TOK_L_INT; break;
        case 64: dts.data.prim = TOK_L_LINT; break;
        case 128: dts.data.prim = TOK_L_LLINT; break;
        case 9: dts.data.prim = TOK_L_SSUINT; break;
        case 17: dts.data.prim = TOK_L_SUINT; break;
        case 33: dts.data.prim = TOK_L_UINT; break;
        case 65: dts.data.prim = TOK_L_LUINT; break;
        case 129: dts.data.prim = TOK_L_LLUINT; break;
        default: break;
    }


    if (int_dts > 0 && parser->cur->type != TOK_LBRACK) {
        return dts;
    }

    if (parser_expect(parser, TOK_FLOAT)) {
        dts.data.prim = TOK_L_FLOAT;
    } else if (parser_expect(parser, TOK_DOUBLE)) {
        dts.data.prim = TOK_L_DOUBLE;
    } else if (parser_expect(parser, TOK_BOOL)) {
        dts.data.prim = TOK_L_BOOL;
    } else if (parser_expect(parser, TOK_CHAR)) {
        dts.data.prim = TOK_L_CHAR;
    } else if (parser_expect(parser, TOK_STRING)) {
        dts.data.prim = TOK_L_STRING;
    } else if (parser_expect(parser, TOK_SIZE)) {
        dts.data.prim = TOK_L_SIZE;
    }


    if (parser_expect(parser, TOK_LBRACK)) {
        if (!(parser->cur->type ==  TOK_L_SSUINT ||
            parser->cur->type ==  TOK_L_SUINT ||
            parser->cur->type ==  TOK_L_UINT)) {
            REPORT_ERROR(parser->lexer, "E_PROP_ARR_SZ", parser->cur->value);
            dts.data.prim = -1;
            return dts;
        }

        dts.arr = (size_t)strtoul(parser->cur->value, &endptr, 10);
        parser_consume(parser);


        if (parser_expect(parser, TOK_RBRACK)) {
            dts.is_arr = true;
            return dts;
        }

        REPORT_ERROR(parser->lexer, "E_CLOSE_BRACK", parser->cur->value);
        dts.data.prim = -1;
        return dts;
    }

    return dts;
}

AST_Node* parser_parse_typestart(Parser* parser) {
    ASTN_DataTypeSpecifier dts = parser_parse_dt_spec(parser);
    if (dts.data.prim != 0 && !parser_expect(parser, TOK_COLON)) {
        REPORT_ERROR(parser->lexer, "E_FN_AF_DTSSPEC");
    }


    if (parser->cur->type == TOK_FN) { 
        AST_Node* n = parser_parse_function_decl(parser);
        n->data.stm.data.function_decl.data_type_specifier = dts;
        return n;
    }


    if (parser_expect(parser, TOK_LBRACK)) {
        ASTN_MutableTypes comps;
        ASTN_ReturnTypes rt;

        rt.data_type_specifier = calloc(1, sizeof(ASTN_MutableTypes));
        rt.item_size = sizeof(ASTN_MutableTypes);
        rt.size = 1;

        if (!parser_expect(parser, TOK_LPAREN)) {
            REPORT_ERROR(parser->lexer, "E_PAREN_FOR_MULTI_VARIABLE_RETS");
        }

        comps = parser_parse_compatibilities(parser);

        if (parser_expect(parser, TOK_RBRACK)) {
            if (!(parser_expect(parser, TOK_PUB)  || parser_expect(parser, TOK_PRIV)
            || parser_expect(parser, TOK_GLOB) || parser_expect(parser, TOK_COLON))) {
                REPORT_ERROR(parser->lexer, "E_FN_AF_RESPEC");    
            }

            if (parser->cur->type != TOK_FN) {
                REPORT_ERROR(parser->lexer, "E_FN_AF_RESPEC");
            }

            AST_Node* n = parser_parse_function_decl(parser);

            rt.data_type_specifier[0] = comps;
            n->data.stm.data.function_decl.returns = rt;
            
            return n;
        }

        if (parser->cur->type != TOK_COMMA) {
            REPORT_ERROR(parser->lexer, "E_VAL_DTS_SQ");
            return NULL;
        }

        rt.size += 1;
        rt.data_type_specifier = realloc(rt.data_type_specifier, rt.size  * rt.item_size);

        while (parser->cur->type == TOK_COMMA) {
            parser_consume(parser);

            if (!parser_expect(parser, TOK_LPAREN)) {
                REPORT_ERROR(parser->lexer, "E_PAREN_FOR_MULTI_VARIABLE_RETS");
            }

            rt.data_type_specifier[rt.size - 1] = parser_parse_compatibilities(parser);
            rt.size += 1;
            rt.data_type_specifier = realloc(rt.data_type_specifier, rt.size  * rt.item_size);
        }

        if (!parser_expect(parser, TOK_RBRACK)) {
            REPORT_ERROR(parser->lexer, "E_BRACK_MULTI_RET");
        }

        if (!parser_expect(parser, TOK_COLON)) {
            REPORT_ERROR(parser->lexer, "E_COLON_AF_MULTI_RET_DECL");
        }

        AST_Node* n2 = parser_parse_function_decl(parser);
        n2->data.stm.data.function_decl.returns = rt;
        
        return n2;
    }

    if (parser_expect(parser, TOK_LPAREN)) {
        ASTN_MutableTypes mut =  parser_parse_compatibilities(parser);

        if (!(parser_expect(parser, TOK_PUB)  || parser_expect(parser, TOK_PRIV)
        || parser_expect(parser, TOK_GLOB) || parser_expect(parser, TOK_COLON))) {
            REPORT_ERROR(parser->lexer, "E_FN_AF_MUTSPEC");    
        }

        if (parser->cur->type != TOK_FN) {
            REPORT_ERROR(parser->lexer, "E_FN_AF_MUTSPEC");
        }

        AST_Node* n = parser_parse_function_decl(parser);
        n->data.stm.data.function_decl.compatibles = mut;
        return n;
    }


    return NULL;
}

ASTN_MutableTypes parser_parse_compatibilities(Parser* parser) {
    ASTN_MutableTypes mt;

    mt.data_type_specifier = calloc(1, sizeof(ASTN_DataTypeSpecifier));
    mt.item_size = sizeof(ASTN_DataTypeSpecifier);
    mt.size = 1;

    ASTN_DataTypeSpecifier dts = parser_parse_dt_spec(parser);
    if (dts.data.prim == 0) {
        REPORT_ERROR(parser->lexer, "E_VAL_DTS_MULTI");
    }

    if (parser_expect(parser, TOK_RPAREN)) {
        mt.data_type_specifier[mt.size - 1] = dts;
        return mt; 
    }

    if (parser->cur->type != TOK_COMMA) {
        REPORT_ERROR(parser->lexer, "E_VAL_DTS_SQ");
    }

    mt.data_type_specifier[0] = dts;
    mt.size += 1;

    while (parser->cur->type == TOK_COMMA) {
        parser_consume(parser);

        mt.data_type_specifier[mt.size - 1] = parser_parse_dt_spec(parser);
        mt.size += 1;
        mt.data_type_specifier = realloc(mt.data_type_specifier, mt.size  * mt.item_size);
    }

    if (!parser_expect(parser, TOK_RPAREN)) {
        REPORT_ERROR(parser->lexer, "E_PAREN_DTS_MULTI");
    }

    return mt;        

}


ASTN_Call parser_parse_call(Parser* parser, uint8_t scopeOS) {
    ASTN_Call call;

    Symbol* symb = symtbl_lookup(parser->tbl, parser->cur->value, 0, 0, parser->recent_root);

    if (symb == NULL) {
        call.identifier = 0;
        return call;
    }

    switch (symb->data.type) {
        case SYMBOL_FUNCTION:
            call.type = CALL_FN;
            break;
        case SYMBOL_CLASS:
            call.type = CALL_CLASS;
            break;
        case SYMBOL_STRUCT:
            call.type = CALL_STRUCT;
            break;
        default:
            call.identifier = 0;
            return call;
    }

    parser_consume(parser);

    if (!(parser_expect(parser, TOK_LPAREN))) {
        call.identifier = 0;
        return call;
    } 

    call.identifier = symb->data.id;

    ASTN_CallParams* params = calloc(1, sizeof(ASTN_CallParams));

    params->size = 0;
    params->item_size = sizeof(AST_Node*);
    params->parameter = calloc(1, sizeof(AST_Node*));


    while (parser->cur->type != TOK_RPAREN) {
        params->parameter[params->size] = parser_parse_expr(parser, scopeOS);

        if (!(parser_expect(parser, TOK_COMMA)) && (parser->cur->type != TOK_RPAREN)) {
            REPORT_ERROR(parser->lexer, "E_PARAMS_COMMA", parser->cur->value);
            call.identifier = 0;
            return call;
        }         

        params->parameter = realloc(params->parameter, (params->size + 1) * sizeof(AST_Node));
        params->size++;
    }

    call.params = params;

    parser_expect(parser, TOK_RPAREN);

    printf("[STACK CALL]: %i\n", call.identifier);

    // if (call.params && call.params->parameter) {
    //     for (size_t i = 0; i < call.params->size; i++) {
    //         printf("\t arg [%zu]: %i\n", i, call.params->parameter[i]->data.expr.data.primary.data.literal.type);
    //     }
    // }

    return call;
}


ASTN_PrimaryExpr parser_parse_prim_expr(Parser* parser, uint8_t scopeOS) {
    ASTN_PrimaryExpr expr;
    expr.type = -1;

    ASTN_Literal lit = parser_parse_literal(parser);
    if (lit.type != -1) {
        expr.type = PRIMARY_LITERAL;
        expr.data.literal = lit;
        return expr;
    }


    if (parser->cur->type == TOK_IDEN) {
        Symbol* symb = symtbl_lookup(parser->tbl, parser->cur->value, 0, 0, 0);
        if (symb) {
            if (symb->data.type == SYMBOL_FUNCTION || symb->data.type == SYMBOL_CLASS ||
                symb->data.type == SYMBOL_STRUCT) {
                expr.type = PRIMARY_CALL;
                expr.data.call = parser_parse_call(parser, scopeOS);
            } else if (symb->data.type == SYMBOL_MODULE) {
                expr.type = PRIMARY_IDENTIFIER;
                expr.data.identifier = symb->data.id;
                parser_consume(parser);
            }
        } else {
            Symbol* symb2 = symtbl_lookup(parser->tbl, parser->cur->value, parser->scope, scopeOS, parser->recent_root);
            if (symb2) {
                expr.type = PRIMARY_IDENTIFIER;
                expr.data.identifier = symb2->data.id;
                parser_consume(parser);
            } else {
                REPORT_ERROR(parser->lexer, "U_USOF_UNDEFV");
                expr.type = PRIMARY_IDENTIFIER;
                expr.data.identifier = 0;
            }
        }
    }

    return expr;
}


ASTN_FactorExpr parser_parse_factor_expr(Parser* parser, uint8_t scopeOS) {
    ASTN_FactorExpr expr;
    expr.type = -1;

    if (parser->cur->type == TOK_MINUS_MINUS || parser->cur->type == TOK_ADD_ADD ||
        parser->cur->type == TOK_MINUS || parser->cur->type == TOK_BANG) {
        expr.type = FACTOR_UNARY_OP;
        expr.data.unary_op.op = parser->cur->type;
        parser_consume(parser);
        expr.data.unary_op.expr = parser_parse_expression(parser, scopeOS);
        if (!expr.data.unary_op.expr) {
            return expr;
        }
    } else {
        expr.data.primary = parser_parse_prim_expr(parser, scopeOS);

        if (expr.data.primary.type == -1) {
            REPORT_ERROR(parser->lexer, "E_PROP_EXP", parser->cur->value);
            return expr;
        }
        expr.type = FACTOR_PRIMARY;
    }

    return expr;
}


ASTN_TermExpr parser_parse_term_expr(Parser* parser, uint8_t scopeOS) {
    ASTN_TermExpr expr;
    expr.type = -1;

    expr.data.factor = parser_parse_factor_expr(parser, scopeOS);

    if (expr.data.factor.type == -1) {
        return expr;
    }
    expr.type = TERM_FACTOR;

    while (parser->cur->type == TOK_ASTK_ASTK) {
        ASTN_TermExpr new_expr;
        new_expr.type = TERM_BINARY_OP;

        ASTN_Expression* left = malloc(sizeof(ASTN_Expression));
        if (!left) {
            return expr;
        }

        if (expr.data.factor.type == FACTOR_UNARY_OP) {
            left->type = EXPR_FACTOR;
            left->data.factor = expr.data.factor;
        } else if (expr.data.factor.type == FACTOR_PRIMARY) {
            switch (expr.data.factor.data.primary.type) {
                case PRIMARY_IDENTIFIER:
                    left->type = EXPR_IDENTIFIER;
                    left->data.identifier = expr.data.factor.data.primary.data.identifier;
                    break;
                case PRIMARY_LITERAL:
                    left->type = EXPR_LITERAL;
                    left->data.literal = expr.data.factor.data.primary.data.literal;
                    break;
                case PRIMARY_CALL:
                    left->type = EXPR_FUNCTION_CALL;
                    left->data.function_call = expr.data.factor.data.primary.data.call;
                    break;
            }
        } else {
            REPORT_ERROR(parser->lexer, "UNA_PARSE_EXPR");
            while (!(parser_expect(parser, TOK_SC))) {
                parser_consume(parser);
            }
            parser_consume(parser);
            return expr;
        }

        new_expr.data.binary_op.left = left;

        new_expr.data.binary_op.op = TOK_ASTK_ASTK;
        parser_consume(parser);

        new_expr.data.binary_op.right = malloc(sizeof(ASTN_Expression));

        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.left);
            return expr;
        }

        new_expr.data.binary_op.right = parser_parse_expression(parser, scopeOS); 
        
        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.left);
            free(new_expr.data.binary_op.right);
            return expr;
        }

        expr = new_expr;
    }

    return expr;
}

ASTN_MultiplicationExpr parser_parse_mult_expr(Parser* parser, uint8_t scopeOS) {
    ASTN_MultiplicationExpr expr;
    expr.type = -1;

    expr.data.term = parser_parse_term_expr(parser, scopeOS);

    if (expr.data.term.type == -1) {
        return expr;
    }

    expr.type = MULTIPLICATION_TERM;

    while (parser->cur->type == TOK_ASTK || parser->cur->type == TOK_SLASH || parser->cur->type == TOK_PERC) {
        ASTN_MultiplicationExpr new_expr;
        new_expr.type = MULTIPLICATION_BINARY_OP;

        ASTN_Expression* left = malloc(sizeof(ASTN_Expression));
        if (!left) {
            return expr;
        }
        
        if (expr.data.term.type == TERM_BINARY_OP) {
            left->type = EXPR_TERM;
            left->data.term = new_expr.data.term;
        } else if (expr.data.term.data.factor.type == FACTOR_UNARY_OP) {
            left->type = EXPR_FACTOR;
            left->data.factor = expr.data.term.data.factor;
        } else if (expr.data.term.data.factor.type == FACTOR_PRIMARY) {
            switch (expr.data.term.data.factor.data.primary.type) {
                case PRIMARY_IDENTIFIER:
                    left->type = EXPR_IDENTIFIER;
                    left->data.identifier = expr.data.term.data.factor.data.primary.data.identifier;
                    break;
                case PRIMARY_LITERAL:
                    left->type = EXPR_LITERAL;
                    left->data.literal = expr.data.term.data.factor.data.primary.data.literal;
                    break;
                case PRIMARY_CALL:
                    left->type = EXPR_FUNCTION_CALL;
                    left->data.function_call = expr.data.term.data.factor.data.primary.data.call;
                    break;
            }
        } else {
            REPORT_ERROR(parser->lexer, "UNA_PARSE_EXPR");
            while (!(parser_expect(parser, TOK_SC))) {
                parser_consume(parser);
            }
            parser_consume(parser);
            return expr;
        }

        new_expr.data.binary_op.left = left;

        new_expr.data.binary_op.op = parser->cur->type;
        parser_consume(parser);

        new_expr.data.binary_op.right = malloc(sizeof(ASTN_Expression));

        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.left);
            return expr;
        }

        new_expr.data.binary_op.right = parser_parse_expression(parser, scopeOS);
        
        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.left);
            free(new_expr.data.binary_op.right);
            return expr;
        }

        expr = new_expr;

    }

    return expr;
}

ASTN_AdditionExpr parser_parse_add_expr(Parser* parser, uint8_t scopeOS) {
    ASTN_AdditionExpr expr;
    expr.type = -1;

    expr.data.multiplication = parser_parse_mult_expr(parser, scopeOS);

    if (expr.data.multiplication.type == -1) {
        return expr;
    }

    expr.type = ADDITION_MULTIPLICATION;

    while (parser->cur->type == TOK_ADD || parser->cur->type == TOK_MINUS) {
        ASTN_AdditionExpr new_expr;
        new_expr.type = ADDITION_BINARY_OP;

        ASTN_Expression* left = malloc(sizeof(ASTN_Expression));
        if (!left) {
            return expr;
        }
        
        if (expr.data.multiplication.type == MULTIPLICATION_BINARY_OP) {
            left->type = EXPR_MULTIPLICATION;
            left->data.multiplication = new_expr.data.multiplication;
        } else if (expr.data.multiplication.data.term.type == TERM_BINARY_OP) {
            left->type = EXPR_TERM;
            left->data.term = new_expr.data.multiplication.data.term;
        } else if (expr.data.multiplication.data.term.data.factor.type == FACTOR_UNARY_OP) {
            left->type = EXPR_FACTOR;
            left->data.factor = expr.data.multiplication.data.term.data.factor;
        } else if (expr.data.multiplication.data.term.data.factor.type == FACTOR_PRIMARY) {
            switch (expr.data.multiplication.data.term.data.factor.data.primary.type) {
                case PRIMARY_IDENTIFIER:
                    left->type = EXPR_IDENTIFIER;
                    left->data.identifier = expr.data.multiplication.data.term.data.factor.data.primary.data.identifier;
                    break;
                case PRIMARY_LITERAL:
                    left->type = EXPR_LITERAL;
                    left->data.literal = expr.data.multiplication.data.term.data.factor.data.primary.data.literal;
                    break;
                case PRIMARY_CALL:
                    left->type = EXPR_FUNCTION_CALL;
                    left->data.function_call = expr.data.multiplication.data.term.data.factor.data.primary.data.call;
                    break;
            }
        } else {
            REPORT_ERROR(parser->lexer, "UNA_PARSE_EXPR");
            while (!(parser_expect(parser, TOK_SC))) {
                parser_consume(parser);
            }
            parser_consume(parser);
            return expr;
        }

        new_expr.data.binary_op.left = left;

        new_expr.data.binary_op.op = parser->cur->type;
        parser_consume(parser);

        new_expr.data.binary_op.right = malloc(sizeof(ASTN_Expression));

        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.right);
            return expr;
        }
        
        new_expr.data.binary_op.right = parser_parse_expression(parser, scopeOS);

        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.left);
            free(new_expr.data.binary_op.right);
            return expr;
        }

        expr = new_expr;
    }

    return expr;
}

ASTN_BitwiseExpr parser_parse_bitw_expr(Parser* parser, uint8_t scopeOS) {
    ASTN_BitwiseExpr expr;
    expr.type = -1;

    expr.data.addition = parser_parse_add_expr(parser, scopeOS);

    if (expr.data.addition.type == -1) {
        return expr;
    }

    expr.type = BITWISE_ADDITION;

    while (parser->cur->type == TOK_AMPER || parser->cur->type == TOK_PIPE || parser->cur->type == TOK_GT_GT || parser->cur->type == TOK_LT_LT) {
        ASTN_BitwiseExpr new_expr;
        new_expr.type = BITWISE_BINARY_OP;

        ASTN_Expression* left = malloc(sizeof(ASTN_Expression));
        
        if (!left) {
            return expr;
        }

        if (expr.data.addition.type == ADDITION_BINARY_OP) {
            left->type = EXPR_ADDITION;
            left->data.addition = expr.data.addition;
        } else if (expr.data.addition.data.multiplication.type == MULTIPLICATION_BINARY_OP) {
            left->type = EXPR_MULTIPLICATION;
            left->data.multiplication = expr.data.addition.data.multiplication;
        } else if (expr.data.addition.data.multiplication.data.term.type == TERM_BINARY_OP) {
            left->type = EXPR_TERM;
            left->data.term = expr.data.addition.data.multiplication.data.term;
        } else if (expr.data.addition.data.multiplication.data.term.data.factor.type == FACTOR_UNARY_OP) {
            left->type = EXPR_FACTOR;
            left->data.factor = expr.data.addition.data.multiplication.data.term.data.factor;
        } else if (expr.data.addition.data.multiplication.data.term.data.factor.type == FACTOR_PRIMARY) {
            switch (expr.data.addition.data.multiplication.data.term.data.factor.data.primary.type) {
                case PRIMARY_IDENTIFIER:
                    left->type = EXPR_IDENTIFIER;
                    left->data.identifier = expr.data.addition.data.multiplication.data.term.data.factor.data.primary.data.identifier;
                    break;
                case PRIMARY_LITERAL:
                    left->type = EXPR_LITERAL;
                    left->data.literal = expr.data.addition.data.multiplication.data.term.data.factor.data.primary.data.literal;
                    break;
                case PRIMARY_CALL:
                    left->type = EXPR_FUNCTION_CALL;
                    left->data.function_call = expr.data.addition.data.multiplication.data.term.data.factor.data.primary.data.call;
                    break;
            }
        } else {
            REPORT_ERROR(parser->lexer, "UNA_PARSE_EXPR");
            while (!(parser_expect(parser, TOK_SC))) {
                parser_consume(parser);
            }
            parser_consume(parser);
            return expr;
        }

        new_expr.data.binary_op.left = left;

        new_expr.data.binary_op.op = parser->cur->type;
        parser_consume(parser);

        new_expr.data.binary_op.right = malloc(sizeof(ASTN_Expression));

        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.left);
            return expr;
        }
        
        new_expr.data.binary_op.right = parser_parse_expression(parser, scopeOS);
        
        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.left);
            free(new_expr.data.binary_op.right);
            return expr;
        }

        expr = new_expr;
    }

    return expr;
}

ASTN_ComparisonExpr parser_parse_comp_expr(Parser* parser, uint8_t scopeOS) {
    ASTN_ComparisonExpr expr;
    expr.type = -1;

    expr.data.bitwise = parser_parse_bitw_expr(parser, scopeOS);

    if (expr.data.bitwise.type == -1) {
        return expr;
    }

    expr.type = COMPARISON_BITWISE;

    while (parser->cur->type == TOK_LT_EQ || parser->cur->type == TOK_GT_EQ || parser->cur->type == TOK_BANG_EQ || parser->cur->type == TOK_EQ_EQ || parser->cur->type == TOK_LT || parser->cur->type == TOK_GT) {
        ASTN_ComparisonExpr new_expr;
        new_expr.type = COMPARISON_BINARY_OP;

        ASTN_Expression* left = malloc(sizeof(ASTN_Expression));
        if (!left) {
            return expr;
        }

        if (expr.data.bitwise.type == BITWISE_BINARY_OP) {
            left->type = EXPR_BITWISE;
            left->data.bitwise = expr.data.bitwise;
        } else if (expr.data.bitwise.data.addition.type == ADDITION_BINARY_OP) {
            left->type = EXPR_ADDITION;
            left->data.addition = expr.data.bitwise.data.addition;
        } else if (expr.data.bitwise.data.addition.data.multiplication.type == MULTIPLICATION_BINARY_OP) {
            left->type = EXPR_MULTIPLICATION;
            left->data.multiplication = expr.data.bitwise.data.addition.data.multiplication;
        } else if (expr.data.bitwise.data.addition.data.multiplication.data.term.type == TERM_BINARY_OP) {
            left->type = EXPR_TERM;
            left->data.term = expr.data.bitwise.data.addition.data.multiplication.data.term;
        } else if (expr.data.bitwise.data.addition.data.multiplication.data.term.data.factor.type == FACTOR_UNARY_OP) {
            left->type = EXPR_FACTOR;
            left->data.factor = expr.data.bitwise.data.addition.data.multiplication.data.term.data.factor;
        } else if (expr.data.bitwise.data.addition.data.multiplication.data.term.data.factor.type == FACTOR_PRIMARY) {
            switch (expr.data.bitwise.data.addition.data.multiplication.data.term.data.factor.data.primary.type) {
                case PRIMARY_IDENTIFIER:
                    left->type = EXPR_IDENTIFIER;
                    left->data.identifier = expr.data.bitwise.data.addition.data.multiplication.data.term.data.factor.data.primary.data.identifier;
                    break;
                case PRIMARY_LITERAL:
                    left->type = EXPR_LITERAL;
                    left->data.literal = expr.data.bitwise.data.addition.data.multiplication.data.term.data.factor.data.primary.data.literal;
                    break;
                case PRIMARY_CALL:
                    left->type = EXPR_FUNCTION_CALL;
                    left->data.function_call = expr.data.bitwise.data.addition.data.multiplication.data.term.data.factor.data.primary.data.call;
                    break;
            }
        } else {
            REPORT_ERROR(parser->lexer, "UNA_PARSE_EXPR");
            while (!(parser_expect(parser, TOK_SC))) {
                parser_consume(parser);
            }
            parser_consume(parser);
            return expr;
        }

        new_expr.data.binary_op.left = left;

        new_expr.data.binary_op.op = parser->cur->type;
        parser_consume(parser);

        new_expr.data.binary_op.right = malloc(sizeof(ASTN_Expression));
        
        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.left);
            return expr;
        }
        
        new_expr.data.binary_op.right = parser_parse_expression(parser, scopeOS);
        
        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.left);
            free(new_expr.data.binary_op.right);
            return expr;
        }

        expr = new_expr;
    }

    return expr;
}



ASTN_Expression* parser_parse_expression(Parser* parser, uint8_t scopeOS) {
    ASTN_Expression* expr = malloc(sizeof(ASTN_Expression));
    if (expr == NULL) {
        return NULL;
    }

    expr->type = -1;
    bool expect_db_close = false;

    if (parser->cur->type == TOK_LPAREN) {
        parser_consume(parser);
        ASTN_Expression* nested_expr = parser_parse_expression(parser, scopeOS);
        nested_expr->type = EXPR_NEST;
        if (nested_expr == NULL) {
            free(expr);
            return NULL;
        }

        if (parser->cur->type == TOK_RPAREN) parser_consume(parser);

        while (parser->cur->type == TOK_ASTK_ASTK || parser->cur->type == TOK_ASTK || parser->cur->type == TOK_SLASH || parser->cur->type == TOK_PERC
        || parser->cur->type == TOK_ADD || parser->cur->type == TOK_MINUS || parser->cur->type == TOK_AMPER || parser->cur->type == TOK_PIPE
        || parser->cur->type == TOK_GT_GT || parser->cur->type == TOK_LT_LT || parser->cur->type == TOK_LT || parser->cur->type == TOK_GT
        || parser->cur->type == TOK_ADD || parser->cur->type == TOK_MINUS || parser->cur->type == TOK_LT_EQ || parser->cur->type == TOK_GT_EQ
        || parser->cur->type == TOK_BANG_EQ || parser->cur->type == TOK_EQ_EQ
        ) {
            expect_db_close = true;
            int op_type = parser->cur->type;
            parser_consume(parser); 

            ASTN_Expression* right_expr = parser_parse_expression(parser, scopeOS);
            if (right_expr == NULL) {
                free(expr);
                free(nested_expr);
                return NULL;
            }

            ASTN_Expression* compound_expr = malloc(sizeof(ASTN_Expression));
            if (compound_expr == NULL) {
                free(expr);
                free(nested_expr);
                free(right_expr);
                return NULL;
            }
            compound_expr->type = op_type;
            compound_expr->data.nest.data.binary_op.left = nested_expr;
            compound_expr->data.nest.data.binary_op.right = right_expr;

            nested_expr = compound_expr;
        }

        if (parser->cur->type == TOK_RPAREN && expect_db_close) parser_consume(parser);

        *expr = *nested_expr;
        return expr;
    }

    ASTN_ComparisonExpr x = (ASTN_ComparisonExpr){0};
    x = parser_parse_comp_expr(parser, scopeOS);

    if (x.type == COMPARISON_BINARY_OP) {
        expr->type = EXPR_COMPARISON;
        expr->data.comparison = x;
    } else if (x.data.bitwise.type == BITWISE_BINARY_OP) {
        expr->type = EXPR_BITWISE;
        expr->data.bitwise = x.data.bitwise;
    } else if (x.data.bitwise.data.addition.type == ADDITION_BINARY_OP) {
        expr->type = EXPR_ADDITION;
        expr->data.addition = x.data.bitwise.data.addition;
    } else if (x.data.bitwise.data.addition.data.multiplication.type == MULTIPLICATION_BINARY_OP) {
        expr->type = EXPR_MULTIPLICATION;
        expr->data.multiplication = x.data.bitwise.data.addition.data.multiplication;
    } else if (x.data.bitwise.data.addition.data.multiplication.data.term.type == TERM_BINARY_OP) {
        expr->type = EXPR_TERM;
        expr->data.term = x.data.bitwise.data.addition.data.multiplication.data.term;
    } else if (x.data.bitwise.data.addition.data.multiplication.data.term.data.factor.type == FACTOR_UNARY_OP) {
        expr->type = EXPR_FACTOR;
        expr->data.factor = x.data.bitwise.data.addition.data.multiplication.data.term.data.factor;
    } else if (x.data.bitwise.data.addition.data.multiplication.data.term.data.factor.type == FACTOR_PRIMARY) {
        switch (x.data.bitwise.data.addition.data.multiplication.data.term.data.factor.data.primary.type) {
            case PRIMARY_IDENTIFIER:
                expr->type = EXPR_IDENTIFIER;
                expr->data.identifier = x.data.bitwise.data.addition.data.multiplication.data.term.data.factor.data.primary.data.identifier;
                break;
            case PRIMARY_LITERAL:
                expr->type = EXPR_LITERAL;
                expr->data.literal = x.data.bitwise.data.addition.data.multiplication.data.term.data.factor.data.primary.data.literal;
                break;
            case PRIMARY_CALL:
                expr->type = EXPR_FUNCTION_CALL;
                expr->data.function_call = x.data.bitwise.data.addition.data.multiplication.data.term.data.factor.data.primary.data.call;
                break;
        }
    } else {
        REPORT_ERROR(parser->lexer, "UNA_PARSE_EXPR");
        while (!(parser_expect(parser, TOK_SC))) {
            parser_consume(parser);
        }
        parser_consume(parser);
        return NULL;
    }

    

    if (expr->type != EXPR_FACTOR && (parser->cur->type == TOK_MINUS_MINUS || parser->cur->type == TOK_ADD_ADD ||
    parser->cur->type == TOK_MINUS || parser->cur->type == TOK_BANG)) {
        ASTN_Expression* fa_expr = malloc(sizeof(ASTN_Expression));
        
        expr->type = EXPR_FACTOR;
        expr->data.factor.type = FACTOR_UNARY_OP;
        expr->data.factor.data.unary_op.op = parser->cur->type;

        expr->data.factor.data.unary_op.expr = expr;

        parser_consume(parser);

        return fa_expr;
    }

    return expr;
}


AST_Node* parser_parse_expr(Parser* parser, uint8_t scopeOS) {
    AST_Node* node = ast_init(EXPR);    
    
    ASTN_Expression* expr = parser_parse_expression(parser, scopeOS);

    if (expr == NULL) {
        node->data.expr.type = EXPR_ERR;
        return node;
    }

    node->data.expr = *expr;

    return node;
}


ASTN_Parameter* parser_parse_parameter(Parser* parser) {
    ASTN_Parameter* param = calloc(1, sizeof(ASTN_Parameter));
    param->data_type_specifier = parser_parse_dt_spec(parser);

    if (param->data_type_specifier.data.prim == 0) {
        REPORT_ERROR(parser->lexer, "E_DTS_FN_PARAM", parser->cur->value);
        return  NULL;
    }

    if (!(parser_expect(parser, TOK_COLON))) {
        REPORT_ERROR(parser->lexer, "E_PARAM_COLON", parser->cur->value);
        return NULL;
    }   

    Symbol* sym = symbol_init((char*)parser->cur->value, SYMBOL_VARIABLE, parser->scope, parser->nest, parser->lexer->cl, parser->lexer->cc, 0);
    symtbl_insert(parser, sym, parser->cur->value);
    sym->data.data.param = param->data_type_specifier;
    param->identifier = sym->data.id;


    if (!(parser_expect(parser, TOK_IDEN))) {
        REPORT_ERROR(parser->lexer, "E_PARAM_IDEN", parser->cur->value);
        return NULL;
    }

    return param;
}

ASTN_Parameters* parser_parse_parameters(Parser* parser) {
    if (!(parser_expect(parser, TOK_LPAREN))) {
        REPORT_ERROR(parser->lexer, "E_FN_PARAMETERS", parser->cur->value);
        return NULL;
    }

    ASTN_Parameters* params = calloc(1, sizeof(ASTN_Parameters));
    params->size = 0;
    params->item_size = sizeof(ASTN_Parameter*);

    params->parameter = calloc(1, sizeof(ASTN_Parameter*));

    while (parser->cur->type != TOK_RPAREN) {
        params->parameter[params->size] = parser_parse_parameter(parser);

        if (!(parser_expect(parser, TOK_COMMA)) && (parser->cur->type != TOK_RPAREN)) {
            REPORT_ERROR(parser->lexer, "E_PARAMS_COMMA", parser->cur->value);
            return NULL;
        }         
        
        params->size++;
    }

    if (!(parser_expect(parser, TOK_RPAREN))) {
        REPORT_ERROR(parser->lexer, "E_PARAM_PAREN", parser->cur->value);
        return NULL;
    }
    
    return params;
}

ASTN_Module* parser_parse_module(Parser* parser) {
    ASTN_Module* mod = malloc(sizeof(ASTN_Module));
    if (!mod) {
        return NULL;
    }
    mod->module = NULL;
    mod->head_module = NULL;

    ASTN_Module* current_module = mod;

    while (parser->cur->type == TOK_IDEN || parser->cur->type == TOK_PERIOD) {
        if (parser->cur->type == TOK_PERIOD) {
            parser_consume(parser);
            continue;
        }

        char* token = parser->cur->value;
        if (!token) {
            free(mod);
            return NULL;
        }
        
        ASTN_Module* new_module = malloc(sizeof(ASTN_Module));
        if (!new_module) {
            free(mod);
            return NULL;
        }
        
        new_module->module = strdup(token);
        if (!new_module->module) {
            free(mod);
            free(new_module);
            return NULL;
        }
        
        new_module->head_module = current_module;
        current_module = new_module;

        parser_consume(parser);
    }
    
    symtbl_insert(parser, symbol_init(
        current_module->module, SYMBOL_MODULE, 0, parser->nest, parser->lexer->cl, parser->lexer->cc, 0
    ), current_module->module);

    return current_module;
}


AST_Node* parser_parse_import(Parser* parser) {
    parser_consume(parser);

    AST_Node* statement = ast_init(STMT);
    if (!statement) {
        return NULL;
    }

    ASTN_ImportDecl import;

    import.modules.items = NULL;
    import.modules.size = 0;
    import.modules.item_size = sizeof(ASTN_Module*);

    import.alias = NULL;
    import.source = NULL;

    while (parser->cur->type == TOK_IDEN || parser->cur->type == TOK_COMMA) {
        if (parser->cur->type == TOK_IDEN) {
            ASTN_Module* module = parser_parse_module(parser);

            import.modules.items = realloc(import.modules.items, (import.modules.size + 1) * sizeof(ASTN_Module*));
            import.modules.items[import.modules.size++] = module;
        } else if (parser->cur->type == TOK_COMMA) {
            if (import.modules.size < 1) {
                REPORT_ERROR(parser->lexer, "E_MODULE_BEF_COMMA", parser->cur->value);
                return NULL;
            }
            parser_consume(parser);
        }
    }

    statement->data.stm.type = STMT_IMPORT_DECL;

    if (parser_expect(parser, TOK_FROM)) {
        import.source = parser_parse_module(parser);
        for (size_t i = 0; i < import.modules.size; i++) {
            import.modules.items[i]->head_module = import.source;
        }
    }

    if (parser_expect(parser, TOK_FROM)) {
        if (import.source) {
            REPORT_ERROR(parser->lexer, "U_DB_SOURCE_DECL", import.source->module, parser->cur->value);
            return NULL;
        }
    }

    if (parser_expect(parser, TOK_AS)) {
        if (import.modules.size > 1) {
            REPORT_ERROR(parser->lexer, "U_MODULE_MULT_ALIAS", import.modules.size, parser->cur->value);
            return NULL;
        }
        import.alias = parser->cur->value;
        parser_consume(parser);
    }

    statement->data.stm.data.import_decl = import;
    
    parser_expect(parser, TOK_SC);

    return statement;
}

AST_Node* parser_parse_attr_decl(Parser* parser) {
    ASTN_AttributeDecl attr;
    attr.list = NULL;

    parser_consume(parser);

    if (parser->cur->type != TOK_IDEN) {
        REPORT_ERROR(parser->lexer, "E_FN_NAME", parser->cur->value);
        return NULL;
    }

    char* name = parser->cur->value;
    Symbol* symb = symbol_init((char*)parser->cur->value, SYMBOL_ATTR, 0, parser->nest, parser->lexer->cl, parser->lexer->cc, 0);
    parser_consume(parser);

    PES(parser);

    if (parser_expect(parser, TOK_EXT)) {
        ASTN_AttributeList* ext_list = calloc(1, sizeof(ASTN_AttributeList));
        ext_list->size = 0;
        ext_list->item_size = sizeof(AST_Node*);
        ext_list->items = calloc(1, sizeof(AST_Node*));

        if (!parser_parse_extend_attr(parser, ext_list)) {
            return NULL;
        }

        attr.list = ext_list;
    }

    if (!parser_expect(parser, TOK_FN_ARROW)) {
        return NULL;
    }

    if (!parser_expect(parser, TOK_LBRACE)) {
        return NULL;
    }

    int extendedn = -1;

    ASTN_AttributeList* list;
    if (attr.list == NULL) {
        list = calloc(1, sizeof(ASTN_AttributeList));
        list->size = 0;
        list->item_size = sizeof(AST_Node*);
        list->items = calloc(1, list->item_size);
        attr.list = list;
    } else {
        extendedn = attr.list->size;
        list = attr.list;
        list->items = realloc(list->items, (list->size + 1) * list->item_size);
    }

    while (parser->cur->type != TOK_RBRACE) {
        AST_Node* node = NULL;
        AST_Node* tmpnode = NULL;
        switch (parser->cur->type) {
            case TOK_FN:
                node = ast_init(STMT);
                node->data.stm.type = STMT_ATTR_UNIT;
                node->data.stm.data.attribute_unit.type = ATTR_FUNCTION;
                node->data.stm.data.attribute_unit.data.fn = parser_parse_function_decl(parser);
                node->data.stm.data.attribute_unit.scope = parser->scope;

                if (extendedn > -1) {
                    for (size_t i = 0; i < extendedn; i++) {
                        if (list->items[i]->data.stm.data.attribute_unit.data.fn->data.stm.data.function_decl.identifier == node->data.stm.data.attribute_unit.data.fn->data.stm.data.function_decl.identifier) {
                            list->items[i]->data.stm.data.attribute_unit.data.fn = node->data.stm.data.attribute_unit.data.fn;
                            break;
                        }
                    }
                }

                list->items = realloc(list->items, (list->size + 1) * list->item_size);
                list->items[list->size++] = node;

                break;
            case TOK_VAR:
            case TOK_MUT:
            case TOK_CONST:
                tmpnode = ast_init(STMT);
                tmpnode->data.stm.type = STMT_VARIABLE_DECL;
                tmpnode->data.stm.data.variable_decl = parser_parse_var_decl(parser, 0);

                if (!parser_expect(parser, TOK_SC)) {
                    REPORT_ERROR(parser->lexer, "E_SC");
                    return NULL;
                }


                node = ast_init(STMT);
                node->data.stm.type = STMT_ATTR_UNIT;
                node->data.stm.data.attribute_unit.type = ATTR_VARIABLE;
                node->data.stm.data.attribute_unit.data.var = tmpnode;
                node->data.stm.data.attribute_unit.scope = parser->scope;

                list->items = realloc(list->items, (list->size + 1) * list->item_size);
                list->items[list->size++] = node;

                break;
            case TOK_IDEN:
                for (size_t i = 0; i < extendedn; i++) {
                    if (list->items[i]->data.stm.data.attribute_unit.data.var->data.stm.data.variable_decl.iden.sg == symtbl_hash(parser->cur->value, list->items[i]->data.stm.data.attribute_unit.scope)) {
                        parser_consume(parser);

                        if (!parser_expect(parser, TOK_EQ)) {
                            REPORT_ERROR(parser->lexer, "E_MOD_AFRATTR");
                            return NULL;
                        }

                        list->items[i]->data.stm.data.attribute_unit.data.var->data.stm.data.variable_decl.expr = parser_parse_expr(parser, 0);

                        if (!parser_expect(parser, TOK_SC)) {
                            REPORT_ERROR(parser->lexer, "E_SC");
                            return NULL;
                        }

                        break;
                    }
                }
                

                break;
            default:
                REPORT_ERROR(parser->lexer, "E_UNEXPECTED_TOKEN", parser->cur->value);
                free(list->items);
                free(list);
                return NULL;
        }
    }

    parser_consume(parser);

    attr.list = list;

    AST_Node* node = ast_init(STMT);
    node->data.stm.type = STMT_ATTR_DECL;
    node->data.stm.data.attribute_decl = attr;


    symb->data.data.attr = attr;
    symtbl_insert(parser, symb, name);

    return node;
}

ASTN_VariableDecl parser_parse_var_decl(Parser* parser, uint8_t scopeOS) {
    ASTN_VariableDecl var;
    var.storage = -1;


    if (!(parser->cur->type == TOK_VAR || parser->cur->type == TOK_CONST || parser->cur->type == TOK_MUT)) {
        REPORT_ERROR(parser->lexer, "E_ACCSPEC_VAR_DECL");
        return var;
    }

    var.storage = parser->cur->type;
    parser_consume(parser);

    bool has_dts = false, has_acc = false;
    var.mem = 8;

    while (parser->cur->type != TOK_COLON && (!(has_dts && has_acc))) {
        if (!has_dts) {
            if (parser_expect(parser, TOK_LPAREN)) {
                parser_parse_compatibilities(parser);
                // memory
                has_dts = true;
                continue;
            }

            ASTN_DataTypeSpecifier dts = parser_parse_dt_spec(parser);
            if (dts.data.prim != 0) {
                var.data_type_specifier = dts;
                var.mem = parser_mem_for(&var.data_type_specifier);
                has_dts = true;
                continue;
            }
        }
        
        if (!has_acc && (parser->cur->type == TOK_PUB || parser->cur->type == TOK_PRIV || parser->cur->type == TOK_GLOB)) {
            var.access = parser->cur->type;
            has_acc = true;
            parser_consume(parser);
            continue;
        }

        break;
    }

    if (!parser_expect(parser, TOK_COLON)) {
        REPORT_ERROR(parser->lexer, "E_COLON_VAR_DECL");
        var.storage = -1;
        return var;
    }


    int* identifiers = calloc(1, sizeof(int));
    size_t size = 0;

    while (parser->cur->type == TOK_IDEN) {
        identifiers = realloc(identifiers, (size + 1) * sizeof(int));

        Symbol* symb = symbol_init((char*)parser->cur->value, SYMBOL_VARIABLE, parser->scope, parser->nest, parser->lexer->cl, parser->lexer->cc, 0);
        symb->data.data.var = var;
        symtbl_insert(parser, symb, parser->cur->value);

        parser_consume(parser);

        identifiers[size++] = symb->data.id;
        
        if (parser->cur->type == TOK_COMMA) {
            parser_consume(parser);
        } else {
            break;
        }
    }

    if (size == 0) {
        REPORT_ERROR(parser->lexer, "E_IDENTIFIER_VAR_DECL");
        free(identifiers);
        var.storage = -1;
        return var;
    }

    if (size == 1) {
        var.iden.mult.size = 0;
        var.iden.sg = identifiers[0];
        free(identifiers);
    } else {
        var.iden.mult.items = identifiers;
        var.iden.mult.size = size;
    }


    if (parser->cur->type == TOK_SC) {
        var.expr = NULL;
        return var;
    } 


    if (parser->cur->type == TOK_EQ) {
        parser_consume(parser);

        if (size > 1) {
            var.iden.mult.expr = malloc(size * sizeof(AST_Node));
            
            AST_Node* pn = parser_parse_expr(parser, scopeOS);

            if (pn->data.expr.type == -1) {
                var.storage = -1;
                return var;
            }

    
            var.iden.mult.expr[0] = pn;

            size_t i = 1;
            
            while (parser->cur->type == TOK_COMMA) {
                parser_consume(parser);



                AST_Node* n = parser_parse_expr(parser, scopeOS);
    
                if (n->data.expr.type == -1) {
                    var.storage = -1;
                    return var;
                }

                if (i > size - 1) {
                    REPORT_ERROR(parser->lexer, "U_EXTRA_VAL_ASGN");
                    var.storage = -1;
                    return var;
                } 

                var.iden.mult.expr[i] = n;
                i++;
            }

            if (i == 1) {
                for (size_t di = i; di < size; di++) {
                    var.iden.mult.expr[di] = var.iden.mult.expr[0]; 
                }
            }


            if (i < size) {
                REPORT_ERROR(parser->lexer, "E_EQ_VAR_EXPR", size, i, size);
            } 

            return var;
        }
        
        if (parser->cur->type == TOK_LBRACE) {
            parser_consume(parser);

            var.expr = malloc(sizeof(AST_Node));
            var.expr->data.expr.data.literal.type = TOK_FN_ARROW;
            var.expr->data.expr.data.literal.value.array.items = malloc(sizeof(AST_Node));

            if (parser_expect(parser, TOK_RBRACE)) {
                return var;
            }

            var.expr->data.expr.data.literal.value.array.size = 0;
            var.expr->data.expr.data.literal.value.array.items
                [var.expr->data.expr.data.literal.value.array.size]
                = parser_parse_expr(parser, scopeOS);

            while (parser->cur->type == TOK_COMMA) {
                parser_consume(parser);

                var.expr->data.expr.data.literal.value.array.size++;

                var.expr->data.expr.data.literal.value.array.items =
                    realloc(var.expr->data.expr.data.literal.value.array.items,
                        (
                        sizeof(AST_Node) * 
                        var.expr->data.expr.data.literal.value.array.size
                        )
                    );

                var.expr->data.expr.data.literal.value.array.items
                    [var.expr->data.expr.data.literal.value.array.size]
                    = parser_parse_expr(parser, scopeOS);    
            }

            

            if (!parser_expect(parser, TOK_RBRACE)) {
                REPORT_ERROR(parser->lexer, "E_RBRACE");
                var.storage = -1;
                return var;
            }

            return var;
        }

        var.expr = parser_parse_expr(parser, scopeOS);

        if (var.expr->data.expr.type == -1) {
            var.storage = -1;
            return var;
        }
    }

    return var;
}

ASTN_AssignmentStm parser_parse_assgn(Parser* parser, uint8_t scopeOS) {
    ASTN_AssignmentStm assgn;
    
    Symbol* symb = symtbl_lookup(parser->tbl, parser->cur->value, parser->scope, scopeOS, parser->recent_root);

    if (symb == NULL || symb->data.type != SYMBOL_VARIABLE) {
        assgn.sg.id = 0;
        return assgn;
    }

    parser_consume(parser);

    assgn.mult.ids = calloc(1, sizeof(int));
    assgn.mult.size = 1;


    while (parser->cur->type == TOK_COMMA) {
        parser_consume(parser);

        Symbol* symb2 = symtbl_lookup(parser->tbl, parser->cur->value, parser->scope, scopeOS, parser->recent_root);

        if (symb2 == NULL || symb2->data.type != SYMBOL_VARIABLE) {
            assgn.sg.id = 0;
            REPORT_ERROR(parser->lexer, "E_VAR_IN_ASSGN");
            return assgn;
        }

        assgn.mult.size += 1;
        assgn.mult.ids = realloc(assgn.mult.ids, assgn.mult.size * sizeof(int));
        assgn.mult.ids[assgn.mult.size - 1] = symb2->data.id;   
    }

    if (parser->cur->type == TOK_ADD_EQ || parser->cur->type == TOK_MINUS_EQ || parser->cur->type == TOK_SLASH_EQ || parser->cur->type == TOK_ASTK_EQ || parser->cur->type == TOK_PERC_EQ || parser->cur->type == TOK_EQ) {
        assgn.op = parser->cur->type;
        parser_consume(parser);
    }

    if (assgn.mult.size > 1 && assgn.op != TOK_EQ) {
        assgn.sg.id = 0;
        REPORT_ERROR(parser->lexer, "U_OPAS_TOMULT");
        return assgn;
    }


    if (assgn.mult.size == 1) {
        assgn.sg.id = symb->data.id;

        AST_Node* n2 =  parser_parse_expr(parser, scopeOS);
        if (n2->data.expr.type == -1) {
            assgn.sg.id = 0;
            REPORT_ERROR(parser->lexer, "E_PROP_EQ");
            return assgn;
        }

        assgn.sg.expr = n2;

        return assgn;
    }

    assgn.mult.exprs = malloc(assgn.mult.size * sizeof(AST_Node));
    
    AST_Node* pn = parser_parse_expr(parser, scopeOS);

    if (pn->data.expr.type == -1) {
        assgn.sg.id = 0;
        REPORT_ERROR(parser->lexer, "E_PROP_EQ");
        return assgn;
    }

    assgn.mult.exprs[0] = pn;

    size_t i = 1;

    while (parser->cur->type == TOK_COMMA) {
        parser_consume(parser);

        AST_Node* n = parser_parse_expr(parser, scopeOS);

        if (n->data.expr.type == -1) {
            assgn.sg.id = 0;
            REPORT_ERROR(parser->lexer, "E_PROP_EQ");
            return assgn;
        }

        if (i > assgn.mult.size - 1) {
            REPORT_ERROR(parser->lexer, "U_EXTRA_VAL_ASGN");
            assgn.sg.id = 0;
            return assgn;
        } 

        assgn.mult.exprs[i] = n;
        i++;
    }

    if (i == 1) {
        for (size_t di = i; di < assgn.mult.size; di++) {
            assgn.mult.exprs[di] = assgn.mult.exprs[0]; 
        }
    }


    if (i < assgn.mult.size) {
        REPORT_ERROR(parser->lexer, "E_EQ_VAR_EXPR", assgn.mult.size, i, assgn.mult.size);
    }    

    return assgn;
}



AST_Node* parser_parse_function_decl(Parser* parser) {        
    if (!(parser_expect(parser, TOK_FN))) {
        return NULL;
    }

    char* name = strdup(parser->cur->value);

    if (parser->cur->type != TOK_IDEN) {
        REPORT_ERROR(parser->lexer, "E_FN_NAME", parser->cur->value);
        return NULL;
    }

    parser_consume(parser);

    if (!(parser_expect(parser, TOK_FN_ARROW))) {
        REPORT_ERROR(parser->lexer, "E_FN_ARROW", parser->cur->value);
        return NULL;
    }

    Symbol* symb = symbol_init(name, SYMBOL_FUNCTION, 0, parser->nest, parser->lexer->cl, parser->lexer->cc, 0);

    PES(parser);
    parser->recent_root = parser->scope;
    PRN(parser);

    AST_Node* node = ast_init(STMT);
    node->data.stm.type = STMT_FUNCTION_DECL;
    node->data.stm.data.function_decl.parameters = parser_parse_parameters(parser);
    if (node->data.stm.data.function_decl.parameters == NULL) {
        return NULL;
    }


    if (parser->cur->type == TOK_SC) {
        parser_consume(parser);
        node->data.stm.data.function_decl.statements = NULL;
        
        node->data.stm.data.function_decl.identifier = symb->data.id;
        symb->data.data.fn = node->data.stm.data.function_decl;

        symtbl_insert(parser, symb, name);
        
        return node;
    }

    parser_expect(parser, TOK_LBRACE);


    node->data.stm.data.function_decl.statements = parser_parse_statements(parser, 0);
    if (node->data.stm.data.function_decl.statements == NULL) {
        return NULL;
    }

    if (!parser_expect(parser, TOK_RBRACE)) {
        REPORT_ERROR(parser->lexer, "E_RBRACE");
        return NULL;
    }

    node->data.stm.data.function_decl.identifier = symb->data.id;
    symb->data.data.fn = node->data.stm.data.function_decl;

    
    symtbl_insert(parser, symb, name);

    free(name);

    return node;
}

ASTN_StructMemberDecl parser_parse_struct_mem(Parser* parser) {
    ASTN_StructMemberDecl stm;
    stm.storage = -1;


    if (!(parser->cur->type == TOK_VAR || parser->cur->type == TOK_CONST || parser->cur->type == TOK_MUT)) {
        REPORT_ERROR(parser->lexer, "E_ACCSPEC_VAR_DECL");
        return stm;
    }

    stm.storage = parser->cur->type;
    parser_consume(parser);

    ASTN_DataTypeSpecifier dts = parser_parse_dt_spec(parser);
    if (dts.data.prim != 0) {
        stm.data_type_specifier = dts;
    }

    if (!parser_expect(parser, TOK_COLON)) {
        REPORT_ERROR(parser->lexer, "E_COLON_VAR_DECL");
        stm.storage = -1;
        return stm;
    }

    if (parser->cur->type != TOK_IDEN) {
        REPORT_ERROR(parser->lexer, "E_IDEN_DECL");
        stm.storage = -1;
        return stm;
    }

    Symbol* symb = symbol_init(parser->cur->value, SYMBOL_VARIABLE, parser->scope, parser->nest, parser->lexer->cl, parser->lexer->cc, 0);
    symtbl_insert(parser, symb, parser->cur->value);
    parser_consume(parser);
    
    stm.identifier = symb->data.id;

    if (!parser_expect(parser, TOK_SC)) {
        REPORT_ERROR(parser->lexer, "E_SC");
        stm.storage = -1;
        return stm;
    }

    return stm;
}

AST_Node* parser_parse_struct_decl(Parser* parser) {
    ASTN_StructDecl stm;
    size_t sz = 0;

    parser_consume(parser);

    if (parser->cur->type != TOK_IDEN) {
        REPORT_ERROR(parser->lexer, "E_IDEN_DECL");
        return NULL;
    }

    AST_Node* node = ast_init(STMT);
    node->data.stm.type = STMT_STRUCT_DECL;

    char* name = parser->cur->value;
    Symbol* symb = symbol_init(name, SYMBOL_STRUCT, 0, parser->nest, parser->lexer->cl, parser->lexer->cc, 0);
    parser_consume(parser);

    PES(parser);

    stm.identifier = symb->data.id;

    if (parser_expect(parser, TOK_SC)) {
        node->data.stm.data.struct_decl = stm;
        symb->data.data.stru = stm;
        symtbl_insert(parser, symb, name);

        return node;
    }

    if (!parser_expect(parser, TOK_LBRACE)) {
        REPORT_ERROR(parser->lexer, "E_LBRACE");
        return NULL;
    }

    stm.members.size = 0;
    stm.members.item_size = sizeof(ASTN_StructMemberDecl);
    stm.members.items = calloc(1, sizeof(ASTN_StructMemberDecl));

    while (parser->cur->type != TOK_RBRACE) {
        ASTN_StructMemberDecl mem = parser_parse_struct_mem(parser);
        if (mem.storage == -1) {
            return NULL;
        }

        sz += parser_mem_for(&mem.data_type_specifier);

        stm.members.items = realloc(stm.members.items, (stm.members.size + 1) * stm.members.item_size);
        stm.members.items[stm.members.size++] = mem;
    }

    parser_consume(parser);

    node->data.stm.data.struct_decl = stm;
    symb->data.data.stru = stm;
    symb->data.ty_size = sz;


    symtbl_insert(parser, symb, name);

    return node;
}




bool parser_parse_extend_attr(Parser* parser, ASTN_AttributeList* list) {
    while (parser->cur->type != TOK_FN_ARROW && parser->cur->type != TOK_SC) {
        bool is_class = false;
        Symbol* symb = NULL;

        if (parser->cur->type != TOK_ATTR && parser->cur->type != TOK_IDEN) {
            REPORT_ERROR(parser->lexer, "E_ATTRS_AF_EXT", parser->cur->value);
            return false;
        }

        if (parser->cur->type == TOK_IDEN) {
            symb = symtbl_lookup(parser->tbl, parser->cur->value, 0, 0, parser->recent_root);

            if (!symb) {
                REPORT_ERROR(parser->lexer, "E_VALID_ATTR", parser->cur->value);
                return false;
            }

            if (symb->data.type == SYMBOL_CLASS) {
                is_class = true;
            }
        }

        if (parser->cur->type != TOK_IDEN) {
            if (!parser_expect(parser, TOK_ATTR)) {
                REPORT_ERROR(parser->lexer, "E_VALID_ATTR", parser->cur->value);
                return false;
            }

            is_class = false;
        }

        while (true) {
            if (parser->cur->type == TOK_IDEN) {
                break;
            }

            parser_consume(parser);

            if (!parser_expect(parser, TOK_PERIOD)) {
                break;
            }
        }


        symb = symtbl_lookup(parser->tbl, parser->cur->value, 0, 0, parser->recent_root);

        parser_consume(parser);

        if (!symb) {
            REPORT_ERROR(parser->lexer, "U_REF_UDEV", parser->cur->value);
            return false;
        }


        if (is_class) {
            for (size_t i = 0; i < symb->data.data.clas.attributes->size; i++) {
                list->items = realloc(list->items, (list->size + 1) * sizeof(AST_Node*));
                AST_Node* itm = symb->data.data.clas.attributes->items[i];
                itm->data.stm.data.attribute_unit.re_scope = parser->scope;
                list->items[list->size] = itm;
                list->size++;
            }
        } else if (symb->data.type == SYMBOL_ATTR) {
            for (size_t i = 0; i < symb->data.data.attr.list->size; i++) {
                list->items = realloc(list->items, (list->size + 1) * sizeof(AST_Node*));
                AST_Node* itm = symb->data.data.attr.list->items[i];
                itm->data.stm.data.attribute_unit.re_scope = parser->scope;
                list->items[list->size] = itm;
                list->size++;
            }
        }

        if (parser_expect(parser, TOK_COMMA)) {
            continue;
        } else if (parser->cur->type == TOK_FN_ARROW || parser->cur->type == TOK_SC) {
            break;
        } else {
            REPORT_ERROR(parser->lexer, "E_UNEXPECTED_TOKEN", parser->cur->value);
            return false;
        }
    }

    return true;
}


AST_Node* parser_parse_class_decl(Parser* parser) {
    ASTN_ClassDecl stm;
    size_t sz = 0;
    stm.attributes = NULL;

    parser_consume(parser);    

    if (parser->cur->type != TOK_IDEN) {
        REPORT_ERROR(parser->lexer, "E_CLASS_NAME", parser->cur->value);
        return NULL;
    }

    Symbol* symb = symbol_init((const char*)parser->cur->value, SYMBOL_CLASS, 0, parser->nest, parser->lexer->cl, parser->lexer->cc, 0);
    stm.identifier = symb->data.id;
    char* iden = parser->cur->value;

    parser_consume(parser);

    PES(parser);

    if (parser_expect(parser, TOK_EXT)) {
        ASTN_AttributeList* ext_list = calloc(1, sizeof(ASTN_AttributeList));
        ext_list->size = 0;
        ext_list->item_size = sizeof(AST_Node*);
        ext_list->items = calloc(1, sizeof(AST_Node*));



        if (!parser_parse_extend_attr(parser, ext_list)) {
            return NULL;
        }

        stm.attributes = ext_list;
    }



    AST_Node* node = ast_init(STMT);
    node->data.stm.type = STMT_CLASS_DECL;

    if (parser->cur->type == TOK_SC) {
        node->data.stm.data.class_decl = stm;
        symb->data.data.clas = stm;

        symtbl_insert(parser, symb, iden);
        parser_consume(parser);

        return NULL;
    }


    if (!parser_expect(parser, TOK_FN_ARROW)) {
        return NULL;
    }

    if (!parser_expect(parser, TOK_LBRACE)) {
        return NULL;
    }


    int extendedn = -1;

    ASTN_AttributeList* list;
    if (stm.attributes == NULL) {
        list = calloc(1, sizeof(ASTN_AttributeList));
        list->size = 0;
        list->item_size = sizeof(AST_Node*);
        list->items = calloc(1, list->item_size);
        stm.attributes = list;
    } else {
        extendedn = stm.attributes->size;
        list = stm.attributes;
        list->items = realloc(list->items, (list->size + 1) * list->item_size);
    }

    while (parser->cur->type != TOK_RBRACE) {
        AST_Node* node = NULL;
        AST_Node* tmpnode = NULL;
        switch (parser->cur->type) {
            case TOK_FN:
                node = ast_init(STMT);
                node->data.stm.type = STMT_ATTR_UNIT;
                node->data.stm.data.attribute_unit.type = ATTR_FUNCTION;
                node->data.stm.data.attribute_unit.data.fn = parser_parse_function_decl(parser);
                node->data.stm.data.attribute_unit.scope = parser->scope;

                if (extendedn > -1) {
                    for (size_t i = 0; i < extendedn; i++) {
                        if (list->items[i]->data.stm.data.attribute_unit.data.fn->data.stm.data.function_decl.identifier == node->data.stm.data.attribute_unit.data.fn->data.stm.data.function_decl.identifier) {
                            list->items[i]->data.stm.data.attribute_unit.data.fn = node->data.stm.data.attribute_unit.data.fn;
                            break;
                        }
                    }
                }

                list->items = realloc(list->items, (list->size + 1) * list->item_size);
                list->items[list->size++] = node;

                

                break;
            case TOK_VAR:
            case TOK_MUT:
            case TOK_CONST:
                tmpnode = ast_init(STMT);
                tmpnode->data.stm.type = STMT_VARIABLE_DECL;
                tmpnode->data.stm.data.variable_decl = parser_parse_var_decl(parser, 0);
                
                if (!parser_expect(parser, TOK_SC)) {
                    REPORT_ERROR(parser->lexer, "E_SC");
                    return NULL;
                }

                node = ast_init(STMT);
                node->data.stm.type = STMT_ATTR_UNIT;
                node->data.stm.data.attribute_unit.type = ATTR_VARIABLE;
                node->data.stm.data.attribute_unit.data.var = tmpnode;
                node->data.stm.data.attribute_unit.scope = parser->scope;

                sz += node->data.stm.data.attribute_unit.data.var->data.stm.data.variable_decl.mem;


                list->items = realloc(list->items, (list->size + 1) * list->item_size);
                list->items[list->size++] = node;

                break;
            case TOK_IDEN:
                for (size_t i = 0; i < extendedn; i++) {
                    if (list->items[i]->data.stm.data.attribute_unit.data.var->data.stm.data.variable_decl.iden.sg == symtbl_hash(parser->cur->value, list->items[i]->data.stm.data.attribute_unit.scope)) {
                        parser_consume(parser);

                        if (!parser_expect(parser, TOK_EQ)) {
                            REPORT_ERROR(parser->lexer, "E_MOD_AFRATTR");
                            return NULL;
                        }

                        list->items[i]->data.stm.data.attribute_unit.data.var->data.stm.data.variable_decl.expr = parser_parse_expr(parser, 0);

                        if (!parser_expect(parser, TOK_SC)) {
                            REPORT_ERROR(parser->lexer, "E_SC");
                            return NULL;
                        }

                        break;
                    }
                }
                

                break;
            default:
                REPORT_ERROR(parser->lexer, "E_UNEXPECTED_TOKEN", parser->cur->value);
                free(list->items);
                free(list);
                return NULL;
        }
    }


    parser_consume(parser);

    stm.attributes = list;
    node->data.stm.data.class_decl = stm;
    symb->data.data.clas = stm;
    symb->data.ty_size = sz;
    

    symtbl_insert(parser, symb, iden);

    return node;
}

AST_Node* parser_parse_err_decl(Parser* parser) {
    ASTN_ErrDecl stm;

    parser_consume(parser);

    if (parser->cur->type != TOK_IDEN) {
        REPORT_ERROR(parser->lexer, "E_IDEN_DECL");
        return NULL;
    }

    AST_Node* node = ast_init(STMT);
    node->data.stm.type = STMT_ERR_DECL;

    char* name = parser->cur->value;
    Symbol* symb = symbol_init(parser->cur->value, SYMBOL_ERR, 0, parser->nest, parser->lexer->cl, parser->lexer->cc, 0);
    parser_consume(parser);

    stm.identifier = symb->data.id;

    if (parser_expect(parser, TOK_SC)) {
        node->data.stm.data.err_decl = stm;
        symb->data.data.err = stm;
        symtbl_insert(parser, symb, name);

        return node;
    }

    if (!parser_expect(parser, TOK_LBRACE)) {
        REPORT_ERROR(parser->lexer, "E_LBRACE");
        return NULL;
    }

    PES(parser);

    stm.members.size = 0;
    stm.members.dtss = calloc(1, sizeof(ASTN_DataTypeSpecifier));
    stm.members.identifiers = calloc(1, sizeof(uint32_t));

    while (parser->cur->type != TOK_RBRACE) {
        ASTN_DataTypeSpecifier dts = parser_parse_dt_spec(parser);
        
        if (!parser_expect(parser, TOK_COLON)) {
            REPORT_ERROR(parser->lexer, "E_COLON_VAR_DECL");
            return NULL;
        }

        if (parser->cur->type != TOK_IDEN) {
            REPORT_ERROR(parser->lexer, "E_IDEN_DECL");
            return NULL;
        }

        Symbol* symb = symbol_init(parser->cur->value, SYMBOL_VARIABLE, parser->scope, parser->nest, parser->lexer->cl, parser->lexer->cc, 0);
        symtbl_insert(parser, symb, parser->cur->value);

        parser_consume(parser);

        stm.members.identifiers = realloc(stm.members.identifiers, (stm.members.size + 1) * sizeof(uint8_t));
        stm.members.identifiers[stm.members.size] = symb->data.id; 

        stm.members.dtss = realloc(stm.members.dtss, (stm.members.size + 1) * sizeof(ASTN_DataTypeSpecifier));
        stm.members.dtss[stm.members.size] = dts;

        stm.members.size++;

        if (!parser_expect(parser, TOK_COMMA)) {
            break;
        }
    }

    parser_consume(parser);
    node->data.stm.data.err_decl = stm;
    symb->data.data.err = stm;
    
    symtbl_insert(parser, symb, name);

    return node;
}


AST_Node* parser_parse_enum_decl(Parser* parser) {
    ASTN_EnumDecl stm;
    parser_consume(parser);

    if (parser->cur->type != TOK_IDEN) {
        REPORT_ERROR(parser->lexer, "E_IDEN_DECL");
        return NULL;
    }

    AST_Node* node = ast_init(STMT);
    node->data.stm.type = STMT_ENUM_DECL;


    char* name = parser->cur->value;
    Symbol* symb = symbol_init(name, SYMBOL_ENUM, 0, parser->nest, parser->lexer->cl, parser->lexer->cc, 0);
    parser_consume(parser);

    stm.identifier = symb->data.id;

    if (!parser_expect(parser, TOK_LBRACE)) {
        REPORT_ERROR(parser->lexer, "E_LBRACE");
        return NULL;
    }
    
    stm.members.size = 0;
    stm.members.items = calloc(1, sizeof(uint32_t));

    while (parser->cur->type != TOK_RBRACE) {
        stm.members.items = realloc(stm.members.items, (stm.members.size + 1) * sizeof(uint32_t));

        if (parser->cur->type == TOK_IDEN) {
            Symbol* symb2 = symbol_init(parser->cur->value, SYMBOL_VARIABLE, 0, parser->nest, parser->lexer->cl, parser->lexer->cc, 0);
            stm.members.items[stm.members.size] = symb2->data.id;
            symtbl_insert(parser, symb2, parser->cur->value);
            parser_consume(parser);
        }
        
        if (!(parser_expect(parser, TOK_COMMA)) && (parser->cur->type != TOK_RBRACE)) {
            REPORT_ERROR(parser->lexer, "E_PARAMS_COMMA", parser->cur->value);
            return NULL;
        }
        stm.members.size++;     
    }

    parser_consume(parser);

    printf("stm.members.size: %zu\n", stm.members.size);

    node->data.stm.data.enum_decl = stm;
    symb->data.data.enu = stm;
    symb->data.ty_size = stm.members.size * 4;

    symtbl_insert(parser, symb, name);

    return node;
}


ASTN_ConditionalStm parser_parse_cond_stm(Parser* parser, uint8_t scopeOS) {
    ASTN_ConditionalStm stm;
    parser_consume(parser);

    __uint128_t temp = parser->scope;

    if (!parser_expect(parser, TOK_LPAREN)) {
        REPORT_ERROR(parser->lexer, "E_LPAREN");
        return stm;
    }

    stm.if_condition = parser_parse_expr(parser, scopeOS);

    if (!parser_expect(parser, TOK_RPAREN)) {
        REPORT_ERROR(parser->lexer, "E_RPAREN");
        return stm;
    }

    if (!parser_expect(parser, TOK_LBRACE)) {
        REPORT_ERROR(parser->lexer, "E_LBRACE");
        return stm;
    }

    PES(parser);
    scopeOS += 1;

    stm.if_statements = parser_parse_statements(parser, scopeOS);

    if (!parser_expect(parser, TOK_RBRACE)) {
        REPORT_ERROR(parser->lexer, "E_RBRACE");
        return stm;
    }

    if (parser->cur->type == TOK_ELSE) {
        parser_consume(parser);
        if (!parser_expect(parser, TOK_LBRACE)) {
            REPORT_ERROR(parser->lexer, "E_LBRACE");
            return stm;
        }

        PES(parser);
        scopeOS += 1;

        stm.else_statements = parser_parse_statements(parser, scopeOS);

        if (!parser_expect(parser, TOK_RBRACE)) {
            REPORT_ERROR(parser->lexer, "E_RBRACE");
            return stm;
        }
    }

    stm.elif_branches.conditions = calloc(1, sizeof(AST_Node));
    stm.elif_branches.statements = calloc(1, sizeof(ASTN_Statements));
    stm.elif_branches.size = 0;


    while (parser->cur->type == TOK_ELIF) {
        parser_consume(parser);
        
        if (!parser_expect(parser, TOK_LPAREN)) {
            REPORT_ERROR(parser->lexer, "E_LPAREN");
            return stm;
        }

        AST_Node* elif_condition = parser_parse_expr(parser, scopeOS);


        if (!parser_expect(parser, TOK_RPAREN)) {
            REPORT_ERROR(parser->lexer, "E_RPAREN");
            return stm;
        }

        if (!parser_expect(parser, TOK_LBRACE)) {
            REPORT_ERROR(parser->lexer, "E_LBRACE");
            return stm;
        }
        
        PES(parser);
        scopeOS += 1;

        ASTN_Statements* elif_statements = parser_parse_statements(parser, scopeOS);

        if (!parser_expect(parser, TOK_RBRACE)) {
            REPORT_ERROR(parser->lexer, "E_RBRACE");
            return stm;
        }

        stm.elif_branches.conditions = realloc(stm.elif_branches.conditions, (stm.elif_branches.size + 1) * sizeof(AST_Node*));
        stm.elif_branches.statements = realloc(stm.elif_branches.statements, (stm.elif_branches.size + 1) * sizeof(ASTN_Statements*));
        
        stm.elif_branches.conditions[stm.elif_branches.size] = elif_condition;
        stm.elif_branches.statements[stm.elif_branches.size] = elif_statements;
        stm.elif_branches.size++;
    }


    if (parser->cur->type == TOK_ELSE) {
        parser_consume(parser);

        if (!parser_expect(parser, TOK_LBRACE)) {
            REPORT_ERROR(parser->lexer, "E_LBRACE");
            return stm;
        }

        PES(parser);
        scopeOS += 1;

        stm.else_statements = parser_parse_statements(parser, scopeOS);

        if (!parser_expect(parser, TOK_RBRACE)) {
            REPORT_ERROR(parser->lexer, "E_RBRACE");
            return stm;
        }

    }

    parser->scope = temp;

    return stm;
}

ASTN_ForStm parser_parse_for_stm(Parser* parser, uint8_t scopeOS) {
    ASTN_ForStm stm;
    parser_consume(parser);

    if (!parser_expect(parser, TOK_LPAREN)) {
        REPORT_ERROR(parser->lexer, "E_LPAREN");
        return stm;
    }


    uint64_t temp = parser->scope;
    uint64_t temp2;


    PES(parser);
    scopeOS += 1;

    if (parser->cur->type == TOK_VAR || parser->cur->type == TOK_MUT || parser->cur->type == TOK_CONST) {
        stm.initial_expr.decl = parser_parse_var_decl(parser, scopeOS);
    } else {
        stm.initial_expr.norm = parser_parse_expr(parser, scopeOS);
    }


    if (parser_expect(parser, TOK_COLON)) {
        temp2 = parser->scope;
        parser->scope = temp;
        stm.data.range_expr =  parser_parse_expr(parser, scopeOS);
        parser->scope = temp2;
    } else if (parser_expect(parser, TOK_COLON_COLON)) {
        temp2 = parser->scope;
        parser->scope = temp;
        stm.data.iter_expr =  parser_parse_expr(parser, scopeOS);
        parser->scope = temp2;
    } else if (parser_expect(parser, TOK_SC)) {
        stm.data.generic.condition_expr = parser_parse_expr(parser, scopeOS);

        if (!parser_expect(parser, TOK_SC)) {
            REPORT_ERROR(parser->lexer, "E_SC");
            return stm;
        }

    
        stm.data.generic.next_expr = parser_parse_expr(parser, scopeOS);
    } else {
        REPORT_ERROR(parser->lexer, "E_PROP_FOR_LOOP");
        return stm;
    }



    if (!parser_expect(parser, TOK_RPAREN)) {
        REPORT_ERROR(parser->lexer, "E_RPAREN");
        return stm;
    }

    if (!parser_expect(parser, TOK_LBRACE)) {
        REPORT_ERROR(parser->lexer, "E_LBRACE");
        return stm;
    }
    
    stm.statements = parser_parse_statements(parser, scopeOS);

    if (!parser_expect(parser, TOK_RBRACE)) {
        REPORT_ERROR(parser->lexer, "E_RBRACE");
        return stm;
    }

    parser->scope = temp;

    return stm;
}

ASTN_SwitchStm parser_parse_switch_stm(Parser* parser, uint8_t scopeOS) {
    ASTN_SwitchStm stm;
    parser_consume(parser);

    __uint128_t temp = parser->scope;

    if (!parser_expect(parser, TOK_LPAREN)) {
        REPORT_ERROR(parser->lexer, "E_LPAREN");
        return stm;
    }

    AST_Node* expr = parser_parse_expr(parser, scopeOS);

    if (expr->data.expr.type != EXPR_IDENTIFIER && expr->data.expr.type != EXPR_LITERAL && expr->data.expr.type != EXPR_FUNCTION_CALL) {
        REPORT_ERROR(parser->lexer, "E_SWABLSTM");
        return stm;
    }

    stm.condition_expr = expr;

    if (!parser_expect(parser, TOK_RPAREN)) {
        REPORT_ERROR(parser->lexer, "E_RPAREN");
        return stm;
    }

    if (!parser_expect(parser, TOK_LBRACE)) {
        REPORT_ERROR(parser->lexer, "E_LBRACE");
        return stm;
    }

    PES(parser);
    scopeOS += 1;

    stm.clauses.value = calloc(1, sizeof(AST_Node*));
    stm.clauses.statements = calloc(1, sizeof(ASTN_Statements*));
    stm.clauses.size = 0;

    bool default_case_found = false;

    while (parser->cur->type != TOK_RBRACE) {
        if (parser->cur->type == TOK_CASE) {
            parser_consume(parser);

            AST_Node* expr = parser_parse_expr(parser, scopeOS);

            if (expr->data.expr.type != EXPR_IDENTIFIER && expr->data.expr.type != EXPR_LITERAL && expr->data.expr.type != EXPR_FUNCTION_CALL) {
                REPORT_ERROR(parser->lexer, "E_SWABLSTM");
                return stm;
            }

            if (!parser_expect(parser, TOK_COLON)) {
                REPORT_ERROR(parser->lexer, "E_COLON");
                return stm;
            }
            
            ASTN_Statements* stms = parser_parse_statements(parser, scopeOS);

            stm.clauses.statements = realloc(stm.clauses.statements, (stm.clauses.size + 1) * sizeof(ASTN_Statements*));
            stm.clauses.value = realloc(stm.clauses.value, (stm.clauses.size + 1) * sizeof(AST_Node*));
            stm.clauses.value[stm.clauses.size] = expr;
            stm.clauses.statements[stm.clauses.size] = stms;

            stm.clauses.size++;
        } else if (parser->cur->type == TOK_DEFAULT) {
            if (default_case_found) {
                REPORT_ERROR(parser->lexer, "E_MULTIPLE_DEFAULT");
                return stm;
            }
            default_case_found = true;

            parser_consume(parser);

            if (!parser_expect(parser, TOK_COLON)) {
                REPORT_ERROR(parser->lexer, "E_COLON");
                return stm;
            }

            ASTN_Statements* stms = parser_parse_statements(parser, scopeOS);

            stm.default_stms = stms;
        }
    }

    parser->scope = temp;
    parser_consume(parser);

    return stm;
}



ASTN_TryStm parser_parse_try_stm(Parser* parser, uint8_t scopeOS) {
    ASTN_TryStm stm;
    parser_consume(parser);

    __uint128_t temp = parser->scope;

    if (!parser_expect(parser, TOK_LBRACE)) {
        REPORT_ERROR(parser->lexer, "E_LBRACE");
        return stm;
    }

    PES(parser);
    scopeOS += 1;

    stm.try_statements = parser_parse_statements(parser, scopeOS);

    if (!parser_expect(parser, TOK_RBRACE)) {
        REPORT_ERROR(parser->lexer, "E_RBRACE");
        return stm;
    }

    if (parser->cur->type == TOK_FINALLY) {
        parser_consume(parser);
        if (!parser_expect(parser, TOK_LBRACE)) {
            REPORT_ERROR(parser->lexer, "E_LBRACE");
            return stm;
        }

        PES(parser);
        scopeOS += 1;

        stm.finally_statements = parser_parse_statements(parser, scopeOS);

        if (!parser_expect(parser, TOK_RBRACE)) {
            REPORT_ERROR(parser->lexer, "E_RBRACE");
            return stm;
        }
    }

    stm.except_branches.errors = calloc(1, sizeof(uint32_t));
    stm.except_branches.statements = calloc(1, sizeof(ASTN_Statements));
    stm.except_branches.size = 0;


    while (parser->cur->type == TOK_EXCEPT) {
        parser_consume(parser);
        
        if (parser->cur->type != TOK_IDEN) {
            REPORT_ERROR(parser->lexer, "E_EXCPET_IDEN");
            return stm;
        }

        Symbol* sym = symtbl_lookup(parser->tbl, parser->cur->value, 0, 0, parser->recent_root);
        if (!sym || sym->data.type != SYMBOL_ERR) {
            REPORT_ERROR(parser->lexer, "E_PROP_ERRTT");
            return stm;
        }

        parser_consume(parser);

        if (!parser_expect(parser, TOK_LBRACE)) {
            REPORT_ERROR(parser->lexer, "E_LBRACE");
            return stm;
        }
        
        PES(parser);
        scopeOS += 1;

        ASTN_Statements* stms = parser_parse_statements(parser, scopeOS);

        if (!parser_expect(parser, TOK_RBRACE)) {
            REPORT_ERROR(parser->lexer, "E_RBRACE");
            return stm;
        }

        stm.except_branches.errors = realloc(stm.except_branches.errors, (stm.except_branches.size + 1) * sizeof(uint32_t));
        stm.except_branches.statements = realloc(stm.except_branches.statements, (stm.except_branches.size + 1) * sizeof(ASTN_Statements*));

        stm.except_branches.errors[stm.except_branches.size] = sym->data.id;
        stm.except_branches.statements[stm.except_branches.size] = stms;
        stm.except_branches.size++;
    }


    if (parser->cur->type == TOK_FINALLY) {
        parser_consume(parser);
        if (!parser_expect(parser, TOK_LBRACE)) {
            REPORT_ERROR(parser->lexer, "E_LBRACE");
            return stm;
        }

        PES(parser);
        scopeOS += 1;

        stm.finally_statements = parser_parse_statements(parser, scopeOS);

        if (!parser_expect(parser, TOK_RBRACE)) {
            REPORT_ERROR(parser->lexer, "E_RBRACE");
            return stm;
        }
    }


    parser->scope = temp;

    return stm;
}

ASTN_WhileStm parser_parse_while_stm(Parser* parser, uint8_t scopeOS) {
    ASTN_WhileStm stm;
    parser_consume(parser);

    __uint128_t temp = parser->scope;

    if (!parser_expect(parser, TOK_LPAREN)) {
        REPORT_ERROR(parser->lexer, "E_LPAREN");
        return stm;
    }

    stm.condition_expr = parser_parse_expr(parser, scopeOS);

    if (!parser_expect(parser, TOK_RPAREN)) {
        REPORT_ERROR(parser->lexer, "E_RPAREN");
        return stm;
    }

    if (!parser_expect(parser, TOK_LBRACE)) {
        REPORT_ERROR(parser->lexer, "E_LBRACE");
        return stm;
    }

    PES(parser);
    scopeOS += 1;

    stm.statements = parser_parse_statements(parser, scopeOS);

    if (!parser_expect(parser, TOK_RBRACE)) {
        REPORT_ERROR(parser->lexer, "E_RBRACE");
        return stm;
    }

    parser->scope = temp;

    return stm;
}

ASTN_ReturnStm parser_parse_return_stm(Parser* parser, uint8_t scopeOS) {
    parser_consume(parser);
    ASTN_ReturnStm statement;

    statement.value.expr = parser_parse_expr(parser, scopeOS);

    if (parser->cur->type != TOK_COMMA) {
        return statement;    
    }

    statement.value.exprs->expr = calloc(2, sizeof(AST_Node*));
    statement.value.exprs->item_size = sizeof(AST_Node*);
    statement.value.exprs->size = 2;

    statement.value.exprs->expr[0] = statement.value.expr;

    while (parser->cur->type == TOK_COMMA) {
        parser_consume(parser);

        statement.value.exprs->expr[statement.value.exprs->size - 1] = parser_parse_expr(parser, scopeOS);
        statement.value.exprs->size += 1;
        statement.value.exprs->expr = realloc(statement.value.exprs->expr, statement.value.exprs->size  * statement.value.exprs->item_size);
    }

    return statement;
}

ASTN_ThrowStm parser_parse_throw_stm(Parser* parser, uint8_t scopeOS) {
    parser_consume(parser);
    ASTN_ThrowStm statement;

    if (parser->cur->type != TOK_IDEN) {
        REPORT_ERROR(parser->lexer, "E_THROW_IDEN");
        return statement;
    }

    Symbol* sym = symtbl_lookup(parser->tbl, parser->cur->value, 0, 0, parser->recent_root);
    if (!sym || sym->data.type != SYMBOL_ERR) {
        REPORT_ERROR(parser->lexer, "E_PROP_ERRTT");
        return statement;
    }

    parser_consume(parser);
    
    statement.iden = sym->data.id;

    if (parser->cur->type == TOK_SC) {
        return statement;
    }


    if (!(parser_expect(parser, TOK_LPAREN))) {
        return statement;
    } 


    ASTN_CallParams* params = calloc(1, sizeof(ASTN_CallParams));

    params->size = 0;
    params->item_size = sizeof(AST_Node*);
    params->parameter = calloc(1, sizeof(AST_Node*));


    while (parser->cur->type != TOK_RPAREN) {
        params->parameter[params->size] = parser_parse_expr(parser, scopeOS);

        if (!(parser_expect(parser, TOK_COMMA)) && (parser->cur->type != TOK_RPAREN)) {
            REPORT_ERROR(parser->lexer, "E_PARAMS_COMMA", parser->cur->value);
            return statement;
        }         

        params->parameter = realloc(params->parameter, (params->size + 1) * sizeof(AST_Node));
        params->size++;
    }

    parser_consume(parser);

    statement.params = params;

    return statement;
}


ASTN_Statement parser_parse_statement(Parser* parser, uint8_t scopeOS) {
    // allow root scope look ups

    ASTN_Statement stm;
    stm.type = -1;

    switch (parser->cur->type) {
        case TOK_RETURN:
            stm.type = STMT_RETURN;
            stm.data.return_stm = parser_parse_return_stm(parser, scopeOS);
            break;
        case TOK_THROW:
            stm.type = STMT_THROW;
            stm.data.throw_stm = parser_parse_throw_stm(parser, scopeOS);
            break;
        case TOK_VAR:
        case TOK_CONST:
        case TOK_MUT:
            stm.type = STMT_VARIABLE_DECL;
            stm.data.variable_decl = parser_parse_var_decl(parser, scopeOS);
            break;
        case TOK_IDEN:
            stm.type = STMT_CALL;
            stm.data.call = parser_parse_call(parser, scopeOS);
            if (stm.data.call.identifier != 0) { break; }
            
            stm.type = STMT_ASSGN;
            stm.data.assgn = parser_parse_assgn(parser, scopeOS);
            if (stm.data.assgn.sg.id != 0) { break; }

            stm.type = STMT_EXPRESSION;
            stm.data.expression = *parser_parse_expression(parser, scopeOS);
            break;
        case TOK_IF:
            stm.type = STMT_CONDITIONAL;
            stm.data.conditional = parser_parse_cond_stm(parser, scopeOS);
            return stm; break;
        case TOK_WHILE:
            stm.type = STMT_WHILE_LOOP;
            stm.data.while_loop = parser_parse_while_stm(parser, scopeOS);
            return stm; break;
        case TOK_SWITCH:
            stm.type = STMT_SWITCH;
            stm.data.switch_stm = parser_parse_switch_stm(parser, scopeOS);
            return stm; break;
        case TOK_TRY:
            stm.type = STMT_TRY;
            stm.data.try_stm = parser_parse_try_stm(parser, scopeOS);
            return stm; break;
        case TOK_FOR:
            stm.type = STMT_FOR_LOOP;
            stm.data.for_loop = parser_parse_for_stm(parser, scopeOS);
            return stm; break;
        case TOK_BREAK:
            parser_consume(parser);
            stm.type = STMT_BREAK;
            stm.data.brak = true;
            break;
        case TOK_CONTINUE:
            parser_consume(parser);
            stm.type = STMT_CONTINUE;
            stm.data.cont = true;
            break;
        default:
            break;
    }

    if (!(parser_expect(parser, TOK_SC))) {
        REPORT_ERROR(parser->lexer, "E_SC_AFSTM");
        return stm;
    }

    return stm;
}

ASTN_Statements* parser_parse_statements(Parser* parser, uint8_t scopeOS) {
    ASTN_Statements* stms = malloc(sizeof(ASTN_Statements));
    stms->statement = NULL;
    stms->size = 0;
    stms->item_size = sizeof(ASTN_Statement*);

    while (parser->cur->type != TOK_EOF && parser->cur->type != TOK_RBRACE && parser->cur->type != TOK_CASE && parser->cur->type != TOK_DEFAULT) {
        AST_Node* node = ast_init(STMT);
        node->data.stm = parser_parse_statement(parser, scopeOS);
        
        if (node->data.stm.type != -1) {
            stms->statement = realloc(stms->statement, (stms->size + 1) * stms->item_size);
            stms->statement[stms->size] = node;
            stms->size++;
        }
    }

    return stms;
}


AST_Node* parser_parse_mep_decl(Parser* parser) {
    symtbl_insert(parser, symbol_init(
        (char*)"main", SYMBOL_MEP, 0, parser->nest, parser->lexer->cl, parser->lexer->cc, 0
    ), "main");

    parser_consume(parser);
    
    if (!(parser_expect(parser, TOK_FN))) {
        return NULL;
    }

    if (!(parser_expect_spec_value(parser, TOK_IDEN, "main"))) {
        REPORT_ERROR(parser->lexer, "E_MEP_NAME_MAIN", parser->cur->value);
        return NULL;
    }

    if (!(parser_expect(parser, TOK_FN_ARROW))) {
        REPORT_ERROR(parser->lexer, "E_FN_ARROW", parser->cur->value);
        return NULL;
    }

    AST_Node* node = ast_init(MEP);
    node->data.mep.parameters = parser_parse_parameters(parser);
    if (node->data.mep.parameters == NULL) {
        return NULL;
    }  

    parser_expect(parser, TOK_LBRACE);

    PES(parser);
    parser->recent_root = parser->scope;
    PRN(parser);

    node->data.mep.statements = parser_parse_statements(parser, 0);
    if (node->data.mep.statements == NULL) {
        return NULL;
    }

    return node;
}

size_t parser_mem_for(ASTN_DataTypeSpecifier* dts) {

    size_t sz = 0;

    switch (dts->data.prim) {
        case TOK_L_SSINT:  sz = 1;  break;  // int8_t
        case TOK_L_SINT:   sz = 2;  break;  // int16_t
        case TOK_L_INT:    sz = 4;  break;  // int32_t
        case TOK_L_LINT:   sz = 8;  break;  // int64_t
        case TOK_L_LLINT:  sz = 16; break;  // int128_t
        case TOK_L_SSUINT: sz = 1;  break;  // uint8_t
        case TOK_L_SUINT:  sz = 2;  break;  // uint16_t
        case TOK_L_UINT:   sz = 4;  break;  // uint32_t
        case TOK_L_LUINT:  sz = 8;  break;  // uint64_t
        case TOK_L_LLUINT: sz = 16; break;  // uint128_t
        case TOK_L_FLOAT:  sz = 4;  break;  // float
        case TOK_L_DOUBLE: sz = 8;  break;  // double
        case TOK_L_BOOL:   sz = 1;  break;  // bool
        case TOK_L_SIZE:   sz = sizeof(size_t); break; // machine specefic
        case TOK_L_CHAR: 
        case TOK_L_STRING: sz = 0;
        default: sz = -1; break;  
    }

    if (sz == -1) {
        sz = dts->data.cust_type;
    }

    if (dts->is_arr) {
        sz = dts->arr * sz;
    }

    return sz;
}

void symtbl_insert(Parser* parser, Symbol* symbol, char* raw_symb) {
    if (!parser->tbl) {
        exit(EXIT_FAILURE);
        return;
    }

    Symbol* checks = parser->tbl->symbol;
    while (checks != NULL) {
        if (checks->data.id == symbol->data.id) {
            REPORT_ERROR(parser->lexer, "U_REDEF", raw_symb, checks->data.decl_line, checks->data.decl_col);
            return; 
        }
        checks = checks->next;
    }

    if (parser->tbl->symbol == NULL) {
        parser->tbl->symbol = symbol;
        return;
    }



    Symbol* current = parser->tbl->symbol;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = symbol;
}
