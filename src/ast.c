#include "ast.h"

AST_Node* ast_init(int type) {
    AST_Node* node = calloc(1, sizeof(AST_Node));

    node->type = type;
    
    if (type == ROOT) {
        node->data.root = true;
    }

    return node;
}

void ast_free(AST_Node* node) {
    if (!node) {
        return;
    }

    AST_Node* current = node;
    AST_Node* next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
}