#ifndef __LEXER_H__
#define __LEXER_H__

#include <stdarg.h>

#include "kilate/string.h"
#include "kilate/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  TOKEN_KEYWORD,     // work, return
  TOKEN_IDENTIFIER,  // hello, main
  TOKEN_STRING,      // "Hello world!"
  TOKEN_LPAREN,      // (
  TOKEN_RPAREN,      // )
  TOKEN_LBRACE,      // {
  TOKEN_RBRACE,      // }
  TOKEN_RARROW,      // ->
  TOKEN_LARROW,      // <-
  TOKEN_COLON,       // :
  TOKEN_TYPE,        // bool
  TOKEN_BOOL,        // true, false
  TOKEN_INT,         // 123
  TOKEN_FLOAT,       // 1.23
  TOKEN_LONG,        // 123l
  TOKEN_COMMA,       // ,,
  TOKEN_ASSIGN,      // =
  TOKEN_VAR,         // var
  TOKEN_LET,         // let
  TOKEN_EOF          // end of file.
} token_kind_t;

typedef struct {
  size_t column;
  size_t line;
  token_kind_t type;
  char* text;
} token_t;

typedef vector_t token_vector_t;

token_t* token_make(token_kind_t, char *, size_t, size_t);

char * tokentype_tostr(token_kind_t);

typedef struct {
  size_t __pos__;
  size_t __column__;
  size_t __line__;
  token_vector_t* tokens;
  char* __input__;
} lexer_t;

lexer_t * lexer_make(char *);

void lexer_delete(lexer_t *);

void lexer_advance(lexer_t *);

char * lexer_read_string(lexer_t *, bool*);

void lexer_tokenize(lexer_t *);

void lexer_error(lexer_t *, char *, ...);

#ifdef __cplusplus
}
#endif

#endif