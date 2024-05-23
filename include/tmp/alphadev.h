#ifndef ALPHADEV_H
#define ALPHADEV_H

#include "p_info.h"
#include "ast.h"
#include "symtbl.h"

#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#define PRINT_AST_NODE print_ast_node
#define PRINT_SYMB_TBL print_symb_tbl


void print_ast_node(AST_Node* node, int indent_level);
void print_symb_tbl(Symbol* cur);

#endif // ALPHADEV_H