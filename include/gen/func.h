#ifndef GEN_FUNC_H
#define GEN_FUNC_H

#include <gen/utils.h>

#include <stdio.h>

#include "ast.h"
#include "gen.h"
#include "symtbl.h"

void gen_fn(ASTN_FunctionDecl fn, Generator* gen);
void gen_fnd(ASTN_FunctionDecl fn, Generator* gen);

void gen_call(ASTN_Call call, FILE* fp, SymTable* tbl);

#endif // GEN_FUNC_H