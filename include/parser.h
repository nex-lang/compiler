#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include "symtbl.h"

typedef struct Parser {
    Lexer* lexer;
    Token* cur;
    AST_Node* tree;
    SymTable* tbl;
} Parser;

Parser* parser_init(char* filename);
void parser_free(Parser* parser);

bool parser_expectsq(Parser* parser, ...);
bool parser_expect(Parser* parser, uint8_t expected);
void parser_consume(Parser* parser);
void parser_parse(Parser* parser);
void parser_parse_expression(Parser* parser);
ASTN_Literal parser_parse_literal(Parser* parser);

AST_Node* parser_parse_import(Parser* parser);
AST_Node* parser_parse_mep_decl(Parser* parser);
AST_Node* parser_parse_function_decl(Parser* parser);
AST_Node* parser_parse_function_call(Parser* parser);

ASTN_Statements* parser_parse_statements(Parser* parser);
ASTN_Statement* parser_parse_statement(Parser* parser);
ASTN_Module* parser_parse_module(Parser* parser);
ASTN_Parameters* parser_parse_parameters(Parser* parser);
ASTN_Parameter* parser_parse_parameter(Parser* parser);
ASTN_Statements* parser_parser_statements(Parser* parser);
ASTN_ReturnStm parser_parse_return_stm(Parser* parser);
// ASTN_FunctionCallParams* parser_parse_fncall_parameters(Parser* parser);


ASTN_DataTypeSpecifier parser_parse_dt_spec(Parser* parser);

#endif // PARSER_H