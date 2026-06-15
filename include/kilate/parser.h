#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdarg.h>

#include "kilate/lexer.h"
#include "kilate/node.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct parser_t parser_t;

parser_t *parser_make (const char *, token_vector_t *);

void parser_delete (parser_t *);

void parser_parse_program (parser_t *);

node_vector_t *parser_get_nodes (parser_t *);

#ifdef __cplusplus
}
#endif

#endif
