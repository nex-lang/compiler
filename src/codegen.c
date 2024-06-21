#include "codegen.h"

unsigned long hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; 
    }
    return hash;
}

unsigned long write_string_symb(ASM_StringSymbol **head, const char *str, int *counter) {
    unsigned long hash = hash_string(str);

    char label[32];
    snprintf(label, sizeof(label), "str_%lu_%d", hash, (*counter)++);

    size_t str_len = strlen(str);
    char *new_value = (char *)malloc(str_len + 2);
    strcpy(new_value, str);
    new_value[str_len] = '\n';
    new_value[str_len + 1] = '\0';

    ASM_StringSymbol *new_node = (ASM_StringSymbol *)malloc(sizeof(ASM_StringSymbol));
    new_node->label = strdup(label);
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


void generate_code_for_statement(AST_Node* statement, FILE *fp, int *counter, ASM_StringSymbol **head) {
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
        default:
            break;
    }
}

void generate_code_for_ast(AST_Node *node, FILE *fp, int *counter, ASM_StringSymbol **head) {
    if (node == NULL) {
        return;
    }

    switch (node->type) {
        case MEP:
            fprintf(fp, "_start:\n");

            for (size_t i = 0; i < node->data.mep.statements->size; i++) {
                generate_code_for_statement(node->data.mep.statements->statement[i], fp, counter, head);
            }

            fprintf(fp, "    mov eax, 60\n");
            fprintf(fp, "    xor edi, edi\n");
            fprintf(fp, "    syscall\n\n");

            break;
        default:
            break;
    }

    generate_code_for_ast(node->left, fp, counter, head);
    generate_code_for_ast(node->right, fp, counter, head);
}

void write_string_literals(FILE *fp, ASM_StringSymbol *head) {
    ASM_StringSymbol *current = head;
    while (current != NULL) {
        fprintf(fp, "%s:\n", current->label);
        fprintf(fp, "    .asciz \"%s\"\n", current->value);
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
    ASM_StringSymbol *head = NULL;

    fprintf(fp, ".global _start\n");
    fprintf(fp, ".intel_syntax noprefix\n\n");

    generate_code_for_ast(root, fp, &counter, &head);

    write_string_literals(fp, head);

    fclose(fp);

    ASM_StringSymbol *current = head;
    while (current != NULL) {
        ASM_StringSymbol *next = current->next;
        free(current->label);
        free(current->value);
        free(current);
        current = next;
    }
}