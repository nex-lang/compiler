#include "codegen.h"

#include "token.h"

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
    
        
    gen->cur_variables.ac_size = 0;
    gen->cur_variables.vars = malloc(sizeof(StackVar));

    gen->char_literals->head = NULL;
    gen->char_literals->counter = 0;

    gen->cur_csvariables.size = 0;
    gen->cur_csvariables.vars = malloc(sizeof(char*));
    gen->cur_csvariables.id = malloc(sizeof(uint32_t));

    gen->cur_csvariables.char_size = 0;
    gen->cur_csvariables.char_vars = malloc(sizeof(char*));
    gen->cur_csvariables.char_id = malloc(sizeof(uint32_t));


    if (gen->char_literals == NULL) {
        perror("Memory allocation failed");
        fclose(gen->fp);
        free(gen->str_literals);
        free(gen);
        exit(EXIT_FAILURE);
    }

    fprintf(gen->fp, "section .text\n");
    fprintf(gen->fp, "global _start\n\n");

    fprintf(gen->fp, "%%include \"src/asm/print.asm\"\n\n");

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
    sprintf(hex_str, "0x%016f", value);
}

void float_to_ieee_hex(float value, char* hex_str) {
    union {
        float f;
        uint32_t u;
    } float_union;

    float_union.f = value;
    sprintf(hex_str, "0x%08X", float_union.u);
}

arthdata get_arth_regsize(size_t sz) {
    arthdata result;

    switch (sz) {
        case 1:
            result.reg = "al";
            result.sireg = "bl";
            result.size = "byte";
            break;
        case 2:
            result.reg = "ax";
            result.sireg = "bx";
            result.size = "word";
            break;
        case 4:
            result.reg = "eax";
            result.sireg = "ebx";
            result.size = "dword";
            break;
        case 8:
            result.reg = "rax";
            result.sireg = "rbx";
            result.size = "qword";
            break;
        default:
            result.reg = "unknown";
            result.size = "unknown";
            break;
    }

    return result;
}


void gen_print_prep(Generator* gen, size_t size, size_t offset) {
    if (size == 1) {
        fprintf(gen->fp, "    movsx rax, byte [rsp + %zu]\n", offset);         
    } else if (size == 2) {
        fprintf(gen->fp, "    movsx rax, word [rsp + %zu]\n", offset);         
    } else if (size == 4) {
        fprintf(gen->fp, "    mov rax, dword [rsp + %zu]\n", offset);         
    } else if (size == 8) {
        fprintf(gen->fp, "    mov rax, qword [rsp + %zu]\n", offset);         
    }
}

bool gen_for_char_str(Generator* gen, uint32_t iden) {
    for (size_t i = 0; i < gen->cur_csvariables.char_size; i++) {
        if (gen->cur_csvariables.char_id[i] == iden) {
            fprintf(gen->fp, "    lea rsi, [%s]\n", gen->cur_csvariables.char_vars[i]);         
            fprintf(gen->fp, "    mov rcx, 9\n");
            return true;
        }
    }

    for (size_t i = 0; i < gen->cur_csvariables.size; i++) {
        if (gen->cur_csvariables.id[i] == iden) {
            printf("%s\n", gen->cur_csvariables.vars[i]);

            fprintf(gen->fp, "    lea rsi, [%s]\n", gen->cur_csvariables.vars[i]); 
            fprintf(gen->fp, "    mov rcx, 10\n");
            return true;
        }
    }

    return false;
}

void stackvar_push(Generator* gen, size_t offset, uint32_t id, size_t size) {
    StackVar* var = malloc(sizeof(StackVar));

    var->offset = offset;
    var->id = id;
    var->size = size;

    gen->cur_variables.ac_size += 1;
    gen->cur_variables.vars = realloc(gen->cur_variables.vars, sizeof(StackVar) * (gen->cur_variables.ac_size));
    gen->cur_variables.vars[gen->cur_variables.ac_size - 1] = var;
}

void csstackvar_push(Generator* gen, char* str, bool is_char, uint32_t iden) {
    if (is_char == true) {
        gen->cur_csvariables.char_size += 1;
        
        gen->cur_csvariables.char_vars = realloc(gen->cur_csvariables.char_vars, sizeof(char*) * (gen->cur_csvariables.char_size));
        gen->cur_csvariables.char_id = realloc(gen->cur_csvariables.char_id, sizeof(uint32_t) * (gen->cur_csvariables.char_size));
        
        gen->cur_csvariables.char_vars[gen->cur_csvariables.char_size - 1] = strdup(str);
        gen->cur_csvariables.char_id[gen->cur_csvariables.char_size - 1] = iden;
    } else {
        gen->cur_csvariables.size += 1;

        gen->cur_csvariables.vars = realloc(gen->cur_csvariables.vars, sizeof(char*) * (gen->cur_csvariables.size));
        gen->cur_csvariables.id = realloc(gen->cur_csvariables.id, sizeof(uint32_t) * (gen->cur_csvariables.size));
        

        gen->cur_csvariables.vars[gen->cur_csvariables.size - 1] = strdup(str);
        gen->cur_csvariables.id[gen->cur_csvariables.size - 1] = iden;
    }
}


void handle_literal_agn(Generator* gen, ASTN_VariableDecl decl) {
    char* value;
    unsigned long char_label_hash;  
    unsigned long str_label_hash;  

    switch (decl.data_type_specifier.data.prim) {
        case TOK_L_SSINT:
            fprintf(gen->fp, "    mov byte [rsp + %zu], %d\n", (gen->cur_variables.size -= 1), (int8_t)decl.expr->data.expr.data.literal.value.int_.norm);            
            stackvar_push(gen, gen->cur_variables.size, decl.iden.sg, 1);
            break;
        case TOK_L_SINT:
            fprintf(gen->fp, "    mov word [rsp + %zu], %d\n", (gen->cur_variables.size -= 2), (int16_t)decl.expr->data.expr.data.literal.value.int_.norm);
            stackvar_push(gen, gen->cur_variables.size, decl.iden.sg, 2);
            break;
        case TOK_L_INT:
            fprintf(gen->fp, "    mov dword [rsp + %zu], %d\n", (gen->cur_variables.size -= 4), (int32_t)decl.expr->data.expr.data.literal.value.int_.norm);
            stackvar_push(gen, gen->cur_variables.size, decl.iden.sg, 4);
            break;
        case TOK_L_LINT:
            fprintf(gen->fp, "    mov rax, %ld\n", (int64_t)decl.expr->data.expr.data.literal.value.int_.norm);
            fprintf(gen->fp, "    mov qword [rsp + %zu], rax\n", (gen->cur_variables.size -= 8));
            stackvar_push(gen, gen->cur_variables.size, decl.iden.sg, 8);
            break;
        case TOK_L_LLINT:
            fprintf(gen->fp, "    mov rax, %lu\n", decl.expr->data.expr.data.literal.value.int_.bit128.low);
            fprintf(gen->fp, "    mov rdx, %lu\n", decl.expr->data.expr.data.literal.value.int_.bit128.high);
            fprintf(gen->fp, "    mov qword [rsp + %zu], rax\n", (gen->cur_variables.size -= 8));
            fprintf(gen->fp, "    mov qword [rsp + %zu], rdx\n", (gen->cur_variables.size -= 8));
            stackvar_push(gen, gen->cur_variables.size, decl.iden.sg, 16);
            break;
        case TOK_L_SSUINT:
            fprintf(gen->fp, "    mov byte [rsp + %zu], %u\n", (gen->cur_variables.size -= 1), (uint8_t)decl.expr->data.expr.data.literal.value.uint.norm);
            stackvar_push(gen, gen->cur_variables.size, decl.iden.sg, 1);
            break;
        case TOK_L_SUINT:
            fprintf(gen->fp, "    mov word [rsp + %zu], %u\n", (gen->cur_variables.size -= 2), (uint16_t)decl.expr->data.expr.data.literal.value.uint.norm);
            stackvar_push(gen, gen->cur_variables.size, decl.iden.sg, 2);
            break;
        case TOK_L_UINT:
            fprintf(gen->fp, "    mov dword [rsp + %zu], %u\n", (gen->cur_variables.size -= 4),  (uint32_t)decl.expr->data.expr.data.literal.value.uint.norm);
            stackvar_push(gen, gen->cur_variables.size, decl.iden.sg, 4);
            break;
        case TOK_L_LUINT:
            fprintf(gen->fp, "    mov rax, %lu\n", (uint64_t)decl.expr->data.expr.data.literal.value.uint.norm);
            fprintf(gen->fp, "    mov qword [rsp + %zu], rax\n", (gen->cur_variables.size -= 8));
            stackvar_push(gen, gen->cur_variables.size, decl.iden.sg, 8);
            break;
        case TOK_L_LLUINT:
            fprintf(gen->fp, "    mov rax, %lu\n", decl.expr->data.expr.data.literal.value.uint.bit128.low);
            fprintf(gen->fp, "    mov rdx, %lu\n", decl.expr->data.expr.data.literal.value.uint.bit128.high);
            fprintf(gen->fp, "    mov qword [rsp + %zu], rax\n", (gen->cur_variables.size -= 8));
            fprintf(gen->fp, "    mov qword [rsp + %zu], rdx\n", (gen->cur_variables.size -= 8));
            stackvar_push(gen, gen->cur_variables.size, decl.iden.sg, 16);
            break;
        case TOK_L_FLOAT:
            value = malloc(11);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }

            float_to_ieee_hex(decl.expr->data.expr.data.literal.value.float_.bit32, value);

            fprintf(gen->fp, "    mov dword [rsp + %zu], %s\n", (gen->cur_variables.size -= 4), value);
            stackvar_push(gen, gen->cur_variables.size, decl.iden.sg, 4);
            break;
        case TOK_L_DOUBLE:
            value = malloc(19);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            
            double_to_ieee_hex(decl.expr->data.expr.data.literal.value.float_.bit64, value);

            fprintf(gen->fp, "    mov qword [rsp + %zu], %s\n", (gen->cur_variables.size -= 8), value);
            stackvar_push(gen, gen->cur_variables.size, decl.iden.sg, 8);
            break;
        case TOK_L_CHAR:
            char_label_hash = gen_char_symb(&(gen->char_literals->head), decl.expr->data.expr.data.literal.value.character, &(gen->char_literals->counter), true, decl.iden.sg);
            char _label[50];
            snprintf(_label, sizeof(_label), "char_%ld_%d", char_label_hash, gen->char_literals->counter - 1);
            csstackvar_push(gen, _label, true, decl.iden.sg);
            break;
        case TOK_L_STRING:
            str_label_hash = gen_str_symb(&(gen->str_literals->head), decl.expr->data.expr.data.literal.value.string, &(gen->str_literals->counter), true, decl.iden.sg);
            char label[50];
            snprintf(label, sizeof(label), "str_%ld_%d", str_label_hash, gen->str_literals->counter - 1);
            csstackvar_push(gen, label, false, decl.iden.sg);
            break;
        case TOK_TRUE:
        case TOK_FALSE:
            fprintf(gen->fp, "    mov byte [rsp + %zu], %u\n", (gen->cur_variables.size -= 1), decl.expr->data.expr.data.literal.value.boolean);
            stackvar_push(gen, gen->cur_variables.size, decl.iden.sg, 1);
            break;
        case TOK_L_SIZE:
            fprintf(gen->fp, "    mov qword [rsp + %zu], rax\n", (gen->cur_variables.size -= 8));
            stackvar_push(gen, gen->cur_variables.size, decl.iden.sg, 8);
            break;
        default:
            fprintf(gen->fp, "    mov dword [rsp + %zu], 0\n", (gen->cur_variables.size -= 8));
            stackvar_push(gen, gen->cur_variables.size, decl.iden.sg, 8);
            break;
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
    char *new_value = malloc(str_len + 1);
    if (new_value == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    strcpy(new_value, str);
    new_value[str_len] = '\0';

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
        fprintf(fp, "    db \"%s\", 10, 0\n", current->value);
        current = current->next;
    }
}

void gen_char_lits(FILE* fp, ASM_CharSymbol* head) {
    ASM_CharSymbol* current = head;
    while (current != NULL) {
        fprintf(fp, "%s:\n", current->label);
        fprintf(fp, "    db '%c'\n", current->value);
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
            ASTN_Expression return_expr = statement->data.stm.data.return_stm.value.expr->data.expr;
            if (return_expr.data.identifier) {
                for (size_t i = 0; i < gen->cur_variables.ac_size; i++) {
                    if (gen->cur_variables.vars[i]->id == return_expr.data.identifier) {
                        fprintf(gen->fp, "    movzx edi, byte [rsp + %zu]\n", gen->cur_variables.vars[i]->offset);
                        break;
                    }
                }
                break;
            }

            fprintf(gen->fp, "    mov edi, %d\n", 0);
            break;
        }
        case STMT_CALL: {
            if (statement->data.stm.data.call.identifier == -1124075304) {
                ASTN_Expression expr = statement->data.stm.data.call.params->parameter[0]->data.expr; 
                if (expr.data.identifier) {
                    if (gen_for_char_str(gen, expr.data.identifier)) {
                        fprintf(gen->fp, "    call print\n");
                        break;
                    }

                    for (size_t i = 0; i < gen->cur_variables.ac_size; i++) {
                        if (gen->cur_variables.vars[i]->id == expr.data.identifier) {
                            gen_print_prep(gen, gen->cur_variables.vars[i]->size, gen->cur_variables.vars[i]->offset);
                            fprintf(gen->fp, "    mov rcx, 8\n"); 
                            fprintf(gen->fp, "    call print\n");
                            break;
                        }
                    }
                } else {
                    char *string_value = statement->data.stm.data.call.params->parameter[0]->data.expr.data.literal.value.string;
                    unsigned long label_hash = gen_str_symb(&(gen->str_literals->head), string_value, &(gen->str_literals->counter), false, 0);
                    fprintf(gen->fp, "    lea rsi, [str_%lu_%d]\n", label_hash, gen->str_literals->counter - 1);
                    fprintf(gen->fp, "    mov rdx, %zu\n", strlen(string_value) + 2);
                    fprintf(gen->fp, "    syscall\n");
                }
                break;
            }
            break;
        }
        case STMT_VARIABLE_DECL: {
            if (statement->data.stm.data.variable_decl.iden.mult.size > 1) {
                // for (int i = 0; i < statement->data.stm.data.variable_decl.iden.mult.size; i++) {
                //     var_name = statement->data.stm.data.variable_decl.iden.mult.items[i];

                //     fprintf(gen->fp, "    mov rax, %s\n", "0");
                //     fprintf(gen->fp, "    push rax\n");
                //     free(value);
                // }
            } else {
                handle_literal_agn(gen, statement->data.stm.data.variable_decl);
            }
            break;
        }
        case STMT_ASSGN: {
            gen_assgn(statement, gen);
            break;
        }
        case STMT_EXPRESSION: {
            gen_assgn(statement, gen);
            break;
        }
        default:
            break;
    }
}


void gen_assgn(AST_Node* stm, Generator* gen) {
    size_t offset = 0;
    size_t szl, szr, ofr, ofl;
    arthdata set_dta, dtal, dtar;
    for (size_t i = 0; i < gen->cur_variables.ac_size; i++) {
        if (gen->cur_variables.vars[i]->id == stm->data.stm.data.assgn.sg.id) {
            offset = gen->cur_variables.vars[i]->offset;
            set_dta = get_arth_regsize(gen->cur_variables.vars[i]->size);
            break;
        }
    }

    ASTN_Expression ex = stm->data.stm.data.assgn.sg.expr->data.expr;
    
    switch (ex.type) {
        case EXPR_FACTOR:
            if (ex.data.factor.data.unary_op.expr) {
                for (size_t i = 0; i < gen->cur_variables.ac_size; i++) {
                    if (gen->cur_variables.vars[i]->id == ex.data.factor.data.unary_op.expr->data.identifier) {
                        dtal = get_arth_regsize(gen->cur_variables.vars[i]->size);
                        fprintf(gen->fp, "    mov %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, gen->cur_variables.vars[i]->offset);
                        break;
                    }
                }
            } 
            
            if (ex.data.factor.data.unary_op.op == TOK_ADD_ADD) {
                fprintf(gen->fp, "    inc %s\n", dtal.reg);
            } else if (ex.data.factor.data.unary_op.op == TOK_MINUS_MINUS) {
                fprintf(gen->fp, "    dec %s\n", dtal.reg);
            }

            fprintf(gen->fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, dtal.reg);
            break;
        case EXPR_TERM:
            if (ex.data.term.data.binary_op.left) {
                for (size_t i = 0; i < gen->cur_variables.ac_size; i++) {
                    if (gen->cur_variables.vars[i]->id == ex.data.term.data.binary_op.left->data.identifier) {
                        dtal = get_arth_regsize(gen->cur_variables.vars[i]->size);
                        fprintf(gen->fp, "    movsx rax, %s [rsp + %zu]\n", dtal.size, gen->cur_variables.vars[i]->offset);
                        break;
                    }
                }
            } 

            if (ex.data.term.data.binary_op.right) {
                for (size_t i = 0; i < gen->cur_variables.ac_size; i++) {
                    if (gen->cur_variables.vars[i]->id == ex.data.term.data.binary_op.right->data.identifier) {
                        dtar = get_arth_regsize(gen->cur_variables.vars[i]->size);
                        fprintf(gen->fp, "    movsx rcx, %s [rsp + %zu]\n", dtar.size, gen->cur_variables.vars[i]->offset);
                        break;
                    }
                }
            } 

            fprintf(gen->fp, "    call term\n");
            fprintf(gen->fp, "    mov [rsp + %zu], rax\n", offset);
            break;
        case EXPR_MULTIPLICATION:
            if (ex.data.multiplication.data.binary_op.op == TOK_ASTK) {
            if (ex.data.multiplication.data.binary_op.left) {
                for (size_t i = 0; i < gen->cur_variables.ac_size; i++) {
                    if (gen->cur_variables.vars[i]->id == ex.data.multiplication.data.binary_op.left->data.identifier) {
                        dtal = get_arth_regsize(gen->cur_variables.vars[i]->size);
                        fprintf(gen->fp, "    mov %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, gen->cur_variables.vars[i]->offset);
                        break;
                    }
                }
            }

            
            if (ex.data.multiplication.data.binary_op.right) {
                for (size_t i = 0; i < gen->cur_variables.ac_size; i++) {
                    if (gen->cur_variables.vars[i]->id == ex.data.multiplication.data.binary_op.right->data.identifier) {
                        dtar = get_arth_regsize(gen->cur_variables.vars[i]->size);
                        fprintf(gen->fp, "    mov %s, %s [rsp + %zu]\n", dtar.sireg, dtar.size, gen->cur_variables.vars[i]->offset);
                        break;
                    }
                }
            }
            fprintf(gen->fp, "    movzx %s, %s\n", set_dta.reg, dtal.reg);
            fprintf(gen->fp, "    movzx %s, %s\n", set_dta.sireg, dtar.sireg);

            fprintf(gen->fp, "    imul %s, %s\n", set_dta.reg, set_dta.sireg);

            
            fprintf(gen->fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
            } else if (ex.data.multiplication.data.binary_op.op == TOK_SLASH) {
                if (ex.data.multiplication.data.binary_op.left) {
                    for (size_t i = 0; i < gen->cur_variables.ac_size; i++) {
                        if (gen->cur_variables.vars[i]->id == ex.data.multiplication.data.binary_op.left->data.identifier) {
                            dtal = get_arth_regsize(gen->cur_variables.vars[i]->size);
                            fprintf(gen->fp, "    mov %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, gen->cur_variables.vars[i]->offset);
                            break;
                        }
                    }
                }

                if (ex.data.multiplication.data.binary_op.right) {
                    for (size_t i = 0; i < gen->cur_variables.ac_size; i++) {
                        if (gen->cur_variables.vars[i]->id == ex.data.multiplication.data.binary_op.right->data.identifier) {
                            dtar = get_arth_regsize(gen->cur_variables.vars[i]->size);
                            fprintf(gen->fp, "    mov %s, %s [rsp + %zu]\n", dtar.sireg, dtar.size, gen->cur_variables.vars[i]->offset);
                            break;
                        }
                    }
                }

                char* rem_reg;
                char* qo_reg;

                if (strcmp(dtal.reg, "rax") == 0 || strcmp(dtar.reg, "rax") == 0) {
                    rem_reg = "rdx";
                    qo_reg = "rax";
                } else if (strcmp(dtal.reg, "eax") == 0 || strcmp(dtar.reg, "eax") == 0) {
                    rem_reg = "edx";
                    qo_reg = "eax";
                } else if (strcmp(dtal.reg, "ax") == 0 || strcmp(dtar.reg, "ax") == 0) {
                    rem_reg = "dx";
                    qo_reg = "ax";
                } else {
                    rem_reg = "ah";
                    qo_reg = "al";
                } 


                fprintf(gen->fp, "    xor %s, %s\n", rem_reg, rem_reg);
                fprintf(gen->fp, "    div %s\n", dtar.sireg);

                fprintf(gen->fp, "    movsx %s, %s\n", set_dta.reg, qo_reg);
                fprintf(gen->fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
            }
            break;
        case EXPR_ADDITION:
            if (ex.data.addition.data.binary_op.op == TOK_ADD) {
                if (ex.data.addition.data.binary_op.left) {
                for (size_t i = 0; i < gen->cur_variables.ac_size; i++) {
                    if (gen->cur_variables.vars[i]->id == ex.data.addition.data.binary_op.left->data.identifier) {
                        dtal = get_arth_regsize(gen->cur_variables.vars[i]->size);
                        szl = gen->cur_variables.vars[i]->size;
                        ofl = gen->cur_variables.vars[i]->offset;
                        break;
                    }
                } }

                if (ex.data.addition.data.binary_op.right) {
                for (size_t i = 0; i < gen->cur_variables.ac_size; i++) {
                    if (gen->cur_variables.vars[i]->id == ex.data.addition.data.binary_op.right->data.identifier) {
                        dtar = get_arth_regsize(gen->cur_variables.vars[i]->size);
                        szr = gen->cur_variables.vars[i]->size;
                        ofr = gen->cur_variables.vars[i]->offset;

                        if (szl == szr) {
                            fprintf(gen->fp, "    mov %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, ofl);
                            fprintf(gen->fp, "    add %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, ofr);
                            break;
                        }

                        if (szr > szl) {
                            fprintf(gen->fp, "    movsx %s, %s [rsp + %zu]\n", dtar.reg, dtal.size, ofl);
                            fprintf(gen->fp, "    add %s, %s [rsp + %zu]\n", dtar.reg, dtar.size, ofr);
                        
                            fprintf(gen->fp, "    movsx %s, %s\n", set_dta.reg, dtar.reg);
                            fprintf(gen->fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
                        }

                        if (szl > szr) {
                            fprintf(gen->fp, "    mov %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, ofl);
                            fprintf(gen->fp, "    movsx %s, %s [rsp + %zu]\n", dtal.sireg, dtar.size, ofr);
 
                            fprintf(gen->fp, "    add %s, %s\n", dtal.reg, dtal.sireg);

                            fprintf(gen->fp, "    movsx %s, %s\n", set_dta.reg, dtal.reg);
                            fprintf(gen->fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
                        }
                        
                        break;
                    }
                } }

                fprintf(gen->fp, "    movsx %s, %s\n", set_dta.reg, dtal.reg);
                fprintf(gen->fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
            } else if (ex.data.addition.data.binary_op.op == TOK_MINUS) {
                if (ex.data.addition.data.binary_op.left) {
                for (size_t i = 0; i < gen->cur_variables.ac_size; i++) {
                    if (gen->cur_variables.vars[i]->id == ex.data.addition.data.binary_op.left->data.identifier) {
                        dtal = get_arth_regsize(gen->cur_variables.vars[i]->size);
                        szl = gen->cur_variables.vars[i]->size;
                        ofl = gen->cur_variables.vars[i]->offset;
                        break;
                    }
                } }
                if (ex.data.addition.data.binary_op.right) {
                for (size_t i = 0; i < gen->cur_variables.ac_size; i++) {
                    if (gen->cur_variables.vars[i]->id == ex.data.addition.data.binary_op.right->data.identifier) {
                        dtar = get_arth_regsize(gen->cur_variables.vars[i]->size);
                        szr = gen->cur_variables.vars[i]->size;
                        ofr = gen->cur_variables.vars[i]->offset;
                        
                        if (szl == szr) {
                            fprintf(gen->fp, "    mov %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, ofl);
                            fprintf(gen->fp, "    sub %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, ofr);
                            break;
                        }

                        if (szr > szl) {
                            fprintf(gen->fp, "    movsx %s, %s [rsp + %zu]\n", dtar.reg, dtal.size, ofl);
                            fprintf(gen->fp, "    sub %s, %s [rsp + %zu]\n", dtar.reg, dtar.size, ofr);
                        
                            fprintf(gen->fp, "    movsx %s, %s\n", set_dta.reg, dtar.reg);
                            fprintf(gen->fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
                        }

                        if (szl > szr) {
                            fprintf(gen->fp, "    mov %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, ofl);
                            fprintf(gen->fp, "    movsx %s, %s [rsp + %zu]\n", dtal.sireg, dtar.size, ofr);
 
                            fprintf(gen->fp, "    sub %s, %s\n", dtal.reg, dtal.sireg);

                            fprintf(gen->fp, "    movsx %s, %s\n", set_dta.reg, dtal.reg);
                            fprintf(gen->fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
                        }
                    }
                } }                
            }
            break;
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

            if (total_mem > 0) {
                fprintf(gen->fp, "    add rsp, %zu\n", total_mem);
            }


            fprintf(gen->fp, "    mov rax, 60\n");
            fprintf(gen->fp, "    syscall\n\n");


            break;
        default:
            break;
    }

    generate_program(node->left, gen);
    generate_program(node->right, gen);
}



void generate_data(Generator* gen) {
    fprintf(gen->fp, "\nsection .data\n");
    gen_string_lits(gen->fp, gen->str_literals->head);
    gen_char_lits(gen->fp, gen->char_literals->head);
}

void GEN(AST_Node *root) {
    Generator* gen = gen_init("prog.asm");

    generate_program(root, gen);
    generate_data(gen);

    gen_free(gen);
}