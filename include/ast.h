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
typedef struct ASTN_ErrDecl ASTN_ErrDecl;
typedef struct ASTN_ConditionalStm ASTN_ConditionalStm;
typedef struct ASTN_ForStm ASTN_ForStm;
typedef struct ASTN_SwitchStm ASTN_SwitchStm;
typedef struct ASTN_TryStm ASTN_TryStm;
typedef struct ASTN_WhileStm ASTN_WhileStm;
typedef struct ASTN_ReturnStm ASTN_ReturnStm;
typedef struct ASTN_ThrowStm ASTN_ThrowStm;

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
    } type;

    union {
        ASTN_Call call;
        int32_t identifier;
        ASTN_Literal literal;
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
            ASTN_Expression* expr;
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
            ASTN_Expression* left;
            ASTN_Expression* right;
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
            ASTN_Expression* left;
            ASTN_Expression* right;
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
            ASTN_Expression* left;
            ASTN_Expression* right;
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
            ASTN_Expression* left;
            ASTN_Expression* right;
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
            ASTN_Expression* left;
            ASTN_Expression* right;
        } binary_op;
    } data;
} ASTN_ComparisonExpr;

typedef struct ASTN_Expression {
    enum {
        EXPR_FUNCTION_CALL,
        EXPR_IDENTIFIER,
        EXPR_LITERAL,
        EXPR_PRIMARY,
        EXPR_FACTOR,
        EXPR_TERM,
        EXPR_MULTIPLICATION,
        EXPR_ADDITION,
        EXPR_BITWISE,
        EXPR_COMPARISON,
        EXPR_NEST
    } type;

    union {
        ASTN_Call function_call;
        int32_t identifier;
        struct {
            union {
                struct {
                    ASTN_Expression* left;
                    ASTN_Expression* right;
                } binary_op;
            } data;
        } nest;
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

    __uint128_t re_scope;
    __uint128_t scope;
} ASTN_AttributeUnit;

typedef struct ASTN_AttributeList {
    AST_Node** items;

    size_t size;
    size_t item_size;

} ASTN_AttributeList;


typedef struct ASTN_AttributeDecl {
    ASTN_AttributeList* list;
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
    int32_t identifier;
} ASTN_StructMemberDecl;

typedef struct ASTN_StructDecl {
    int access;
    int32_t identifier;
    struct {
        ASTN_StructMemberDecl* items;
        size_t size;
        size_t item_size;
    } members;
} ASTN_StructDecl;

typedef struct ASTN_ClassDecl {
    int32_t identifier;

    ASTN_FunctionDecl init, free;

    ASTN_AttributeList* attributes;
} ASTN_ClassDecl;

typedef struct ASTN_EnumDecl {
    int32_t identifier;
    struct {
        uint32_t* items;
        size_t size;
        size_t item_size;
    } members;
} ASTN_EnumDecl;

typedef struct ASTN_ErrDecl {
    int32_t identifier;
    struct {
        ASTN_DataTypeSpecifier* dtss;
        uint32_t* identifiers;
        size_t size;
        size_t item_size;
    } members;
} ASTN_ErrDecl;

typedef struct ASTN_ConditionalStm {
    AST_Node* if_condition;
    ASTN_Statements* if_statements;
    struct {
        AST_Node** conditions;
        ASTN_Statements** statements;
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

    ASTN_Statements* statements;
} ASTN_ForStm;

typedef struct ASTN_SwitchStm {
    AST_Node* condition_expr;
    ASTN_Statement* default_stms;
    struct {
        AST_Node** value;
        ASTN_Statements** statements;
        size_t size;
        size_t item_size;
    } clauses;
} ASTN_SwitchStm;

typedef struct ASTN_TryStm {
    ASTN_Statements* try_statements;
    struct {
        uint32_t* errors;
        ASTN_Statements** statements;
        size_t size;
        size_t item_size_a, item_size_b;
    } except_branches;
    ASTN_Statements* finally_statements;
} ASTN_TryStm;

typedef struct ASTN_WhileStm {
    AST_Node* condition_expr;
    ASTN_Statements* statements;
} ASTN_WhileStm;

typedef struct ASTN_ReturnStm {
    AST_Node* expr;
} ASTN_ReturnStm;

typedef struct ASTN_ThrowStm {
    ASTN_CallParams* params;

    uint8_t iden;
} ASTN_ThrowStm;

typedef struct ASTN_Statement {
    enum {
        STMT_ATTR_UNIT,
        STMT_ATTR_DECL,
        STMT_VARIABLE_DECL,
        STMT_FUNCTION_DECL,
        STMT_CALL,
        STMT_STRUCT_DECL,
        STMT_CLASS_DECL,
        STMT_ERR_DECL,
        STMT_ENUM_DECL,
        STMT_IMPORT_DECL,
        STMT_CONDITIONAL,
        STMT_FOR_LOOP,
        STMT_SWITCH,
        STMT_TRY,
        STMT_WHILE_LOOP,
        STMT_EXPRESSION,
        STMT_RETURN,
        STMT_THROW,
        STMT_BREAK,
        STMT_CONTINUE
    } type;

    union {
        ASTN_AttributeUnit attribute_unit;
        ASTN_AttributeDecl attribute_decl;
        ASTN_VariableDecl variable_decl;
        ASTN_FunctionDecl function_decl;
        ASTN_Call call;
        ASTN_StructDecl struct_decl;
        ASTN_ClassDecl class_decl;
        ASTN_ErrDecl err_decl;
        ASTN_EnumDecl enum_decl;
        ASTN_ImportDecl import_decl;
        ASTN_ConditionalStm conditional;
        ASTN_ForStm for_loop;
        ASTN_SwitchStm switch_stm;
        ASTN_TryStm try_stm;
        ASTN_WhileStm while_loop;
        ASTN_Expression expression;
        ASTN_ReturnStm return_stm;
        ASTN_ThrowStm throw_stm;
        bool brak;
        bool cont;
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
