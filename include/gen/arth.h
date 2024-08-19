#ifndef GEN_ARTH_H
#define GEN_ARTH_H

#include <stdlib.h>

#include "utils/ieee.h"

#include "ast.h"
#include "stack.h"

// typedef struct GenArthData {
    // char* reg;
    // char* sireg;
    // char* size; 
// } GenArthData;


char* gen_binop(AST_Node* expr, FILE* fp, char* res);

#endif // GEN_ARTH_H