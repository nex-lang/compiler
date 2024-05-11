#include <criterion/criterion.h>

#include "lexer.h"

TestSuite(lexer);

Test(lexer, init) {
    Lexer* lexer = lexer_init("../examples/test/1.nex");
    
    cr_assert_not_null(lexer, 
        "lexer: lexer struct shouldn't be null");

    cr_assert_eq(lexer->buf_size, 1075,
        "lexer: lexer buffer initialized incorrectly %d -> %d", 1075, lexer->buf_size);
    cr_assert_eq(lexer->i, 0,
        "lexer: lexer index initialized incorrectly %d -> %d", 0, lexer->i);
    cr_assert_eq(lexer->cl, 1,
        "lexer: lexer current line initialized incorrectly %d -> %d", 1, lexer->cl);
    cr_assert_eq(lexer->cc, 1,
        "lexer: lexer current column initialized incorrectly %d -> %d", 1, lexer->cc);
    cr_assert_eq(lexer->c, 'i',
        "lexer: lexer current char initialized incorrectly %c -> %c", 'i', lexer->c);

    lexer_free(lexer);
}

Test(lexer, lex) {
    Lexer* lexer = lexer_init("../examples/test/1.nex");

    char* values[] = {
        "import", "std", ".", "types", ";", "import", "input", ",", "fprint", "from", "std", ".", "io", ";", "import", "sin", ",", "pow", "from", "std", ".", "math", ";", "glob", "const", "float", ":",
        "pi_f", "=", "3.1415f", ";", "glob", "const", "double", ":", "pi_d", "=", "3.14159265358979d", ";", "fn", "verify_input", "=>", "(", "str", ":", "msg", ",", "str", ":", "condition", ",", "short", "int",
        "fail_value", ")", "{", "if", "!", "(", "eval", ".", "expr", "(", "condition", ")", ")", "{", "return", "fail_value", ";", "}", "if", "!", "(", "condition", ".", "is_true", ")", "{", "return", "fail_value", ";", "}",
        "return", "io", ".", "input", "(", "msg", ")", ";", "}", ":", "fn", "main", "=>", "(", "int", ":", "argc", ",", "str", "[", "]", ":", "argv", ")", "{", "mut", ":", "x", "=", "verify_input", "(", "Enter a number you want to use f(Φ): ", ",", "type", "(", "x", ")", "==", "type",
        ".", "s__uint__", ",", "-1", ")", ";", "mut", ":", "p", "=", "verify_input", "(", "Enter 0 for float precison and 1 for double precison: ", ",", "type", "(", "p", ")", "==", "type", ".", "__bool__", ",", "-1", ")", ";", "if", "(", "radius", ">", "0", "&", "&", "precison", ">", "0", ")",
        "{", "fprint", "(", "Invalid options selected, retry", ")", ";", "return", "-1", ";", "}", "return", "function_phi", "(", "x", ",", "p", ")", ";", "}", "(", "double", ",", "float", ")", ":", "fn", "function_phi", "=>", "(", "short", "uint", ":", "x", ",", "bool", ":", "p", ")", "{", "if", "(",
        "p", ")", "{", "return", "(", "2", "*", "x", ".", "pow", "(", "2", ")", "+", "3", "*", "x", "+", "1", ")", "*", "pi_d", ".", "sin", "(", ")", "+", "(", "x", ".", "pow", "(", "4", ")", "/", "(", "7", "%", "x",
        ")" ,")" ,"-" ,"e_d" ,"." ,"pow" ,"(" ,"(" ,"x" ,"+" ,"2" ,")" ,")" ,";" ,"}" ,"return" ,"(" ,"2" ,"*" ,"x" ,"." ,"pow" ,"(" ,"2" ,")" ,"+" ,"3" ,"*" ,"x" ,"+" ,"1" ,")",
        "*", "pi_f", ".", "sin", "(", ")", "+", "(", "x", ".", "pow", "(", "4", ")", "/", "(", "7", "%", "x", ")", ")", "-", "e_f", ".", "pow", "(", "(", "x", "+", "2", ")", ")", ";"
    };

    int types[] = {
        54, 2, 19, 2, 22, 54, 2, 20, 2, 51, 2, 19, 2, 22, 54, 2, 20, 2, 51, 2, 19, 2, 22, 53, 45, 50, 21, 2, 11, 79, 22, 53, 45, 46, 21, 2, 11,
        80, 22, 52, 2, 24, 3, 64, 21, 2, 20, 64, 21, 2, 20, 61, 55, 2, 4, 5, 2, 12, 3, 2, 19, 2, 3, 2, 4, 4, 5, 2, 2, 22, 6,
        2, 12, 3, 2, 19, 2, 4, 5, 2, 2, 22, 6, 2, 2, 19, 2, 3, 2, 4, 22, 6, 21, 52, 2, 24, 3, 55, 21, 2, 20, 64,
        7, 8, 21, 2, 4, 5, 58, 21, 2, 11, 2, 3, 82, 20, 2, 3, 2, 4, 27, 2, 19, 2, 20, 72, 4, 22, 58, 21,
        2, 11, 2, 3, 82, 20, 2, 3, 2, 4, 27, 2, 19, 2, 20, 72, 4, 22, 2, 3, 2, 10, 77, 14, 14, 2, 10, 77, 4, 5, 2,
        3, 82, 4, 22, 2, 72, 22, 6, 2, 2, 3, 2, 20, 2, 4, 22, 6, 3, 46, 20, 50, 4, 21, 52, 2, 24, 3, 61, 67, 21, 2,
        20, 42, 21, 2, 4, 5, 2, 3, 2, 4, 5, 2, 3, 77, 15, 2, 19, 2, 3, 77, 4, 16, 77, 15, 2, 16, 77, 4, 15, 2, 19, 2, 3, 4, 16,
        3, 2, 19, 2, 3, 77, 4, 18, 3, 77, 13, 2, 4, 4, 17, 2, 19, 2, 3, 3, 2, 
        16, 77, 4, 4, 22, 6, 2, 3, 77, 15, 2, 19, 2, 3, 77, 4, 16, 77, 15, 2, 16, 77, 4, 15, 2, 19, 2, 3,
        4, 16, 3, 2, 19, 2, 3, 77, 4, 18, 3, 77, 13, 2, 4, 4, 17, 2, 19, 2, 3, 3, 2, 16, 77, 4, 4, 22
    };


    for (int i = 0; i < 304; i++) {
        Token* token = lexer_next_token(lexer);
        cr_assert_str_eq(token->value, values[i], 
            "lexer: mismatch in expected token value: expected: %s found: %s", values[i], token->value);
        cr_assert_eq(token->type, types[i],
            "lexer: mismatch in expected token type: expected: %d found: %d", types[i], token->type);
    }

    lexer_free(lexer);
}