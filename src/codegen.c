#include "codegen.h"

void GEN(AST_Node *root) {
    FILE *fp = fopen("prog.asm", "w");
    if (fp == NULL) {
        perror("Error opening output file");
        return;
    }

    fprintf(fp, "section .text\n");
    fprintf(fp, "global _start\n\n");

    
    generate_code_for_ast(root, fp);

    fclose(fp);
}

void generate_code_for_ast(AST_Node *node, FILE *fp) {
    if (node == NULL) {
        return;
    }

    switch (node->type) {
        case MEP:
            fprintf(fp, "_start:\n");

            for (size_t i = 0; i < node->data.mep.statements->size; i++) {
                generate_code_for_statement(node->data.mep.statements->statement[i], fp);
            }

            fprintf(fp, "    mov eax, 60        ; System call number for exit (sys_exit)\n");
            fprintf(fp, "    syscall            ; Invoke the system call\n\n");

            break;
        default:
            break;
    }

    generate_code_for_ast(node->left, fp);
    generate_code_for_ast(node->right, fp);
}

void generate_code_for_statement(AST_Node *statement, FILE *fp) {
    if (statement == NULL) {
        return;
    }

    switch (statement->data.stm.type) {
        case STMT_RETURN:

            int return_value = statement->data.stm.data.return_stm.expr->data.expr.data.literal.value.uint.bit64;
            fprintf(fp, "    mov edi, %d        ;\n", return_value);
            break;
        default:
            break;
    }
}
