#include "parser.h"

#include "stdio.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        return 1;
    }

    Parser* parser = parser_init((char*)argv[1]);

    parser_parse(parser);
    parser_free(parser);

    return 0;
}