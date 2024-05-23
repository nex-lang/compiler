#include "alphadev.h"

void print_indent(int indent_level) {
    for (int i = 0; i < indent_level; ++i) {
        printf("    "); // 4 spaces per indent level
    }
}

void print_ast_node(AST_Node* node, int indent_level);

void print_astn_literal(ASTN_Literal* literal, int indent_level) {
    print_indent(indent_level);
    printf("Literal: ");
    switch (literal->type) {
        case 0:
            printf("Integer: %d\n", literal->value.int_.bit32);
            break;
        case 1:
            printf("String: %s\n", literal->value.string);
            break;
        // Add more cases for other types as needed
        default:
            printf("Unknown Literal Type\n");
    }
}

// Function to print ASTN_DataTypeSpecifier
void print_astn_data_type_specifier(ASTN_DataTypeSpecifier* data_type_specifier, int indent_level) {
    print_indent(indent_level);
    printf("DataTypeSpecifier: ");
    if (data_type_specifier->is_arr) {
        printf("Array of ");
    }
    // Assuming prim is primitive type, cust_type is custom type
    printf("Type: %d\n", data_type_specifier->data.prim); // or cust_type
}

// Recursive function to print ASTN_Expression
void print_astn_expression(ASTN_Expression* expression, int indent_level) {
    if (!expression) return;

    print_indent(indent_level);
    printf("Expression Type: %d\n", expression->type);
    switch (expression->type) {
        case EXPR_LITERAL:
            print_astn_literal(&expression->data.literal, indent_level + 1);
            break;
        case EXPR_IDENTIFIER:
            print_indent(indent_level + 1);
            printf("Identifier: %s\n", expression->data.identifier);
            break;
        case EXPR_PRIMARY:
            print_ast_node((AST_Node*)&expression->data.primary, indent_level + 1);
            break;
        case EXPR_FACTOR:
            print_ast_node((AST_Node*)&expression->data.factor, indent_level + 1);
            break;
        case EXPR_TERM:
            print_ast_node((AST_Node*)&expression->data.term, indent_level + 1);
            break;
        case EXPR_MULTIPLICATION:
            print_ast_node((AST_Node*)&expression->data.multiplication, indent_level + 1);
            break;
        case EXPR_ADDITION:
            print_ast_node((AST_Node*)&expression->data.addition, indent_level + 1);
            break;
        case EXPR_BITWISE:
            print_ast_node((AST_Node*)&expression->data.bitwise, indent_level + 1);
            break;
        // Add more cases for other expression types as needed
        default:
            print_indent(indent_level + 1);
            printf("Unknown Expression Type\n");
    }
}

// Function to print ASTN_Parameters
void print_astn_parameters(ASTN_Parameters* parameters, int indent_level) {
    print_indent(indent_level);
    printf("Parameters: %zu\n", parameters->size);
    for (size_t i = 0; i < parameters->size; ++i) {
        print_indent(indent_level + 1);
        printf("Parameter %zu:\n", i);
        print_astn_data_type_specifier(&parameters->parameter[i]->data_type_specifier, indent_level + 2);
        print_indent(indent_level + 2);
        printf("Identifier: %s\n", parameters->parameter[i]->identifier);
    }
}

// Function to print ASTN_Statements
void print_astn_statements(ASTN_Statements* statements, int indent_level) {
    print_indent(indent_level);
    printf("Statements: %zu\n", statements->size);
    for (size_t i = 0; i < statements->size; ++i) {
        print_ast_node((AST_Node*)statements->statement[i], indent_level + 1);
    }
}

// Function to print ASTN_FunctionDecl
void print_astn_function_decl(ASTN_FunctionDecl* function_decl, int indent_level) {
    print_indent(indent_level);
    printf("FunctionDecl:\n");
    print_indent(indent_level + 1);
    printf("Access: %d\n", function_decl->access);
    print_indent(indent_level + 1);
    printf("Storage: %d\n", function_decl->storage);
    print_astn_data_type_specifier(&function_decl->data_type_specifier, indent_level + 1);
    print_astn_parameters(function_decl->parameters, indent_level + 1);
    print_astn_statements(&function_decl->statements, indent_level + 1);
}

// Function to print ASTN_ImportDecl
void print_astn_import_decl(ASTN_ImportDecl* import_decl, int indent_level) {
    print_indent(indent_level);
    printf("ImportDecl:\n");
    print_indent(indent_level + 1);
    printf("Modules: %zu\n", import_decl->modules.size);
    for (size_t i = 0; i < import_decl->modules.size; ++i) {
        print_indent(indent_level + 2);
        printf("Module %zu: %s\n", i, import_decl->modules.items[i]->module);
    }
    if (import_decl->alias) {
        print_indent(indent_level + 1);
        printf("Alias: %s\n", import_decl->alias);
    }
}

// Function to print ASTN_VariableDecl
void print_astn_variable_decl(ASTN_VariableDecl* variable_decl, int indent_level) {
    print_indent(indent_level);
    printf("VariableDecl:\n");
    print_indent(indent_level + 1);
    printf("Access: %d\n", variable_decl->access);
    print_indent(indent_level + 1);
    printf("Storage: %d\n", variable_decl->storage);
    print_astn_data_type_specifier(&variable_decl->data_type_specifier, indent_level + 1);
    print_indent(indent_level + 1);
    printf("Identifier: %s\n", variable_decl->identifier);
    if (variable_decl->expr) {
        print_indent(indent_level + 1);
        printf("Expression:\n");
        print_astn_expression(variable_decl->expr, indent_level + 2);
    }
}

// Function to print ASTN_ConditionalStm
void print_astn_conditional_stm(ASTN_ConditionalStm* conditional, int indent_level) {
    print_indent(indent_level);
    printf("Conditional Statement:\n");
    print_indent(indent_level + 1);
    printf("If Condition:\n");
    print_astn_expression(conditional->if_condition, indent_level + 2);
    print_astn_statements(&conditional->if_statement, indent_level + 1);
    for (size_t i = 0; i < conditional->elif_branches.size; ++i) {
        print_indent(indent_level + 1);
        printf("Else If Condition %zu:\n", i);
        print_astn_expression(conditional->elif_branches.condition[i], indent_level + 2);
        print_astn_statements(conditional->elif_branches.statements[i], indent_level + 1);
    }
    if (conditional->else_statements.size > 0) {
        print_indent(indent_level + 1);
        printf("Else:\n");
        print_astn_statements(&conditional->else_statements, indent_level + 2);
    }
}

// Function to print ASTN_ForStm
void print_astn_for_stm(ASTN_ForStm* for_stm, int indent_level) {
    print_indent(indent_level);
    printf("For Loop:\n");
    print_indent(indent_level + 1);
    printf("Variable: %s\n", for_stm->var);
    print_astn_variable_decl(&for_stm->var_decl, indent_level + 1);
    print_indent(indent_level + 1);
    printf("Condition:\n");
    print_astn_expression(for_stm->condition_expr, indent_level + 2);
    print_indent(indent_level + 1);
    printf("Next Expression:\n");
    print_astn_expression(for_stm->next_expr, indent_level + 2);
    print_astn_statements(&for_stm->statements, indent_level + 1);
}

// Function to print ASTN_SwitchStm
void print_astn_switch_stm(ASTN_SwitchStm* switch_stm, int indent_level) {
    print_indent(indent_level);
    printf("Switch Statement:\n");
    print_indent(indent_level + 1);
    printf("Condition:\n");
    print_astn_expression(switch_stm->condition_expr, indent_level + 2);
    for (size_t i = 0; i < switch_stm->clauses.size; ++i) {
        print_indent(indent_level + 1);
        printf("Case Clause %zu:\n", i);
        print_astn_expression(switch_stm->clauses.items[i]->condition_expr, indent_level + 2);
        print_astn_statements(&switch_stm->clauses.items[i]->statements, indent_level + 2);
    }
}

// Function to print ASTN_TryStm
void print_astn_try_stm(ASTN_TryStm* try_stm, int indent_level) {
    print_indent(indent_level);
    printf("Try Statement:\n");
    print_astn_statements(&try_stm->try_statements, indent_level + 1);
    // for (size_t i = 0; i < try_stm->except_branches.size; ++i) {
    //     print_indent(indent_level + 1);
    //     printf("Catch Clause %zu:\n", i);
    //     print_astn_parameters(try_stm->except_branches.items[i]->parameters, indent_level + 2);
    //     print_astn_statements(&try_stm->except_branches.items[i]->statements, indent_level + 2);
    // }
    if (try_stm->finally_statements.size > 0) {
        print_indent(indent_level + 1);
        printf("Finally:\n");
        print_astn_statements(&try_stm->finally_statements, indent_level + 2);
    }
}

// Function to print ASTN_ReturnStm
void print_astn_return_stm(ASTN_ReturnStm* return_stm, int indent_level) {
    print_indent(indent_level);
    printf("Return Statement:\n");
    print_astn_expression(return_stm->expr, indent_level + 1);
}

// Function to print ASTN_Statement
void print_astn_statement(ASTN_Statement* statement, int indent_level) {
    print_indent(indent_level);
    printf("Statement Type: %d\n", statement->type);
    switch (statement->type) {
        case STMT_VARIABLE_DECL:
            print_astn_variable_decl(&statement->data.variable_decl, indent_level + 1);
            break;
        case STMT_FUNCTION_DECL:
            print_astn_function_decl(&statement->data.function_decl, indent_level + 1);
            break;
        case STMT_IMPORT_DECL:
            print_astn_import_decl(&statement->data.import_decl, indent_level + 1);
            break;
        case STMT_EXPRESSION:
            print_astn_expression(&statement->data.expression, indent_level + 1);
            break;
        case STMT_CONDITIONAL:
            print_astn_conditional_stm(&statement->data.conditional, indent_level + 1);
            break;
        case STMT_FOR_LOOP:
            print_astn_for_stm(&statement->data.for_loop, indent_level + 1);
            break;
        case STMT_SWITCH:
            print_astn_switch_stm(&statement->data.switch_stm, indent_level + 1);
            break;
        case STMT_TRY:
            print_astn_try_stm(&statement->data.try_stm, indent_level + 1);
            break;
        case STMT_RETURN:
            print_astn_return_stm(&statement->data.return_stm, indent_level + 1);
            break;
        // Add cases for other statement types as needed
        default:
            print_indent(indent_level + 1);
            printf("Unknown Statement Type\n");
    }
}

void print_ast_node(AST_Node* node, int indent_level) {
    if (!node) return;

    print_indent(indent_level);
    printf("AST_Node Type: %d\n", node->type);
    switch (node->type) {
        case STMT:
            print_astn_statement(&node->data.stm, indent_level + 1);
            break;
        case EXPR:
            print_astn_expression(&node->data.expr, indent_level + 1);
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