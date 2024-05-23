#include "alphadev.h"

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
                    printf("Variable Declaration\n");
                    break;
                case STMT_FUNCTION_DECL:
                    print_indent(indent_level + 2);
                    printf("Function Declaration\n");
                    break;
                case STMT_IMPORT_DECL:
                    print_indent(indent_level + 2);
                    printf("Import Declaration\n");
                    break;
                case STMT_EXPRESSION:
                    print_indent(indent_level + 2);
                    printf("Expression\n");
                    break;
                // Add cases for other statement types as needed
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
                    printf("Identifier: %s\n", node->data.expr.data.identifier);
                    break;
                // Add cases for other expression types as needed
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
    if (node->right) {
        print_indent(indent_level + 1);
        printf("Right:\n");
        print_ast_node(node->right, indent_level + 2);
    }
}


void print_symb_tbl(Symbol* cur) {
    printf("---------------------------------------------------------------------------------------------------------------------\n");
    printf("| %-10s | %-5s | %-4s | %-10s | %-8s | %-9s | %-11s | %-10s | %-5s | %-5s |\n", 
           "ID", "Scope", "Nest", "Mem Type", "Mem Mod", "Mem Sto", "Access Type", "Type", "Line", "Col");
    printf("---------------------------------------------------------------------------------------------------------------------\n");
    while (cur != NULL) {
        printf("| %-10d | %-5u | %-4u | %-10d | %-8d | %-9d | %-11d | %-10s | %-5d | %-5d |\n", 
               cur->data.id, cur->data.scope, cur->data.nest, cur->data.mem_type, cur->data.mem_mod, cur->data.mem_sto, 
               cur->data.access_type, 
               cur->data.type == SYMBOL_VARIABLE ? "Variable" : 
               cur->data.type == SYMBOL_FUNCTION ? "Function" : 
               cur->data.type == SYMBOL_STRUCT ? "Struct" : 
               cur->data.type == SYMBOL_CLASS ? "Class" : 
               cur->data.type == SYMBOL_ENUM ? "Enum" : 
               cur->data.type == SYMBOL_MEP ? "MEP" : 
               cur->data.type == SYMBOL_MODULE ? "Module" : "Unknown",
               cur->data.decl_line, cur->data.decl_col);

        cur = cur->next;
    }

    printf("---------------------------------------------------------------------------------------------------------------------\n");
}
