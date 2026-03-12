#include "kilate/parser.h"

#include <malloc.h>
#include <stdarg.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilate/debug.h"
#include "kilate/file.h"
#include "kilate/lexer.h"
#include "kilate/native.h"
#include "kilate/node.h"
#include "kilate/string.h"
#include "kilate/vector.h"

parser_t *parser_make(token_vector_t *tokens)
{
        parser_t *p = malloc(sizeof(*p));
        p->tokens = tokens;
        p->nodes = vector_make(sizeof(node_t *));
        p->scope_body = NULL;
        p->__pos__ = 0;
        return p;
}

void parser_delete(parser_t *p)
{
        for (size_t i = 0; i < p->nodes->size; ++i) {
                node_t *n = *(node_t **)vector_get(p->nodes, i);
                node_delete(n);
        }
        vector_delete(p->nodes);
        free(p);
}

token_t *parser_current(parser_t *p, size_t offset)
{
        size_t pos = p->__pos__ + offset;

        if (pos >= p->tokens->size)
                return NULL;

        return *(token_t **)vector_get(p->tokens, pos);
}

token_t *parser_consume(parser_t *p, token_kind_t exType)
{
        token_t *tk = parser_current(p, 0);
        if (tk->type != exType) {
                parser_error(tk, "Expected %s, but got %s",
                             tokentype_tostr(exType),
                             tokentype_tostr(tk->type));
                return NULL;
        }
        p->__pos__++;
        return tk;
}

static void print_nodes(parser_t *p)
{
        printd("Nodes am: %zu\n", p->nodes->size);
        for (size_t i = 0; i < p->nodes->size; i++) {
                node_t *n = *(node_t **)vector_get(p->nodes, i);
                printd("Node(%lu).type = (%s)\n", i,
                       (n) ? node_kind_tostr(n->type) : "(null)");
        }
}

function_node_t *parser_find_function(parser_t *p, char *name)
{
        printd("Searching for fn: %s\n", name);
        printd("Total nodes: %zu\n", p->nodes->size);

        for (size_t i = 0; i < p->nodes->size; i++) {
                function_node_t *fn = *(node_t **)vector_get(p->nodes, i);

                if (!fn)
                        continue;

                printd("Node %zu type: %d\n", i, fn->type);

                if (fn->type == NODE_FUNCTION) {
                        if (!str_equals(fn->function_n.name, name))
                                continue;
                        printd("Fn found: %s\n", fn->function_n.name);
                        return fn;
                }
        }
        return NULL;
}

vardec_node_t *parser_find_var(parser_t *p, char *name)
{
        printd("Searching for var: %s\n", name);
        printd("Total nodes: %zu\n", p->nodes->size);

        // find at top-level nodes
        for (size_t i = 0; i < p->nodes->size; i++) {
                vardec_node_t *var = *(node_t **)vector_get(p->nodes, i);

                if (!var)
                        continue;

                printd("Node %zu type: %d\n", i, var->type);

                if (var->type == NODE_VARDEC) {
                        if (!str_equals(var->vardec_n.name, name))
                                continue;
                        printd("Var found: %s\n", var->vardec_n.name);
                        return var;
                }
        }

        // find at local scope nodes
        for (size_t i = 0; i < p->scope_body->size; i++) {
                vardec_node_t *var = *(node_t **)vector_get(p->scope_body, i);
                if (!var)
                        continue;

                printd("Node %zu type: %d\n", i, var->type);

                if (var->type == NODE_VARDEC) {
                        if (!str_equals(var->vardec_n.name, name))
                                continue;
                        printd("Var found: %s\n", var->vardec_n.name);
                        return var;
                }
        }
        return NULL;
}

char *parser_tokentype_to_str(token_kind_t type)
{
        switch (type) {
        case TOKEN_STRING:
                return "String";
        case TOKEN_BOOL:
                return "Nool";
        case TOKEN_INT:
                return "Int";
        case TOKEN_FLOAT:
                return "Float";
        case TOKEN_LONG:
                return "Long";
        default:
                return "unknown";
        }
}

char *parser_nodevaluetype_to_str(node_value_kind_t type)
{
        switch (type) {
        case NODE_VALUE_TYPE_INT:
                return "Int";
        case NODE_VALUE_TYPE_FLOAT:
                return "Float";
        case NODE_VALUE_TYPE_LONG:
                return "Long";
        case NODE_VALUE_TYPE_STRING:
                return "String";
        case NODE_VALUE_TYPE_BOOL:
                return "Bool";
        case NODE_VALUE_TYPE_FUNC:
                return "Function";
        case NODE_VALUE_TYPE_VAR:
                return "Var";
        default:
                return "Any";
        }
}

node_value_kind_t parser_tokentype_to_nodevaluetype(parser_t *p, token_t *tk)
{
        // printd("121: %d, %s\n", tk->type, tk->text);
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
                token_t *next = parser_current(p, 1);
                // printd("CUR:%s NEXT:%s\n", tk->text, next ? next->text :
                // "NULL"); printd("135:%s\n", next->text);
                if (next->type == TOKEN_LPAREN || next->type == TOKEN_LARROW ||
                    next->type == TOKEN_RARROW) {
                        return NODE_VALUE_TYPE_CALL;
                } else {
                        return NODE_VALUE_TYPE_FUNC_OR_VAR;
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

node_value_kind_t parser_str_to_nodevaluetype(char *value)
{
#ifndef ct
#define ct(str) str_equals(value, str)
#endif

        if (ct("String")) {
                return NODE_VALUE_TYPE_STRING;
        } else if (ct("Bool")) {
                return NODE_VALUE_TYPE_BOOL;
        } else if (ct("Int")) {
                return NODE_VALUE_TYPE_INT;
        } else if (ct("Float")) {
                return NODE_VALUE_TYPE_FLOAT;
        } else if (ct("Long")) {
                return NODE_VALUE_TYPE_LONG;
        } else {
                return NODE_VALUE_TYPE_ANY;
        }

#ifdef ct
#undef ct
#endif
}

node_t *parser_parse_statement(parser_t *p)
{
        static int am = 0;
        am++;

        token_t *tk = parser_current(p, 0);
        printd("parser_parse_statement called %d times, cur token: %s\n", am,
               tk->text);

        if (tk->type == TOKEN_KEYWORD && str_equals(tk->text, "return")) {
                parser_consume(p, TOKEN_KEYWORD);
                token_t *arrow = parser_current(p, 0);
                if (arrow->type == TOKEN_RARROW ||
                    arrow->type == TOKEN_LARROW) {
                        parser_consume(p, arrow->type);
                }

                token_t *next = parser_current(p, 0);
                node_t *rn = alloc_node(NODE_RETURN);

                if (next->type == TOKEN_BOOL) {
                        rn->return_n.b = str_equals(
                            parser_consume(p, TOKEN_BOOL)->text, "true");
                        rn->return_n.type = NODE_VALUE_TYPE_BOOL;
                } else if (next->type == TOKEN_INT) {
                        rn->return_n.i =
                            str_to_int(parser_consume(p, TOKEN_INT)->text);
                        rn->return_n.type = NODE_VALUE_TYPE_INT;
                } else if (next->type == TOKEN_FLOAT) {
                        rn->return_n.f =
                            str_to_float(parser_consume(p, TOKEN_FLOAT)->text);
                        rn->return_n.type = NODE_VALUE_TYPE_FLOAT;
                } else if (next->type == TOKEN_LONG) {
                        rn->return_n.l =
                            str_to_long(parser_consume(p, TOKEN_LONG)->text);
                        rn->return_n.type = NODE_VALUE_TYPE_LONG;
                } else if (next->type == TOKEN_STRING) {
                        rn->return_n.s = parser_consume(p, TOKEN_STRING)->text;
                        rn->return_n.type = NODE_VALUE_TYPE_STRING;
                } else if (next->type == TOKEN_IDENTIFIER) {
                        token_t *id_token = next;
                        token_t *after = parser_current(p, 1);
                        if (after->type == TOKEN_RARROW ||
                            after->type == TOKEN_LARROW ||
                            after->type == TOKEN_LPAREN) {
                                node_t *call =
                                    parser_parse_call_node(p, id_token);
                                rn->return_n.n = call;
                                rn->return_n.type = NODE_VALUE_TYPE_CALL;
                        } else {
                                rn->return_n.s =
                                    parser_consume(p, TOKEN_IDENTIFIER)->text;
                                rn->return_n.type = NODE_VALUE_TYPE_VAR;
                        }
                } else {
                        parser_error(
                            next,
                            "Unsupported value in typed return statement.");
                        return NULL;
                }
                return rn;

        } else if (tk->type == TOKEN_KEYWORD &&
                   str_equals(tk->text, "import")) {
                return parser_parse_import(p);

        } else if (tk->type == TOKEN_KEYWORD && str_equals(tk->text, "work")) {
                return parser_parse_function(p);

        } else if (tk->type == TOKEN_TYPE) {
                char *vartype = parser_consume(p, TOKEN_TYPE)->text;
                char *varname = parser_consume(p, TOKEN_IDENTIFIER)->text;

                parser_consume(p, TOKEN_ASSIGN);

                token_t *valueTk = parser_current(p, 0);

                vardec_node_t *varnode = alloc_node(NODE_VARDEC);
                varnode->vardec_n.name = strdup(varname);
                varnode->vardec_n.type = strdup(vartype);

                if (valueTk->type == TOKEN_STRING) {
                        valueTk = parser_consume(p, TOKEN_STRING);
                        varnode->vardec_n.value.s = valueTk->text;
                        varnode->vardec_n.value.type = NODE_VALUE_TYPE_STRING;
                } else if (valueTk->type == TOKEN_INT) {
                        valueTk = parser_consume(p, TOKEN_INT);
                        int temp = str_to_int(valueTk->text);
                        varnode->vardec_n.value.i = temp;
                        varnode->vardec_n.value.type = NODE_VALUE_TYPE_INT;
                } else if (valueTk->type == TOKEN_FLOAT) {
                        valueTk = parser_consume(p, TOKEN_FLOAT);
                        float temp = str_to_float(valueTk->text);
                        varnode->vardec_n.value.f = temp;
                        varnode->vardec_n.value.type = NODE_VALUE_TYPE_FLOAT;
                } else if (valueTk->type == TOKEN_LONG) {
                        valueTk = parser_consume(p, TOKEN_LONG);
                        long temp = str_to_long(valueTk->text);
                        varnode->vardec_n.value.l = temp;
                        varnode->vardec_n.value.type = NODE_VALUE_TYPE_LONG;
                } else if (valueTk->type == TOKEN_BOOL) {
                        valueTk = parser_consume(p, TOKEN_BOOL);
                        bool temp = str_equals(valueTk->text, "true");
                        varnode->vardec_n.value.b = temp;
                        varnode->vardec_n.value.type = NODE_VALUE_TYPE_BOOL;
                } else if (valueTk->type == TOKEN_IDENTIFIER) {
                        token_t *next = parser_current(p, 1);
                        if (next->type == TOKEN_RARROW ||
                            next->type == TOKEN_LARROW ||
                            next->type == TOKEN_LPAREN) {
                                node_t *call_node =
                                    parser_parse_call_node(p, valueTk);
                                varnode->vardec_n.value.n = call_node;
                                varnode->vardec_n.value.type =
                                    NODE_VALUE_TYPE_CALL;
                        } else {
                                valueTk = parser_consume(p, TOKEN_IDENTIFIER);
                                varnode->vardec_n.value.s = valueTk->text;
                                varnode->vardec_n.value.type =
                                    NODE_VALUE_TYPE_VAR;
                        }
                } else {
                        parser_error(valueTk, "Unsupported value in typed "
                                              "variable declaration.");
                        return NULL;
                }

                char *expected = vartype;
                char *actual = parser_tokentype_to_str(valueTk->type);

                if (varnode->vardec_n.value.type == NODE_VALUE_TYPE_VAR) {
                        node_t *var =
                            parser_find_var(p, varnode->vardec_n.value.s);
                        if (!var)
                                return NULL; // IMPORTANT TODO: Notice Error
                        actual = var->vardec_n.type;
                }

                // if valuetype is a call, so we need to get called function
                // return type
                if (varnode->vardec_n.value.type == NODE_VALUE_TYPE_CALL) {
                        node_t *call = varnode->vardec_n.value.n;
                        node_t *fn =
                            parser_find_function(p, call->call_n.name);
                        if (!fn)
                                return NULL; // IMPORTANT TODO: Notice Error
                        actual = fn->function_n.return_type;
                }

                if (!str_equals(expected, "any") &&
                    !str_equals(expected, actual)) {
                        parser_error(valueTk,
                                     "Type mismatch in declaration of '%s': "
                                     "expected '%s', got '%s', raw: %d",
                                     varname, expected, actual, valueTk->type);
                }
                printd("Var(%s) = (%s)\n", varname, valueTk->text);
                return varnode;
        } else if (tk->type == TOKEN_IDENTIFIER) {
                // TODO: This case can be also a variable declarion of user own
                // types so we need more checks here to really know if its a
                // call or a var decl
                return parser_parse_call_node(p, tk);
        }

        parser_error(tk, "Unknown statement: %s", tk->text);
        return NULL;
}

node_param_vector_t *parser_parse_fnparams(parser_t *p)
{
        node_param_vector_t *params = vector_make(sizeof(param_node_t *));
        print_nodes(p);

        while (true) {
                token_t *param = parser_current(p, 0);
                if (!param || !param->text) {
                        parser_error(param, "Somehow param value is null.");
                        return NULL;
                }

                if (param->type != TOKEN_STRING && param->type != TOKEN_BOOL &&
                    param->type != TOKEN_INT && param->type != TOKEN_FLOAT &&
                    param->type != TOKEN_LONG &&
                    param->type != TOKEN_IDENTIFIER) {
                        break;
                }

                param_node_t *fn_param = alloc_node(NODE_ARG);
                node_value_kind_t vkind =
                    parser_tokentype_to_nodevaluetype(p, param);
                // printd("358: %d, %s\n", vkind, param->text);
                if (vkind == NODE_VALUE_TYPE_CALL) {
                        call_node_t *callnode =
                            parser_parse_call_node(p, param);
                        fn_param->arg_n.type = NODE_VALUE_TYPE_CALL;
                        fn_param->arg_n.n = callnode;
                } else if (vkind == NODE_VALUE_TYPE_FUNC_OR_VAR) {
                        printd("before consume: %s\n", param->text);
                        parser_consume(p, param->type);
                        printd("after consume: %s\n", param->text);
                        bool f = false;
                        function_node_t *fn =
                            parser_find_function(p, param->text);
                        if (fn && !f) {
                                fn_param->arg_n.type = NODE_VALUE_TYPE_FUNC;
                                fn_param->arg_n.n = fn;
                                printd("fn: %s\n", param->text);
                                f = true;
                        }

                        native_function_node_t *nfn =
                            native_find_function(param->text);
                        if (nfn && !f) {
                                fn_param->arg_n.type = NODE_VALUE_TYPE_FUNC;
                                fn_param->arg_n.n = nfn;
                                printd("nfn: %s\n", param->text);
                                f = true;
                        }

                        vardec_node_t *var = parser_find_var(p, param->text);
                        if (var && !f) {
                                fn_param->arg_n.type = NODE_VALUE_TYPE_VAR;
                                fn_param->arg_n.s = param->text;
                                printd("var: %s\n", param->text);
                                f = true;
                        }

                        if (!f) {
                                parser_error(param,
                                             "Param '%s' is not a Function, "
                                             "NativeFunction nor a Variable.",
                                             param->text);
                                return NULL;
                        }
                } else {
                        parser_consume(p, param->type);
                        fn_param->arg_n.type = vkind;
                        fn_param->arg_n.s = strdup(param->text);
                }
                vector_push_back(params, &fn_param);

                token_t *comma = parser_current(p, 0);
                if (comma->type == TOKEN_COMMA) {
                        parser_consume(p, TOKEN_COMMA);
                } else {
                        break;
                }
        }
        return params;
}

void parser_fn_validate_params(node_t *fn, node_param_vector_t *params,
                               token_t *tk)
{
        size_t expected = fn->function_n.params->size;
        if (params->size != expected) {
                parser_error(
                    tk, "Function '%s' expects %zu parameters but got %zu.",
                    fn->function_n.name, expected, params->size);
        }

        for (size_t i = 0; i < expected; ++i) {
                param_node_t *param =
                    *(param_node_t **)vector_get(fn->function_n.params, i);

                param_node_t *callParam =
                    *(param_node_t **)vector_get(params, i);
                node_value_kind_t actualType = callParam->arg_n.type;
                if (actualType == NODE_VALUE_TYPE_VAR ||
                    actualType == NODE_VALUE_TYPE_FUNC)
                        continue;
                if (param->arg_n.type != NODE_VALUE_TYPE_ANY &&
                    param->arg_n.type != actualType) {
                        parser_error(
                            tk,
                            "Argument %zu to '%s' must be of type %s, got %s",
                            i + 1, fn->function_n.name,
                            parser_nodevaluetype_to_str(param->arg_n.type),
                            parser_nodevaluetype_to_str(actualType));
                }
        }
}

node_t *parser_parse_call_node(parser_t *p, token_t *tk)
{
        char *name = parser_consume(p, TOKEN_IDENTIFIER)->text;
        token_t *next = parser_current(p, 0);

        // printd("446: %d, %s\n", next->type, next->text);

        // call with () no params
        if (next->type == TOKEN_LPAREN) {
                parser_consume(p, TOKEN_LPAREN);
                parser_consume(p, TOKEN_RPAREN);

                node_t *fn = parser_find_function(p, name);
                if (fn != NULL) {
                        size_t expected = fn->function_n.params->size;
                        if (expected > 0) {
                                parser_error(
                                    next,
                                    "Function '%s' expects %zu parameters but "
                                    "none were provided.",
                                    name, expected);
                        }
                } else {
                        function_node_t *nativeFn = native_find_function(name);
                        nativeFn->function_n.native = true;
                        if (nativeFn != NULL) {
                                if (nativeFn->function_n.params != NULL &&
                                    nativeFn->function_n.params->size > 0) {
                                        parser_error(
                                            next,
                                            "Native function '%s' expects %zu "
                                            "parameters but none "
                                            "were provided.",
                                            name,
                                            nativeFn->function_n.params->size);
                                }
                        } else {
                                parser_error(tk,
                                             "Function '%s' is not declared "
                                             "and is not a native function.",
                                             name);
                        }
                }

                return call_node_make(name, NULL);
        }

        // call with ->
        if (next->type == TOKEN_RARROW || next->type == TOKEN_LARROW) {
                parser_consume(p, next->type);
                node_arg_vector_t *params = parser_parse_fnparams(p);

                node_t *fn = parser_find_function(p, name);
                if (fn != NULL) {
                        parser_fn_validate_params(fn, params, next);
                        return call_node_make(name, params);
                }

                // check if its native
                native_function_node_t *nativeFn = native_find_function(name);
                if (nativeFn != NULL) {
                        return call_node_make(name, params);
                }

                parser_error(tk,
                             "Function '%s' is not declared and is not a "
                             "native function.",
                             name);
        }
        parser_error(next, "Unexpected token after identifier: %s",
                     next->text);
        return NULL;
}

node_t *parser_parse_import(parser_t *p)
{
        parser_consume(p, TOKEN_KEYWORD);
        token_t *path_token = parser_consume(p, TOKEN_STRING);

        file_t file;
        file_open(&file, path_token->text, FILE_MODE_READ);

        char *src = file_read_text(&file);
        if (!src) {
                parser_error(path_token, "Failed to read import: %s",
                             path_token->text);
        }

        lexer_t *lexer = lexer_make(src);
        lexer_tokenize(lexer);

        parser_t *new_parser = parser_make(lexer->tokens);
        parser_parse_program(new_parser);

        for (size_t i = 0; i < new_parser->nodes->size; i++) {
                node_t **fnPtr = (node_t **)vector_get(new_parser->nodes, i);
                node_t *fn = *fnPtr;
                if (fn->type == NODE_FUNCTION) {
                        node_t *copy = node_copy(fn);
                        vector_push_back(p->nodes, &copy);
                }
        }

        parser_delete(new_parser);
        lexer_delete(lexer);
        file_close(&file);
        free(src);

        return NULL;
}

node_t *parser_parse_function(parser_t *p)
{
        token_t *tk = parser_consume(p, TOKEN_KEYWORD);
        if (!(str_equals(tk->text, "work"))) {
                parser_error(tk, "Unexpected token: %s\n", tk->text);
        }

        function_node_t *fn = alloc_node(NODE_FUNCTION);
        fn->function_n.name = parser_consume(p, TOKEN_IDENTIFIER)->text;

        parser_consume(p, TOKEN_LPAREN);
        fn->function_n.params = vector_make(sizeof(param_node_t *));
        fn->function_n.native = false;

        while (true) {
                token_t *next = parser_current(p, 0);
                if (next->type == TOKEN_RPAREN) {
                        parser_consume(p, TOKEN_RPAREN);
                        break;
                }

                char *type = parser_consume(p, TOKEN_TYPE)->text;
                parser_consume(p, TOKEN_COLON);
                char *name = parser_consume(p, TOKEN_IDENTIFIER)->text;

                param_node_t *param = malloc(sizeof(*param));
                param->arg_n.s = strdup(name);
                param->arg_n.type = parser_str_to_nodevaluetype(type);
                // param->typeStr = strdup(type);
                vector_push_back(fn->function_n.params, &param);

                next = parser_current(p, 0);
                if (next->type == TOKEN_COMMA) {
                        parser_consume(p, TOKEN_COMMA);
                }
        }

        if ((parser_current(p, 0))->type == TOKEN_COLON) {
                parser_consume(p, TOKEN_COLON);
                fn->function_n.return_type =
                    parser_consume(p, TOKEN_TYPE)->text;
        }

        parser_consume(p, TOKEN_LBRACE);
        fn->function_n.body = vector_make(sizeof(node_t *));
        p->scope_body = fn->function_n.body;
        while ((parser_current(p, 0))->type != TOKEN_RBRACE) {
                node_t *n = parser_parse_statement(p);
                vector_push_back(fn->function_n.body, &n);
        }

        parser_consume(p, TOKEN_RBRACE);
        p->scope_body = NULL;

        if (fn->function_n.body->size == 0) {
                parser_error((parser_current(p, 0)),
                             "Function '%s' is empty, remove or implement it.",
                             fn->function_n.name);
        }

        if (fn->function_n.return_type != NULL) {
                node_t *lastnode = *(node_t **)vector_get(
                    fn->function_n.body, fn->function_n.body->size - 1);
                if (lastnode != NULL) {
                        if (lastnode->type != NODE_RETURN) {
                                parser_error((*(token_t **)vector_get(
                                                 p->tokens, p->__pos__)),
                                             "Function '%s' must end with "
                                             "return statement.",
                                             fn->function_n.name);
                        }
                        node_value_kind_t retType;
                        if (str_equals(fn->function_n.return_type, "Int")) {
                                retType = NODE_VALUE_TYPE_INT;
                        } else if (str_equals(fn->function_n.return_type,
                                              "Flat")) {
                                retType = NODE_VALUE_TYPE_FLOAT;
                        } else if (str_equals(fn->function_n.return_type,
                                              "Long")) {
                                retType = NODE_VALUE_TYPE_LONG;
                        } else if (str_equals(fn->function_n.return_type,
                                              "Dtring")) {
                                retType = NODE_VALUE_TYPE_STRING;
                        } else if (str_equals(fn->function_n.return_type,
                                              "Bool")) {
                                retType = NODE_VALUE_TYPE_BOOL;
                        } else if (str_equals(fn->function_n.return_type,
                                              "Any")) {
                                retType = NODE_VALUE_TYPE_ANY;
                        } else {
                                retType = lastnode->return_n.type;
                        }
                        if (retType != lastnode->return_n.type) {
                                parser_error(
                                    (*(token_t **)vector_get(p->tokens,
                                                             p->__pos__)),
                                    "The expected return type of function "
                                    "'%s' is '%s', but what was "
                                    "received was: '%s'",
                                    fn->function_n.name,
                                    fn->function_n.return_type,
                                    parser_nodevaluetype_to_str(retType));
                        }
                }
        }
        return fn;
}

void parser_parse_program(parser_t *p)
{
        do {
                node_t *n = parser_parse_statement(p);

                if (n != NULL) {
                        vector_push_back(p->nodes, &n);
                }
        } while ((parser_current(p, 0))->type != TOKEN_EOF);
}

void parser_error(token_t *tk, char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        fprintf(stderr, "[Error At %zu:%zu] ", tk->line, tk->column);
        vprintf(fmt, args);
        printf("\n");
        va_end(args);
        exit(1);
}
