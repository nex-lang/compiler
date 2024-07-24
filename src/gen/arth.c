#include "gen/arth.h"

#include <string.h>

#include "token.h"

GenArthData get_arth_regsize(size_t sz) {
    GenArthData result;

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

void arth_lit_expr(AST_Node* stm, StackVars cur_variables, FILE* fp) {
    size_t offset = 0;
    size_t szl, szr, ofr, ofl;
    GenArthData set_dta, dtal, dtar;
    for (size_t i = 0; i < cur_variables.ac_size; i++) {
        if (cur_variables.vars[i]->id == stm->data.stm.data.assgn.sg.id) {
            offset = cur_variables.vars[i]->offset;
            set_dta = get_arth_regsize(cur_variables.vars[i]->size);
            break;
        }
    }

    ASTN_Expression ex = stm->data.stm.data.assgn.sg.expr->data.expr;
    
    switch (ex.type) {
        case EXPR_FACTOR:
            if (ex.data.factor.data.unary_op.expr) {
                for (size_t i = 0; i < cur_variables.ac_size; i++) {
                    if (cur_variables.vars[i]->id == ex.data.factor.data.unary_op.expr->data.identifier) {
                        dtal = get_arth_regsize(cur_variables.vars[i]->size);
                        fprintf(fp, "    mov %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, cur_variables.vars[i]->offset);
                        break;
                    }
                }
            } 
            
            if (ex.data.factor.data.unary_op.op == TOK_ADD_ADD) {
                fprintf(fp, "    inc %s\n", dtal.reg);
            } else if (ex.data.factor.data.unary_op.op == TOK_MINUS_MINUS) {
                fprintf(fp, "    dec %s\n", dtal.reg);
            }

            fprintf(fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, dtal.reg);
            break;
        case EXPR_TERM:
            if (ex.data.term.data.binary_op.left) {
                for (size_t i = 0; i < cur_variables.ac_size; i++) {
                    if (cur_variables.vars[i]->id == ex.data.term.data.binary_op.left->data.identifier) {
                        dtal = get_arth_regsize(cur_variables.vars[i]->size);
                        fprintf(fp, "    movsx rax, %s [rsp + %zu]\n", dtal.size, cur_variables.vars[i]->offset);
                        break;
                    }
                }
            } 

            if (ex.data.term.data.binary_op.right) {
                for (size_t i = 0; i < cur_variables.ac_size; i++) {
                    if (cur_variables.vars[i]->id == ex.data.term.data.binary_op.right->data.identifier) {
                        dtar = get_arth_regsize(cur_variables.vars[i]->size);
                        fprintf(fp, "    movsx rcx, %s [rsp + %zu]\n", dtar.size, cur_variables.vars[i]->offset);
                        break;
                    }
                }
            } 

            fprintf(fp, "    call term\n");
            fprintf(fp, "    mov [rsp + %zu], rax\n", offset);
            break;
        case EXPR_MULTIPLICATION:
            if (ex.data.multiplication.data.binary_op.left) {
                for (size_t i = 0; i < cur_variables.ac_size; i++) {
                    if (cur_variables.vars[i]->id == ex.data.multiplication.data.binary_op.left->data.identifier) {
                        dtal = get_arth_regsize(cur_variables.vars[i]->size);
                        fprintf(fp, "    mov %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, cur_variables.vars[i]->offset);
                        break;
                    }
                }
            }

            if (ex.data.multiplication.data.binary_op.right) {
                for (size_t i = 0; i < cur_variables.ac_size; i++) {
                    if (cur_variables.vars[i]->id == ex.data.multiplication.data.binary_op.right->data.identifier) {
                        dtar = get_arth_regsize(cur_variables.vars[i]->size);
                        fprintf(fp, "    mov %s, %s [rsp + %zu]\n", dtar.sireg, dtar.size, cur_variables.vars[i]->offset);
                        break;
                    }
                }
            }

            if (ex.data.multiplication.data.binary_op.op == TOK_ASTK) {
                fprintf(fp, "    movsx %s, %s\n", set_dta.reg, dtal.reg);
                fprintf(fp, "    movsx %s, %s\n", set_dta.sireg, dtar.sireg);

                fprintf(fp, "    imul %s, %s\n", set_dta.reg, set_dta.sireg);

                
                fprintf(fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
            } else if (ex.data.multiplication.data.binary_op.op == TOK_SLASH) {


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


                fprintf(fp, "    xor %s, %s\n", rem_reg, rem_reg);
                fprintf(fp, "    div %s\n", dtar.sireg);

                fprintf(fp, "    movsx %s, %s\n", set_dta.reg, qo_reg);
                fprintf(fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
            }
            break;
        case EXPR_ADDITION:
            if (ex.data.addition.data.binary_op.left) {
                for (size_t i = 0; i < cur_variables.ac_size; i++) {
                    if (cur_variables.vars[i]->id == ex.data.addition.data.binary_op.left->data.identifier) {
                        dtal = get_arth_regsize(cur_variables.vars[i]->size);
                        szl = cur_variables.vars[i]->size;
                        ofl = cur_variables.vars[i]->offset;
                        break;
                    }
                }
            }

            if (ex.data.addition.data.binary_op.right) {
                for (size_t i = 0; i < cur_variables.ac_size; i++) {
                    if (cur_variables.vars[i]->id == ex.data.addition.data.binary_op.right->data.identifier) {
                        dtar = get_arth_regsize(cur_variables.vars[i]->size);
                        szr = cur_variables.vars[i]->size;
                        ofr = cur_variables.vars[i]->offset;
                    }
                }
            }

            if (ex.data.addition.data.binary_op.op == TOK_ADD) {
                if (szl == szr) {
                    fprintf(fp, "    mov %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, ofl);
                    fprintf(fp, "    add %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, ofr);

                    fprintf(fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
                    break;
                }

                if (szr > szl) {
                    fprintf(fp, "    movsx %s, %s [rsp + %zu]\n", dtar.reg, dtal.size, ofl);
                    fprintf(fp, "    add %s, %s [rsp + %zu]\n", dtar.reg, dtar.size, ofr);
                
                    fprintf(fp, "    movsx %s, %s\n", set_dta.reg, dtar.reg);
                    fprintf(fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
                }

                if (szl > szr) {
                    fprintf(fp, "    mov %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, ofl);
                    fprintf(fp, "    movsx %s, %s [rsp + %zu]\n", dtal.sireg, dtar.size, ofr);

                    fprintf(fp, "    add %s, %s\n", dtal.reg, dtal.sireg);

                    fprintf(fp, "    movsx %s, %s\n", set_dta.reg, dtal.reg);
                    fprintf(fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
                }
                
                break;

                fprintf(fp, "    movsx %s, %s\n", set_dta.reg, dtal.reg);
                fprintf(fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
            } else if (ex.data.addition.data.binary_op.op == TOK_MINUS) {
                if (szl == szr) {
                    fprintf(fp, "    mov %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, ofl);
                    fprintf(fp, "    sub %s, %s [rsp + %zu]\n", dtal.reg, dtar.size, ofr);
                    
                    fprintf(fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
                    break;
                }

                if (szr > szl) {
                    fprintf(fp, "    movsx %s, %s [rsp + %zu]\n", dtar.reg, dtal.size, ofl);
                    fprintf(fp, "    sub %s, %s [rsp + %zu]\n", dtar.reg, dtar.size, ofr);
                
                    fprintf(fp, "    movsx %s, %s\n", set_dta.reg, dtar.reg);
                    fprintf(fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
                }

                if (szl > szr) {
                    fprintf(fp, "    mov %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, ofl);
                    fprintf(fp, "    movsx %s, %s [rsp + %zu]\n", dtal.sireg, dtar.size, ofr);

                    fprintf(fp, "    sub %s, %s\n", dtal.reg, dtal.sireg);

                    fprintf(fp, "    movsx %s, %s\n", set_dta.reg, dtal.reg);
                    fprintf(fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
                }
            }
            break;
        case EXPR_BITWISE:
            if (ex.data.bitwise.data.binary_op.left) {
                for (size_t i = 0; i < cur_variables.ac_size; i++) {
                    if (cur_variables.vars[i]->id == ex.data.bitwise.data.binary_op.left->data.identifier) {
                        dtal = get_arth_regsize(cur_variables.vars[i]->size);
                        szl = cur_variables.vars[i]->size;
                        ofl = cur_variables.vars[i]->offset;
                        break;
                    }
                } 
            }

            if (ex.data.bitwise.data.binary_op.right) {
                for (size_t i = 0; i < cur_variables.ac_size; i++) {
                    if (cur_variables.vars[i]->id == ex.data.bitwise.data.binary_op.right->data.identifier) {
                        dtar = get_arth_regsize(cur_variables.vars[i]->size);
                        szr = cur_variables.vars[i]->size;
                        ofr = cur_variables.vars[i]->offset;
                    }
                }
            }

            if (ex.data.bitwise.data.binary_op.op == TOK_LT_LT) {
                fprintf(fp, "    mov %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, ofl);

                if (strcmp(dtar.reg, "rax") == 0) {
                    fprintf(fp, "    mov rcx, %s [rsp + %zu]\n", dtar.size, ofr);
                    fprintf(fp, "    shl %s, rcx\n", dtal.reg); 

                    fprintf(fp, "    movsx %s, %s\n", set_dta.reg, dtal.reg);
                    fprintf(fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
                    break;
                }

                fprintf(fp, "    movsx rcx, %s [rsp + %zu]\n", dtar.size, ofr);
                fprintf(fp, "    shl %s, cl\n", dtal.reg); 

                fprintf(fp, "    movsx %s, %s\n", set_dta.reg, dtal.reg);
                fprintf(fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
            } else if (ex.data.bitwise.data.binary_op.op == TOK_GT_GT) {
                fprintf(fp, "    mov %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, ofl);

                if (strcmp(dtar.reg, "rax") == 0) {
                    fprintf(fp, "    mov rcx, %s [rsp + %zu]\n", dtar.size, ofr);
                    fprintf(fp, "    shr %s, cl\n", dtal.reg); 

                    fprintf(fp, "    movsx %s, %s\n", set_dta.reg, dtal.reg);
                    fprintf(fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
                    break;
                }

                fprintf(fp, "    movsx rcx, %s [rsp + %zu]\n", dtar.size, ofr);
                fprintf(fp, "    shr %s, cl\n", dtal.reg); 

                fprintf(fp, "    movsx %s, %s\n", set_dta.reg, dtal.reg);
                fprintf(fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
            } else if (ex.data.bitwise.data.binary_op.op == TOK_PIPE) { 
                fprintf(fp, "    mov %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, ofl);

                char* _reg;

                if (strcmp(dtal.reg, "rax") == 0 || strcmp(dtar.reg, "rax") == 0) {
                    _reg = "rcx";
                } else if (strcmp(dtal.reg, "eax") == 0 || strcmp(dtar.reg, "eax") == 0) {
                    _reg = "ecx";
                } else if (strcmp(dtal.reg, "ax") == 0 || strcmp(dtar.reg, "ax") == 0) {
                    _reg = "cx";
                } else {
                    _reg = "ch";
                } 


                if (szr == szl) {
                    fprintf(fp, "    mov %s, %s [rsp + %zu]\n", _reg, dtar.size, ofr);
                    fprintf(fp, "    or %s, %s\n", dtal.reg, _reg); 

                    fprintf(fp, "    movsx %s, %s\n", set_dta.reg, dtal.reg);
                    fprintf(fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
                    break;
                }
                

                fprintf(fp, "    movsx rcx, %s [rsp + %zu]\n", dtar.size, ofr);
                fprintf(fp, "    or %s, %s\n", dtal.reg, _reg); 

                fprintf(fp, "    movsx %s, %s\n", set_dta.reg, dtal.reg);
                fprintf(fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
            } else if (ex.data.bitwise.data.binary_op.op == TOK_AMPER) {
                fprintf(fp, "    mov %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, ofl);

                char* _reg;

                if (strcmp(dtal.reg, "rax") == 0 || strcmp(dtar.reg, "rax") == 0) {
                    _reg = "rcx";
                } else if (strcmp(dtal.reg, "eax") == 0 || strcmp(dtar.reg, "eax") == 0) {
                    _reg = "ecx";
                } else if (strcmp(dtal.reg, "ax") == 0 || strcmp(dtar.reg, "ax") == 0) {
                    _reg = "cx";
                } else {
                    _reg = "ch";
                } 


                if (szr == szl) {
                    fprintf(fp, "    mov %s, %s [rsp + %zu]\n", _reg, dtar.size, ofr);
                    fprintf(fp, "    and %s, %s\n", dtal.reg, _reg); 

                    fprintf(fp, "    movsx %s, %s\n", set_dta.reg, dtal.reg);
                    fprintf(fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
                    break;
                }
                

                fprintf(fp, "    movsx rcx, %s [rsp + %zu]\n", dtar.size, ofr);
                fprintf(fp, "    and %s, %s\n", dtal.reg, _reg); 

                fprintf(fp, "    movsx %s, %s\n", set_dta.reg, dtal.reg);
                fprintf(fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);
            }
            break;
        case EXPR_COMPARISON:
            if (ex.data.comparison.data.binary_op.left) {
                for (size_t i = 0; i < cur_variables.ac_size; i++) {
                    if (cur_variables.vars[i]->id == ex.data.comparison.data.binary_op.left->data.identifier) {
                        dtal = get_arth_regsize(cur_variables.vars[i]->size);
                        szl = cur_variables.vars[i]->size;
                        ofl = cur_variables.vars[i]->offset;
                        break;
                    }
                } 
            }

            if (ex.data.comparison.data.binary_op.right) {
                for (size_t i = 0; i < cur_variables.ac_size; i++) {
                    if (cur_variables.vars[i]->id == ex.data.comparison.data.binary_op.right->data.identifier) {
                        dtar = get_arth_regsize(cur_variables.vars[i]->size);
                        szr = cur_variables.vars[i]->size;
                        ofr = cur_variables.vars[i]->offset;
                    }
                }
            }

            const char* set_instr = NULL;
            switch (ex.data.bitwise.data.binary_op.op) {
                case TOK_LT: set_instr = "setl"; break;
                case TOK_GT: set_instr = "setg"; break;
                case TOK_LT_EQ: set_instr = "setle"; break;
                case TOK_GT_EQ: set_instr = "setge"; break;
                case TOK_EQ_EQ: set_instr = "sete"; break;
            }

            if (szl == szr) {
                fprintf(fp, "    mov %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, ofl);
                fprintf(fp, "    cmp %s, %s [rsp + %zu]\n", dtal.reg, dtar.size, ofr);
            } else if (szr > szl) {
                fprintf(fp, "    movsx %s, %s [rsp + %zu]\n", dtar.reg, dtal.size, ofl);                
                fprintf(fp, "    cmp %s, %s [rsp + %zu]\n", dtar.reg, dtar.size, ofr);
            } else {
                fprintf(fp, "    mov %s, %s [rsp + %zu]\n", dtal.reg, dtal.size, ofl);
                fprintf(fp, "    movsx %s, %s [rsp + %zu]\n", dtal.sireg, dtar.size, ofr);
                fprintf(fp, "    cmp %s, %s\n", dtal.reg, dtal.sireg);
            }

            fprintf(fp, "    %s al\n", set_instr);
            fprintf(fp, "    movsx %s, al\n", set_dta.reg);

            fprintf(fp, "    mov %s [rsp + %zu], %s\n", set_dta.size, offset, set_dta.reg);

        default:
            break;
    }
}