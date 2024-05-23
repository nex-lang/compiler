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

    return (Parser*)parser;
}

void parser_free(Parser* parser) {
    /*
    De-initializes provided parser
    */

    if (!parser) {
        return;
        exit(EXIT_FAILURE);
    }

    lexer_free(parser->lexer);
    token_free(parser->cur);
    ast_free(parser->tree);

    Symbol* cur = parser->tbl->symbol;
    while (cur != NULL) {
        printf("ID: %d\n", cur->data.id);
        printf("Scope: %u\n", cur->data.scope);
        printf("Nest: %u\n", cur->data.nest);
        printf("Memory Type: %d\n", cur->data.mem_type);
        printf("Memory Mod: %d\n", cur->data.mem_mod);
        printf("Memory Storage: %d\n", cur->data.mem_sto);
        printf("Access Type: %d\n", cur->data.access_type);
        printf("Type: ");
        switch (cur->data.type) {
            case SYMBOL_VARIABLE:
                printf("Variable\n");
                break;
            case SYMBOL_FUNCTION:
                printf("Function\n");
                break;
            case SYMBOL_STRUCT:
                printf("Struct\n");
                break;
            case SYMBOL_CLASS:
                printf("Class\n");
                break;
            case SYMBOL_ENUM:
                printf("Enum\n");
                break;
            case SYMBOL_MEP:
                printf("MEP\n");
            case SYMBOL_MODULE:
                printf("Module\n");
            default:
                printf("Unknown\n");
                break;
        }
        printf("Declaration Line: %d\n", cur->data.decl_line);
        printf("Declaration Column: %d\n", cur->data.decl_col);
        printf("---------------------------------------------\n");

        cur = cur->next;
    }


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
        return; // parsing complete
    }

    parser->cur = lexer_next_token(parser->lexer);
}

void parser_parse(Parser* parser) {
    while (parser->cur->type != TOK_EOF) {
        switch (parser->cur->type) {
            // case TOK_IMPORT:
            //     parser->tree->right = parser_parse_import(parser);
            //     break;
            // case TOK_COLON:
            //     parser->tree->right = parser_parse_mep_decl(parser);
            //     break;
            // case TOK_FN:
            //     parser->tree->right = parser_parse_function_decl(parser);
            default:
                break;
        }
        parser->tree = parser->tree->right;
        parser_consume(parser);
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
        default:
            break;
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

ASTN_FunctionCall parser_parse_function_call(Parser* parser) {
    ASTN_FunctionCall fn;
    Symbol* symb = symtbl_lookup(parser->tbl, parser->cur->value, 0);

    if (symb == NULL) {
        fn.identifier = 0;
        return fn;
    }

    printf("%i\n", symb->data.id);
    fn.identifier = symb->data.id;

    return fn;
}

ASTN_PrimaryExpr parser_parse_prim_expr(Parser* parser) {
    (void)0;
}

ASTN_FactorExpr parser_parse_factor_expr(Parser* parser) {
    (void)0;
}

ASTN_TermExpr parser_parse_term_expr(Parser* parser) {
    (void)0;
}

ASTN_MultiplicationExpr parser_parse_mult_expr(Parser* parser) {
    (void)0;
}

ASTN_AdditionExpr parser_parse_add_expr(Parser* parser) {
    (void)0;
}

ASTN_BitwiseExpr parser_parse_bitw_expr(Parser* parser) {
    (void)0;
}

AST_Node* parser_parse_expression(Parser* parser) {
    (void)0;
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
    
    symtbl_insert(parser->tbl, symbol_init(
        current_module->module, SYMBOL_MODULE, 0, 0, 0, 0, 0, 0, parser->lexer->cl, parser->lexer->cc 
    ));

    return current_module;
}


AST_Node* parser_parse_import(Parser* parser) {
    parser_consume(parser);

    AST_Node* statement = ast_init(STMT);
    if (!statement) {
        // Handle memory allocation failure
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

AST_Node* parser_parse_var_decl(Parser* parser) {
    (void)0;
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

    AST_Node* node = ast_init(STMT);
    node->data.stm.data.function_decl.parameters = parser_parse_parameters(parser);
    if (node->data.stm.data.function_decl.parameters == NULL) {
        return NULL;
    }

    parser_expect(parser, TOK_LBRACE);

    node->data.mep.statements = parser_parse_statements(parser);
    if (node->data.mep.statements == NULL) {
        return NULL;
    }

    symtbl_insert(parser->tbl, symbol_init(
        (char*)name_tok->value, SYMBOL_FUNCTION, 0, 0, 0, 0, 0, 0, parser->lexer->cl, parser->lexer->cc
    ));

    return node;
}

ASTN_StructMemberDecl parser_parse_struct_mem(Parser* parser) {
    (void)0;
}

AST_Node* parser_parse_struct_decl(Parser* parser) {
    (void)0;
}

AST_Node* parser_parse_class_decl(Parser* parser) {
    (void)0;
}

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
    ASTN_ReturnStm statement;


    if (!(parser_expect(parser, TOK_RETURN))) {
        statement.expr->type = -1;
    } 

    statement.expr->type = PRIMARY_LITERAL;
    statement.expr->data.literal = parser_parse_literal(parser);

    return statement;    
}

ASTN_Statement* parser_parse_statement(Parser* parser) {
    ASTN_Statement* stm = malloc(sizeof(ASTN_Statement));

    while (parser->cur->type != TOK_EOF) {
        switch (parser->cur->type) {
            case TOK_RETURN:
                stm->type = STMT_RETURN;
                stm->data.return_stm = parser_parse_return_stm(parser);
            default:
                stm = NULL;
                break;
        }
    }

    return stm;
}

ASTN_Statements* parser_parse_statements(Parser* parser) {
    ASTN_Statements* stms = malloc(sizeof(ASTN_Statements));
    stms->statement = NULL;
    stms->size = 0;
    stms->item_size = sizeof(ASTN_Statement*);

    while (parser->cur->type != TOK_EOF && parser->cur->type != TOK_RBRACE) {
        ASTN_Statement* statement = parser_parse_statement(parser);
        if (statement != NULL) {
            stms->statement = realloc(stms->statement, (stms->size + 1) * stms->item_size);
            stms->statement[stms->size++] = statement;
        }
        
        if (!(parser_expect(parser, TOK_SC))) {
            REPORT_ERROR(parser->lexer, "E_SC_AFSTM");
            return NULL;
        }
        printf("statement: %i", statement->type);
    }

    return stms;
}


AST_Node* parser_parse_mep_decl(Parser* parser) {
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

    AST_Node* node = ast_init(STMT);
    node->data.mep.parameters = parser_parse_parameters(parser);
    if (node->data.mep.parameters == NULL) {
        return NULL;
    }  

    parser_expect(parser, TOK_LBRACE);

    node->data.mep.statements = parser_parse_statements(parser);
    if (node->data.mep.statements == NULL) {
        return NULL;
    }

    symtbl_insert(parser->tbl, symbol_init(
        (char*)"MEP", SYMBOL_MEP, 0, 0, 0, 0, 0, 0, parser->lexer->cl, parser->lexer->cc
    ));


    return node;
}

