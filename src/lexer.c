#include "lexer.h"
#include "io.h"

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

char* keywords[NO_OF_KEYWORDS] = {
    "as", "attr", "bool", "char", "class", "const", "double", "enum", "ext",
    "false", "float", "fn", "from", "glob", "import", "int", "l_long", "long", "mut",
    "priv", "pub", "return", "short", "s_short", "size", "str", "struct", "true", "uint", "var"
};

Lexer* lexer_init(char* filename) {
    /*
    Initializes lexer to lex provided values
    return: pointer to a propperly initalized lexer struct
    */

    Lexer* lexer = calloc(1, sizeof(Lexer));

    if (!lexer) {
        exit(EXIT_FAILURE);
    }

    lexer->buf = io_load_file(filename);
    lexer->buf_size = strlen(lexer->buf);

    lexer->i = 0;
    lexer->c = lexer->buf[lexer->i];
    
    lexer->cc = 1;
    lexer->cl = 1;

    return (Lexer*)lexer;
}

void lexer_free(Lexer* lexer) {
    /*
    De-initializes provided lexer
    */

    if (!lexer) {
        return;
        exit(EXIT_FAILURE);
    }

    free(lexer->buf);
    free(lexer);

    lexer = NULL;
}

Token* lexer_token_init(Lexer* lexer, char* value, uint8_t type) {
    /*
    Initializes token with provided values 
    return: pointer to a propperly initalized token struct
    */

    Token* token = calloc(1, sizeof(Token));

    if (!token) {
        exit(EXIT_FAILURE);
    }

    token->type = type;  
    token->col = lexer->cc;
    token->line = lexer->cl;

    token->value = strdup(value);

    return (Token*)token;
}

void token_free(Token* token) {
    /*
    De-initializes provided token
    */

    if (!token) {
        exit(EXIT_FAILURE);
        return;
    }

    free(token->value);
    free(token);

    token = NULL;
}

Token* lexer_next_token(Lexer* lexer) {
    Token* token;
    lexer_handle_fillers(lexer);

    if (lexer->c == '\0' || lexer->i + 1 >= lexer->buf_size) {
        token = lexer_token_init(lexer, "\0", TOK_EOF);
    } else if (isalpha(lexer->c) || lexer->c == '_') {
        token = lexer_handle_alpha(lexer);
    } else if (isdigit(lexer->c)) {
        token = lexer_handle_numeric(lexer, false);
    } else {
        token = lexer_handle_1char(lexer);
        if (token->type == TOK_ERROR) {
            lexer_handle_error(lexer);
            return lexer_next_token(lexer);
        }
    }

    return token;
}

char* lexer_peek(Lexer* lexer, int8_t offset) {
    /*
    Extracts provided # of characters ahead of the current lexer position
    Returns: NULL (EOF), EXIT_FAILURE (alloc error), char* (valid peek)
    */


    if (offset < 0 && (size_t)(-offset) > lexer->i) {
        return NULL;
    }

    size_t peek_start = (size_t)(offset < 0 ? lexer->i + offset : lexer->i);
    size_t peek_length = (size_t)(offset < 0 ? -offset : offset);

    if (peek_start >= lexer->buf_size || peek_start + peek_length > lexer->buf_size) {
        return NULL;
    }

    char* info = malloc((peek_length + 1) * sizeof(char));

    if (!info) {
        exit(EXIT_FAILURE);
    }

    strncpy(info, lexer->buf + peek_start, peek_length);
    info[peek_length] = '\0';

    return info;
}

char lexer_peep(Lexer* lexer, int8_t offset) {
    /*
    Extracts character of position of the provided # from the current lexer position
    Returns: '\0' (EOF), char (valid peek)
    */

    size_t peek_index = (size_t)(lexer->i + offset);

    if (offset < 0 && (size_t)(-offset) > lexer->i) {
        return '\0';
    }

    if (peek_index >= lexer->buf_size) {
        return '\0';
    }

    return lexer->buf[peek_index];
}


void lexer_advance(Lexer* lexer, uint8_t offset) {
    /*
    Advances current lexer position by provided # of charachters; returns NULL if EOF
    */

    if (lexer->buf[lexer->i + offset] == '\0' || lexer->i + offset >= lexer->buf_size) {
        return;
    }

    lexer->i += offset;
    lexer->cc += 1;
    lexer->c = lexer->buf[lexer->i];
}

void lexer_handle_error(Lexer* lexer) {
    while (lexer->c != ';' && lexer->c != '\0' && lexer->c != ' ' && lexer->c != '\n' &&
           lexer->c != '\v' && lexer->c != '\t' && lexer->buf[lexer->i + 1] != '\0' &&
           lexer->i + 1 < lexer->buf_size) {
        lexer_advance(lexer, 1);
    }
}

void lexer_handle_fillers(Lexer* lexer) {
    /*
    Skips un-importaint values (whitespace, newline, horizontal/vertical tab)
    */

    switch (lexer->c) {
        case ' ':
            lexer_advance(lexer, 1);
            break;
        case '\n':
        case '\v':
            lexer->cl += 1;
            lexer_advance(lexer, 1);
            lexer->cc = 1;
            break;
        case '\t':
            lexer_advance(lexer, 2);
            break;
        default:
            break;
    }
}

Token* lexer_handle_alpha(Lexer* lexer) {
    /* 
    Identifies and creates TOK_IDEN or keyword tokens [TOK_IMPORT -> TOK_AS]
    return: identifiers and keywords
    */

    char* buf = calloc(1, sizeof(char));

    while (isalnum(lexer->c) || lexer->c == '_') {
        buf = realloc(buf, (strlen(buf) + 2) * sizeof(char));
        
        if (!buf) {
            exit(EXIT_FAILURE);
        }
        
        if (strlen(buf) > MAX_IDENTIFIER_LEN) {
            REPORT_ERROR(lexer, "E_SHORTER_LENIDEN", MAX_IDENTIFIER_LEN);
            lexer_handle_error(lexer);
            return lexer_next_token(lexer);
        }

        strcat(buf, (char[]){lexer->c, '\0'}); 
        lexer_advance(lexer, 1);
    } 

    if (strlen(buf) > MAX_KEYWORD_LEN) {
        // early check to avoid keyword checking loop
        return lexer_token_init(lexer, buf, TOK_IDEN);
    }

    uint8_t KWCHAR_TYPE_MAP[NO_OF_KEYWORDS] = {
        TOK_AS, TOK_ATTR, TOK_BOOL, TOK_CHAR, TOK_CLASS, TOK_CONST, TOK_DOUBLE, TOK_ENUM, TOK_EXT,
        TOK_FALSE, TOK_FLOAT, TOK_FN, TOK_FROM, TOK_GLOB, TOK_IMPORT, TOK_INT, TOK_L_LONG, TOK_LONG, TOK_MUT,
        TOK_PRIV, TOK_PUB, TOK_RETURN, TOK_SHORT, TOK_S_SHORT, TOK_SIZE, TOK_STRING, TOK_STRUCT, TOK_TRUE, TOK_UINT, TOK_VAR
    };

    for (uint8_t i = 0; i < NO_OF_KEYWORDS ; i++) {
        if (strcmp(KEYWORDS[i], buf) == 0) {
            return lexer_token_init(lexer, buf, KWCHAR_TYPE_MAP[i]);
        }
    }

    return lexer_token_init(lexer, buf, TOK_IDEN);
}

Token* lexer_handle_numeric(Lexer* lexer, bool is_negative) {
    /*
    Identifies and creates numeric tokens 
    return: numeric tokens [TOK_L_SSINT-> TOK_L_DOUBLE]
    */
   
    int type = TOK_ERROR;
    __uint8_t npre_decimal = 0;
    char* buf = calloc(2, sizeof(char));

    if (!buf) {
        exit(EXIT_FAILURE);
    }

    if (is_negative) {
        // register sign
        strcat(buf, (char[]){'-', '\0'}); 
        lexer_advance(lexer, 1);
    }

    lexer_process_digits(lexer, &buf, false);
    type = lexer_process_int_type(buf);
    

    if (lexer->c == '.') {
        // register decimal
        buf = realloc(buf, (strlen(buf) + 2) * sizeof(char));
        
        if (!buf) {
            exit(EXIT_FAILURE);
        }

        strcat(buf, (char[]){lexer->c, '\0'});
        lexer_advance(lexer, 1);

        npre_decimal = strlen(buf); // # of digits before decimal

        lexer_process_digits(lexer, &buf, true);

        type = lexer_process_decimal_type(buf, (strlen(buf) - npre_decimal));
    }

    if (type == TOK_ERROR) {
        REPORT_ERROR(lexer, "U_NUM_LIT_TYPE");
        lexer_handle_error(lexer);
        return lexer_next_token(lexer);
    }

    return lexer_token_init(lexer, buf, type);
}

Token* lexer_handle_1char(Lexer* lexer) {
    /*
    Identifies and creates tokens for op-erator and checks for possible operator combinations
    return: operators[TOK_LT -> TOK_COLON_COLON] or, char/string/int literal tokens
    */

    char next_char = lexer_peep(lexer, 1);

    switch (lexer->c) {
        case '<':
            return lexer_process_pos_singlechar(lexer, next_char,
                '=', '<', TOK_LT, TOK_LT_EQ, TOK_LT_LT);
            break;
        case '>':
            return lexer_process_pos_singlechar(lexer, next_char,
                '=', '>', TOK_GT, TOK_GT_EQ, TOK_GT_GT);
            break;
        case '*':
            return lexer_process_pos_singlechar(lexer, next_char,
                '=', '*', TOK_ASTK, TOK_ASTK_EQ, TOK_ASTK_ASTK);
            break;
        case '+':
            return lexer_process_pos_singlechar(lexer, next_char, 
                '=', '+', TOK_ADD, TOK_ADD_EQ, TOK_ADD_ADD);
            break;
        case '/':
            return lexer_process_pos_singlechar(lexer, next_char,
                '=', '/', TOK_SLASH, TOK_SLASH_EQ, TOK_ERROR);
            break;
        case ':':
            return lexer_process_pos_singlechar(lexer, next_char,
                ':', '\0', TOK_COLON, TOK_COLON_COLON, TOK_ERROR);
            break;
        case '=':
            return lexer_process_pos_singlechar(lexer, next_char,
                '=', '>', TOK_EQ, TOK_EQ_EQ, TOK_FN_ARROW);
            break;
        case '!':
            return lexer_process_pos_singlechar(lexer, next_char,
                '=', '\0', TOK_BANG, TOK_BANG_EQ, TOK_ERROR);
            break;
        case '%':
            return lexer_process_pos_singlechar(lexer, next_char,
                '=', '\0', TOK_PERC, TOK_PERC_EQ, TOK_ERROR);
            break;
        case '&':
            lexer_advance(lexer, 1); return lexer_token_init(lexer, "&", TOK_AMPER);
            break;
        case '.':
            lexer_advance(lexer, 1); return lexer_token_init(lexer, ".", TOK_PERIOD);
            break;
        case ',':
            lexer_advance(lexer, 1); return lexer_token_init(lexer, ",", TOK_COMMA);
            break;
        case ';':
            lexer_advance(lexer, 1); return lexer_token_init(lexer, ";", TOK_SC);
            break;
        case '(':
            lexer_advance(lexer, 1); return lexer_token_init(lexer, "(", TOK_LPAREN);
            break;
        case ')':
            lexer_advance(lexer, 1); return lexer_token_init(lexer, ")", TOK_RPAREN);
            break;
        case '{':
            lexer_advance(lexer, 1); return lexer_token_init(lexer, "{", TOK_LBRACE);
            break;
        case '}':
            lexer_advance(lexer, 1); return lexer_token_init(lexer, "}", TOK_RBRACE);
            break;
        case '[':
            lexer_advance(lexer, 1); return lexer_token_init(lexer, "[", TOK_LBRACK);
            break;
        case ']':
            lexer_advance(lexer, 1); return lexer_token_init(lexer, "]", TOK_RBRACK);
            break;
        case '-':
            return lexer_process_minus_op(lexer, next_char);
            break;
        case '\'':
            return lexer_process_single_quote(lexer);
            break;
        case '\"':
            return lexer_process_double_quote(lexer);
            break;
        default:
            break;
    }

    return lexer_next_token(lexer);
}

void lexer_process_digits(Lexer* lexer, char** buf, bool has_decimal) {

    while (isdigit(lexer->c) ||
           ((lexer->c == 'f' || lexer->c == 'F' || lexer->c == 'd' || lexer->c == 'D') && has_decimal == true)) {
        size_t len = strlen(*buf);
        *buf = realloc(*buf, (len + 2) * sizeof(char));

        if (!*buf) {
            exit(EXIT_FAILURE);
        }

        (*buf)[len] = lexer->c;
        (*buf)[len + 1] = '\0';


        lexer_advance(lexer, 1);
    }

    if (lexer->c == '_' || lexer->c == '\'') {
        if (isdigit(lexer_peep(lexer, -1)) && isdigit(lexer_peep(lexer, 1))) {
            lexer_advance(lexer, 1);
            lexer_process_digits(lexer, buf, has_decimal);
        } else {
            free(*buf);
            lexer_handle_error(lexer);
        }
    } else if (lexer->c == '.' && has_decimal) {
        free(*buf);
        lexer_handle_error(lexer);
    }
}


uint8_t lexer_process_decimal_type(char* buf, uint8_t diadc) {
    /*
    Processes provided buffer and resolves type; TOK_ERROR if invalid
    return: TOK_ERROR, TOK_L_FLOAT (32-bit float), or TOK_L_DOUBLE (64-bit float)
    */

    bool has_Fpfx = false; // has float prefix
    bool has_Dpfx = false; // has double prefix

    for (uint8_t i = 0; i < strlen(buf); i++) {
        if (buf[i] == 'f' || buf[i] == 'F') {
            if (has_Fpfx) { 
                return TOK_ERROR;
            }
            has_Fpfx = true;
        } else if (buf[i] == 'd' || buf[i] == 'D') {
            if (has_Dpfx) { 
                return TOK_ERROR;
            }
            has_Dpfx = true;
        }
    }

    if (has_Fpfx) {
        return TOK_L_FLOAT;
    } else if (has_Dpfx) {
        return TOK_L_DOUBLE;
    }

    if (diadc <= MAX_FLOAT_LIT_DIGITS) {
        return TOK_L_FLOAT;
    } else if (diadc <= MAX_DOUBLE_LIT_DIGITS) {
        return TOK_L_DOUBLE;
    }

    return TOK_ERROR;
}


uint8_t lexer_process_int_type(char* buf) {
    /*
	Processes provided buf-fer and resolutes type; TOK_ERROR if invalid
	return: TOK_ERROR, TOK_L_(S|SS|L|LL)INT or TOK_L_(S|SS|L|LL)UINT 
	*/

    int64_t signed_val;
    uint64_t unsigned_val;

    if (sscanf(buf, "%ld" SCNd64, &signed_val) == 1) {
        return (signed_val >= 0) ? TOK_L_LUINT : TOK_L_LINT;
    } else if (sscanf(buf, "%ld" SCNu64, &unsigned_val) == 1) {
        return (unsigned_val >= 0) ? TOK_L_LLUINT : TOK_ERROR;
    } else if (sscanf(buf, "%ld" SCNd32, &signed_val) == 1) {
        return (signed_val >= 0) ? TOK_L_UINT : TOK_L_INT;
    } else if (sscanf(buf, "%ld" SCNd16, &signed_val) == 1) {
        return (signed_val >= 0) ? TOK_L_SUINT : TOK_L_SINT;
    } else if (sscanf(buf, "%ld" SCNd8, &signed_val) == 1) {
        return (signed_val >= 0) ? TOK_L_SSUINT : TOK_L_SSINT;
    }

    return TOK_ERROR;
}

Token* lexer_process_pos_singlechar(Lexer* lexer, char next_char,
    char c_pos1, char c_pos2,
    uint8_t t_pos0, uint8_t t_pos1, uint8_t t_pos2) {

    char c_pos0 = lexer->c;

    if (next_char == c_pos1) {
        lexer_advance(lexer, 2);
        return lexer_token_init(lexer, (char*)(&(char[]){c_pos0, c_pos1, '\0'}), t_pos1);
    } else if ((next_char == c_pos2) && (c_pos2 != '\0')){
        lexer_advance(lexer, 2);
        return lexer_token_init(lexer, (char*)(&(char[]){c_pos0, c_pos2, '\0'}), t_pos2);
    }

    lexer_advance(lexer, 1);
    return lexer_token_init(lexer, (char*)(&(char[]){c_pos0, '\0'}), t_pos0);
}
                    
Token* lexer_process_minus_op(Lexer* lexer, char next_char) {
    if (next_char == '=') {
        lexer_advance(lexer, 2);
        return lexer_token_init(lexer, "-=", TOK_MINUS_EQ);
    } else if (next_char == '*') {
        lexer_advance(lexer, 2);
        return lexer_token_init(lexer, "--", TOK_MINUS_MINUS);
    } else if isdigit(next_char) {
        return lexer_handle_numeric(lexer, true);
    }

    lexer_advance(lexer, 1);
    return lexer_token_init(lexer, "-", TOK_MINUS);
}

Token* lexer_process_single_quote(Lexer* lexer) {
    lexer_advance(lexer, 1); // consume '

    char* value = calloc(2, sizeof(char));
    if (!value) {
        exit(EXIT_FAILURE);
    }
    value[0] = lexer->c;
    value[1] = '\0';

    lexer_advance(lexer, 1); // consume char

    char next_char = (lexer_peek(lexer, 1))[0];

    if (next_char == '\\') {
        lexer_advance(lexer, 1); // consume '\'
        lexer_process_escape_code(lexer, &value);
    }

    if (lexer->c == '\'') {
        lexer_advance(lexer, 1); // consume '
        return lexer_token_init(lexer, value, TOK_L_CHAR);
    }

    REPORT_ERROR(lexer, "E_CHAR_TERMINATOR");
    lexer_handle_error(lexer);
    return lexer_next_token(lexer);
}

Token* lexer_process_double_quote(Lexer* lexer) {
    lexer_advance(lexer, 1); // consume "
    
    char* value = calloc(1, sizeof(char) + 1);

    if (!value) {
        exit(EXIT_FAILURE);
    }

    while (lexer->c != '"' && lexer->c != '\0') {
        if (lexer->c == '\\') {
            lexer_advance(lexer, 1); // consume '\'
            lexer_process_escape_code(lexer, &value);
        } else {
            value = realloc(value, (strlen(value) + 2) * sizeof(char));
            
            if (!value) {
                exit(EXIT_FAILURE);
            }
            
            strncat(value, &(lexer->c), 1);
            lexer_advance(lexer, 1); // consume char
        }
    }

    if (lexer->c == '"') {
        lexer_advance(lexer, 1);
        return lexer_token_init(lexer, value, TOK_L_STRING);
    }

    REPORT_ERROR(lexer, "E_STRING_TERMINATOR");
    lexer_handle_error(lexer);
    return lexer_next_token(lexer);
}

void lexer_process_escape_code(Lexer* lexer, char** buf) {
    char next_char = (lexer_peek(lexer, 1))[0];
    char esc_chars[12] = "\n\r\t\b\a\f\\\'\"\0";

    if (next_char == 'x' || next_char == 'u') {
        int len = (next_char == 'x') ? 5 : 7; 
        char digits[5];
        char* peeked_digits = lexer_peek(lexer, len - 1);

        if (peeked_digits && isxdigit(peeked_digits[0]) && isxdigit(peeked_digits[1])) {
            strncpy(digits, peeked_digits, len - 1);
            digits[len - 1] = '\0';
            strncat(*buf, digits, len - 1);
            lexer_advance(lexer, len);
        } else {
            strncat(*buf, &lexer->c, 1);
            lexer_advance(lexer, 2);
        }

        free(peeked_digits);
    } else if (strchr(esc_chars, next_char)) {
        *buf = realloc(*buf, (strlen(*buf) + 2) * sizeof(char));
        if (!*buf) exit(EXIT_FAILURE);
        strcat(*buf, (char[]){'\\', next_char, '\0'});
        lexer_advance(lexer, 2);
    } else {
        strncat(*buf, &lexer->c, 1);
        lexer_advance(lexer, 2);
    }
}

struct ErrorTemplate templates[] = {
    {"U_NO_OF_DECIMAL", "Unexpected number of digits after decimal point - configuration expects: %d found: %d"},
    {"U_NUM_LIT_TYPE", "Unexpected numeric literal found - accepted formats: docs::literals::numeric"},
    {"U_DB_SOURCE_DECL", "Unexpected double source declration, first sourced: '%s' again: '%s' - expects: single 'from' statement"},
    {"U_MODULE_MULT_ALIAS", "Unexpected attempt to alias %d seperate identifiers into single '%s' - find proper syntax: docs::imports"},

    {"E_SHORTER_LENIDEN", "Expected a shorter identifier length - configuration expects: <= %d"},
    {"E_CHAR_TERMINATOR", "Expected a (') character literal terminator after starting of character literal"},
    {"E_STRING_TERMINATOR", "Expected a (\") string literal terminator after starting of string literal"},
    {"E_DTS_FN_PARAM", "Expected a valid data type specifier while specifying parameters for a function, '%s' needs a type"}
};

char* lexer_get_reference(Lexer* lexer) {
    size_t line_start = lexer->i;

    while (line_start > 0 && lexer->buf[line_start - 1] != '\n') {
        line_start--;
    }

    size_t line_end = lexer->i;

    while (lexer->buf[line_end] != '\0' && lexer->buf[line_end] != '\n') {
        line_end++;
    }

    size_t line_length = line_end - line_start;
    char* line_content = (char*)malloc((line_length + 2) * sizeof(char));

    if (!line_content) {
        free(line_content);
        exit(EXIT_FAILURE);
    }

    strncpy(line_content, lexer->buf + line_start, line_length);

    line_content[line_length] = '\0';

    return line_content;
}

void lexer_report_error(Lexer* lexer, char* error_code, ...) {
    if (error_code[0] != 'U' && error_code[0] != 'E') {
        return;
    }

    for (size_t i = 0; i < (sizeof(templates) / sizeof(templates[0])); i++) {        
        if (strcmp(templates[i].code, error_code) == 0) {
            va_list args;
            va_start(args, error_code);

            char* final_content = malloc(
                (strlen(templates[i].content) + sizeof(args)) * sizeof(char)
            );

            if (final_content != NULL) {
                vsprintf(final_content, templates[i].content, args);
            }

            va_end(args);

            char* refrence = lexer_get_reference(lexer);

            printf("[%d : %d] > %s\n\t%d | %s\n", lexer->cl, lexer->cc, final_content, lexer->cl, refrence);
            free(final_content);
            return;
        }
    }

    char* refrence = lexer_get_reference(lexer);
    printf("[%d : %d] > %s\n\t%d | %s\n", lexer->cl, lexer->cc, error_code, lexer->cl, refrence);
}