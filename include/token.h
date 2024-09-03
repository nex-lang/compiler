#ifndef TOKEN_H
#define TOKEN_H

#include "p_info.h"
#include "utils/bit128.h"

#define NO_OF_KEYWORDS 50
#define KEYWORDS keywords
#define MAX_KEYWORD_LEN 7

#define IS_LITERAL(type) \
    (((type) >= TOK_L_I8 && (type) <= TOK_L_SIZE) || (type) == TOK_TRUE || (type) == TOK_FALSE)

#define INT128_MIN ((__int128_t)0x80000000000000000000000000000000)
#define INT128_MAX ((__int128_t)0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)

typedef struct Token {
    unsigned int line, col;
    char* value;
    enum TokenType {
        // Special tokens
        TOK_ERROR,              // Error token
        TOK_EOF,                // End-of-file token (\0)
        TOK_IDEN,               // Identifier token (alphabets, digits, underscore)

        // Symbols
        TOK_LPAREN,             // (
        TOK_RPAREN,             // )
        TOK_LBRACE,             // {
        TOK_RBRACE,             // }
        TOK_LBRACK,             // [
        TOK_RBRACK,             // ]
        TOK_LT,                 // <
        TOK_GT,                 // >
        TOK_EQ,                 // =
        TOK_BANG,               // !
        TOK_PERC,               // %
        TOK_AMPER,              // &
        TOK_ASTK,               // *
        TOK_ADD,                // +
        TOK_MINUS,              // -
        TOK_SLASH,              // /
        TOK_PERIOD,             // .
        TOK_COMMA,              // ,
        TOK_COLON,              // :
        TOK_SC,                 // ;
        TOK_PIPE,               // |
        TOK_PIPE_PIPE,          // || 
        TOK_FN_ARROW,           //
        TOK_LT_EQ,              // <=
        TOK_GT_EQ,              // >=
        TOK_EQ_EQ,              // ==
        TOK_PERC_EQ,            // %=
        TOK_BANG_EQ,            // !=
        TOK_ASTK_EQ,            // *=
        TOK_ADD_EQ,             // +=
        TOK_MINUS_EQ,           // -=
        TOK_SLASH_EQ,           // /=
        TOK_LT_LT,              // <<
        TOK_GT_GT,              // >>
        TOK_ASTK_ASTK,          // **
        TOK_ADD_ADD,            // ++
        TOK_MINUS_MINUS,        // --
        TOK_COLON_COLON,        // ::
        TOK_ATHER,              // @
        TOK_CARET,              // ^

        // Keywords
        TOK_AS,                 // as
        TOK_ATTR,               // attr (ibute)
        TOK_BOOL,               // bool
        TOK_CHAR,               // char
        TOK_CLASS,              // class
        TOK_CONST,              // const
        TOK_DOUBLE,             // double
        TOK_ENUM,               // enum
        TOK_EXT,                // ext
        TOK_FALSE,              // false
        TOK_FLOAT,              // float
        TOK_FROM,               // from
        TOK_FN,                 // fn
        TOK_GLOB,               // glob
        TOK_IMPORT,             // import
        TOK_I8,                // i8
        TOK_I16,                // i16
        TOK_I32,                // i32
        TOK_I64,                // i64
        TOK_I128,                // i128
        TOK_MUT,                // mut
        TOK_PRIV,               // priv
        TOK_PUB,                // pub
        TOK_RETURN,             // return
        TOK_SIZE,               // size
        TOK_STRING,             // str
        TOK_STRUCT,             // struct
        TOK_TRUE,               // true
        TOK_U8,                // u8
        TOK_U16,                // u16
        TOK_U32,                // u32
        TOK_U64,                // u64
        TOK_U128,                // u128
        TOK_VAR,                // var
        TOK_IF,
        TOK_ELIF,
        TOK_ELSE,
        TOK_FOR,
        TOK_WHILE,
        TOK_SWITCH,
        TOK_CASE,
        TOK_TRY,
        TOK_EXCEPT,
        TOK_FINALLY,
        TOK_BREAK,
        TOK_CONTINUE,
        TOK_ERR, // err
        TOK_THROW,
        TOK_DEFAULT,
        TOK_NEW, // new

        // Literals
        TOK_L_I8,            // 8-bit signed integer
        TOK_L_I16,             // 16-bit signed integer
        TOK_L_I32,              // 32-bit signed integer
        TOK_L_I64,             // 64-bit signed integer
        TOK_L_I128,            // 128-bit signed integer
        TOK_L_U8,           // 8-bit unsigned integer
        TOK_L_U16,            // 16-bit unsigned integer
        TOK_L_U32,             // 32-bit unsigned integer
        TOK_L_U64,            // 64-bit unsigned integer
        TOK_L_U128,           // 128-bit unsigned integer
        TOK_L_FLOAT,            // 32-bit floating-point
        TOK_L_DOUBLE,           // 64-bit double-precision floating-point
        TOK_L_CHAR,             // 8-bit character
        TOK_L_STRING,           // Dynamic array of characters (string)
        TOK_L_BOOL,             // 8-bit boolean
        TOK_L_SIZE,              // System-specific bit-size
        TOK_L_ARRAY,
        TOK_L_PTR,
    } type;
} Token;

#endif // TOKEN_H