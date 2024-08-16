#ifndef GEN_UTILS_H
#define GEN_UTILS_H

#include <stdint.h>

#include "token.h"
#include "ast.h"

char* shortkw(uint8_t kw);

char* glval(AST_Node* expr);
int16_t gltype(AST_Node* expr);

#endif // GEN_UTILS_H