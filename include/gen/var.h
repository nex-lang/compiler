#ifndef GEN_VAR_H
#define GEN_VAR_H

#include "gen/stack.h"

#include <stdio.h>

#include "ast.h"

void gen_var_decl(ASTN_VariableDecl decl, FILE* fp);

#endif // GEN_VAR_H