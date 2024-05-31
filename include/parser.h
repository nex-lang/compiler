#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "tmp/alphadev.h"

typedef struct Parser {
    Lexer* lexer;
    Token* cur;
    AST_Node* tree;
    AST_Node* root;
    SymTable* tbl;

    __uint128_t root_scope;
    __uint128_t scope;
    uint8_t nest;
} Parser;

Parser* parser_init(char* filename);
void parser_free(Parser* parser);

bool parser_expectsq(Parser* parser, ...);
bool parser_expect(Parser* parser, uint8_t expected);
void parser_consume(Parser* parser);

#define PES(parser) ((parser)->scope += 1)
#define PER(parser) ((parser)->root_scope += 1)
#define PEN(parser) ((parser)->nest += 1)

#define PRS(parser) ((parser)->scope = (parser)->root_scope)
#define PRN(parser) ((parser)->nest = 0)
#define PRR(parser) ((parser)->root_scope = 0)

int parse_stospec(Parser* parser, bool expect_further);
int parse_accspec(Parser* parser, bool expect_further);

ASTN_Literal parser_parse_literal(Parser* parser);
ASTN_DataTypeSpecifier parser_parse_dt_spec(Parser* parser, bool expect_further);

ASTN_Call parser_parse_call(Parser* parser);
ASTN_PrimaryExpr parser_parse_prim_expr(Parser* parser);
ASTN_FactorExpr parser_parse_factor_expr(Parser* parser);
ASTN_TermExpr parser_parse_term_expr(Parser* parser);
ASTN_MultiplicationExpr parser_parse_mult_expr(Parser* parser);
ASTN_AdditionExpr parser_parse_add_expr(Parser* parser);
ASTN_BitwiseExpr parser_parse_bitw_expr(Parser* parser);
ASTN_ComparisonExpr parser_parse_comp_expr(Parser* parser);
ASTN_Expression parser_parse_expression(Parser* parser);
AST_Node* parser_parse_expr(Parser* parser);

ASTN_Parameter* parser_parse_parameter(Parser* parser);
ASTN_Parameters* parser_parse_parameters(Parser* parser);

ASTN_Module* parser_parse_module(Parser* parser);
AST_Node* parser_parse_import(Parser* parser);

AST_Node* parser_parse_attr_decl(Parser* parser);
// void parser_parse_extend_attr(Parser* parser, ASTN_AttributeList* src, ASTN_AttributeList* dest);

ASTN_VariableDecl parser_parse_var_decl(Parser* parser);
AST_Node* parser_parse_function_decl(Parser* parser);

ASTN_StructMemberDecl parser_parse_struct_mem(Parser* parser);
AST_Node* parser_parse_struct_decl(Parser* parser);

bool parser_parse_extend_attr(Parser* parser, ASTN_AttributeList* list);
AST_Node* parser_parse_class_decl(Parser* parser);
AST_Node* parser_parse_enum_decl(Parser* parser);

ASTN_ConditionalStm parser_parse_cond_stm(Parser* parser);
ASTN_ForStm parser_parse_for_stm(Parser* parser);
ASTN_SwitchStm parser_parse_switch_stm(Parser* parser);
ASTN_CaseClause parser_parse_case_clause(Parser* parser);
ASTN_TryStm parser_parse_try_stm(Parser* parser);
ASTN_WhileStm parser_parse_while_stm(Parser* parser);
ASTN_ReturnStm parser_parse_return_stm(Parser* parser);

ASTN_Statement parser_parse_statement(Parser* parser);
ASTN_Statements* parser_parse_statements(Parser* parser);

AST_Node* parser_parse_mep_decl(Parser* parser);

#endif // PARSER_H