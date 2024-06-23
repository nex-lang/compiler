#include "codegen.h"

unsigned long hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c == *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

unsigned long write_string_symb(ASM_StringSymbol **head, const char *str, int *counter) {
    unsigned long hash = hash_string(str);

    char label[32];
    snprintf(label, sizeof(label), "str_%lu_%d", hash, (*counter)++);

    size_t str_len = strlen(str);
    char *new_value = (char *)malloc(str_len + 2);
    if (new_value == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    strcpy(new_value, str);
    new_value[str_len] = '\n'; // Append newline character
    new_value[str_len + 1] = '\0';

    ASM_StringSymbol *new_node = (ASM_StringSymbol *)malloc(sizeof(ASM_StringSymbol));
    if (new_node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    new_node->label = strdup(label);
    if (new_node->label == NULL) {
        fprintf(stderr, "String duplication failed\n");
        exit(EXIT_FAILURE);
    }
    new_node->value = new_value;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        ASM_StringSymbol *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }

    return hash;
}

unsigned long write_var_symb(ASM_VarSymbol **head, uint32_t id, const char *value, int *counter) {
    char label[32];
    snprintf(label, sizeof(label), "var_%u_%d", id, (*counter)++);

    ASM_VarSymbol *new_node = (ASM_VarSymbol *)malloc(sizeof(ASM_VarSymbol));
    if (new_node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    new_node->label = strdup(label);
    if (new_node->label == NULL) {
        fprintf(stderr, "String duplication failed\n");
        exit(EXIT_FAILURE);
    }
    new_node->value = value; // Initialize value as needed
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        ASM_VarSymbol *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }

    return id;
}

void generate_code_for_statement(AST_Node* statement, FILE *fp, int *counter, int* var_counter, ASM_StringSymbol **head, ASM_VarSymbol **var_head, Stack *var_stack) {
    if (statement == NULL) {
        return;
    }

    switch (statement->data.stm.type) {
        case STMT_RETURN: {
            int return_value = statement->data.stm.data.return_stm.expr->data.expr.data.literal.value.uint.bit64;
            fprintf(fp, "    mov edi, %d\n", return_value);
            break;
        }
        case STMT_CALL: {
            if (statement->data.stm.data.call.identifier == -1124075304) {
                const char *string_value = statement->data.stm.data.call.params->parameter[0]->data.expr.data.literal.value.string;
                unsigned long label_hash = write_string_symb(head, string_value, counter);
                fprintf(fp, "    mov rax, 1\n");
                fprintf(fp, "    mov rdi, 1\n");
                fprintf(fp, "    lea rsi, [str_%lu_%d]\n", label_hash, *counter - 1);
                fprintf(fp, "    mov rdx, %zu\n", strlen(string_value) + 2);
                fprintf(fp, "    syscall\n");
            }
            break;
        }
        case STMT_VARIABLE_DECL: {
            uint32_t var_name = 0 ;
            char* value = NULL;

            if (statement->data.stm.data.variable_decl.iden.mult.size) {
                for (int i = 0; i < statement->data.stm.data.variable_decl.iden.mult.size; i++) {
                    var_name = statement->data.stm.data.variable_decl.iden.mult.items[i];
                }
                
            } else {
                var_name = statement->data.stm.data.variable_decl.iden.sg;
            }

            if (var_name == statement->data.stm.data.variable_decl.iden.sg) {
                size_t value_str_size = snprintf(NULL, 0, "%lu", statement->data.stm.data.variable_decl.expr->data.expr.data.literal.value.uint.bit64) + 1;
                value = (char *)malloc(value_str_size);
                if (value == NULL) {
                    fprintf(stderr, "Memory allocation failed\n");
                    exit(EXIT_FAILURE);
                }
                snprintf(value, value_str_size, "%lu", statement->data.stm.data.variable_decl.expr->data.expr.data.literal.value.uint.bit64);
            }

            stack_push(var_stack, var_name);

            write_var_symb(var_head, var_name, value, var_counter);

            break;
        }
        default:
            break;
    }
}

void generate_code_for_ast(AST_Node *node, FILE *fp, int *counter, int* var_counter, ASM_StringSymbol **head, ASM_VarSymbol **var_head, Stack *var_stack) {
    if (node == NULL) {
        return;
    }

    switch (node->type) {
        case MEP:
            fprintf(fp, "_start:\n");

            for (size_t i = 0; i < node->data.mep.statements->size; i++) {
                generate_code_for_statement(node->data.mep.statements->statement[i], fp, counter, var_counter, head, var_head, var_stack);
            }

            fprintf(fp, "    mov eax, 60\n");
            fprintf(fp, "    syscall\n\n");

            fprintf(fp, "    xor edi, edi\n");

            break;
        default:
            break;
    }

    generate_code_for_ast(node->left, fp, counter, var_counter, head, var_head, var_stack);
    generate_code_for_ast(node->right, fp, counter, var_counter, head, var_head, var_stack);
}

void write_string_literals(FILE *fp, ASM_StringSymbol *head) {
    ASM_StringSymbol *current = head;
    while (current != NULL) {
        fprintf(fp, "%s:\n", current->label);
        fprintf(fp, "    .asciz \"%s\"\n", current->value);
        current = current->next;
    }
}

void write_var_decl(FILE *fp, ASM_VarSymbol *head) {
    ASM_VarSymbol *current = head;
    while (current != NULL) {
        fprintf(fp, "%s:\n", current->label);
        fprintf(fp, "    .quad %s\n", current->value); // Adjust format as per your variable type

        current = current->next;
    }
}

void GEN(AST_Node *root) {
    FILE *fp = fopen("prog.asm", "w");
    if (fp == NULL) {
        perror("Error opening output file");
        return;
    }

    int counter = 0;
    int var_counter = 0; // Counter for variables
    ASM_StringSymbol *head_string = NULL;
    ASM_VarSymbol *head_var = NULL;

    fprintf(fp, ".global _start\n");
    fprintf(fp, ".intel_syntax noprefix\n\n");

    Stack* var_stack = stack_init(10);

    generate_code_for_ast(root, fp, &counter, &var_counter, &head_string, &head_var, var_stack);

    stack_free(var_stack);

    write_string_literals(fp, head_string);

    fprintf(fp, ".section .data\n\n");

    write_var_decl(fp, head_var);

    fclose(fp);

    ASM_StringSymbol *current_string = head_string;
    while (current_string != NULL) {
        ASM_StringSymbol *next = current_string->next;
        free(current_string->label);
        free(current_string->value);
        free(current_string);
        current_string = next;
    }

    ASM_VarSymbol *current_var = head_var;
    while (current_var != NULL) {
        ASM_VarSymbol *next = current_var->next;
        free(current_var->label);
        free(current_var);
        current_var = next;
    }
}
