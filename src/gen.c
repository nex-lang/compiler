#include "gen.h"

#include "ast.h"
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

    return gen;
}

void gen_free(Generator* gen) {
    fclose(gen->fp);
    free(gen);
}

void gen_stmt(AST_Node* statement, Generator* gen) {
    if (statement == NULL) {
        return;
    }

    switch (statement->data.stm.type) {
        case STMT_RETURN:
            int16_t lit_t = gltype(statement->data.stm.data.return_stm.value.expr);
            
            if (lit_t == -1) {
                // HANDLE ACTUAL EXPR RETURNS
                WO(gen->fp, 1, "ret\n"); 
                break;
            }

            char* str = glval(statement->data.stm.data.return_stm.value.expr);

            if (str == NULL) {
                // ERROR
                WO(gen->fp, 1, "ret\n"); 
                break;
            }

            WO(gen->fp, 1, "ret %s %s\n", shortkw((uint8_t)lit_t), str);
            break; 
        case STMT_CALL:
            gen_call(statement->data.stm.data.call, gen->fp, gen->tbl);
            break;
        case STMT_VARIABLE_DECL:
            gen_var_decl(statement->data.stm.data.variable_decl, gen->fp);
            break;
        case STMT_ASSGN:
            gen_assgn(statement->data.stm.data.assgn, gen->fp, gen->tbl);
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
            WO(gen->fp, 0, "define i32 @_start() {\n");

            for (size_t i = 0; i < node->data.mep.statements->size; i++) {
                gen_stmt(node->data.mep.statements->statement[i], gen);
            }

            WO(gen->fp, 0, "}");
            break;
        case STMT:
            if (node->data.stm.type == STMT_FUNCTION_DECL) {
                gen_fn(node->data.stm.data.function_decl, gen);
                break;
            } 
                // gen_import(node->data.stm.data.function_decl, gen);
            // }
        default:
            break;
    }

    if (node->next != NULL) {
        generate_program(node->next, gen);
    }
}

void GEN(AST_Node *root, char* name, SymTable* tbl) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s.inr", name);


    Generator* gen = gen_init(filename);
    gen->tbl = tbl;
    
    generate_program(root, gen);
    // generate_data(gen);

    gen_free(gen);
}