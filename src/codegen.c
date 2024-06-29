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

    gen->char_literals = malloc(sizeof(CharLiteralManager));
    
    if (gen->char_literals == NULL) {
        perror("Memory allocation failed");
        fclose(gen->fp);
        free(gen->str_literals);
        free(gen);
        exit(EXIT_FAILURE);
    }
    
    gen->char_literals->head = NULL;
    gen->char_literals->counter = 0;

    fprintf(gen->fp, ".global _start\n");
    fprintf(gen->fp, ".intel_syntax noprefix\n\n");

    return gen;
}

void gen_free(Generator* gen) {
    fclose(gen->fp);

    ASM_StringSymbol *current_string = gen->str_literals->head;
    while (current_string != NULL) {
        ASM_StringSymbol *str_next = current_string->next;
        free(current_string->label);
        free(current_string->value);
        free(current_string);
        current_string = str_next;
    }

    ASM_CharSymbol *current_char = gen->char_literals->head;
    while (current_char != NULL) {
        ASM_CharSymbol *char_next = current_char->next;
        free(current_char->label);
        free(current_char);
        current_char = char_next;
    }
}


unsigned long hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

void double_to_ieee_hex(double value, char* hex_str) {
    union {
        double dval;
        uint64_t uval;
    } u = { .dval = value };

    uint64_t sign_bit = u.uval & 0x8000000000000000ULL;
    uint64_t exponent_bits = u.uval & 0x7FF0000000000000ULL;
    uint64_t significand_bits = u.uval & 0x000FFFFFFFFFFFFFULL;

    uint16_t exponent = (exponent_bits >> 52) - 1023; // Adjusted exponent bias

    sprintf(hex_str, "%s%llX%llX", (sign_bit == 0) ? "0x" : "-0x", (unsigned long long)exponent, (unsigned long long)significand_bits);
}


void float_to_ieee_hex(float value, char* hex_str) {
    union {
        float fval;
        uint32_t uval;
    } u = {.fval = value };

    uint32_t sign_bit = u.uval & 0x80000000U;
    uint32_t exponent_bits = u.uval & 0x7F800000U;
    uint32_t significand_bits = u.uval & 0x007FFFFFU;

    int8_t exponent = (exponent_bits >> 23) - 127;

    sprintf(hex_str, "%s%X%X", (sign_bit == 0) ? "0x" : "-0x", exponent, significand_bits);
}


void handle_literal_agn(Generator* gen, ASTN_VariableDecl* decl) {
    char* value;
    size_t value_str_size;
    unsigned long label_hash;  

    ASTN_Expression* variable_decl = &decl->expr->data.expr;

    switch (variable_decl->data.literal.type) {
        case TOK_L_SSINT:
            fprintf(gen->fp, "    mov byte ptr [rsp + %zu], %d\n", (gen->cur_variables.size -= 1), variable_decl->data.literal.value.int_.bit8);            
            break;
        case TOK_L_SINT:
            fprintf(gen->fp, "    mov word ptr [rsp + %zu], %d\n", (gen->cur_variables.size -= 2), variable_decl->data.literal.value.int_.bit16);
            break;
        case TOK_L_INT:
            fprintf(gen->fp, "    mov dword ptr [rsp + %zu], %d\n", (gen->cur_variables.size -= 4), variable_decl->data.literal.value.int_.bit32);
            break;
        case TOK_L_LINT:
            printf("hello\n");
            fprintf(gen->fp, "    mov rax, %ld\n", variable_decl->data.literal.value.int_.bit64);
            fprintf(gen->fp, "    mov qword [rsp + %zu], rax\n", (gen->cur_variables.size -= 8));
            break;
        case TOK_L_LLINT:
            fprintf(gen->fp, "    mov rax, %lu\n", variable_decl->data.literal.value.int_.bit128.low);
            fprintf(gen->fp, "    mov rdx, %lu\n", variable_decl->data.literal.value.int_.bit128.high);
            fprintf(gen->fp, "    mov qword [rsp + %zu], rax\n", (gen->cur_variables.size -= 8));
            fprintf(gen->fp, "    mov qword [rsp + %zu], rdx\n", (gen->cur_variables.size -= 8));
            break;
        case TOK_L_SSUINT:
            fprintf(gen->fp, "    mov byte ptr [rsp + %zu], %u\n", (gen->cur_variables.size -= 1), variable_decl->data.literal.value.uint.bit8);
            break;
        case TOK_L_SUINT:
            fprintf(gen->fp, "    mov word ptr [rsp + %zu], %u\n", (gen->cur_variables.size -= 2), variable_decl->data.literal.value.uint.bit16);
            break;
        case TOK_L_UINT:
            fprintf(gen->fp, "    mov dword ptr [rsp + %zu], %u\n", (gen->cur_variables.size -= 4),  variable_decl->data.literal.value.uint.bit32);
            break;
        case TOK_L_LUINT:
            fprintf(gen->fp, "    mov rax, %lu\n", variable_decl->data.literal.value.uint.bit64);
            fprintf(gen->fp, "    mov qword [rsp + %zu], rax\n", (gen->cur_variables.size -= 8));
            break;
        case TOK_L_LLUINT:
            fprintf(gen->fp, "    mov rax, %lu\n", variable_decl->data.literal.value.uint.bit128.low);
            fprintf(gen->fp, "    mov rdx, %lu\n", variable_decl->data.literal.value.uint.bit128.high);
            fprintf(gen->fp, "    mov qword [rsp + %zu], rax\n", (gen->cur_variables.size -= 8));
            fprintf(gen->fp, "    mov qword [rsp + %zu], rdx\n", (gen->cur_variables.size -= 8));
            break;
        case TOK_L_FLOAT:
            value = malloc(11);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }

            float_to_ieee_hex(variable_decl->data.literal.value.float_.bit32, value);

            fprintf(gen->fp, "    mov eax, %s\n", value);
            fprintf(gen->fp, "    movd xmm0, eax\n");
            fprintf(gen->fp, "    cvttss2si rax, xmm0\n");
            fprintf(gen->fp, "    mov qword [rsp + %zu], rax\n", (gen->cur_variables.size -= 4));
            break;
        case TOK_L_DOUBLE:
            value = malloc(18);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            
            float_to_ieee_hex(variable_decl->data.literal.value.float_.bit32, value);

            fprintf(gen->fp, "    mov eax, %s\n", value);
            fprintf(gen->fp, "    movd xmm0, eax\n");
            fprintf(gen->fp, "    cvttss2si rax, xmm0\n");
            fprintf(gen->fp, "    mov qword [rsp + %zu], rax\n", (gen->cur_variables.size -= 8));
            break;
        case TOK_L_CHAR:
            label_hash = gen_char_symb(&(gen->char_literals->head), variable_decl->data.literal.value.character, &(gen->char_literals->counter), true, decl->iden.sg);
            // fprintf(gen->fp, "    mov rax, [char_%ld_%d]\n", label_hash, gen->char_literals->counter - 1);
            // fprintf(gen->fp, "    push rax\n");    
            break;
        case TOK_L_STRING:
            label_hash = gen_str_symb(&(gen->str_literals->head), variable_decl->data.literal.value.string, &(gen->str_literals->counter), true, decl->iden.sg);            
            // fprintf(gen->fp, "    mov rax, [str_%ld_%d]\n", label_hash, gen->str_literals->counter - 1);
            // fprintf(gen->fp, "    push rax\n");    
            break;
        case TOK_TRUE:
        case TOK_FALSE:
            fprintf(gen->fp, "    mov byte ptr [rsp + %zu], %u\n", (gen->cur_variables.size -= 1), variable_decl->data.literal.value.boolean);
            break;
        case TOK_L_SIZE:
                        fprintf(gen->fp, "    mov rax, %lu\n", variable_decl->data.literal.value.size);
            fprintf(gen->fp, "    mov qword [rsp + %zu], rax\n", (gen->cur_variables.size -= 8));
            break;
        default:
            fprintf(stderr, "Unsupported literal type\n");
            exit(EXIT_FAILURE);
    }
}

unsigned long gen_str_symb(ASM_StringSymbol** head, const char* str, int* counter, bool is_def, uint32_t dest_hash) {
    unsigned long hash;

    char label[32];
    
    if (is_def) {
        hash = dest_hash;
    } else if (dest_hash == 0 && str != NULL) {
        hash = hash_string(str);
    }

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

unsigned long gen_char_symb(ASM_CharSymbol** head, char ch, int* counter, bool is_def, uint32_t dest_hash) {
    unsigned long hash;
    char label[32];

    if (is_def) {
        hash = dest_hash;
    } else {
        hash = hash_string(&ch);
    }

    snprintf(label, sizeof(label), "char_%lu_%d", hash, (*counter)++);

    ASM_CharSymbol* new_node = malloc(sizeof(ASM_CharSymbol));
    if (new_node == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    new_node->label = strdup(label);
    if (new_node->label == NULL) {
        perror("String duplication failed");
        exit(EXIT_FAILURE);
    }
    new_node->value = ch;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        ASM_CharSymbol* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }

    return hash;
}


void gen_string_lits(FILE* fp, ASM_StringSymbol* head) {
    ASM_StringSymbol* current = head;
    while (current != NULL) {
        fprintf(fp, "%s:\n", current->label);
        fprintf(fp, "    .asciz \"%s\"\n", current->value);
        current = current->next;
    }
}

void gen_char_lits(FILE* fp, ASM_CharSymbol* head) {
    ASM_CharSymbol* current = head;
    while (current != NULL) {
        fprintf(fp, "%s:\n", current->label);
        fprintf(fp, "    .byte '%c'\n", current->value);
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
                printf("hello");
                char *string_value = statement->data.stm.data.call.params->parameter[0]->data.expr.data.literal.value.string;
                unsigned long label_hash = gen_str_symb(&(gen->str_literals->head), string_value, &(gen->str_literals->counter), false, 0);
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

            if (statement->data.stm.data.variable_decl.iden.mult.size) {
                // for (int i = 0; i < statement->data.stm.data.variable_decl.iden.mult.size; i++) {
                //     var_name = statement->data.stm.data.variable_decl.iden.mult.items[i];

                //     fprintf(gen->fp, "    mov rax, %s\n", "0");
                //     fprintf(gen->fp, "    push rax\n");
                //     free(value);
                // }
            } else {
                var_name = statement->data.stm.data.variable_decl.iden.sg;
                handle_literal_agn(gen, &statement->data.stm.data.variable_decl);
            }
            break;
        }

        default:
            break;
    }
}

void generate_program(AST_Node* node, Generator* gen) {
    if (node == NULL) {
        return;
    }

    switch (node->type) {
        case MEP:
            fprintf(gen->fp, "_start:\n");

            size_t total_mem = 0;
            for (size_t i = 0; i < node->data.mep.statements->size; i++) {
                if (node->data.mep.statements->statement[i]->data.stm.type == STMT_VARIABLE_DECL) {
                    total_mem += node->data.mep.statements->statement[i]->data.stm.data.variable_decl.mem;
                }
            }

            if (total_mem > 0) {
                fprintf(gen->fp, "    sub rsp, %zu\n", total_mem);
            }

            gen->cur_variables.size = total_mem;

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

    generate_program(node->left, gen);
    generate_program(node->right, gen);
}



void generate_data(Generator* gen) {
    fprintf(gen->fp, "\n.data\n");
    gen_string_lits(gen->fp, gen->str_literals->head);
    gen_char_lits(gen->fp, gen->char_literals->head);
}

void GEN(AST_Node *root) {
    Generator* gen = gen_init("prog.asm");

    generate_program(root, gen);
    generate_data(gen);

    gen_free(gen);
}