#ifndef SAO_H
#define SAO_H

#include "parser.h"

void SAO(AST_Node** roots, SymTable** tbls, char** files, uint32_t count, uint32_t cur);
void trav(AST_Node** roots, SymTable** tbls, char** files, uint32_t count, AST_Node* node, SymTable* tbl);
bool resolve_sym(SymTable* src, SymTable* dest, ASTN_Modules mods);
int get_source_file_index(char** source_files, uint32_t source_files_count, char* filename);

#endif // SAO_H 
// build/nex examples/ret0.nex examples/vec.nex