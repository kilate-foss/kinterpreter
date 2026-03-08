#include "kilate/parser.h"

#include <stdarg.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "kilate/file.h"
#include "kilate/hashmap.h"
#include "kilate/lexer.h"
#include "kilate/native.h"
#include "kilate/node.h"
#include "kilate/string.h"
#include "kilate/vector.h"

parser_t *parser_make(token_vector_t * tokens) {
  parser_t *p = malloc(sizeof(*p));
  p->tokens = tokens;
  p->nodes = vector_make(sizeof(node_t *));
  p->__pos__ = 0;
  return p;
}

void parser_delete(parser_t *p) {
  for (size_t i = 0; i < p->nodes->size; ++i) {
    node_t *n = *(node_t **)vector_get(p->nodes, i);
    node_delete(n);
  }
  vector_delete(p->nodes);
  free(p);
}

token_t * parser_consume(parser_t *p, token_kind_t exType) {
  token_t * tk =
      *(token_t **)vector_get(p->tokens, p->__pos__);
  if (tk->type != exType) {
    parser_error(tk, "Expected %s, but got %s",
                     tokentype_tostr(exType),
                     tokentype_tostr(tk->type));
    return NULL;
  }
  p->__pos__++;
  return tk;
}

node_t * parser_find_function(parser_t *p, char*name) {
  for (size_t i = 0; i < p->nodes->size; i++) {
    node_t * fn = *(node_t **)vector_get(p->nodes, i);
    if (fn != NULL) {
      if (fn->type == NODE_FUNCTION) {
        if (str_equals(fn->function_n.fn_name, name)) {
          return fn;
        }
      }
    }
  }
  return NULL;
}

char * parser_tokentype_to_str(token_kind_t type) {
  switch (type) {
    case TOKEN_STRING:
      return "string";
    case TOKEN_BOOL:
      return "bool";
    case TOKEN_INT:
      return "int";
    case TOKEN_FLOAT:
      return "float";
    case TOKEN_LONG:
      return "long";
    default:
      return "unknow";
  }
}

char * parser_nodevaluetype_to_str(node_value_kind_t type) {
  switch (type) {
    case NODE_VALUE_TYPE_INT:
      return "int";
    case NODE_VALUE_TYPE_FLOAT:
      return "float";
    case NODE_VALUE_TYPE_LONG:
      return "long";
    case NODE_VALUE_TYPE_STRING:
      return "string";
    case NODE_VALUE_TYPE_BOOL:
      return "bool";
    case NODE_VALUE_TYPE_FUNC:
      return "function";
    case NODE_VALUE_TYPE_VAR:
      return "var";
    default:
      return "any";
  }
}

node_value_kind_t parser_tokentype_to_nodevaluetype(parser_t *p,
                                                         token_t * tk) {
  switch (tk->type) {
    case TOKEN_STRING:
      return NODE_VALUE_TYPE_STRING;
    case TOKEN_BOOL:
      return NODE_VALUE_TYPE_BOOL;
    case TOKEN_INT:
      return NODE_VALUE_TYPE_INT;
    case TOKEN_FLOAT:
      return NODE_VALUE_TYPE_FLOAT;
    case TOKEN_LONG:
      return NODE_VALUE_TYPE_LONG;
    case TOKEN_IDENTIFIER: {
      token_t * next =
          *(token_t **)vector_get(p->tokens, p->__pos__);
      if (next->type == TOKEN_LPAREN) {
        return NODE_VALUE_TYPE_FUNC;
      } else {
        return NODE_VALUE_TYPE_VAR;
      }
    }
    case TOKEN_TYPE: {
      if (str_equals(tk->text, "any")) {
        return NODE_VALUE_TYPE_ANY;
      }
    }
    default:
      return NODE_VALUE_TYPE_ANY;
  }
}

node_value_kind_t parser_str_to_nodevaluetype(char*value) {
#ifndef ct
#define ct(str) str_equals(value, str)
#endif

  if (ct("string")) {
    return NODE_VALUE_TYPE_STRING;
  } else if (ct("bool")) {
    return NODE_VALUE_TYPE_BOOL;
  } else if (ct("int")) {
    return NODE_VALUE_TYPE_INT;
  } else if (ct("float")) {
    return NODE_VALUE_TYPE_FLOAT;
  } else if (ct("long")) {
    return NODE_VALUE_TYPE_LONG;
  } else {
    return NODE_VALUE_TYPE_ANY;
  }

#ifdef ct
#undef ct
#endif
}

node_t * parser_parse_statement(parser_t *p) {
  token_t * tk =
      *(token_t **)vector_get(p->tokens, p->__pos__);

  if (tk->type == TOKEN_KEYWORD && str_equals(tk->text, "return")) {
    parser_consume(p, TOKEN_KEYWORD);
    token_t * arrow =
        *(token_t **)vector_get(p->tokens, p->__pos__);
    if (arrow->type == TOKEN_RARROW || arrow->type == TOKEN_LARROW) {
      parser_consume(p, arrow->type);
    }
    token_t * next =
        *(token_t **)vector_get(p->tokens, p->__pos__);
    void* value;
    node_value_kind_t type;

    if (next->type == TOKEN_BOOL) {
      bool rawBool =
          str_equals(parser_consume(p, TOKEN_BOOL)->text, "true");
      value = (void*)(intptr_t)rawBool;
      type = NODE_VALUE_TYPE_BOOL;
    } else if (next->type == TOKEN_INT) {
      value = (void*)(intptr_t)str_to_int(
          parser_consume(p, TOKEN_INT)->text);
      type = NODE_VALUE_TYPE_INT;
    } else if (next->type == TOKEN_FLOAT) {
      float fval =
          str_to_float(parser_consume(p, TOKEN_FLOAT)->text);
      value = malloc(sizeof(float));
      memcpy(value, &fval, sizeof(float));
      type = NODE_VALUE_TYPE_FLOAT;
    } else if (next->type == TOKEN_LONG) {
      long lval = str_to_long(parser_consume(p, TOKEN_LONG)->text);
      value = (void*)(intptr_t)lval;
      type = NODE_VALUE_TYPE_LONG;
    } else if (next->type == TOKEN_STRING) {
      value = parser_consume(p, TOKEN_STRING)->text;
      type = NODE_VALUE_TYPE_STRING;
    } else if (next->type == TOKEN_IDENTIFIER) {
      token_t * id_token = next;
      token_t * after =
          *(token_t **)vector_get(p->tokens, p->__pos__ + 1);
      if (after->type == TOKEN_RARROW || after->type == TOKEN_LARROW ||
          after->type == TOKEN_LPAREN) {
        node_t * call = parser_parse_call_node(p, id_token);
        value = call;
        type = NODE_VALUE_TYPE_CALL;
      } else {
        value = parser_consume(p, TOKEN_IDENTIFIER)->text;
        type = NODE_VALUE_TYPE_VAR;
      }
    } else {
      parser_error(next, "Unsupported value in typed return statement.");
      return NULL;
    }
    return return_node_make(type, value);

  } else if (tk->type == TOKEN_KEYWORD &&
             str_equals(tk->text, "import")) {
    return parser_parse_import(p);

  } else if (tk->type == TOKEN_KEYWORD &&
             str_equals(tk->text, "work")) {
    return parser_parse_function(p);

  } else if (tk->type == TOKEN_VAR || tk->type == TOKEN_LET) {
    p->__pos__++;
    char*var_name = parser_consume(p, TOKEN_IDENTIFIER)->text;
    parser_consume(p, TOKEN_ASSIGN);
    token_t * var_valueTk =
        *(token_t **)vector_get(p->tokens, p->__pos__);

    node_value_kind_t var_value_type;
    char*varInferredType;
    void* var_value;

    if (var_valueTk->type == TOKEN_STRING) {
      var_value = parser_consume(p, TOKEN_STRING)->text;
      var_value_type = NODE_VALUE_TYPE_STRING;
    } else if (var_valueTk->type == TOKEN_INT) {
      int temp = str_to_int(parser_consume(p, TOKEN_INT)->text);
      var_value = (void*)(intptr_t)temp;
      var_value_type = NODE_VALUE_TYPE_INT;
    } else if (var_valueTk->type == TOKEN_FLOAT) {
      float fval =
          str_to_float(parser_consume(p, TOKEN_FLOAT)->text);
      var_value = malloc(sizeof(float));
      memcpy(var_value, &fval, sizeof(float));
      var_value_type = NODE_VALUE_TYPE_FLOAT;
    } else if (var_valueTk->type == TOKEN_LONG) {
      long lval = str_to_long(parser_consume(p, TOKEN_LONG)->text);
      var_value = (void*)(intptr_t)lval;
      var_value_type = NODE_VALUE_TYPE_LONG;
    } else if (var_valueTk->type == TOKEN_BOOL) {
      bool rawBool =
          str_equals(parser_consume(p, TOKEN_BOOL)->text, "true");
      var_value = (void*)(intptr_t)rawBool;
      var_value_type = NODE_VALUE_TYPE_BOOL;
    } else if (var_valueTk->type == TOKEN_IDENTIFIER) {
      token_t * id_token = var_valueTk;
      token_t * next =
          *(token_t **)vector_get(p->tokens, p->__pos__ + 1);
      if (next->type == TOKEN_RARROW || next->type == TOKEN_LARROW ||
          next->type == TOKEN_LPAREN) {
        node_t * call_node = parser_parse_call_node(p, id_token);
        var_value = call_node;
        var_value_type = NODE_VALUE_TYPE_CALL;
      } else {
        var_value = parser_consume(p, TOKEN_IDENTIFIER)->text;
        var_value_type = NODE_VALUE_TYPE_VAR;
      }
    } else {
      var_value = parser_consume(p, var_valueTk->type)->text;
      var_value_type = NODE_VALUE_TYPE_ANY;
    }
    varInferredType = parser_nodevaluetype_to_str(var_value_type);
    return var_dec_node_make(var_name, varInferredType, var_value_type,
                                 var_value);

  } else if (tk->type == TOKEN_TYPE) {
    char*var_type = parser_consume(p, TOKEN_TYPE)->text;
    char*var_name = parser_consume(p, TOKEN_IDENTIFIER)->text;
    parser_consume(p, TOKEN_ASSIGN);
    token_t * valueTk =
        *(token_t **)vector_get(p->tokens, p->__pos__);

    node_value_kind_t var_value_type;
    void* var_value;

    if (valueTk->type == TOKEN_STRING) {
      valueTk = parser_consume(p, TOKEN_STRING);
      var_value = valueTk->text;
      var_value_type = NODE_VALUE_TYPE_STRING;
    } else if (valueTk->type == TOKEN_INT) {
      valueTk = parser_consume(p, TOKEN_INT);
      int temp = str_to_int(valueTk->text);
      var_value = (void*)(intptr_t)temp;
      var_value_type = NODE_VALUE_TYPE_INT;
    } else if (valueTk->type == TOKEN_FLOAT) {
      valueTk = parser_consume(p, TOKEN_FLOAT);
      float fval = str_to_float(valueTk->text);
      var_value = malloc(sizeof(float));
      memcpy(var_value, &fval, sizeof(float));
      var_value_type = NODE_VALUE_TYPE_FLOAT;
    } else if (valueTk->type == TOKEN_LONG) {
      valueTk = parser_consume(p, TOKEN_LONG);
      long lval = str_to_long(valueTk->text);
      var_value = (void*)(intptr_t)lval;
      var_value_type = NODE_VALUE_TYPE_LONG;
    } else if (valueTk->type == TOKEN_BOOL) {
      valueTk = parser_consume(p, TOKEN_BOOL);
      bool bval = str_equals(valueTk->text, "true");
      var_value = (void*)(intptr_t)bval;
      var_value_type = NODE_VALUE_TYPE_BOOL;
    } else if (valueTk->type == TOKEN_IDENTIFIER) {
      token_t * next =
          *(token_t **)vector_get(p->tokens, p->__pos__ + 1);
      if (next->type == TOKEN_RARROW || next->type == TOKEN_LARROW ||
          next->type == TOKEN_LPAREN) {
        node_t * call_node = parser_parse_call_node(p, valueTk);
        var_value = call_node;
        var_value_type = NODE_VALUE_TYPE_CALL;
      } else {
        valueTk = parser_consume(p, TOKEN_IDENTIFIER);
        var_value = valueTk->text;
        var_value_type = NODE_VALUE_TYPE_VAR;
      }
    } else {
      parser_error(valueTk,
                       "Unsupported value in typed variable declaration.");
      return NULL;
    }

    char*expected = var_type;
    char*actual = parser_tokentype_to_str(valueTk->type);
    if (!str_equals(expected, "any") && !str_equals(expected, actual)) {
      parser_error(
          valueTk,
          "Type mismatch in declaration of '%s': expected '%s', got '%s",
          var_name, expected, actual);
    }
    return var_dec_node_make(var_name, var_type, var_value_type, var_value);
  } else if (tk->type == TOKEN_IDENTIFIER) {
    return parser_parse_call_node(p, tk);
  }

  parser_error(tk, "Unknown statement: %s", tk->text);
  return NULL;
}

node_fnparam_vector_t * parser_parse_fnparams(parser_t *p) {
  node_fnparam_vector_t * params =
      vector_make(sizeof(node_fnparam_vector_t *));

  while (true) {
    token_t * param =
        *(token_t **)vector_get(p->tokens, p->__pos__);

    if (param->type != TOKEN_STRING && param->type != TOKEN_BOOL &&
        param->type != TOKEN_INT && param->type != TOKEN_FLOAT &&
        param->type != TOKEN_LONG && param->type != TOKEN_IDENTIFIER) {
      break;
    }

    node_fnparam_t* fnParam = malloc(sizeof(node_fnparam_t));
    fnParam->value = strdup(param->text);
    fnParam->type = parser_tokentype_to_nodevaluetype(p, param);
    vector_push_back(params, &fnParam);

    parser_consume(p, param->type);

    token_t * comma =
        *(token_t **)vector_get(p->tokens, p->__pos__);
    if (comma->type == TOKEN_COMMA) {
      parser_consume(p, TOKEN_COMMA);
    } else {
      break;
    }
  }
  return params;
}

void parser_fn_validate_params(node_t * fn,
                                   node_fnparam_vector_t * params,
                                   token_t * tk) {
  size_t expected = fn->function_n.fn_params->size;
  if (params->size != expected) {
    parser_error(tk, "Function '%s' expects %zu parameters but got %zu.",
                     fn->function_n.fn_name, expected, params->size);
  }

  for (size_t i = 0; i < expected; ++i) {
    node_fnparam_t* param =
        *(node_fnparam_t**)vector_get(fn->function_n.fn_params, i);

    node_fnparam_t* callParam =
        *(node_fnparam_t**)vector_get(params, i);
    node_value_kind_t actualType = callParam->type;
    if (actualType == NODE_VALUE_TYPE_VAR || actualType == NODE_VALUE_TYPE_FUNC)
      continue;
    if (param->type != NODE_VALUE_TYPE_ANY && param->type != actualType) {
      parser_error(tk, "Argument %zu to '%s' must be of type %s, got %s",
                       i + 1, fn->function_n.fn_name,
                       parser_nodevaluetype_to_str(param->type),
                       parser_nodevaluetype_to_str(actualType));
    }
  }
}

node_t * parser_parse_call_node(parser_t *p, token_t * tk) {
  char*name = parser_consume(p, TOKEN_IDENTIFIER)->text;
  token_t * next =
      *(token_t **)vector_get(p->tokens, p->__pos__);

  // call with () no params
  if (next->type == TOKEN_LPAREN) {
    parser_consume(p, TOKEN_LPAREN);
    parser_consume(p, TOKEN_RPAREN);

    node_t * fn = parser_find_function(p, name);
    if (fn != NULL) {
      size_t expected = fn->function_n.fn_params->size;
      if (expected > 0) {
        parser_error(
            next,
            "Function '%s' expects %zu parameters but none were provided.",
            name, expected);
      }
    } else {
      native_fnentry_t* nativeFn = native_find_function(name);
      if (nativeFn != NULL) {
        if (nativeFn->requiredParams != NULL &&
            nativeFn->requiredParams->size > 0) {
          parser_error(
              next,
              "Native function '%s' expects %zu parameters but none "
              "were provided.",
              name, nativeFn->requiredParams->size);
        }
      } else {
        parser_error(
            tk,
            "Function '%s' is not declared and is not a native function.",
            name);
      }
    }

    return call_node_make(name, NULL);
  }

  // call with ->
  if (next->type == TOKEN_RARROW || next->type == TOKEN_LARROW) {
    parser_consume(p, next->type);
    node_fnparam_vector_t * params = parser_parse_fnparams(p);

    node_t * fn = parser_find_function(p, name);
    if (fn != NULL) {
      parser_fn_validate_params(fn, params, next);
      return call_node_make(name, params);
    }

    // check if its native
    native_fnentry_t* nativeFn = native_find_function(name);
    if (nativeFn != NULL) {
      return call_node_make(name, params);
    }

    parser_error(
        tk, "Function '%s' is not declared and is not a native function.",
        name);
  }
  parser_error(tk, "Unexpected token after identifier: %s", next->text);
  return NULL;
}

node_t * parser_parse_import(parser_t * p) {
  parser_consume(p, TOKEN_KEYWORD);
  token_t * path_token = parser_consume(p, TOKEN_STRING);

  file_t *file = file_open(path_token->text, FILE_MODE_READ);
  if (!file) {
    parser_error(path_token, "Failed to import file: %s", path_token->text);
  }

  char*src = file_read_text(file);
  if (!src) {
    parser_error(path_token, "Failed to read import: %s", path_token->text);
  }

  lexer_t* lexer = lexer_make(src);
  lexer_tokenize(lexer);

  parser_t * new_parser = parser_make(lexer->tokens);
  parser_parse_program(new_parser);

  for (size_t i = 0; i < new_parser->nodes->size; i++) {
    node_t ** fnPtr = (node_t **)vector_get(new_parser->nodes, i);
    node_t * fn = *fnPtr;
    if (fn->type == NODE_FUNCTION) {
      node_t * copy = node_copy(fn);
      vector_push_back(p->nodes, &copy);
    }
  }

  parser_delete(new_parser);
  lexer_delete(lexer);
  file_close(file);
  free(src);

  return NULL;
}

node_t * parser_parse_function(parser_t * p) {
  token_t * tk = parser_consume(p, TOKEN_KEYWORD);
  if (!(str_equals(tk->text, "work"))) {
    parser_error(tk, "Unexpected token: %s\n", tk->text);
  }
  char*name = parser_consume(p, TOKEN_IDENTIFIER)->text;

  parser_consume(p, TOKEN_LPAREN);
  node_fnparam_vector_t * params = vector_make(sizeof(node_fnparam_t*));

  while (true) {
    token_t * next =
        *(token_t **)vector_get(p->tokens, p->__pos__);
    if (next->type == TOKEN_RPAREN) {
      parser_consume(p, TOKEN_RPAREN);
      break;
    }

    char*type = parser_consume(p, TOKEN_TYPE)->text;
    parser_consume(p, TOKEN_COLON);
    char*name = parser_consume(p, TOKEN_IDENTIFIER)->text;

    node_fnparam_t* param = malloc(sizeof(node_fnparam_t));
    param->value = strdup(name);
    param->type = parser_str_to_nodevaluetype(type);
    // param->typeStr = strdup(type);
    vector_push_back(params, &param);

    next = *(token_t **)vector_get(p->tokens, p->__pos__);
    if (next->type == TOKEN_COMMA) {
      parser_consume(p, TOKEN_COMMA);
    }
  }

  char*return_type = NULL;
  if ((*(token_t **)vector_get(p->tokens, p->__pos__))->type ==
      TOKEN_COLON) {
    parser_consume(p, TOKEN_COLON);
    return_type = parser_consume(p, TOKEN_TYPE)->text;
  }

  parser_consume(p, TOKEN_LBRACE);
  node_vector_t * body = vector_make(sizeof(node_t *));
  while (
      (*(token_t **)vector_get(p->tokens, p->__pos__))->type !=
      TOKEN_RBRACE) {
    node_t *n = parser_parse_statement(p);
    vector_push_back(body, &n);
  }

  parser_consume(p, TOKEN_RBRACE);

  if (body->size == 0) {
    parser_error(
        (*(token_t **)vector_get(p->tokens, p->__pos__)),
        "Function '%s' is empty, remove or implement it.", name);
  }

  if (return_type != NULL) {
    node_t * lastnode = *(node_t **)vector_get(body, body->size - 1);
    if (lastnode != NULL) {
      if (lastnode->type != NODE_RETURN) {
        parser_error(
            (*(token_t **)vector_get(p->tokens, p->__pos__)),
            "Function '%s' must end with return statement.", name);
      }
      node_value_kind_t retType;
      if (str_equals(return_type, "int")) {
        retType = NODE_VALUE_TYPE_INT;
      } else if (str_equals(return_type, "float")) {
        retType = NODE_VALUE_TYPE_FLOAT;
      } else if (str_equals(return_type, "long")) {
        retType = NODE_VALUE_TYPE_LONG;
      } else if (str_equals(return_type, "string")) {
        retType = NODE_VALUE_TYPE_STRING;
      } else if (str_equals(return_type, "bool")) {
        retType = NODE_VALUE_TYPE_BOOL;
      } else if (str_equals(return_type, "any")) {
        retType = NODE_VALUE_TYPE_ANY;
      } else {
        retType = lastnode->return_n.return_type;
      }
      if (retType != lastnode->return_n.return_type) {
        parser_error(
            (*(token_t **)vector_get(p->tokens, p->__pos__)),
            "The expected return type of function '%s' is '%s', but what was "
            "received was: '%s'",
            name, return_type, parser_nodevaluetype_to_str(retType));
      }
    }
  }

  return function_node_make(name, return_type, body, params);
}

void parser_parse_program(parser_t *p) {
  do {
    node_t *n = parser_parse_statement(p);

    if (n != NULL) {
      vector_push_back(p->nodes, &n);
    }
  } while (
      (*(token_t **)vector_get(p->tokens, p->__pos__))->type !=
      TOKEN_EOF);
}

void parser_error(token_t * tk, char*fmt, ...) {
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "[Error At %zu:%zu] ", tk->line, tk->column);
  vprintf(fmt, args);
  printf("\n");
  va_end(args);
  exit(1);
}
