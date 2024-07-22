void AST_free(AST_Node* node) {
    if (!node) {
        return;
    }
    
    switch (node->type) {
        case ASTN_PROGRAM:
            for (size_t i = 0; i < node->data.program.statements.size; i++) {
                AST_free(node->data.program.statements.statement[i]);
            }
            free(node->data.program.statements.statement);
            break;
        case ASTN_MEP:
            free(node->data.mep.parameters);
            break;
        case ASTN_STM:
            switch (node->data.stm.type) {
                case ASTN_EXPRESSION:
                    AST_free(node->data.expression);
                    break;
                case ASTN_IMPORT_DECL:
                    for (size_t i = 0; i < node->data.stm.data.import_decl.modules.size; i++) {
                        AST_free(node->data.stm.data.import_decl.modules.items[i]);
                    }
                    free(node->data.stm.data.import_decl.modules.items);
                    break;
                case ASTN_VARIABLE_DECL:
                    AST_free(node->data.stm.data.variable_decl.expr);
                    break;
                case ASTN_FUNCTION_DECL:
                    for (size_t i = 0; i < node->data.stm.data.function_decl.statements.size; i++) {
                        AST_free(node->data.stm.data.function_decl.statements.statement[i]);
                    }
                    free(node->data.stm.data.function_decl.statements.statement);
                    break;
                case ASTN_STRUCT_DECL:
                    for (size_t i = 0; i < node->data.stm.data.struct_decl.members.size; i++) {
                        AST_free(node->data.stm.data.struct_decl.members.items[i]);
                    }
                    free(node->data.stm.data.struct_decl.members.items);
                    break;
                case ASTN_STRUCT_MEMBER_DECL:
                    AST_free(node->data.stm.data.struct_member_decl.data_type_specifier);
                    break;
                case ASTN_CLASS_DECL:
                    for (size_t i = 0; i < node->data.stm.data.class_decl.variables.size; i++) {
                        AST_free(node->data.stm.data.class_decl.variables.items[i]);
                    }
                    free(node->data.stm.data.class_decl.variables.items);
                    for (size_t i = 0; i < node->data.stm.data.class_decl.functions.size; i++) {
                        AST_free(node->data.stm.data.class_decl.functions.items[i]);
                    }
                    free(node->data.stm.data.class_decl.functions.items);
                    AST_free(node->data.stm.data.class_decl.init);
                    AST_free(node->data.stm.data.class_decl.free);
                    break;
                case ASTN_ENUM_DECL:
                    for (size_t i = 0; i < node->data.stm.data.enum_decl.members.size; i++) {
                        free(node->data.stm.data.enum_decl.members.items[i]);
                    }
                    free(node->data.stm.data.enum_decl.members.items);
                    break;
                case ASTN_CONDITIONAL_STM:
                    AST_free(node->data.stm.data.conditional_stm.if_condition);
                    for (size_t i = 0; i < node->data.stm.data.conditional_stm.elif_branches.size; i++) {
                        free(node->data.stm.data.conditional_stm.elif_branches.condition[i]);
                        for (size_t j = 0; j < node->data.stm.data.conditional_stm.elif_branches.statements[i].size; j++) {
                            AST_free(node->data.stm.data.conditional_stm.elif_branches.statements[i].statement[j]);
                        }
                        free(node->data.stm.data.conditional_stm.elif_branches.statements[i].statement);
                    }
                    free(node->data.stm.data.conditional_stm.elif_branches.condition);
                    free(node->data.stm.data.conditional_stm.elif_branches.statements.condition);
                    free(node->data.stm.data.conditional_stm.elif_branches.statements.statements);
                    AST_free(node->data.stm.data.conditional_stm.else_statements);
                    break;
                case ASTN_FOR_STM:
                    AST_free(node->data.stm.data.for_stm.condition_expr);
                    AST_free(node->data.stm.data.for_stm.next_expr);
                    AST_free(node->data.stm.data.for_stm.var_decl.expr);
                    for (size_t i = 0; i < node->data.stm.data.for_stm.statements.size; i++) {
                        AST_free(node->data.stm.data.for_stm.statements.statement[i]);
                    }
                    free(node->data.stm.data.for_stm.statements.statement);
                    break;
                case ASTN_SWITCH_STM:
                    AST_free(node->data.stm.data.switch_stm.condition_expr);
                    for (size_t i = 0; i < node->data.stm.data.switch_stm.clauses.size; i++) {
                        AST_free(node->data.stm.data.switch_stm.clauses.items[i].condition_expr);
                        for (size_t j = 0; j < node->data.stm.data.switch_stm.clauses.items[i].statements.size; j++) {
                            AST_free(node->data.stm.data.switch_stm.clauses.items[i].statements.statement[j]);
                        }
                        free(node->data.stm.data.switch_stm.clauses.items[i].statements.statement);
                    }
                    free(node->data.stm.data.switch_stm.clauses.items);
                    break;
                case ASTN_CASE_CLAUSE:
                    AST_free(node->data.stm.data.case_clause.condition_expr);
                    for (size_t i = 0; i < node->data.stm.data.case_clause.statements.size; i++) {
                        AST_free(node->data.stm.data.case_clause.statements.statement[i]);
                    }
                    free(node->data.stm.data.case_clause.statements.statement);
                    break;
                case ASTN_TRY_STM:
                    for (size_t i = 0; i < node->data.stm.data.try_stm.try_statements.size; i++) {
                        AST_free(node->data.stm.data.try_stm.try_statements.statement[i]);
                    }
                    free(node->data.stm.data.try_stm.try_statements.statement);
                    for (size_t i = 0; i < node->data.stm.data.try_stm.except_branches.size; i++) {
                        AST_free(node->data.stm.data.try_stm.except_branches.condition[i]);
                        for (size_t j = 0; j < node->data.stm.data.try_stm.except_branches.statements[i].size; j++) {
                            AST_free(node->data.stm.data.try_stm.except_branches.statements[i].statement[j]);
                        }
                        free(node->data.stm.data.try_stm.except_branches.statements[i].statement);
                    }
                    free(node->data.stm.data.try_stm.except_branches.condition);
                    free(node->data.stm.data.try_stm.except_branches.statements.condition);
                    free(node->data.stm.data.try_stm.except_branches.statements.statements);
                    AST_free(node->data.stm.data.try_stm.finally_statements);
                    break;
                case ASTN_WHILE_STM:
                    AST_free(node->data.stm.data.while_stm.condition_expr);
                    for (size_t i = 0; i < node->data.stm.data.while_stm.statements.size; i++) {
                        AST_free(node->data.stm.data.while_stm.statements.statement[i]);
                    }
                    free(node->data.stm.data.while_stm.statements.statement);
                    break;
                case ASTN_RETURN_STM:
                    AST_free(node->data.stm.data.return_stm.expr);
                    break;
                case ASTN_KEYWORD:
                    break;
                default:
                    break;
            }
    }
}