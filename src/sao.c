#include "sao.h"


void SAO(AST_Node *root) {
    trav(root);

    // optimize and analyze
}

void trav(AST_Node *node) {
    // switch (node->type) {
    //     case MEP:
    //         analyze_function(&node->data.mep);
    //         break;
    //     case STMT:
    //         analyze_return_statement(&node->data.stm);
    //         break;
    //     default:
    //         printf("Analyzing node of type: %d\n", node->type);
    //         break;
    // }

    if (node->right != NULL) {
        trav(node->right);
    }
    
    if (node->left != NULL) {
        trav(node->left);
    }
}
