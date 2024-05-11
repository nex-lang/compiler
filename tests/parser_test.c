#include <criterion/criterion.h>

#include "parser.h"

TestSuite(parser);

Test(parser, init) {
    Lexer* lexer = lexer_init("../examples/test/1.nex");
    Parser* parser = symbtbl_init(lexer);

    cr_assert_not_null(parser->lexer,
        "parser: lexer shouldn't be null");
    cr_assert_not_null(parser->cur,
        "parser: current token shouldn't be null"
    );
    cr_assert_not_null(parser->tree,
        "parser: ast shouldn't be null");
    cr_assert_not_null(parser->tbl,
        "parser: table shouldn't be null");

    parser_free(parser);
}

Test(parser, handle_literal) {

}