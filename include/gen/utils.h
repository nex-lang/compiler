#ifndef GEN_UTILS_H
#define GEN_UTILS_H

#include <stdint.h>

#include "token.h"
#include "ast.h"
#include "symtbl.h"

char* shortkw(uint8_t kw);

char* glval(AST_Node* expr);
int16_t gltype(AST_Node* expr);

char* get_fid(int iden);
char* get_fidtyvar(int iden, SymTable* tbl);

char* get_id(ASTN_Litlist iden);
char** get_ids(ASTN_Litlist iden);

#endif // GEN_UTILS_H