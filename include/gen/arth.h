#ifndef GEN_ARTH_H
#define GEN_ARTH_H

#include <stdlib.h>

#include "utils/ieee.h"

#include "ast.h"
#include "stack.h"

typedef struct GenArthData {
    char* reg;
    char* sireg;
    char* size; 
} GenArthData;

GenArthData get_arth_regsize(size_t sz);
void arth_lit_expr(AST_Node* stm, StackVars cur_variables, FILE* fp);

#endif // GEN_ARTH_H