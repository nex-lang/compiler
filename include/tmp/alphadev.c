#include "alphadev.h"

#include "token.h"

void print_indent(int indent_level) {
    for (int i = 0; i < indent_level; ++i) {
        printf("    "); // 4 spaces per indent level
    }
}

void print_ast_node(AST_Node* node, int indent_level) {
    if (!node) return;

    print_indent(indent_level);
    printf("AST_Node Type: %d\n", node->type);

    switch (node->type) {
        case STMT:
            print_indent(indent_level + 1);
            printf("Statement Type: %d\n", node->data.stm.type);
            switch (node->data.stm.type) {
                case STMT_VARIABLE_DECL:
                    print_indent(indent_level + 2);
                    printf("Variable Declaration (symb: ");
                    if (node->data.stm.data.variable_decl.iden.mult.size) {
                        for (int i = 0; i < node->data.stm.data.variable_decl.iden.mult.size; i++) {
                            printf("%i, ", node->data.stm.data.variable_decl.iden.mult.items[i]);
                        }
                    } else {
                        printf("%i", node->data.stm.data.variable_decl.iden.sg);
                    }

                    printf(")\n");

                    break;
                case STMT_FUNCTION_DECL:
                    print_indent(indent_level + 2);
                    printf("Function Declaration (symb id: %i)\n", node->data.stm.data.function_decl.identifier);
                    print_indent(indent_level + 3);
                    
                    printf("Parameters: %zu\n", node->data.stm.data.function_decl.parameters->size);

                    for (int i = 0; i < node->data.stm.data.function_decl.parameters->size; i++) {
                        print_indent(indent_level + 4);
                        printf("%i: %i \n", (i + 1), node->data.stm.data.function_decl.parameters->parameter[i]->identifier);
                    }

                    print_indent(indent_level + 3);

                    printf("Statements: %zu\n", node->data.stm.data.function_decl.statements->size);

                    for (int i = 0; i < node->data.stm.data.function_decl.statements->size; i++) {
                        print_ast_node(node->data.stm.data.function_decl.statements->statement[i], indent_level + 4);
                    }

                    break;
                case STMT_IMPORT_DECL:
                    print_indent(indent_level + 2);
                    printf("Import Declaration\n");
                    print_indent(indent_level + 3);
                    printf("Modules:\n");
                    print_indent(indent_level + 3);
                    for (int i = 0; i < node->data.stm.data.import_decl.modules.size; i++) {
                        printf("%i: %s ", (i+1), node->data.stm.data.import_decl.modules.items[i]->module);
                    }
                    printf("\n");
                    break;
                case STMT_ASSGN:
                    print_indent(indent_level + 2);
                    printf("Assignment Statement\n");
                    if (node->data.stm.data.assgn.sg.id) {
                        print_ast_node(node->data.stm.data.assgn.sg.expr, indent_level + 2);   
                    }
                    break;
                case STMT_EXPRESSION:
                    print_indent(indent_level + 2);
                    printf("Expression\n");
                    break;
                case STMT_RETURN:
                    print_indent(indent_level + 2);
                    printf("Return Statement\n");
                    print_ast_node(node->data.stm.data.return_stm.value.expr, indent_level + 2);
                    break;
                case STMT_ATTR_DECL:
                    print_indent(indent_level + 2);
                    printf("Attribute Statement\n");
                    break;
                case STMT_CLASS_DECL:
                    print_indent(indent_level + 2);
                    printf("Class Statement\n");
                    break;
                case STMT_CONDITIONAL:
                    print_indent(indent_level + 2);
                    printf("Conditional Statement\n");
                    break;
                case STMT_WHILE_LOOP:
                    print_indent(indent_level + 2);
                    printf("While Statement\n");
                    break;
                case STMT_SWITCH:
                    print_indent(indent_level + 2);
                    printf("Switch Statement\n");
                    break;
                case STMT_TRY:
                    print_indent(indent_level + 2);
                    printf("Try Statement\n");
                    break;
                case STMT_FOR_LOOP:
                    print_indent(indent_level + 2);
                    printf("For Statement\n");
                    break;
                default:
                    print_indent(indent_level + 2);
                    printf("Unknown Statement Type\n");
                    break;
            }
            break;
        case EXPR:
            print_indent(indent_level + 1);
            printf("Expression Type: %d\n", node->data.expr.type);
            switch (node->data.expr.type) {
                case EXPR_LITERAL:
                    print_indent(indent_level + 2);
                    printf("Literal\n");
                    break;
                case EXPR_IDENTIFIER:
                    print_indent(indent_level + 2);
                    printf("Identifier: %i\n", node->data.expr.data.identifier);
                    break;
                case EXPR_FUNCTION_CALL:
                    print_indent(indent_level + 2);
                    printf("Function Call\n");
                    break;
                case EXPR_NEST:
                    print_indent(indent_level + 2);
                    printf("Nested Expression\n");
                    break;
                case EXPR_FACTOR:
                    print_indent(indent_level + 2);
                    printf("Factor\n");
                    print_expr(node->data.expr.data.factor.data.unary_op.expr, indent_level, -1);
                    break;
                case EXPR_TERM:
                    print_indent(indent_level + 2);
                    printf("Term\n");
                    print_expr(node->data.expr.data.term.data.binary_op.left, indent_level, 1);
                    print_expr(node->data.expr.data.term.data.binary_op.right, indent_level, 0);
                    break;
                case EXPR_MULTIPLICATION:
                    print_indent(indent_level + 2);
                    printf("Multiplication\n");
                    print_expr(node->data.expr.data.multiplication.data.binary_op.left, indent_level, 1);
                    print_expr(node->data.expr.data.multiplication.data.binary_op.right, indent_level, 0);
                    break;
                case EXPR_ADDITION:
                    print_indent(indent_level + 2);
                    printf("Addition\n");
                    print_expr(node->data.expr.data.addition.data.binary_op.left, indent_level, 1);
                    print_expr(node->data.expr.data.addition.data.binary_op.right, indent_level, 0);
                    break;
                case EXPR_BITWISE:
                    print_indent(indent_level + 2);
                    printf("Bitwise Operation\n");
                    print_expr(node->data.expr.data.bitwise.data.binary_op.left, indent_level, 1);
                    print_expr(node->data.expr.data.bitwise.data.binary_op.right, indent_level, 0);
                    break;
                case EXPR_COMPARISON:
                    print_indent(indent_level + 2);
                    printf("Comparison\n");
                    print_expr(node->data.expr.data.comparison.data.binary_op.left, indent_level, 1);
                    print_expr(node->data.expr.data.comparison.data.binary_op.right, indent_level, 0);
                    break;
                default:
                    print_indent(indent_level + 2);
                    printf("Unknown Expression Type\n");
                    break;
            }
            break;
        case ROOT:
            print_indent(indent_level + 1);
            printf("Root Node\n");
            break;
        case MEP:
            print_indent(indent_level + 1);
            printf("MEP\n");

            print_indent(indent_level + 2);
            printf("Parameters: %zu\n", node->data.mep.parameters->size);

            for (int i = 0; i < node->data.mep.parameters->size; i++) {
                print_indent(indent_level + 3);
                printf("%i: %s \n", (i + 1), node->data.mep.parameters->parameter[i]->identifier);
            }

            print_indent(indent_level + 2);

            printf("Statements: %zu\n", node->data.mep.statements->size);

            for (int i = 0; i < node->data.mep.statements->size; i++) {
                print_ast_node(node->data.mep.statements->statement[i], indent_level + 3);
            }

            break;
            break;
        default:
            print_indent(indent_level + 1);
            printf("Unknown Node Type\n");
            break;
    }

    if (node->left) {
        print_indent(indent_level + 1);
        printf("Left:\n");
        print_ast_node(node->left, indent_level + 2);
    }
    if (node->parent) {
        print_indent(indent_level + 1);
        printf("Parent:\n");
        print_ast_node(node->left, indent_level + 2);
    }
    if (node->right) {
        print_indent(indent_level + 1);
        printf("Right:\n");
        print_ast_node(node->right, indent_level + 2);
    }
}


void print_expr(ASTN_Expression* expr, int indent_level, int dir) {
    print_indent(indent_level + 1);
    if (dir == 1) {
        printf("Binary left:\n");        
    } else if (dir == 0) {
        printf("Binary right:\n");        
    } else if (dir == -1) {
        printf("Unary:\n");        
    }

    if (expr == NULL) {
        return;      
    }


    switch (expr->type) {
        case EXPR_LITERAL:
            print_indent(indent_level + 2);
            printf("Literal: ");
            switch (expr->data.literal.type) {
                case TOK_L_SSINT:
                case TOK_L_SINT:
                case TOK_L_INT:
                case TOK_L_LINT:
                    printf("int64: %ld\n", expr->data.literal.value.int_.norm);
                    break;
                case TOK_L_LLINT:
                    printf("int128: low = %ld, high = %ld\n", expr->data.literal.value.int_.bit128.low, expr->data.literal.value.int_.bit128.high);
                    break;
                case TOK_L_SSUINT:
                case TOK_L_SUINT:
                case TOK_L_UINT:
                case TOK_L_LUINT:
                    printf("uint64: %lu\n", expr->data.literal.value.uint.norm);
                    break;
                case TOK_L_LLUINT:
                    printf("uint128: low = %lu, high = %lu\n", expr->data.literal.value.uint.bit128.low, expr->data.literal.value.uint.bit128.high);
                    break;
                case TOK_L_FLOAT:
                    printf("float32: %f\n", expr->data.literal.value.float_.bit32);
                    break;
                case TOK_L_DOUBLE:
                    printf("float64: %f\n", expr->data.literal.value.float_.bit64);
                    break;
                case TOK_L_CHAR:
                    printf("char: %c\n", expr->data.literal.value.character);
                    break;
                case TOK_L_STRING:
                    printf("string: %s\n", expr->data.literal.value.string);
                    break;
                case TOK_TRUE:
                    printf("boolean: true\n");
                    break;
                case TOK_FALSE:
                    printf("boolean: false\n");
                    break;
                case TOK_L_SIZE:
                    printf("size: %zu\n", expr->data.literal.value.size);
                    break;
            }
            break;
        case EXPR_IDENTIFIER:
            print_indent(indent_level + 2);
            printf("Identifier: %i\n", expr->data.literal.type);
            break;
        case EXPR_FUNCTION_CALL:
            print_indent(indent_level + 2);
            printf("Function Call\n");
            break;
        case EXPR_NEST:
            print_indent(indent_level + 2);
            printf("Nested Expression\n");
            break;
        case EXPR_FACTOR:
            print_indent(indent_level + 2);
            printf("Factor\n");
            print_expr(expr->data.factor.data.unary_op.expr, indent_level + 1, -1);
            break;
        case EXPR_TERM:
            print_indent(indent_level + 2);
            printf("Term\n");
            print_expr(expr->data.term.data.binary_op.left, indent_level + 1, 1);
            print_expr(expr->data.term.data.binary_op.right, indent_level + 1, 0);
            break;
        case EXPR_MULTIPLICATION:
            print_indent(indent_level + 2);
            printf("Multiplication\n");
            print_expr(expr->data.multiplication.data.binary_op.left, indent_level + 1, 1);
            print_expr(expr->data.multiplication.data.binary_op.right, indent_level + 1, 0);
            break;
        case EXPR_ADDITION:
            print_indent(indent_level + 2);
            printf("Addition\n");
            print_expr(expr->data.addition.data.binary_op.left, indent_level + 1, 1);
            print_expr(expr->data.addition.data.binary_op.right, indent_level + 1, 0);
            break;
        case EXPR_BITWISE:
            print_indent(indent_level + 2);
            printf("Bitwise Operation\n");
            print_expr(expr->data.bitwise.data.binary_op.left, indent_level + 1, 1);
            print_expr(expr->data.bitwise.data.binary_op.right, indent_level + 1, 0);
            break;
        case EXPR_COMPARISON:
            print_indent(indent_level + 2);
            printf("Comparison\n");
            print_expr(expr->data.comparison.data.binary_op.left, indent_level + 1, 1);
            print_expr(expr->data.comparison.data.binary_op.right, indent_level + 1, 0);
            break;

    }
}

void print_symb_tbl(Symbol* cur) {
    printf("--------------------------------------------------------------------\n");
    printf("| %-15s | %-5s | %-4s | %-10s | %-5s | %-5s |\n", 
           "ID", "Scope", "Nest", "Type", "Line", "Col");
    printf("--------------------------------------------------------------------\n");
    while (cur != NULL) {
        printf("| %-15d | %-5u | %-4u | %-10s | %-5d | %-5d |\n", 
               cur->data.id, cur->data.scope, cur->data.nest,
               cur->data.type == SYMBOL_ATTR ? "Attribute" : 
               cur->data.type == SYMBOL_VARIABLE ? "Variable" : 
               cur->data.type == SYMBOL_FUNCTION ? "Function" : 
               cur->data.type == SYMBOL_STRUCT ? "Struct" : 
               cur->data.type == SYMBOL_CLASS ? "Class" : 
               cur->data.type == SYMBOL_ENUM ? "Enum" : 
               cur->data.type == SYMBOL_MEP ? "MEP" : 
               cur->data.type == SYMBOL_ERR ? "ERR" : 
               cur->data.type == SYMBOL_MODULE ? "Module" : "Unknown",
               cur->data.decl_line, cur->data.decl_col);

        cur = cur->next;
    }
    printf("--------------------------------------------------------------------\n");
}
