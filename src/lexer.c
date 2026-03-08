#include "kilate/lexer.h"

#include <ctype.h>
#include <stdarg.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilate/bool.h"
#include "kilate/error.h"
#include "kilate/string.h"

lexer_t* lexer_make(char * input) {
  lexer_t* lexer = malloc(sizeof(lexer_t));
  lexer->__pos__ = 0;
  lexer->__input__ = strdup(input);
  lexer->tokens = vector_make(sizeof(token_t*));
  lexer->__line__ = 1;
  lexer->__column__ = 1;
  return lexer;
}

void lexer_delete(lexer_t* lexer) {
  for (size_t i = 0; i < lexer->tokens->size; ++i) {
    token_t* tk = *(token_t**)vector_get(lexer->tokens, i);
    free(tk->text);
    free(tk);
  }
  vector_delete(lexer->tokens);
  free(lexer->__input__);
  free(lexer);
}

token_t* token_make(token_kind_t type,
                          char * text,
                          size_t line,
                          size_t column) {
  token_t* tk = malloc(sizeof(token_t));
  tk->type = type;
  tk->text = strdup(text);
  tk->line = line;
  tk->column = column;
  return tk;
}

char * tokentype_tostr(token_kind_t type) {
  switch (type) {
    case TOKEN_KEYWORD:
      return "keyword";
    case TOKEN_IDENTIFIER:
      return "identifier";
    case TOKEN_STRING:
      return "string";
    case TOKEN_LPAREN:
      return "left_parenthesis";
    case TOKEN_RPAREN:
      return "right_parenthesis";
    case TOKEN_LBRACE:
      return "left_brace";
    case TOKEN_RBRACE:
      return "right_brace";
    case TOKEN_RARROW:
      return "right_arrow";
    case TOKEN_LARROW:
      return "left_arrow";
    case TOKEN_COLON:
      return "colon";
    case TOKEN_TYPE:
      return "type";
    case TOKEN_BOOL:
      return "boolean";
    case TOKEN_INT:
      return "int";
    case TOKEN_FLOAT:
      return "float";
    case TOKEN_LONG:
      return "double";
    case TOKEN_COMMA:
      return "comma";
    case TOKEN_ASSIGN:
      return "assign";
    case TOKEN_LET:
      return "let";
    case TOKEN_VAR:
      return "var";
    case TOKEN_EOF:
      return "end_of_file";
    default:
      return "unknow_token";
  };
}

void lexer_advance(lexer_t* lexer) {
  if (lexer->__input__[lexer->__pos__] == '\n') {
    lexer->__line__++;
    lexer->__column__ = 1;
  } else {
    lexer->__column__++;
  }
  lexer->__pos__++;
}

char * lexer_read_string(lexer_t* lexer, bool* closed) {
  size_t input_len = str_length(lexer->__input__);
  size_t start = lexer->__pos__ + 1;  // após o "
  size_t buf_size = input_len - start + 1;
  char * buffer = malloc(buf_size);
  size_t buf_index = 0;

  *closed = false;

  lexer_advance(lexer);

  while (lexer->__pos__ < input_len) {
    char ch = lexer->__input__[lexer->__pos__];
    if (ch == '\\') {
      lexer->__pos__++;
      if (lexer->__pos__ >= input_len)
        break;
      char next = lexer->__input__[lexer->__pos__];
      switch (next) {
        case 'n':
          buffer[buf_index++] = '\n';
          break;
        case 't':
          buffer[buf_index++] = '\t';
          break;
        case 'r':
          buffer[buf_index++] = '\r';
          break;
        case '"':
          buffer[buf_index++] = '"';
          break;
        case '\\':
          buffer[buf_index++] = '\\';
          break;
        default:
          buffer[buf_index++] = next;
          break;
      }
      lexer->__pos__++;
    } else if (ch == '"') {
      lexer->__pos__++;
      *closed = true;
      break;
    } else {
      buffer[buf_index++] = ch;
      lexer->__pos__++;
    }
  }

  buffer[buf_index] = '\0';
  return buffer;
}

void lexer_tokenize(lexer_t* lexer) {
  size_t input_len = str_length(lexer->__input__);
  while (lexer->__pos__ < input_len) {
    char c = lexer->__input__[lexer->__pos__];
    if (isspace(c)) {
      lexer_advance(lexer);
      continue;
    }

    switch (c) {
      case '(': {
        size_t tkl = lexer->__line__;
        size_t tkc = lexer->__column__;
        token_t* token = token_make(TOKEN_LPAREN, "(", tkl, tkc);
        vector_push_back(lexer->tokens, &token);

        lexer_advance(lexer);
        continue;
      }
      case ')': {
        size_t tkl = lexer->__line__;
        size_t tkc = lexer->__column__;
        token_t* token = token_make(TOKEN_RPAREN, ")", tkl, tkc);
        vector_push_back(lexer->tokens, &token);

        lexer_advance(lexer);
        continue;
      }
      case '{': {
        size_t tkl = lexer->__line__;
        size_t tkc = lexer->__column__;
        token_t* token = token_make(TOKEN_LBRACE, "{", tkl, tkc);
        vector_push_back(lexer->tokens, &token);

        lexer_advance(lexer);
        continue;
      }
      case '}': {
        size_t tkl = lexer->__line__;
        size_t tkc = lexer->__column__;
        token_t* token = token_make(TOKEN_RBRACE, "}", tkl, tkc);
        vector_push_back(lexer->tokens, &token);

        lexer_advance(lexer);
        continue;
      }
      case ':': {
        size_t tkl = lexer->__line__;
        size_t tkc = lexer->__column__;
        token_t* token = token_make(TOKEN_COLON, ":", tkl, tkc);
        vector_push_back(lexer->tokens, &token);

        lexer_advance(lexer);
        continue;
      }
      case ',': {
        size_t tkl = lexer->__line__;
        size_t tkc = lexer->__column__;
        token_t* token = token_make(TOKEN_COMMA, ",", tkl, tkc);
        vector_push_back(lexer->tokens, &token);

        lexer_advance(lexer);
        continue;
      }
      case '=': {
        size_t tkl = lexer->__line__;
        size_t tkc = lexer->__column__;
        token_t* token = token_make(TOKEN_ASSIGN, "=", tkl, tkc);
        vector_push_back(lexer->tokens, &token);

        lexer_advance(lexer);
        continue;
      }
      case '\n': {
        // lexer->__line__++;
        //   lexer->__column__ = 1;
        lexer_advance(lexer);
        continue;
      }
      case ';': {
        lexer_advance(lexer);
        continue;
      }
    };
    if (str_starts_with(lexer->__input__, "//", lexer->__pos__)) {
      lexer->__pos__ += 2;
      while (lexer->__pos__ < input_len &&
             lexer->__input__[lexer->__pos__] != '\n') {
        lexer_advance(lexer);
      }
      continue;
    }
    if (str_starts_with(lexer->__input__, "->", lexer->__pos__)) {
      size_t tkl = lexer->__line__;
      size_t tkc = lexer->__column__;
      token_t* token = token_make(TOKEN_RARROW, "->", tkl, tkc);
      vector_push_back(lexer->tokens, &token);

      lexer->__pos__ += 2;
      continue;
    }
    if (str_starts_with(lexer->__input__, "<-", lexer->__pos__)) {
      size_t tkl = lexer->__line__;
      size_t tkc = lexer->__column__;
      token_t* token = token_make(TOKEN_LARROW, "->", tkl, tkc);
      vector_push_back(lexer->tokens, &token);

      lexer->__pos__ += 2;
      continue;
    }
    if (c == '"') {
      bool closed;
      char * str = lexer_read_string(lexer, &closed);
      if (!closed) {
        free(str);
        lexer_error(lexer, "Unclosed string literal.");
        break;
      }
      size_t tkl = lexer->__line__;
      size_t tkc = lexer->__column__;
      token_t* token = token_make(TOKEN_STRING, str, tkl, tkc);
      vector_push_back(lexer->tokens, &token);
      free(str);
      continue;
    }
    if (isdigit(c)) {
      size_t start = lexer->__pos__;
      bool has_dot = false;
      while (lexer->__pos__ < input_len) {
        char ch = lexer->__input__[lexer->__pos__];
        if (isdigit(ch)) {
          lexer_advance(lexer);
        } else if (ch == '.' && !has_dot) {
          has_dot = true;
          lexer_advance(lexer);
        } else {
          break;
        }
      }

      bool is_long = false;
      if (lexer->__pos__ < input_len &&
          (lexer->__input__[lexer->__pos__] == 'l' ||
           lexer->__input__[lexer->__pos__] == 'L')) {
        is_long = true;
        lexer_advance(lexer);
      }

      char * number =
          str_substring(lexer->__input__, start, lexer->__pos__);
      if (number == NULL) {
        lexer_error(lexer, "Failed to extract number");
      }

      size_t tkl = lexer->__line__;
      size_t tkc = lexer->__column__;
      token_kind_t numType = TOKEN_INT;

      if (is_long) {
        numType = TOKEN_LONG;
      } else if (has_dot) {
        numType = TOKEN_FLOAT;
      }

      token_t* token = token_make(numType, number, tkl, tkc);
      vector_push_back(lexer->tokens, &token);
      free(number);
      continue;
    }

    if (isalpha(c) || c == '_') {
      size_t start = lexer->__pos__;
      while (lexer->__pos__ < input_len &&
             (isalpha(lexer->__input__[lexer->__pos__]) ||
              isdigit(lexer->__input__[lexer->__pos__]) ||
              lexer->__input__[lexer->__pos__] == '_')) {
        lexer_advance(lexer);
      }
      char* word = str_substring(lexer->__input__, start, lexer->__pos__);
      if (word == NULL) {
        lexer_error(lexer, "Failed to get word");
        break;
      }
      size_t tkl = lexer->__line__;
      size_t tkc = lexer->__column__;
      if (str_equals(word, "work") || str_equals(word, "return") ||
          str_equals(word, "import")) {
        token_t* token = token_make(TOKEN_KEYWORD, word, tkl, tkc);
        vector_push_back(lexer->tokens, &token);

      } else if (str_equals(word, "true") ||
                 str_equals(word, "false")) {
        token_t* token = token_make(TOKEN_BOOL, word, tkl, tkc);
        vector_push_back(lexer->tokens, &token);

      } else if (str_equals(word, "bool") || str_equals(word, "int") ||
                 str_equals(word, "float") ||
                 str_equals(word, "long") ||
                 str_equals(word, "string") ||
                 str_equals(word, "any")) {
        token_t* token = token_make(TOKEN_TYPE, word, tkl, tkc);
        vector_push_back(lexer->tokens, &token);
      } else if (str_equals(word, "var")) {
        token_t* token = token_make(TOKEN_VAR, word, tkl, tkc);
        vector_push_back(lexer->tokens, &token);
      } else if (str_equals(word, "let")) {
        token_t* token = token_make(TOKEN_LET, word, tkl, tkc);
        vector_push_back(lexer->tokens, &token);
      } else {
        token_t* token = token_make(TOKEN_IDENTIFIER, word, tkl, tkc);
        vector_push_back(lexer->tokens, &token);
      }
      free(word);
      continue;
    }
    lexer_error(lexer, "Unexpected character %c", c);
    break;
  }
  size_t tkl = lexer->__line__;
  size_t tkc = lexer->__column__;
  token_t* token = token_make(TOKEN_EOF, "", tkl, tkc);
  vector_push_back(lexer->tokens, &token);
}

void lexer_error(lexer_t* lexer, char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "[Error at %zu:%zu] ", lexer->__line__, lexer->__column__);
  vprintf(fmt, args);
  printf("\n");
  va_end(args);
  exit(1);
}
