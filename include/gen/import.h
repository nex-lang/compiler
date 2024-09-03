#ifndef GEN_IMPORT_H
#define GEN_IMPORT_H

#include <gen/utils.h>

#include <stdio.h>

#include "ast.h"
#include "gen.h"
#include "symtbl.h"

void gen_import(ASTN_ImportDecl imp, Generator* gen);

void gen_module(ASTN_Module* mod, Generator* gen);

#endif // GEN_IMPORT_H