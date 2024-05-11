#include "parser.h"

#include "stdio.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        return 1;
    }

    Parser* parser = parser_init((char*)argv[1]);

    parser_parse(parser);

    // Token* token = lexer_next_token(parser->lexer);

    // while (token->type != TOK_EOF) {
    //     token = lexer_next_token(parser->lexer);
    // }

    parser_free(parser);

    return 0;
}