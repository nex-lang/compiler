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
    free(node);
}