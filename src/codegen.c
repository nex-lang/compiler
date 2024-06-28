#include "codegen.h"

Generator* gen_init(char* filename) {
    Generator* gen = malloc(sizeof(Generator));
    if (gen == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    
    gen->fp = fopen(filename, "w");
    if (gen->fp == NULL) {
        perror("Error opening output file");
        free(gen);
        return NULL;
    }
    
    gen->str_literals = malloc(sizeof(StringLiteralManager));
    if (gen->str_literals == NULL) {
        perror("Memory allocation failed");
        fclose(gen->fp);
        free(gen);
        exit(EXIT_FAILURE);
    }
    gen->str_literals->head = NULL;
    gen->str_literals->counter = 0;
    
    fprintf(gen->fp, ".global _start\n");
    fprintf(gen->fp, ".intel_syntax noprefix\n\n");

    return gen;
}

unsigned long hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

unsigned long gen_str_symb(ASM_StringSymbol **head, const char *str, int *counter) {
    unsigned long hash = hash_string(str);

    char label[32];
    snprintf(label, sizeof(label), "str_%lu_%d", hash, (*counter)++);

    size_t str_len = strlen(str);
    char *new_value = malloc(str_len + 2);
    if (new_value == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    strcpy(new_value, str);
    new_value[str_len] = '\n';
    new_value[str_len + 1] = '\0';

    ASM_StringSymbol *new_node = malloc(sizeof(ASM_StringSymbol));
    if (new_node == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    new_node->label = strdup(label);
    if (new_node->label == NULL) {
        perror("String duplication failed");
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

void process_variable_decl(Generator* gen, ASTN_Expression* variable_decl, const char* var_name) {
    char* value;
    size_t value_str_size;

    switch (variable_decl->data.literal.type) {
        case TOK_L_SSINT:
            value_str_size = snprintf(NULL, 0, "%d", variable_decl->data.literal.value.int_.bit8) + 1;
            value = malloc(value_str_size);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            snprintf(value, value_str_size, "%d", variable_decl->data.literal.value.int_.bit8);
            break;

        case TOK_L_SINT:
            value_str_size = snprintf(NULL, 0, "%d", variable_decl->data.literal.value.int_.bit16) + 1;
            value = malloc(value_str_size);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            snprintf(value, value_str_size, "%d", variable_decl->data.literal.value.int_.bit16);
            break;

        case TOK_L_INT:
            value_str_size = snprintf(NULL, 0, "%d", variable_decl->data.literal.value.int_.bit32) + 1;
            value = malloc(value_str_size);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            snprintf(value, value_str_size, "%d", variable_decl->data.literal.value.int_.bit32);
            break;

        case TOK_L_LINT:
            value_str_size = snprintf(NULL, 0, "%ld", variable_decl->data.literal.value.int_.bit64) + 1;
            value = malloc(value_str_size);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            snprintf(value, value_str_size, "%ld", variable_decl->data.literal.value.int_.bit64);
            break;

        case TOK_L_LLINT:
            value_str_size = snprintf(NULL, 0, "%lld", variable_decl->data.literal.value.int_.bit128) + 1;
            value = malloc(value_str_size);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            snprintf(value, value_str_size, "%lld", variable_decl->data.literal.value.int_.bit128);
            break;

        case TOK_L_SSUINT:
            value_str_size = snprintf(NULL, 0, "%u", variable_decl->data.literal.value.uint.bit8) + 1;
            value = malloc(value_str_size);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            snprintf(value, value_str_size, "%u", variable_decl->data.literal.value.uint.bit8);
            break;

        case TOK_L_SUINT:
            value_str_size = snprintf(NULL, 0, "%u", variable_decl->data.literal.value.uint.bit16) + 1;
            value = malloc(value_str_size);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            snprintf(value, value_str_size, "%u", variable_decl->data.literal.value.uint.bit16);
            break;

        case TOK_L_UINT:
            value_str_size = snprintf(NULL, 0, "%u", variable_decl->data.literal.value.uint.bit32) + 1;
            value = malloc(value_str_size);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            snprintf(value, value_str_size, "%u", variable_decl->data.literal.value.uint.bit32);
            break;

        case TOK_L_LUINT:
            value_str_size = snprintf(NULL, 0, "%lu", variable_decl->data.literal.value.uint.bit64) + 1;
            value = malloc(value_str_size);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            snprintf(value, value_str_size, "%lu", variable_decl->data.literal.value.uint.bit64);
            break;

        case TOK_L_LLUINT:
            value_str_size = snprintf(NULL, 0, "%llu", variable_decl->data.literal.value.uint.bit128) + 1;
            value = malloc(value_str_size);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            snprintf(value, value_str_size, "%llu", variable_decl->data.literal.value.uint.bit128);
            break;

        case TOK_L_FLOAT:
            value_str_size = snprintf(NULL, 0, "%f", variable_decl->data.literal.value.float_.bit32) + 1;
            value = malloc(value_str_size);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            snprintf(value, value_str_size, "%f", variable_decl->data.literal.value.float_.bit32);
            break;

        case TOK_L_DOUBLE:
            value_str_size = snprintf(NULL, 0, "%lf", variable_decl->data.literal.value.float_.bit64) + 1;
            value = malloc(value_str_size);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            snprintf(value, value_str_size, "%lf", variable_decl->data.literal.value.float_.bit64);
            break;

        case TOK_L_CHAR:
            value_str_size = snprintf(NULL, 0, "%c", variable_decl->data.literal.value.character) + 1;
            value = malloc(value_str_size);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            snprintf(value, value_str_size, "%c", variable_decl->data.literal.value.character);
            break;

        case TOK_L_STRING:
            value_str_size = snprintf(NULL, 0, "%s", variable_decl->data.literal.value.string) + 1;
            value = malloc(value_str_size);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            snprintf(value, value_str_size, "%s", variable_decl->data.literal.value.string);
            break;

        case TOK_L_BOOL:
            value_str_size = snprintf(NULL, 0, "%d", variable_decl->data.literal.value.boolean) + 1;
            value = malloc(value_str_size);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            snprintf(value, value_str_size, "%d", variable_decl->data.literal.value.boolean);
            break;

        case TOK_L_SIZE:
            value_str_size = snprintf(NULL, 0, "%zu", variable_decl->data.literal.value.size) + 1;
            value = malloc(value_str_size);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            snprintf(value, value_str_size, "%zu", variable_decl->data.literal.value.size);
            break;

        default:
            fprintf(stderr, "Unsupported literal type\n");
            exit(EXIT_FAILURE);
    }

    fprintf(gen->fp, "    mov rax, %s\n", value);
    fprintf(gen->fp, "    push rax\n");
    free(value);
}


void gen_string_lits(FILE* fp, ASM_StringSymbol* head) {
    ASM_StringSymbol* current = head;
    while (current != NULL) {
        fprintf(fp, "%s:\n", current->label);
        fprintf(fp, "    .asciz \"%s\"\n", current->value);
        current = current->next;
    }
}

void gen_variable(Generator* gen, size_t size, uint32_t id) {
    
}

void gen_stmt(AST_Node* statement, Generator* gen) {
    if (statement == NULL) {
        return;
    }

    switch (statement->data.stm.type) {
        case STMT_RETURN: {
            int return_value = statement->data.stm.data.return_stm.expr->data.expr.data.literal.value.uint.bit64;
            fprintf(gen->fp, "    mov edi, %d\n", return_value);
            break;
        }
        case STMT_CALL: {
            if (statement->data.stm.data.call.identifier == -1124075304) {
                char *string_value = statement->data.stm.data.call.params->parameter[0]->data.expr.data.literal.value.string;
                unsigned long label_hash = gen_str_symb(&(gen->str_literals->head), string_value, &(gen->str_literals->counter));
                fprintf(gen->fp, "    mov rax, 1\n");
                fprintf(gen->fp, "    mov rdi, 1\n");
                fprintf(gen->fp, "    lea rsi, [str_%lu_%d]\n", label_hash, gen->str_literals->counter - 1);
                fprintf(gen->fp, "    mov rdx, %zu\n", strlen(string_value) + 2);
                fprintf(gen->fp, "    syscall\n");
            }
            break;
        }
        case STMT_VARIABLE_DECL: {
            uint32_t var_name = 0;
            char* value = NULL;

            if (statement->data.stm.data.variable_decl.iden.mult.size) {
                for (int i = 0; i < statement->data.stm.data.variable_decl.iden.mult.size; i++) {
                    var_name = statement->data.stm.data.variable_decl.iden.mult.items[i];

                    size_t value_str_size = snprintf(NULL, 0, "%lu", statement->data.stm.data.variable_decl.expr->data.expr.data.literal.value.uint.bit64) + 1;
                    value = malloc(value_str_size);
                    if (value == NULL) {
                        perror("Memory allocation failed");
                        exit(EXIT_FAILURE);
                    }
                    snprintf(value, value_str_size, "%lu", statement->data.stm.data.variable_decl.expr->data.expr.data.literal.value.uint.bit64);

                    fprintf(gen->fp, "    mov rax, %s\n", value);
                    fprintf(gen->fp, "    push rax\n");
                    free(value);
                }
            } else {
                var_name = statement->data.stm.data.variable_decl.iden.sg;
                size_t value_str_size = snprintf(NULL, 0, "%lu", statement->data.stm.data.variable_decl.expr->data.expr.data.literal.value.uint.bit64) + 1;
                value = malloc(value_str_size);
                if (value == NULL) {
                    perror("Memory allocation failed");
                    exit(EXIT_FAILURE);
                }
                snprintf(value, value_str_size, "%lu", statement->data.stm.data.variable_decl.expr->data.expr.data.literal.value.uint.bit64);

                fprintf(gen->fp, "    mov rax, %s\n", value);
                fprintf(gen->fp, "    push rax\n");
                free(value);
            }
            break;
        }

        default:
            break;
    }
}

void generate(AST_Node* node, Generator* gen) {
    if (node == NULL) {
        return;
    }

    switch (node->type) {
        case MEP:
            fprintf(gen->fp, "_start:\n");

            for (size_t i = 0; i < node->data.mep.statements->size; i++) {
                gen_stmt(node->data.mep.statements->statement[i], gen);
            }

            fprintf(gen->fp, "    mov eax, 60\n");
            fprintf(gen->fp, "    syscall\n\n");

            fprintf(gen->fp, "    xor edi, edi\n");

            break;
        default:
            break;
    }

    generate(node->left, gen);
    generate(node->right, gen);
}

void GEN(AST_Node *root) {
    Generator* gen = gen_init("prog.asm");
    generate(root, gen);
    
    gen_string_lits(gen->fp, gen->str_literals->head);

    fclose(gen->fp);

    ASM_StringSymbol *current_string = gen->str_literals->head;
    while (current_string != NULL) {
        ASM_StringSymbol *next = current_string->next;
        free(current_string->label);
        free(current_string->value);
        free(current_string);
        current_string = next;
    }
}