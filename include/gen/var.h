#ifndef GEN_VAR_H
#define GEN_VAR_H

#include "gen/stack.h"

#include <stdio.h>

#include "ast.h"
#include "symtbl.h"

void gen_var_decl(ASTN_VariableDecl decl, FILE* fp);
void gen_assgn(ASTN_AssignmentStm assgn, FILE* fp, SymTable* tbl);

#endif // GEN_VAR_H