#ifndef LEXER_H
#define LEXER_H

#include "token.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct Lexer {
    char c; // current charachter
    unsigned int i; // current index 

    char* buf; // buffer
    unsigned int buf_size; // buffer size 
     
    uint8_t cc; // current column
    unsigned int cl; // current line
} Lexer;

// allocating and freeing methods

Lexer* lexer_init(char* filename);
void lexer_free(Lexer* lexer);

Token* lexer_token_init(Lexer* lexer, char* value, uint8_t type);
void token_free(Token* token);

// important and general functions 

Token* lexer_next_token(Lexer* lexer);

char* lexer_peek(Lexer* lexer, int8_t offset);
char lexer_peep(Lexer* lexer, int8_t offset);
void lexer_advance(Lexer* lexer, uint8_t offset);

// helper functions

void lexer_handle_fillers(Lexer* lexer);
void lexer_handle_error(Lexer* lexer);

Token* lexer_handle_alpha(Lexer* lexer);
Token* lexer_handle_numeric(Lexer* lexer, bool is_negative);
Token* lexer_handle_1char(Lexer* lexer);

// helper-helper functions

void lexer_process_digits(Lexer* lexer, char** buf, bool has_decimal);
uint8_t lexer_process_decimal_type(char* buf, uint8_t diadc);
uint8_t lexer_process_int_type(char* buf);

Token* lexer_process_pos_singlechar(Lexer* lexer, char next_char,
    char c_pos1, char c_pos2,
    uint8_t t_pos0, uint8_t t_pos1, uint8_t t_pos2);
Token* lexer_process_minus_op(Lexer* lexer, char next_char);

Token* lexer_process_single_quote(Lexer* lexer);
Token* lexer_process_double_quote(Lexer* lexer);
void lexer_process_escape_code(Lexer* lexer, char **buf);

// error manager

#define REPORT_ERROR lexer_report_error

struct ErrorTemplate {
    const char* code;
    const char* content;
};

extern struct ErrorTemplate templates[];

char* lexer_get_reference(Lexer* lexer);
void lexer_report_error(Lexer* lexer, char* error_code, ...);


#endif // LEXER_H