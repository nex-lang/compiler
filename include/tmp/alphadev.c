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
                    printf("Function Declaration (symb id: %i)\n", node->data.stm.data.function_decl.identifier);
                    print_indent(indent_level + 3);
                    
                    printf("Parameters: %zu\n", node->data.stm.data.function_decl.parameters->size);

                    for (int i = 0; i < node->data.stm.data.function_decl.parameters->size; i++) {
                        print_indent(indent_level + 4);
                        printf("%i: %s \n", (i + 1), node->data.stm.data.function_decl.parameters->parameter[i]->identifier);
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
                case STMT_EXPRESSION:
                    print_indent(indent_level + 2);
                    printf("Expression\n");
                    break;
                case STMT_RETURN:
                    print_indent(indent_level + 2);
                    printf("Return Statement\n");
                    print_ast_node(node->data.stm.data.return_stm.expr, indent_level + 2);
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
                    break;
                case EXPR_TERM:
                    print_indent(indent_level + 2);
                    printf("Term\n");
                    break;
                case EXPR_MULTIPLICATION:
                    print_indent(indent_level + 2);
                    printf("Multiplication\n");
                    break;
                case EXPR_ADDITION:
                    print_indent(indent_level + 2);
                    printf("Addition\n");
                    break;
                case EXPR_BITWISE:
                    print_indent(indent_level + 2);
                    printf("Bitwise Operation\n");
                    break;
                case EXPR_COMPARISON:
                    print_indent(indent_level + 2);
                    printf("Comparison\n");
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



void print_symb_tbl(Symbol* cur) {
    printf("---------------------------------------------------------------------------------------------------------------------\n");
    printf("| %-15s | %-5s | %-4s | %-10s | %-8s | %-9s | %-11s | %-10s | %-5s | %-5s |\n", 
           "ID", "Scope", "Nest", "Mem Type", "Mem Mod", "Mem Sto", "Access Type", "Type", "Line", "Col");
    printf("---------------------------------------------------------------------------------------------------------------------\n");
    while (cur != NULL) {
        printf("| %-15d | %-5u | %-4u | %-10d | %-8d | %-9d | %-11d | %-10s | %-5d | %-5d |\n", 
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
