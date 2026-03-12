#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdarg.h>

#include "kilate/lexer.h"
#include "kilate/node.h"
#include "kilate/string.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
        size_t __pos__;
        token_vector_t *tokens;
        node_vector_t *nodes;
        node_vector_t *scope_body; // the current scope body nodes
} parser_t;

parser_t *parser_make(token_vector_t *);

void parser_delete(parser_t *);

void parser_delete_params(str_vector_t *);

token_t *parser_consume(parser_t *, token_kind_t);

node_t *parser_find_function(parser_t *, char *);

char *parser_tokentype_to_str(token_kind_t);

char *parser_nodevaluetype_to_str(node_value_kind_t);

node_value_kind_t parser_tokentype_to_nodevaluetype(parser_t *, token_t *);

node_value_kind_t parser_str_to_nodevaluetype(char *);

node_t *parser_parse_statement(parser_t *);

node_param_vector_t *parser_parse_fnparams(parser_t *);

void parser_fn_validate_params(node_t *, node_param_vector_t *, token_t *);

node_t *parser_parse_call_node(parser_t *, token_t *);

node_t *parser_parse_import(parser_t *);

void parser_parse_program(parser_t *);

node_t *parser_parse_function(parser_t *);

void parser_error(token_t *, char *, ...);

#ifdef __cplusplus
}
#endif

#endif
