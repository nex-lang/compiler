#include "parser.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

Parser* parser_init(char* filename) {
    Parser* parser = calloc(1, sizeof(Parser));

    parser->lexer = lexer_init(filename);
    parser->cur = lexer_next_token(parser->lexer);
    parser->tbl = symtbl_init();
    parser->tree = ast_init(ROOT);
    parser->root = parser->tree;

    parser->root_scope = 0;
    parser->scope = 0;
    parser->nest = 0;

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


    parser->cur = lexer_next_token(parser->lexer);
}

void parser_parse(Parser* parser) {
    while (parser->cur->type != TOK_EOF) {
        switch (parser->cur->type) {
            case TOK_IMPORT:
                parser->tree->right = parser_parse_import(parser);
                break;
            case TOK_COLON:
                parser->tree->right = parser_parse_mep_decl(parser);
                break;
            case TOK_FN:
                parser->tree->right = parser_parse_function_decl(parser);
                break;
            case TOK_IDEN:  
                parser->tree->right = parser_parse_expr(parser);
                break;
            case TOK_ATTR:
                parser->tree->right = parser_parse_attr_decl(parser);                
                break;
            case TOK_CLASS:                
                parser->tree->right = parser_parse_class_decl(parser);;
                break;
            case TOK_STRUCT:
                parser->tree->right = parser_parse_struct_decl(parser);;
                break;
            default:
                break;
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
            lit.value.int_.bit8 = (int8_t)strtol(parser->cur->value, &endptr, 10);
            break;
        case TOK_L_SINT:
            lit.value.int_.bit16 = (int16_t)strtol(parser->cur->value, &endptr, 10);
            break;
        case TOK_L_INT:
            lit.value.int_.bit32 = (int32_t)strtol(parser->cur->value, &endptr, 10);
            break;
        case TOK_L_LINT:
            lit.value.int_.bit64 = (int64_t)strtol(parser->cur->value, &endptr, 10);
            break;
        case TOK_L_LLINT:
            lit.value.int_.bit128 = (__int128_t)strtoll(parser->cur->value, &endptr, 10);
            break;
        case TOK_L_SSUINT:
            lit.value.uint.bit8 = (uint8_t)strtoul(parser->cur->value, &endptr, 10);
            break;
        case TOK_L_SUINT:
            lit.value.uint.bit16 = (uint16_t)strtoul(parser->cur->value, &endptr, 10);
            break;
        case TOK_L_UINT:
            lit.value.uint.bit32 = (uint32_t)strtoul(parser->cur->value, &endptr, 10);
            break;
        case TOK_L_LUINT:
            lit.value.uint.bit64 = (uint64_t)strtoull(parser->cur->value, &endptr, 10);
            break;
        case TOK_L_LLUINT:
            lit.value.uint.bit128 = (__uint128_t)strtoull(parser->cur->value, &endptr, 10);
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
        case TOK_L_BOOL:
            lit.value.boolean = strtol(parser->cur->value, &endptr, 10) != 0;
            break;
        case TOK_L_SIZE:
            lit.value.size = (size_t)strtoull(parser->cur->value, &endptr, 10);
            break;
        default: lit.type = -1; return lit; break;
    }


    parser_consume(parser);
    return lit;
}

ASTN_DataTypeSpecifier parser_parse_dt_spec(Parser* parser, bool expect_further) {
    ASTN_DataTypeSpecifier dts;
    dts.data.prim = 0;
    int int_dts = 0;


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

    if (int_dts > 0) {
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
        if (parser_expect(parser, TOK_RBRACK)) {
            dts.is_arr = true; return dts;
        }
        REPORT_ERROR(parser->lexer, "E_CLOSE_BRACK", parser->cur->value);
        dts.data.prim = 0;
        return dts;
    }

    if (!(expect_further)) {
        return dts;
    }

    return dts;
}

ASTN_Call parser_parse_call(Parser* parser) {
    ASTN_Call call;
    
    Symbol* symb = symtbl_lookup(parser->tbl, parser->cur->value, 0, 0);

    if (symb == NULL || symb->data.type != SYMBOL_FUNCTION) {
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
        params->parameter[params->size] = parser_parse_expr(parser);


        if (!(parser_expect(parser, TOK_COMMA)) && (parser->cur->type != TOK_RPAREN)) {
            REPORT_ERROR(parser->lexer, "E_PARAMS_COMMA", parser->cur->value);
            call.identifier = 0;
            return call;
        }         

        params->parameter = realloc(params->parameter, (params->size + 1) * sizeof(AST_Node));
        params->size += 1;
    }

    call.params = params;
    call.type = CALL_FN;

    parser_expect(parser, TOK_RPAREN);

    printf("[stack call req]: %i\n", call.identifier);

    if (call.params && call.params->parameter) {
        for (size_t i = 0; i < call.params->size; i++) {
            printf("arg [%zu]: %i\n", i, call.params->parameter[i]->data.expr.data.primary.data.literal.type);
        }
    }

    return call;
}


ASTN_PrimaryExpr parser_parse_prim_expr(Parser* parser) {
    ASTN_PrimaryExpr expr;
    expr.type = -1;

    ASTN_Literal lit = parser_parse_literal(parser);
    if (lit.type != -1) {
        expr.type = PRIMARY_LITERAL;
        expr.data.literal = lit;
        return expr;
    }

    if (parser->cur->type == TOK_IDEN) {
        Symbol* symb = symtbl_lookup(parser->tbl, parser->cur->value, 0, 0);
        if (symb) {
            if (symb->data.type == SYMBOL_FUNCTION || symb->data.type == SYMBOL_CLASS ||
                symb->data.type == SYMBOL_STRUCT) {
                expr.type = PRIMARY_CALL;
                expr.data.call = parser_parse_call(parser);
            } else if (symb->data.type == SYMBOL_MODULE) {
                expr.type = PRIMARY_IDENTIFIER;
                expr.data.identifier = symb->data.id;
                parser_consume(parser);
            }
        } else {
            Symbol* symb2 = symtbl_lookup(parser->tbl, parser->cur->value, parser->scope, 0);
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


ASTN_FactorExpr parser_parse_factor_expr(Parser* parser) {
    ASTN_FactorExpr expr;
    expr.type = -1;

    if (parser->cur->type == TOK_MINUS_MINUS || parser->cur->type == TOK_ADD_ADD ||
        parser->cur->type == TOK_MINUS || parser->cur->type == TOK_BANG) {
        expr.type = FACTOR_UNARY_OP;
        expr.data.unary_op.op = parser->cur->type;
        parser_consume(parser);
        expr.data.unary_op.expr = parser_parse_expression(parser);
        if (!expr.data.unary_op.expr) {
            return expr;
        }
    } else {
        expr.data.primary = parser_parse_prim_expr(parser);

        if (expr.data.primary.type == -1) {
            REPORT_ERROR(parser->lexer, "E_PROP_EXP", parser->cur->value);
            return expr;
        }
        expr.type = FACTOR_PRIMARY;
        if (parser->cur->type == TOK_MINUS_MINUS || parser->cur->type == TOK_ADD_ADD) {
            ASTN_FactorExpr post_expr;
            post_expr.type = FACTOR_UNARY_OP;
            post_expr.data.unary_op.op = parser->cur->type;
            parser_consume(parser);
            post_expr.data.unary_op.expr = parser_parse_expression(parser);
            if (!post_expr.data.unary_op.expr) {
                return expr;
            }
            return post_expr;
        }
    }

    return expr;
}


ASTN_TermExpr parser_parse_term_expr(Parser* parser) {
    ASTN_TermExpr expr;
    expr.type = -1;

    expr.data.factor = parser_parse_factor_expr(parser);
    if (expr.data.factor.type == -1) {
        return expr;
    }
    expr.type = TERM_FACTOR;

    while (parser->cur->type == TOK_ASTK_ASTK) {
        ASTN_TermExpr new_expr;
        new_expr.type = TERM_BINARY_OP;

        parser_consume(parser);

        new_expr.data.binary_op.left = malloc(sizeof(ASTN_Expression));
        if (!new_expr.data.binary_op.left) {
            return expr;
        }
        
        if (expr.type != -1) {
            new_expr.data.binary_op.left->type = EXPR_ADDITION;
            new_expr.data.binary_op.left->data.factor = expr.data.factor;
        } else {
            new_expr.data.binary_op.left = parser_parse_expression(parser);
        }

        new_expr.data.binary_op.op = TOK_ASTK_ASTK;

        parser_consume(parser);

        new_expr.data.binary_op.right = malloc(sizeof(ASTN_Expression));
        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.left);
            return expr;
        }

        new_expr.data.binary_op.right = parser_parse_expression(parser); 
        
        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.left);
            free(new_expr.data.binary_op.right);
            return expr;
        }

        expr.type = TERM_BINARY_OP;
        expr.data = new_expr.data;
    }

    return expr;
}

ASTN_MultiplicationExpr parser_parse_mult_expr(Parser* parser) {
    ASTN_MultiplicationExpr expr;
    expr.type = -1;

    expr.data.term = parser_parse_term_expr(parser);

    if (expr.data.term.type == -1) {
        return expr;
    }

    expr.type = MULTIPLICATION_TERM;

    while (parser->cur->type == TOK_ASTK || parser->cur->type == TOK_SLASH || parser->cur->type == TOK_PERC) {
        ASTN_MultiplicationExpr new_expr;
        new_expr.type = MULTIPLICATION_BINARY_OP;

        new_expr.data.binary_op.left = malloc(sizeof(ASTN_Expression));
        if (!new_expr.data.binary_op.left) {
            return expr;
        }
        
        if (expr.type != -1) {
            new_expr.data.binary_op.left->type = EXPR_ADDITION;
            new_expr.data.binary_op.left->data.term = expr.data.term;
        } else {
            new_expr.data.binary_op.left = parser_parse_expression(parser);
        }

        parser_consume(parser);

        new_expr.data.binary_op.right = malloc(sizeof(ASTN_Expression));
        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.left);
            return expr;
        }
        
        new_expr.data.binary_op.right = parser_parse_expression(parser);
        
        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.left);
            free(new_expr.data.binary_op.right);
            return expr;
        }

        expr = new_expr;

    }

    return expr;
}

ASTN_AdditionExpr parser_parse_add_expr(Parser* parser) {
    ASTN_AdditionExpr expr;
    expr.type = -1;

    expr.data.multiplication = parser_parse_mult_expr(parser);

    if (expr.data.multiplication.type == -1) {
        return expr;
    }

    expr.type = ADDITION_MULTIPLICATION;

    while (parser->cur->type == TOK_ADD || parser->cur->type == TOK_MINUS) {
        ASTN_AdditionExpr new_expr;
        new_expr.type = ADDITION_BINARY_OP;

        new_expr.data.binary_op.left = malloc(sizeof(ASTN_Expression));
        if (!new_expr.data.binary_op.left) {
            return expr;
        }
        
        if (expr.type != -1) {
            new_expr.data.binary_op.left->type = EXPR_ADDITION;
            new_expr.data.binary_op.left->data.multiplication = expr.data.multiplication;
        } else {
            new_expr.data.binary_op.left = parser_parse_expression(parser);
        }

        new_expr.data.binary_op.op = parser->cur->type;

        parser_consume(parser);

        new_expr.data.binary_op.right = malloc(sizeof(ASTN_Expression));
        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.left);
            return expr;
        }
        
        new_expr.data.binary_op.right = parser_parse_expression(parser);
        
        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.left);
            free(new_expr.data.binary_op.right);
            return expr;
        }

        expr = new_expr;
    }

    return expr;
}

ASTN_BitwiseExpr parser_parse_bitw_expr(Parser* parser) {
    ASTN_BitwiseExpr expr;
    expr.type = -1;

    expr.data.addition = parser_parse_add_expr(parser);

    if (expr.data.addition.type == -1) {
        return expr;
    }

    expr.type = BITWISE_ADDITION;

    while (parser->cur->type == TOK_AMPER || parser->cur->type == TOK_PIPE || parser->cur->type == TOK_GT_GT || parser->cur->type == TOK_LT_LT) {
        ASTN_BitwiseExpr new_expr;
        new_expr.type = BITWISE_BINARY_OP;

        new_expr.data.binary_op.left = malloc(sizeof(ASTN_Expression));
        if (!new_expr.data.binary_op.left) {
            return expr;
        }

        if (expr.type != -1) {
            new_expr.data.binary_op.left->type = EXPR_ADDITION;
            new_expr.data.binary_op.left->data.addition = expr.data.addition;
        } else {
            new_expr.data.binary_op.left = parser_parse_expression(parser);
        }

        new_expr.data.binary_op.op = parser->cur->type;

        parser_consume(parser);

        new_expr.data.binary_op.right = malloc(sizeof(ASTN_Expression));
        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.left);
            return expr;
        }
        
        new_expr.data.binary_op.right = parser_parse_expression(parser);
        
        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.left);
            free(new_expr.data.binary_op.right);
            return expr;
        }

        expr = new_expr;
    }

    return expr;
}

ASTN_ComparisonExpr parser_parse_comp_expr(Parser* parser) {
    ASTN_ComparisonExpr expr;
    expr.type = -1;

    expr.data.bitwise = parser_parse_bitw_expr(parser);

    if (expr.data.bitwise.type == -1) {
        return expr;
    }

    expr.type = COMPARISON_BITWISE;

    while (parser->cur->type == TOK_LT_EQ || parser->cur->type == TOK_GT_EQ || parser->cur->type == TOK_BANG_EQ || parser->cur->type == TOK_EQ_EQ || parser->cur->type == TOK_LT || parser->cur->type == TOK_GT) {
        ASTN_ComparisonExpr new_expr;
        new_expr.type = COMPARISON_BINARY_OP;

        new_expr.data.binary_op.left = malloc(sizeof(ASTN_Expression));
        if (!new_expr.data.binary_op.left)  {
            return expr;
        }

        if (expr.type != -1) {
            new_expr.data.binary_op.left->type = EXPR_BITWISE;
            new_expr.data.binary_op.left->data.bitwise = expr.data.bitwise;
        } else {
            new_expr.data.binary_op.left = parser_parse_expression(parser);
        }

        new_expr.data.binary_op.op = parser->cur->type;

        parser_consume(parser);

        new_expr.data.binary_op.right = malloc(sizeof(ASTN_Expression));
        
        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.left);
            return expr;
        }
        
        new_expr.data.binary_op.right = parser_parse_expression(parser);
        
        if (!new_expr.data.binary_op.right) {
            free(new_expr.data.binary_op.left);
            free(new_expr.data.binary_op.right);
            return expr;
        }

        expr = new_expr;
    }

    return expr;
}



ASTN_Expression* parser_parse_expression(Parser* parser) {
    ASTN_Expression* expr = malloc(sizeof(ASTN_Expression));
    if (expr == NULL) {
        return NULL;
    }

    expr->type = -1;
    bool expect_db_close;

    if (parser->cur->type == TOK_LPAREN) {
        parser_consume(parser);
        ASTN_Expression* nested_expr = parser_parse_expression(parser);
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

            ASTN_Expression* right_expr = parser_parse_expression(parser);
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

    ASTN_ComparisonExpr x = parser_parse_comp_expr(parser);

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
    }

    return expr;
}


AST_Node* parser_parse_expr(Parser* parser) {
    AST_Node* node = ast_init(EXPR);    
    node->data.expr = *parser_parse_expression(parser);

    return node;
}


ASTN_Parameter* parser_parse_parameter(Parser* parser) {
    ASTN_Parameter* param = calloc(1, sizeof(ASTN_Parameter));
    param->data_type_specifier = parser_parse_dt_spec(parser, false);

    if (param->data_type_specifier.data.prim == 0) {
        REPORT_ERROR(parser->lexer, "E_DTS_FN_PARAM", parser->cur->value);
        return  NULL;
    }

    if (!(parser_expect(parser, TOK_COLON))) {
        REPORT_ERROR(parser->lexer, "E_PARAM_COLON", parser->cur->value);
        return NULL;
    }   

    param->identifier = parser->cur->value;

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


        symtbl_insert(parser, symbol_init(
            (char*)params->parameter[params->size]->identifier, SYMBOL_VARIABLE, parser->scope, parser->nest, 0, 0, 0, 0, parser->lexer->cl, parser->lexer->cc
        ));

        if (!(parser_expect(parser, TOK_COMMA)) && (parser->cur->type != TOK_RPAREN)) {
            REPORT_ERROR(parser->lexer, "E_PARAMS_COMMA", parser->cur->value);
            return NULL;
        }         
        
        params->size += 1;
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
        current_module->module, SYMBOL_MODULE, 0, 0, 0, 0, 0, 0, parser->lexer->cl, parser->lexer->cc 
    ));

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

    Symbol* symb = symbol_init((char*)parser->cur->value, SYMBOL_ATTR, parser->root_scope, 0, 0, 0, 0, 0, parser->lexer->cl, parser->lexer->cc);
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
                tmpnode->data.stm.data.variable_decl = parser_parse_var_decl(parser);

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

                        list->items[i]->data.stm.data.attribute_unit.data.var->data.stm.data.variable_decl.expr = parser_parse_expr(parser);

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


    symb->data.data = node;
    symtbl_insert(parser, symb);

    return node;
}

// AST_Node* parser_parse_attr_decl(Parser* parser) {
//     ASTN_AttributeDecl attr;
//     attr.list = NULL;

//     parser_consume(parser);

//     if (parser->cur->type != TOK_IDEN) {
//         REPORT_ERROR(parser->lexer, "E_FN_NAME", parser->cur->value);
//         return NULL;
//     }

//     Symbol* symb = symbol_init((char*)parser->cur->value, SYMBOL_ATTR, parser->root_scope, 0, 0, 0, 0, 0, parser->lexer->cl, parser->lexer->cc);
//     parser_consume(parser);

//     PES(parser);


//     if (parser_expect(parser, TOK_EXT)) {
//         ASTN_AttributeList* ext_list = calloc(1, sizeof(ASTN_AttributeList));
//         ext_list->size = 0;
//         ext_list->item_size = sizeof(AST_Node*);
//         ext_list->items = calloc(1, sizeof(AST_Node*));

//         if (!parser_parse_extend_attr(parser, ext_list)) {
//             return NULL;
//         }

//         attr.list = ext_list;
//     }

//     if (!parser_expect(parser, TOK_FN_ARROW)) {
//         return NULL;
//     }

//     if (!parser_expect(parser, TOK_LBRACE)) {
//         return NULL;
//     }

//     int extendedn = -1;

//     ASTN_AttributeList* list;
//     if (attr.list == NULL) {
//         list = calloc(1, sizeof(ASTN_AttributeList));
//         list->size = 0;
//         list->item_size = sizeof(AST_Node*);
//         list->items = calloc(1, list->item_size);
//         attr.list = list;
//     } else {
//         extendedn = attr.list->size;
//         list = attr.list;
//         list->size = extendedn;
//         list->item_size = sizeof(AST_Node*);
//         list->items = realloc(list->items, (list->size + 1) * list->item_size);
//     }

//     while (parser->cur->type != TOK_RBRACE) {
//         AST_Node* node = NULL; 
//         AST_Node* tmpnode = NULL;
//         switch (parser->cur->type) {
//             case TOK_FN:
//                 node = ast_init(STMT);
//                 node->data.stm.type = STMT_ATTR_UNIT;
//                 node->data.stm.data.attribute_unit.type = ATTR_FUNCTION;
//                 node->data.stm.data.attribute_unit.data.fn = parser_parse_function_decl(parser);
                
//                 if (extendedn > -1) {
//                     for (size_t i = 0; i < extendedn; i++) {
//                         if (list->items[i]->data.stm.data.attribute_unit.data.fn->data.stm.data.function_decl.identifier == node->data.stm.data.attribute_unit.data.fn->data.stm.data.function_decl.identifier) {
//                             list->items[i]->data.stm.data.attribute_unit.data.fn = node->data.stm.data.attribute_unit.data.fn;
//                             break;
//                         }
//                     }
//                 }

//                 list->items = realloc(list->items, (list->size + 1) * list->item_size);
//                 list->items[list->size++] = node;

//                 break;
//             case TOK_VAR:
//             case TOK_MUT:
//             case TOK_CONST:
//                 tmpnode = ast_init(STMT);
//                 tmpnode->data.stm.type = STMT_VARIABLE_DECL;
//                 tmpnode->data.stm.data.variable_decl = parser_parse_var_decl(parser);


//                 node = ast_init(STMT);
//                 node->data.stm.type = STMT_ATTR_UNIT;
//                 node->data.stm.data.attribute_unit.type = ATTR_VARIABLE;
//                 node->data.stm.data.attribute_unit.data.var = tmpnode;

//                 list->items = realloc(list->items, (list->size + 1) * list->item_size);
//                 list->items[list->size++] = node;


//                 break;
//             case TOK_IDEN:
//                 for (size_t i = 0; i < extendedn; i++) {
//                     /*
//                     FIND A WAY TO: know the scope of "a" and where it was imported from, to check with symb table

//                     attr X => {
//                         var: a;
//                     }

//                     attr Y ext attr.X => {
//                         var: b;
//                     }

//                     attr Z ext attr.Y => {
//                         a = 10; ()
//                     }
                    
//                     */
//                     if (list->items[i]->data.stm.data.attribute_unit.data.var->data.stm.data.variable_decl.iden.sg == symtbl_hash(parser->cur->value, )) {
//                         list->items[i]->data.stm.data.attribute_unit.data.var->data.stm.data.variable_decl.expr->data.
//                         break;
//                     }
                    
//                 }

//                 break;
//             default:
//                 free(list->items);
//                 free(list);
//         }
//     }

//     attr.list = list;

//     AST_Node* node = ast_init(STMT);
//     node->data.stm.type = STMT_ATTR_DECL;
//     node->data.stm.data.attribute_decl = attr;

//     symb->data.data = node;
//     symtbl_insert(parser, symb);

//     printf("SCOPE: %i\n", attr.list->scope);

//     for (int i = 0; i < attr.list->size; i++) {
//         printf("INDEX: %i TYPE: %i \n", i, attr.list->items[i]->data.stm.data.attribute_unit.type);
    
//     }   
    
//     return node;
// }

ASTN_VariableDecl parser_parse_var_decl(Parser* parser) {
    ASTN_VariableDecl var;
    var.storage = -1;


    if (!(parser->cur->type == TOK_VAR || parser->cur->type == TOK_CONST || parser->cur->type == TOK_MUT)) {
        REPORT_ERROR(parser->lexer, "E_ACCSPEC_VAR_DECL");
        return var;
    }

    var.storage = parser->cur->type;
    parser_consume(parser);

    bool has_dts = false, has_acc = false;

    while (parser->cur->type != TOK_COLON && (!(has_dts && has_acc))) {
        if (!has_dts) {
            ASTN_DataTypeSpecifier dts = parser_parse_dt_spec(parser, false);
            if (dts.data.prim != 0) {
                var.data_type_specifier = dts;
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


        Symbol* symb = symbol_init((char*)parser->cur->value, SYMBOL_VARIABLE, parser->scope, 0, 0, 0, 0, 0, parser->lexer->cl, parser->lexer->cc);
        symtbl_insert(parser, symb);

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
        var.iden.sg = identifiers[0];
        free(identifiers);
    } else {
        var.iden.mult.items = identifiers;
        var.iden.mult.size = size;
    }


    if (parser_expect(parser, TOK_SC)) {
        return var;
    } 


    if (parser->cur->type == TOK_EQ) {
        parser_consume(parser);


        if (size > 1) {
            REPORT_ERROR(parser->lexer, "U_MULT_VAL_ASSGN");
            var.storage = -1;
            return var;
        }
        
        var.expr = parser_parse_expr(parser);
    
        if (var.expr->data.expr.type == -1) {
            var.storage = -1;
            return var;
        }
    }

    return var;
}



AST_Node* parser_parse_function_decl(Parser* parser) {        
    if (!(parser_expect(parser, TOK_FN))) {
        return NULL;
    }

    Token* name_tok = parser->cur;
    if (name_tok->type != TOK_IDEN) {
        REPORT_ERROR(parser->lexer, "E_FN_NAME", parser->cur->value);
        return NULL;
    }

    parser_consume(parser);

    if (!(parser_expect(parser, TOK_FN_ARROW))) {
        REPORT_ERROR(parser->lexer, "E_FN_ARROW", parser->cur->value);
        return NULL;
    }

    Symbol* symb = symbol_init((char*)name_tok->value, SYMBOL_FUNCTION, parser->scope, 0, 0, 0, 0, 0, parser->lexer->cl, parser->lexer->cc);


    PES(parser);
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
        symb->data.data = node;

        symtbl_insert(parser, symb);
        
        return node;
    }

    parser_expect(parser, TOK_LBRACE);


    node->data.stm.data.function_decl.statements = parser_parse_statements(parser);
    if (node->data.stm.data.function_decl.statements == NULL) {
        return NULL;
    }

    if (!parser_expect(parser, TOK_RBRACE)) {
        REPORT_ERROR(parser->lexer, "E_RBRACE");
        return NULL;
    }

    node->data.stm.data.function_decl.identifier = symb->data.id;
    symb->data.data = node;
    
    symtbl_insert(parser, symb);


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

    ASTN_DataTypeSpecifier dts = parser_parse_dt_spec(parser, false);
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

    Symbol* symb = symbol_init(parser->cur->value, SYMBOL_VARIABLE, parser->scope, 0, 0, 0, 0, 0, parser->lexer->cl, parser->lexer->cc);
    symtbl_insert(parser, symb);
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

    parser_consume(parser);

    if (parser->cur->type != TOK_IDEN) {
        REPORT_ERROR(parser->lexer, "E_IDEN_DECL");
        return NULL;
    }

    AST_Node* node = ast_init(STMT);
    node->data.stm.type = STMT_STRUCT_DECL;

    Symbol* symb = symbol_init(parser->cur->value, SYMBOL_STRUCT, 0, 0, 0, 0, 0, 0, parser->lexer->cl, parser->lexer->cc);
    parser_consume(parser);

    stm.identifier = symb->data.id;

    if (parser_expect(parser, TOK_SC)) {
        node->data.stm.data.struct_decl = stm;
        symb->data.data = node;
        symtbl_insert(parser, symb);

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

        stm.members.items = realloc(stm.members.items, (stm.members.size + 1) * stm.members.item_size);
        stm.members.items[stm.members.size++] = mem;
    }

    parser_consume(parser);

    node->data.stm.data.struct_decl = stm;
    symb->data.data = node;
    
    symtbl_insert(parser, symb);

    return node;
}




bool parser_parse_extend_attr(Parser* parser, ASTN_AttributeList* list) {
    while (parser->cur->type != TOK_FN_ARROW && parser->cur->type != TOK_SC) {
        char* iden = "\0";
        bool is_class = false;
        __uint128_t scope;
        Symbol* symb;

        if (parser->cur->type != TOK_ATTR && parser->cur->type != TOK_IDEN) {
            REPORT_ERROR(parser->lexer, "E_ATTRS_AF_EXT", parser->cur->value);
            return false;
        }

        if (parser->cur->type == TOK_IDEN) {
            symb = symtbl_lookup(parser->tbl, parser->cur->value, 0, 0);

            if (!symb) {
                REPORT_ERROR(parser->lexer, "E_VALID_ATTR", parser->cur->value);
                return false;
            }

            if (symb->data.type == SYMBOL_CLASS) {
                scope = symb->data.scope + 1;
                is_class = true;
            } else if (symb->data.type != SYMBOL_ATTR) {
                REPORT_ERROR(parser->lexer, "E_VALID_ATTR", parser->cur->value);
                return false;
            }
        }

        if (parser->cur->type != TOK_IDEN) {
            parser_consume(parser);
            parser_consume(parser);
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



        iden = parser->cur->value;

        symb = symtbl_lookup(parser->tbl, parser->cur->value, 0, 0);

        parser_consume(parser);

        if (!symb) {
            REPORT_ERROR(parser->lexer, "U_REF_UDEV", parser->cur->value);
            return false;
        }


        if (is_class) {
            for (size_t i = 0; i < symb->data.data->data.stm.data.class_decl.attributes->size; i++) {
                list->items = realloc(list->items, (list->size + 1) * sizeof(AST_Node*));
                AST_Node* itm = symb->data.data->data.stm.data.class_decl.attributes->items[i];
                itm->data.stm.data.attribute_unit.re_scope = parser->scope;
                list->items[list->size] = itm;
                list->size++;
            }
        } else if (symb->data.type == SYMBOL_ATTR) {
            for (size_t i = 0; i < symb->data.data->data.stm.data.attribute_decl.list->size; i++) {
                list->items = realloc(list->items, (list->size + 1) * sizeof(AST_Node*));
                AST_Node* itm = symb->data.data->data.stm.data.attribute_decl.list->items[i];
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
    stm.attributes = NULL;

    parser_consume(parser);    

    if (parser->cur->type != TOK_IDEN) {
        REPORT_ERROR(parser->lexer, "E_CLASS_NAME", parser->cur->value);
        return NULL;
    }

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

    Symbol* symb = symbol_init(iden, SYMBOL_CLASS, parser->root_scope, 0, 0, 0, 0, 0, parser->lexer->cl, parser->lexer->cc);
    stm.identifier = symb->data.id;

    AST_Node* node = ast_init(STMT);
    node->data.stm.type = STMT_CLASS_DECL;

    if (parser->cur->type == TOK_SC) {
        node->data.stm.data.class_decl = stm;
        symb->data.data = node;

        symtbl_insert(parser, symb);
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
                tmpnode->data.stm.data.variable_decl = parser_parse_var_decl(parser);

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

                        list->items[i]->data.stm.data.attribute_unit.data.var->data.stm.data.variable_decl.expr = parser_parse_expr(parser);

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
    symb->data.data = node;

    printf("finally: %zu\n", node->data.stm.data.class_decl.attributes->size);

    symtbl_insert(parser, symb);

    return node;
}

// AST_Node* parser_parse_class_decl(Parser* parser) {
    // if (!(parser_expect(parser, TOK_CLASS))) {
    //     return NULL;
    // }

    // Token* name_tok = parser->cur;
    // if (name_tok->type != TOK_IDEN) {
    //     REPORT_ERROR(parser->lexer, "E_CLASS_NAME", parser->cur->value);
    //     return NULL;
    // }

    // parser_consume(parser);

    // AST_Node* node = ast_init(STMT);
    // node->data.stm.type = STMT_CLASS_DECL;

    // PES(parser);
    // PRN(parser);

    // if (parser_expect(parser, TOK_EXT)) {
    //     ASTN_AttributeList* list = calloc(1, sizeof(ASTN_AttributeList));

    //     list->size = 0;
    //     list->item_size = sizeof(AST_Node*);
    //     list->items = calloc(1, sizeof(AST_Node*));

    //     while (!(parser_expect(parser, TOK_LBRACE))) {
    //         if (parser->cur->type == TOK_ATTR) {
    //             if (!parser_parse_extend_attr(parser, list)) {
    //                 return NULL;
    //             }
    //         } else {
    //             REPORT_ERROR(parser->lexer, "E_ATTR_KEYWORD_EXPECTED", parser->cur->value);
    //             return NULL;
    //         }
    //     }
    // }

//     // node->data.stm.data.function_decl.parameters = parser_parse_parameters(parser);
//     // if (node->data.stm.data.function_decl.parameters == NULL) {
//     //     return NULL;
//     // }

//     // parser_expect(parser, TOK_LBRACE);

//     // node->data.stm.data.function_decl.statements = parser_parse_statements(parser);
//     // if (node->data.stm.data.function_decl.statements == NULL) {
//     //     return NULL;
//     // }

//     // Symbol* symb = symbol_init((char*)name_tok->value, SYMBOL_FUNCTION, parser->root_scope, 0, 0, 0, 0, 0, parser->lexer->cl, parser->lexer->cc);
    
//     // node->data.stm.data.function_decl.identifier = symb->data.id;
//     // symb->data.data = node;
    
//     // symtbl_insert(parser, symb);

//     return node;
// }

AST_Node* parser_parse_enum_decl(Parser* parser) {
    (void)0;
}


ASTN_ConditionalStm parser_parse_cond_stm(Parser* parser) {
    (void)0;
}

ASTN_ForStm parser_parse_for_stm(Parser* parser) {
    (void)0;
}

ASTN_SwitchStm parser_parse_switch_stm(Parser* parser) {
    (void)0;
}

ASTN_CaseClause parser_parse_case_clause(Parser* parser) {
    (void)0;
}

ASTN_TryStm parser_parse_try_stm(Parser* parser) {
    (void)0;
}

ASTN_WhileStm parser_parse_while_stm(Parser* parser) {
    (void)0;
}

ASTN_ReturnStm parser_parse_return_stm(Parser* parser) {
    parser_consume(parser);
    ASTN_ReturnStm statement;

    statement.expr = parser_parse_expr(parser);

    return statement;    
}



ASTN_Statement parser_parse_statement(Parser* parser) {
    ASTN_Statement stm;
    stm.type = -1;

    switch (parser->cur->type) {
        case TOK_RETURN:
            stm.type = STMT_RETURN;
            stm.data.return_stm = parser_parse_return_stm(parser);
            break;
        case TOK_VAR:
        case TOK_CONST:
        case TOK_MUT:
            stm.type = STMT_VARIABLE_DECL;
            stm.data.variable_decl = parser_parse_var_decl(parser);
            break;
        case TOK_IDEN:
            stm.type = STMT_CALL;
            stm.data.call = parser_parse_call(parser);
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

ASTN_Statements* parser_parse_statements(Parser* parser) {
    ASTN_Statements* stms = malloc(sizeof(ASTN_Statements));
    stms->statement = NULL;
    stms->size = 0;
    stms->item_size = sizeof(ASTN_Statement*);

    while (parser->cur->type != TOK_EOF && parser->cur->type != TOK_RBRACE) {
        AST_Node* node = ast_init(STMT);
        node->data.stm = parser_parse_statement(parser);
        
        if (node->data.stm.type != -1) {
            stms->statement = realloc(stms->statement, (stms->size + 1) * stms->item_size);
            stms->statement[stms->size] = node;
            stms->size += 1;
        }
    }

    return stms;
}


AST_Node* parser_parse_mep_decl(Parser* parser) {
    symtbl_insert(parser, symbol_init(
        (char*)"MEP", SYMBOL_MEP, parser->scope, parser->nest, 0, 0, 0, 0, parser->lexer->cl, parser->lexer->cc
    ));

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

    node->data.mep.statements = parser_parse_statements(parser);
    if (node->data.mep.statements == NULL) {
        return NULL;
    }

    return node;
}

void symtbl_insert(Parser* parser, Symbol* symbol) {
    if (!parser->tbl) {
        exit(EXIT_FAILURE);
        return;
    }

    Symbol* checks = parser->tbl->symbol;
    while (checks != NULL) {
        if (checks->data.id == symbol->data.id) {
            REPORT_ERROR(parser->lexer, "U_ATO_DPRED");
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
