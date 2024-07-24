#include "gen.h"

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



void gen_print_prep(Generator* gen, size_t size, size_t offset) {
    if (size == 1) {
        fprintf(gen->fp, "    movsx rax, byte [rsp + %zu]\n", offset);         
    } else if (size == 2) {
        fprintf(gen->fp, "    movsx rax, word [rsp + %zu]\n", offset);         
    } else if (size == 4) {
        fprintf(gen->fp, "    movsx rax, dword [rsp + %zu]\n", offset);         
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

void handle_literal_agn(Generator* gen, ASTN_VariableDecl decl) {
    char* value;
    unsigned long char_label_hash;  
    unsigned long str_label_hash;  

    switch (decl.data_type_specifier.data.prim) {
        case TOK_L_SSINT:
            fprintf(gen->fp, "    mov byte [rsp + %zu], %d\n", (gen->cur_variables.size -= 1), (int8_t)decl.expr->data.expr.data.literal.value.int_.norm);            
            stackvar_push(&gen->cur_variables,  gen->cur_variables.size, decl.iden.sg, 1);
            break;
        case TOK_L_SINT:
            fprintf(gen->fp, "    mov word [rsp + %zu], %d\n", (gen->cur_variables.size -= 2), (int16_t)decl.expr->data.expr.data.literal.value.int_.norm);
            stackvar_push(&gen->cur_variables, gen->cur_variables.size, decl.iden.sg, 2);
            break;
        case TOK_L_INT:
            fprintf(gen->fp, "    mov dword [rsp + %zu], %d\n", (gen->cur_variables.size -= 4), (int32_t)decl.expr->data.expr.data.literal.value.int_.norm);
            stackvar_push(&gen->cur_variables, gen->cur_variables.size, decl.iden.sg, 4);
            break;
        case TOK_L_LINT:
            fprintf(gen->fp, "    mov rax, %ld\n", (int64_t)decl.expr->data.expr.data.literal.value.int_.norm);
            fprintf(gen->fp, "    mov qword [rsp + %zu], rax\n", (gen->cur_variables.size -= 8));
            stackvar_push(&gen->cur_variables, gen->cur_variables.size, decl.iden.sg, 8);
            break;
        case TOK_L_LLINT:
            fprintf(gen->fp, "    mov rax, %lu\n", decl.expr->data.expr.data.literal.value.int_.bit128.low);
            fprintf(gen->fp, "    mov rdx, %lu\n", decl.expr->data.expr.data.literal.value.int_.bit128.high);
            fprintf(gen->fp, "    mov qword [rsp + %zu], rax\n", (gen->cur_variables.size -= 8));
            fprintf(gen->fp, "    mov qword [rsp + %zu], rdx\n", (gen->cur_variables.size -= 8));
            stackvar_push(&gen->cur_variables, gen->cur_variables.size, decl.iden.sg, 16);
            break;
        case TOK_L_SSUINT:
            fprintf(gen->fp, "    mov byte [rsp + %zu], %u\n", (gen->cur_variables.size -= 1), (uint8_t)decl.expr->data.expr.data.literal.value.uint.norm);
            stackvar_push(&gen->cur_variables, gen->cur_variables.size, decl.iden.sg, 1);
            break;
        case TOK_L_SUINT:
            fprintf(gen->fp, "    mov word [rsp + %zu], %u\n", (gen->cur_variables.size -= 2), (uint16_t)decl.expr->data.expr.data.literal.value.uint.norm);
            stackvar_push(&gen->cur_variables, gen->cur_variables.size, decl.iden.sg, 2);
            break;
        case TOK_L_UINT:
            fprintf(gen->fp, "    mov dword [rsp + %zu], %u\n", (gen->cur_variables.size -= 4),  (uint32_t)decl.expr->data.expr.data.literal.value.uint.norm);
            stackvar_push(&gen->cur_variables, gen->cur_variables.size, decl.iden.sg, 4);
            break;
        case TOK_L_LUINT:
            fprintf(gen->fp, "    mov rax, %lu\n", (uint64_t)decl.expr->data.expr.data.literal.value.uint.norm);
            fprintf(gen->fp, "    mov qword [rsp + %zu], rax\n", (gen->cur_variables.size -= 8));
            stackvar_push(&gen->cur_variables, gen->cur_variables.size, decl.iden.sg, 8);
            break;
        case TOK_L_LLUINT:
            fprintf(gen->fp, "    mov rax, %lu\n", decl.expr->data.expr.data.literal.value.uint.bit128.low);
            fprintf(gen->fp, "    mov rdx, %lu\n", decl.expr->data.expr.data.literal.value.uint.bit128.high);
            fprintf(gen->fp, "    mov qword [rsp + %zu], rax\n", (gen->cur_variables.size -= 8));
            fprintf(gen->fp, "    mov qword [rsp + %zu], rdx\n", (gen->cur_variables.size -= 8));
            stackvar_push(&gen->cur_variables, gen->cur_variables.size, decl.iden.sg, 16);
            break;
        case TOK_L_FLOAT:
            value = malloc(11);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }

            FLOAT_TO_IEEE_HEX(decl.expr->data.expr.data.literal.value.float_.bit32, value);

            fprintf(gen->fp, "    mov dword [rsp + %zu], %s\n", (gen->cur_variables.size -= 4), value);
            stackvar_push(&gen->cur_variables, gen->cur_variables.size, decl.iden.sg, 4);
            break;
        case TOK_L_DOUBLE:
            value = malloc(19);
            if (value == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            
            DOUBLE_TO_IEEE_HEX(decl.expr->data.expr.data.literal.value.float_.bit64, value);

            fprintf(gen->fp, "    mov qword [rsp + %zu], %s\n", (gen->cur_variables.size -= 8), value);
            stackvar_push(&gen->cur_variables, gen->cur_variables.size, decl.iden.sg, 8);
            break;
        case TOK_L_CHAR:
            char_label_hash = gen_char_symb(&(gen->char_literals->head), decl.expr->data.expr.data.literal.value.character, &(gen->char_literals->counter), true, decl.iden.sg);
            char _label[50];
            snprintf(_label, sizeof(_label), "char_%ld_%d", char_label_hash, gen->char_literals->counter - 1);
            csstackvar_push(&gen->cur_csvariables, _label, true, decl.iden.sg);
            break;
        case TOK_L_STRING:
            str_label_hash = gen_str_symb(&(gen->str_literals->head), decl.expr->data.expr.data.literal.value.string, &(gen->str_literals->counter), true, decl.iden.sg);
            char label[50];
            snprintf(label, sizeof(label), "str_%ld_%d", str_label_hash, gen->str_literals->counter - 1);
            csstackvar_push(&gen->cur_csvariables, label, false, decl.iden.sg);
            break;
        case TOK_TRUE:
        case TOK_FALSE:
            fprintf(gen->fp, "    mov byte [rsp + %zu], %u\n", (gen->cur_variables.size -= 1), decl.expr->data.expr.data.literal.value.boolean);
            stackvar_push(&gen->cur_variables, gen->cur_variables.size, decl.iden.sg, 1);
            break;
        case TOK_L_SIZE:
            fprintf(gen->fp, "    mov qword [rsp + %zu], rax\n", (gen->cur_variables.size -= 8));
            stackvar_push(&gen->cur_variables, gen->cur_variables.size, decl.iden.sg, 8);
            break;
        default:
            fprintf(gen->fp, "    mov dword [rsp + %zu], 0\n", (gen->cur_variables.size -= 8));
            stackvar_push(&gen->cur_variables, gen->cur_variables.size, decl.iden.sg, 8);
            break;
    }
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
            arth_lit_expr(statement, gen->cur_variables, gen->fp);
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