#include <criterion/criterion.h>

#include "lexer.h"

TestSuite(lexer);

Test(lexer, init) {
    Lexer* lexer = lexer_init("tests/source/1.nex");

    cr_assert_not_null(lexer, 
        "lexer: lexer struct shouldn't be null");

    // cr_assert_eq(lexer->buf_size, 1075,
        // "lexer: lexer buffer initialized incorrectly %d -> %d", 1075, lexer->buf_size);
    cr_assert_eq(lexer->i, 0,
        "lexer: lexer index initialized incorrectly %d -> %d", 0, lexer->i);
    cr_assert_eq(lexer->cl, 1,
        "lexer: lexer current line initialized incorrectly %d -> %d", 1, lexer->cl);
    cr_assert_eq(lexer->cc, 1,
        "lexer: lexer current column initialized incorrectly %d -> %d", 1, lexer->cc);
    // cr_assert_eq(lexer->c, 'i',
        // "lexer: lexer current char initialized incorrectly %c -> %c", 'i', lexer->c);

    lexer_free(lexer);
}