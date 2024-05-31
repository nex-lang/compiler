#include "p_info.h"
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

typedef struct AST_Node AST_Node;

typedef struct ASTN_Literal ASTN_Literal;
typedef struct ASTN_DataTypeSpecifier ASTN_DataTypeSpecifier;
typedef struct ASTN_Statement ASTN_Statement;
typedef struct ASTN_Expression ASTN_Expression;
typedef struct ASTN_Call ASTN_Call;
typedef struct ASTN_PrimaryExpr ASTN_PrimaryExpr;
typedef struct ASTN_FactorExpr ASTN_FactorExpr;
typedef struct ASTN_TermExpr ASTN_TermExpr;
typedef struct ASTN_MultiplicationExpr ASTN_MultiplicationExpr;
typedef struct ASTN_AdditionExpr ASTN_AdditionExpr;
typedef struct ASTN_BitwiseExpr ASTN_BitwiseExpr;
typedef struct ASTN_ComparisonExpr ASTN_ComparisonExpr;
typedef struct ASTN_Parameters ASTN_Parameters;
typedef struct ASTN_Module ASTN_Module;
typedef struct ASTN_ImportDecl ASTN_ImportDecl;
typedef struct ASTN_AttributeUnit ASTN_AttributeUnit;
typedef struct ASTN_AttributeList ASTN_AttributeList;
typedef struct ASTN_AttributeDecl ASTN_AttributeDecl;
typedef struct ASTN_VariableDecl ASTN_VariableDecl;
typedef struct ASTN_FunctionDecl ASTN_FunctionDecl;
typedef struct ASTN_StructDecl ASTN_StructDecl;
typedef struct ASTN_ClassDecl ASTN_ClassDecl;
typedef struct ASTN_EnumDecl ASTN_EnumDecl;
typedef struct ASTN_ConditionalStm ASTN_ConditionalStm;
typedef struct ASTN_ForStm ASTN_ForStm;
typedef struct ASTN_SwitchStm ASTN_SwitchStm;
typedef struct ASTN_CaseClause ASTN_CaseClause;
typedef struct ASTN_TryStm ASTN_TryStm;
typedef struct ASTN_WhileStm ASTN_WhileStm;
typedef struct ASTN_Keyword ASTN_Keyword;
typedef struct ASTN_ReturnStm ASTN_ReturnStm;

typedef struct ASTN_Statements ASTN_Statements;
typedef struct ASTN_MEP ASTN_MEP;

typedef struct ASTN_Literal {
    int type;
    union {
        struct {
            int8_t bit8;
            int16_t bit16;
            int32_t bit32;
            int64_t bit64;
            __int128_t bit128;
        } int_;
        struct {
            uint8_t bit8;
            uint16_t bit16;
            uint32_t bit32;
            uint64_t bit64;
            __uint128_t bit128;
        } uint;
        struct {
            float bit32;
            double bit64;
        } float_;
        char character;
        char* string;
        int boolean;
        size_t size;
        struct {
            AST_Node** items;
            size_t size;
            size_t item_size;
        } array;
    } value;
} ASTN_Literal;

typedef struct ASTN_DataTypeSpecifier {
    bool is_arr;
    union {
        int prim;
        int cust_type;
    } data;
} ASTN_DataTypeSpecifier;

typedef struct ASTN_Statements {
    AST_Node** statement;
    size_t size;
    size_t item_size;
} ASTN_Statements;

typedef struct ASTN_CallParams {
    AST_Node** parameter;
    size_t size;
    size_t item_size;
} ASTN_CallParams;

typedef struct ASTN_Call {
    enum {
        CALL_FN,
        CALL_STRUCT,
        CALL_CLASS
    } type;

    int32_t identifier;
    ASTN_CallParams* params;
} ASTN_Call;

typedef struct ASTN_PrimaryExpr {
    enum {
        PRIMARY_CALL,
        PRIMARY_IDENTIFIER,
        PRIMARY_LITERAL,
        PRIMARY_EXPRESSION
    } type;

    union {
        ASTN_Call call;
        int32_t identifier;
        ASTN_Literal literal;
        AST_Node* expression;
    } data;
} ASTN_PrimaryExpr;

typedef struct ASTN_FactorExpr {
    enum {
        FACTOR_PRIMARY,
        FACTOR_UNARY_OP
    } type;

    union {
        ASTN_PrimaryExpr primary;
        struct {
            int op;
            ASTN_PrimaryExpr expr;
        } unary_op;
    } data;
} ASTN_FactorExpr;

typedef struct ASTN_TermExpr {
    enum {
        TERM_FACTOR,
        TERM_BINARY_OP
    } type;

    union {
        ASTN_FactorExpr factor;
        struct {
            int op;
            ASTN_FactorExpr right;
            ASTN_FactorExpr left;
        } binary_op;
    } data;
} ASTN_TermExpr;

typedef struct ASTN_MultiplicationExpr {
    enum {
        MULTIPLICATION_TERM,
        MULTIPLICATION_BINARY_OP
    } type;

    union {
        ASTN_TermExpr term;
        struct {
            int op;
            ASTN_TermExpr right;
            ASTN_TermExpr left;
        } binary_op;
    } data;
} ASTN_MultiplicationExpr;

typedef struct ASTN_AdditionExpr {
    enum {
        ADDITION_MULTIPLICATION,
        ADDITION_BINARY_OP
    } type;

    union {
        ASTN_MultiplicationExpr multiplication;
        struct {
            int op;
            ASTN_MultiplicationExpr right;
            ASTN_MultiplicationExpr left;
        } binary_op;
    } data;
} ASTN_AdditionExpr;

typedef struct ASTN_BitwiseExpr {
    enum {
        BITWISE_ADDITION,
        BITWISE_BINARY_OP
    } type;

    union {
        ASTN_AdditionExpr addition;
        struct {
            int op;
            ASTN_AdditionExpr right;
            ASTN_AdditionExpr left;
        } binary_op;
    } data;
} ASTN_BitwiseExpr;

typedef struct ASTN_ComparisonExpr {
    enum {
        COMPARISON_BITWISE,
        COMPARISON_BINARY_OP
    } type;

    union {
        ASTN_BitwiseExpr bitwise;
        struct {
            int op;
            ASTN_BitwiseExpr left;
            ASTN_BitwiseExpr right;
        } binary_op;
    } data;
} ASTN_ComparisonExpr;

typedef struct ASTN_Expression {
    enum {
        EXPR_FUNCTION_CALL,
        EXPR_IDENTIFIER,
        EXPR_LITERAL,
        EXPR_NEST,
        EXPR_FACTOR,
        EXPR_TERM,
        EXPR_MULTIPLICATION,
        EXPR_ADDITION,
        EXPR_BITWISE,
        EXPR_COMPARISON
    } type;

    union {
        ASTN_Call function_call;
        AST_Node* nest;
        int32_t identifier;
        ASTN_Literal literal;
        ASTN_PrimaryExpr primary;
        ASTN_FactorExpr factor;
        ASTN_TermExpr term;
        ASTN_MultiplicationExpr multiplication;
        ASTN_AdditionExpr addition;
        ASTN_BitwiseExpr bitwise;
        ASTN_ComparisonExpr comparison;
    } data;
} ASTN_Expression;

typedef struct ASTN_Parameter {
    ASTN_DataTypeSpecifier data_type_specifier;
    char* identifier;
} ASTN_Parameter;

typedef struct ASTN_Parameters {
    ASTN_Parameter** parameter;
    size_t size;
    size_t item_size;
} ASTN_Parameters;

typedef struct ASTN_Module {
    char* module;
    ASTN_Module* head_module;
} ASTN_Module;

typedef struct ASTN_ImportDecl {
    struct {
        ASTN_Module** items;
        size_t size;
        size_t item_size;
    } modules;

    ASTN_Module* source;
    char* alias;
} ASTN_ImportDecl;


typedef struct ASTN_AttributeUnit {
    // attributes are decl's that require access specifiers
    enum {
        ATTR_FUNCTION,
        ATTR_VARIABLE
    } type;

    union {
        AST_Node* var;
        AST_Node* fn;
    } data;
} ASTN_AttributeUnit;

typedef struct ASTN_AttributeList {
    AST_Node** items;
    size_t size;
    size_t item_size;
} ASTN_AttributeList;


typedef struct ASTN_AttributeDecl {
    ASTN_AttributeList list;
    int32_t identifier;
} ASTN_AttributeDecl;


typedef struct ASTN_VariableDecl {
    int access,
    storage;
    ASTN_DataTypeSpecifier data_type_specifier;
    union {
        int sg;
        struct {
            int* items;
            size_t size;
        } mult;
    } iden;

    AST_Node* expr;
} ASTN_VariableDecl;

typedef struct ASTN_FunctionDecl {
    int access, storage, identifier;
    ASTN_DataTypeSpecifier data_type_specifier;
    ASTN_Parameters* parameters;
    ASTN_Statements* statements;
} ASTN_FunctionDecl;

typedef struct ASTN_StructMemberDecl {
    int storage;
    ASTN_DataTypeSpecifier data_type_specifier;
    char* identifier;
} ASTN_StructMemberDecl;

typedef struct ASTN_StructDecl {
    int access;
    int identifier;
    struct {
        ASTN_StructMemberDecl** items;
        size_t size;
        size_t item_size;
    } members;
} ASTN_StructDecl;

typedef struct ASTN_ClassDecl {
    int identifier;

    ASTN_FunctionDecl init, free;

    struct {
        AST_Node** attributes;
        size_t size;
        size_t item_size;
    } attributes;
} ASTN_ClassDecl;

typedef struct ASTN_EnumDecl {
    int identifier;
    struct {
        char** items;
        size_t size;
        size_t item_size;
    } members;
} ASTN_EnumDecl;

typedef struct ASTN_ConditionalStm {
    AST_Node* if_condition;
    AST_Node* if_statement;
    struct {
        AST_Node** condition;
        AST_Node** statements;
        size_t size;
        size_t item_size_a, item_size_b;
    } elif_branches;
    AST_Node* else_statements;
} ASTN_ConditionalStm;

typedef struct ASTN_ForStm {
    char var[MAX_IDENTIFIER_LEN];
    ASTN_VariableDecl var_decl;

    AST_Node* condition_expr;
    AST_Node* next_expr;

    AST_Node* statements;
} ASTN_ForStm;

typedef struct ASTN_SwitchStm {
    AST_Node* condition_expr;
    struct {
        AST_Node** items;
        size_t size;
        size_t item_size;
    } clauses;
} ASTN_SwitchStm;

typedef struct ASTN_CaseClause {
    AST_Node* condition_expr;
    AST_Node* statements;
} ASTN_CaseClause;

typedef struct ASTN_TryStm {
    AST_Node* try_statements;
    struct {
        AST_Node** condition;
        AST_Node** statements;
        size_t size;
        size_t item_size_a, item_size_b;
    } except_branches;
    AST_Node* finally_statements;
} ASTN_TryStm;

typedef struct ASTN_WhileStm {
    AST_Node* condition_expr;
    AST_Node* statements;
} ASTN_WhileStm;

typedef struct ASTN_Keyword {
    union {
        int break_;
        int continue_;
    } type;
} ASTN_Keyword;

typedef struct ASTN_ReturnStm {
    AST_Node* expr;
} ASTN_ReturnStm;

typedef struct ASTN_Statement {
    enum {
        STMT_ATTR_UNIT,
        STMT_ATTR_DECL,
        STMT_VARIABLE_DECL,
        STMT_FUNCTION_DECL,
        STMT_STRUCT_DECL,
        STMT_CLASS_DECL,
        STMT_ENUM_DECL,
        STMT_IMPORT_DECL,
        STMT_CONDITIONAL,
        STMT_FOR_LOOP,
        STMT_SWITCH,
        STMT_TRY,
        STMT_WHILE_LOOP,
        STMT_EXPRESSION,
        STMT_RETURN,
        STMT_BREAK,
        STMT_CONTINUE
    } type;

    union {
        ASTN_AttributeUnit attribute_unit;
        ASTN_AttributeDecl attribute_decl;
        ASTN_VariableDecl variable_decl;
        ASTN_FunctionDecl function_decl;
        ASTN_StructDecl struct_decl;
        ASTN_ClassDecl class_decl;
        ASTN_EnumDecl enum_decl;
        ASTN_ImportDecl import_decl;
        ASTN_ConditionalStm conditional;
        ASTN_ForStm for_loop;
        ASTN_SwitchStm switch_stm;
        ASTN_TryStm try_stm;
        ASTN_WhileStm while_loop;
        ASTN_Expression expression;
        ASTN_ReturnStm return_stm;
    } data;
} ASTN_Statement;

typedef struct ASTN_MEP {
    ASTN_Parameters* parameters;
    ASTN_Statements* statements;
} ASTN_MEP;

struct AST_Node {
    enum {
        STMT,
        MEP,
        EXPR,
        ROOT
    } type;

    union {
        bool root;
        ASTN_Statement stm;
        ASTN_MEP mep;
        ASTN_Expression expr;
    } data;

    AST_Node* parent;
    AST_Node* right;
    AST_Node* left;
};

AST_Node* ast_init(int type);
void ast_free(AST_Node* node);
