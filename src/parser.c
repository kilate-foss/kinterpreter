#include "kilate/parser.h"

#include <malloc.h>
#include <stdarg.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilate/bool.h"
#include "kilate/debug.h"
#include "kilate/file.h"
#include "kilate/lexer.h"
#include "kilate/native.h"
#include "kilate/node.h"
#include "kilate/string.h"
#include "kilate/vector.h"

struct parser_t
{
        node_vector_t *nodes;
        node_vector_t *scope_body;

        token_vector_t *tokens;

        const char *filename;
        size_t pos;
};

static void
parser_error (parser_t *p, token_t *tk, const char *fmt, ...)
{
        va_list args;
        va_start (args, fmt);
        fprintf (stderr, "parser::error[%s:%zu:%zu]: ", p->filename, tk->line,
                 tk->column);
        vprintf (fmt, args);
        printf ("\n");
        va_end (args);
        exit (1);
}

static token_t *
parser_current (parser_t *p, size_t offset)
{
        size_t pos = p->pos + offset;

        if (pos >= p->tokens->size)
                return NULL;

        return *(token_t **)vector_get (p->tokens, pos);
}

static token_t *
parser_consume (parser_t *p, token_kind_t exType)
{
        token_t *tk = parser_current (p, 0);
        if (tk->type != exType)
        {
                parser_error (p, tk, "Expected %s, but got %s",
                              tokentype_tostr (exType),
                              tokentype_tostr (tk->type));
                return NULL;
        }
        p->pos++;
        return tk;
}

static void
print_nodes (parser_t *p)
{
        printd ("parser::print_nodes+1: Nodes am: %zu\n", p->nodes->size);
        for (size_t i = 0; i < p->nodes->size; i++)
        {
                node_t *n = (node_t *)vector_get (p->nodes, i);
                printd ("parser::print_nodes+4: Node(%lu).type = (%s)\n", i,
                        (n) ? node_kind_tostr (n->type) : "(null)");
        }
}

static node_value_kind_t
parser_tokentype_to_nodevaluetype (parser_t *p, token_t *tk)
{
        // printd("121: %d, %s\n", tk->type, tk->text);
        switch (tk->type)
        {
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
        case TOKEN_IDENTIFIER:
        {
                token_t *next = parser_current (p, 1);
                // printd("CUR:%s NEXT:%s\n", tk->text, next ? next->text :
                // "NULL"); printd("135:%s\n", next->text);
                if (next->type == TOKEN_LPAREN || next->type == TOKEN_LARROW
                    || next->type == TOKEN_RARROW)
                {
                        return NODE_VALUE_TYPE_CALL;
                }
                else
                {
                        return NODE_VALUE_TYPE_FUNC_OR_VAR;
                }
        }
        case TOKEN_TYPE:
        {
                if (str_equals (tk->text, "Any"))
                {
                        return NODE_VALUE_TYPE_ANY;
                }
        }
        default:
                return NODE_VALUE_TYPE_ANY;
        }
}

static function_node_t *
parser_find_function (parser_t *p, const char *name)
{
        printd ("parser::parser_find_function+1: Searching for fn: %s\n",
                name);
        printd ("parser::parser_find_function+2: Total nodes: %zu\n",
                p->nodes->size);

        for (size_t i = 0; i < p->nodes->size; i++)
        {
                function_node_t *fn = (node_t *)vector_get (p->nodes, i);

                if (!fn)
                        continue;

                printd ("parser::parser_find_function+10: Node %zu type: %d\n",
                        i, fn->type);

                if (fn->type == NODE_FUNCTION)
                {
                        if (!str_equals (fn->function_n.name, name))
                                continue;
                        printd (
                            "parser::parser_find_function+15: Fn found: %s\n",
                            fn->function_n.name);
                        return fn;
                }
        }
        return NULL;
}

static vardecl_node_t *
parser_find_var (parser_t *p, const char *name)
{
        printd ("parser::parser_find_var+1: Searching for var: %s\n", name);
        printd ("parser::parser_find_var+2: Total nodes: %zu\n",
                p->nodes->size);

        for (size_t i = 0; i < p->nodes->size; i++)
        {
                vardecl_node_t *var = (node_t *)vector_get (p->nodes, i);

                if (!var)
                        continue;

                printd ("parser::parser_find_function+1: Node %zu type: %d\n",
                        i, var->type);

                if (var->type == NODE_VARDECL)
                {
                        if (!str_equals (var->vardecl_n.name, name))
                                continue;
                        printd ("parser::parser_find_var+15: Var found: %s\n",
                                var->vardecl_n.name);
                        return var;
                }
        }

        if (!p->scope_body)
                return NULL;

        for (size_t i = 0; i < p->scope_body->size; i++)
        {
                vardecl_node_t *var = (node_t *)vector_get (p->scope_body, i);
                if (!var)
                        continue;

                printd ("parser::parser_find_var+25: Node %zu type: %d\n", i,
                        var->type);

                if (var->type == NODE_VARDECL)
                {
                        if (!str_equals (var->vardecl_n.name, name))
                                continue;
                        printd ("parser::parser_find_var+30: Var found: %s\n",
                                var->vardecl_n.name);
                        return var;
                }
        }
        return NULL;
}

static call_node_t parser_parse_call_node (parser_t *, token_t *);

static node_t parser_parse_statement (parser_t *);

static node_param_vector_t *
parser_parse_fnparams (parser_t *p)
{
        node_param_vector_t *params = vector_make (sizeof (param_node_t));
        print_nodes (p);

        while (true)
        {
                token_t *param = parser_current (p, 0);
                if (!param || !param->text)
                {
                        parser_error (p, param,
                                      "Somehow param value is null.");
                        return NULL;
                }

                if (param->type != TOKEN_STRING && param->type != TOKEN_BOOL
                    && param->type != TOKEN_INT && param->type != TOKEN_FLOAT
                    && param->type != TOKEN_LONG
                    && param->type != TOKEN_IDENTIFIER)
                {
                        break;
                }

                param_node_t fn_param = make_node (NODE_ARG);
                node_value_kind_t vkind
                    = parser_tokentype_to_nodevaluetype (p, param);
                // printd("358: %d, %s\n", vkind, param->text);
                if (vkind == NODE_VALUE_TYPE_CALL)
                {
                        call_node_t callnode
                            = parser_parse_call_node (p, param);
                        fn_param.arg_n.type = NODE_VALUE_TYPE_CALL;
                        fn_param.arg_n.n = node_copy (&callnode);
                }
                else if (vkind == NODE_VALUE_TYPE_FUNC_OR_VAR)
                {
                        printd ("parser::parser_parse_fnparams+27: before "
                                "consume: %s\n",
                                param->text);
                        parser_consume (p, param->type);
                        printd ("parser::parser_parse_fnparams+29: after "
                                "consume: %s\n",
                                param->text);
                        bool f = false;
                        function_node_t *fn
                            = parser_find_function (p, param->text);
                        if (fn && !f)
                        {
                                fn_param.arg_n.type = NODE_VALUE_TYPE_FUNC;
                                fn_param.arg_n.n = fn;
                                printd ("parser::parser_parse_fnparams+36: "
                                        "fn: %s\n",
                                        param->text);
                                f = true;
                        }

                        native_function_node_t *nfn
                            = native_find_function (param->text);
                        if (nfn && !f)
                        {
                                fn_param.arg_n.type = NODE_VALUE_TYPE_FUNC;
                                fn_param.arg_n.n = nfn;
                                printd ("parser::parser_parse_fnparams+44: "
                                        "nfn: %s\n",
                                        param->text);
                                f = true;
                        }

                        vardecl_node_t *var = parser_find_var (p, param->text);
                        if (var && !f)
                        {
                                fn_param.arg_n.type = NODE_VALUE_TYPE_VAR;
                                fn_param.arg_n.s = strdup (param->text);
                                printd ("parser::parser_parse_fnparams+51: "
                                        "var: %s\n",
                                        param->text);
                                f = true;
                        }

                        if (!f)
                        {
                                parser_error (p, param,
                                              "Param '%s' is not a Function, "
                                              "NativeFunction nor a Variable.",
                                              param->text);
                                return NULL;
                        }
                }
                else
                {
                        parser_consume (p, param->type);
                        fn_param.arg_n.type = vkind;
                        fn_param.arg_n.s = strdup (param->text);
                }
                vector_push_back (params, &fn_param);

                token_t *comma = parser_current (p, 0);
                if (comma->type == TOKEN_COMMA)
                {
                        parser_consume (p, TOKEN_COMMA);
                }
                else
                {
                        break;
                }
        }

        if (params->size <= 0)
        {
                vector_delete (params);
                return NULL;
        }

        return params;
}

static call_node_t
parser_parse_call_node (parser_t *p, token_t *tk)
{
        const char *name = parser_consume (p, TOKEN_IDENTIFIER)->text;
        token_t *next = parser_current (p, 0);

        call_node_t call = make_node (NODE_CALL);
        call.call_n.name = strdup (name);

        if (next->type == TOKEN_LPAREN)
        {
                parser_consume (p, TOKEN_LPAREN);
                parser_consume (p, TOKEN_RPAREN);
                return call;
        }
        else if (next->type == TOKEN_RARROW || next->type == TOKEN_LARROW)
        {
                parser_consume (p, next->type);

                node_arg_vector_t *params = parser_parse_fnparams (p);
                call.call_n.args = params;
                return call;
        }
        parser_error (p, next, "Unexpected token after %s: %s", tk->text,
                      next->text);
        return (node_t){ .type = NODE_INVALID };
}

static import_node_t
parser_parse_import (parser_t *p)
{
        parser_consume (p, TOKEN_KEYWORD);
        token_t *path_token = parser_consume (p, TOKEN_STRING);

        file_t file;
        file_open (&file, path_token->text, FILE_MODE_READ);

        char *src = file_read_text (&file);
        if (!src)
        {
                parser_error (p, path_token, "Failed to read import: %s",
                              path_token->text);
        }

        lexer_t *lexer = lexer_make (path_token->text, src);
        lexer_tokenize (lexer);

        parser_t *new_parser = parser_make (path_token->text, lexer->tokens);
        parser_parse_program (new_parser);

        for (size_t i = 0; i < new_parser->nodes->size; i++)
        {
                node_t *fn = (node_t *)vector_get (new_parser->nodes, i);
                if (fn->type == NODE_FUNCTION)
                {
                        node_t *copy = node_copy (fn);
                        vector_push_back (p->nodes, copy);
                }
        }

        parser_delete (new_parser);
        lexer_delete (lexer);
        file_close (&file);
        free (src);

        return (node_t){ .type = NODE_INVALID };
}

static function_node_t
parser_parse_function (parser_t *p)
{
        token_t *tk = parser_consume (p, TOKEN_KEYWORD);
        if (!(str_equals (tk->text, "work")))
        {
                parser_error (p, tk, "Unexpected keyword: %s\n", tk->text);
        }

        function_node_t fn = make_node (NODE_FUNCTION);
        fn.function_n.name
            = strdup (parser_consume (p, TOKEN_IDENTIFIER)->text);

        parser_consume (p, TOKEN_LPAREN);
        fn.function_n.params = vector_make (sizeof (param_node_t));
        fn.function_n.native = false;

        while (true)
        {
                token_t *next = parser_current (p, 0);
                if (next->type == TOKEN_RPAREN)
                {
                        parser_consume (p, TOKEN_RPAREN);
                        break;
                }

                char *name = parser_consume (p, TOKEN_IDENTIFIER)->text;
                parser_consume (p, TOKEN_COLON);
                char *type = parser_consume (p, TOKEN_TYPE)->text;

                param_node_t *param = malloc (sizeof (*param));
                param->arg_n.s = strdup (name);
                param->arg_n.type = str_to_node_value_kind (type);
                // param->typeStr = strdup(type);
                vector_push_back (fn.function_n.params, param);

                next = parser_current (p, 0);
                if (next->type == TOKEN_COMMA)
                {
                        parser_consume (p, TOKEN_COMMA);
                }
        }

        if (fn.function_n.params->size <= 0)
        {
                vector_delete (fn.function_n.params);
                fn.function_n.params = NULL;
        }

        if ((parser_current (p, 0))->type == TOKEN_COLON)
        {
                parser_consume (p, TOKEN_COLON);
                fn.function_n.return_type
                    = strdup (parser_consume (p, TOKEN_TYPE)->text);
        }

        parser_consume (p, TOKEN_LBRACE);
        fn.function_n.body = vector_make (sizeof (node_t));
        p->scope_body = fn.function_n.body;
        while ((parser_current (p, 0))->type != TOKEN_RBRACE)
        {
                node_t n = parser_parse_statement (p);
                vector_push_back (fn.function_n.body, &n);
        }

        parser_consume (p, TOKEN_RBRACE);

        if (fn.function_n.body->size == 0)
        {
                parser_error (
                    p, (parser_current (p, 0)),
                    "Function '%s' is empty, remove or implement it.",
                    fn.function_n.name);
        }

        if (fn.function_n.return_type != NULL)
        {
                node_t *lastnode = (node_t *)vector_get (
                    fn.function_n.body, fn.function_n.body->size - 1);
                if (lastnode != NULL)
                {
                        if (lastnode->type != NODE_RETURN)
                        {
                                parser_error (p, parser_current (p, 0),
                                              "Function '%s' must end with "
                                              "return statement.",
                                              fn.function_n.name);
                        }
                        node_value_kind_t retType;
                        if (str_equals (fn.function_n.return_type, "Int"))
                        {
                                retType = NODE_VALUE_TYPE_INT;
                        }
                        else if (str_equals (fn.function_n.return_type,
                                             "Flat"))
                        {
                                retType = NODE_VALUE_TYPE_FLOAT;
                        }
                        else if (str_equals (fn.function_n.return_type,
                                             "Long"))
                        {
                                retType = NODE_VALUE_TYPE_LONG;
                        }
                        else if (str_equals (fn.function_n.return_type,
                                             "String"))
                        {
                                retType = NODE_VALUE_TYPE_STRING;
                        }
                        else if (str_equals (fn.function_n.return_type,
                                             "Bool"))
                        {
                                retType = NODE_VALUE_TYPE_BOOL;
                        }
                        else if (str_equals (fn.function_n.return_type, "Any"))
                        {
                                retType = NODE_VALUE_TYPE_ANY;
                        }
                        else
                        {
                                retType = lastnode->return_n.type;
                        }

                        node_value_kind_t real_type = retType;
                        value_t ln = lastnode->return_n;

                        if (ln.type == NODE_VALUE_TYPE_CALL)
                        {
                                call_node_t *call = ln.n;
                                function_node_t *fn = parser_find_function (
                                    p, call->call_n.name);
                                real_type = str_to_node_value_kind (
                                    fn->function_n.return_type);
                        }

                        if (ln.type == NODE_VALUE_TYPE_VAR)
                        {
                                vardecl_node_t *var
                                    = parser_find_var (p, ln.s);
                                if (!var)
                                {
                                        parser_error (
                                            p, tk, "Variable not exists: %s",
                                            ln.s);
                                }
                                real_type = var->vardecl_n.value.type;
                        }

                        if (retType != real_type)
                        {
                                printd ("parser::parser_parse_function+119: "
                                        "retType(%d), lastnode(%d)\n",
                                        retType, lastnode->return_n.type);
                                parser_error (
                                    p, parser_current (p, 0),
                                    "The expected return type of function "
                                    "'%s' is '%s', but what was "
                                    "received was: '%s'",
                                    fn.function_n.name,
                                    fn.function_n.return_type,
                                    node_value_kind_to_str (retType));
                        }
                }
        }
        p->scope_body = NULL;
        return fn;
}

static node_param_vector_t *
parse_fndecl_params (parser_t *p)
{
        node_param_vector_t *params = vector_make (sizeof (arg_node_t));
        while (true)
        {
                token_t *tk = parser_current (p, 0);
                if (tk->type == TOKEN_RPAREN)
                {
                        parser_consume (p, TOKEN_RPAREN);
                        break;
                }

                const char *name = parser_consume (p, TOKEN_IDENTIFIER)->text;
                parser_consume (p, TOKEN_COLON);
                const char *type = parser_consume (p, TOKEN_TYPE)->text;

                vector_push_back (
                    params,
                    &(node_t){ .type = NODE_ARG,
                               .arg_n.s = strdup (name),
                               .arg_n.type = str_to_node_value_kind (type) });

                tk = parser_current (p, 0);
                if (tk->type == TOKEN_COMMA)
                {
                        parser_consume (p, TOKEN_COMMA);
                }
        }
        if (params->size <= 0)
        {
                vector_delete (params);
                params = NULL;
        }
        return params;
}

static nativedecl_node_t
parser_parse_native_decl (parser_t *p)
{
        token_t *tk;

        tk = parser_consume (p, TOKEN_KEYWORD);
        if (!str_equals (tk->text, "native"))
        {
                parser_error (p, tk, "Unexpected keyword: %s", tk->text);
        }

        /** common function decl parsing */
        nativedecl_node_t node = make_node (NODE_NATIVEDECL);
        struct __function_node *f = &node.function_n;

        parser_consume (p, TOKEN_KEYWORD);
        f->name = parser_consume (p, TOKEN_IDENTIFIER)->text;
        f->native = true;
        parser_consume (p, TOKEN_LPAREN);
        f->params = parse_fndecl_params (p);

        if ((parser_current (p, 0)->type) == TOKEN_COLON)
        {
                parser_consume (p, TOKEN_COLON);
                f->return_type = parser_consume (p, TOKEN_TYPE)->text;
        }
        return node;
}

static node_t
parser_parse_variable (parser_t *p, token_t *tk)
{
        token_t *k;
        if ((k = parser_consume (p, TOKEN_KEYWORD),
             !str_equals (k->text, "let")))
        {
                parser_error (p, k, "Unexpected token after %s: %s", tk->text,
                              k->text);
        }
        const char *varname = parser_consume (p, TOKEN_IDENTIFIER)->text;
        parser_consume (p, TOKEN_COLON);
        const char *vartype = parser_consume (p, TOKEN_TYPE)->text;
        parser_consume (p, TOKEN_ASSIGN);

        token_t *valueTk = parser_current (p, 0);

        vardecl_node_t varnode = make_node (NODE_VARDECL);
        varnode.vardecl_n.name = strdup (varname);
        varnode.vardecl_n.type = strdup (vartype);

        if (valueTk->type == TOKEN_STRING)
        {
                valueTk = parser_consume (p, TOKEN_STRING);
                varnode.vardecl_n.value.s = strdup (valueTk->text);
                varnode.vardecl_n.value.type = NODE_VALUE_TYPE_STRING;
        }
        else if (valueTk->type == TOKEN_INT)
        {
                valueTk = parser_consume (p, TOKEN_INT);
                int temp = str_to_int (valueTk->text);
                varnode.vardecl_n.value.i = temp;
                varnode.vardecl_n.value.type = NODE_VALUE_TYPE_INT;
        }
        else if (valueTk->type == TOKEN_FLOAT)
        {
                valueTk = parser_consume (p, TOKEN_FLOAT);
                float temp = str_to_float (valueTk->text);
                varnode.vardecl_n.value.f = temp;
                varnode.vardecl_n.value.type = NODE_VALUE_TYPE_FLOAT;
        }
        else if (valueTk->type == TOKEN_LONG)
        {
                valueTk = parser_consume (p, TOKEN_LONG);
                long temp = str_to_long (valueTk->text);
                varnode.vardecl_n.value.l = temp;
                varnode.vardecl_n.value.type = NODE_VALUE_TYPE_LONG;
        }
        else if (valueTk->type == TOKEN_BOOL)
        {
                valueTk = parser_consume (p, TOKEN_BOOL);
                bool temp = str_equals (valueTk->text, "true");
                varnode.vardecl_n.value.b = temp;
                varnode.vardecl_n.value.type = NODE_VALUE_TYPE_BOOL;
        }
        else if (valueTk->type == TOKEN_IDENTIFIER)
        {
                token_t *next = parser_current (p, 1);
                if (next->type == TOKEN_RARROW || next->type == TOKEN_LARROW
                    || next->type == TOKEN_LPAREN)
                {
                        node_t call_node = parser_parse_call_node (p, valueTk);
                        varnode.vardecl_n.value.n = node_copy (&call_node);
                        varnode.vardecl_n.value.type = NODE_VALUE_TYPE_CALL;
                }
                else
                {
                        valueTk = parser_consume (p, TOKEN_IDENTIFIER);
                        varnode.vardecl_n.value.s = strdup (valueTk->text);
                        varnode.vardecl_n.value.type = NODE_VALUE_TYPE_VAR;
                }
        }
        else
        {
                parser_error (p, valueTk,
                              "Unsupported value in typed "
                              "variable declaration.");
                return (node_t){ .type = NODE_INVALID };
        }

        const char *expected = vartype;
        const char *actual = token_kind_to_str (valueTk->type);

        if (varnode.vardecl_n.value.type == NODE_VALUE_TYPE_VAR)
        {
                node_t *var = parser_find_var (p, varnode.vardecl_n.value.s);
                if (!var)
                {
                        parser_error (p, valueTk, "Variable not exists: %s",
                                      varnode.vardecl_n.value.s);
                }
                actual = var->vardecl_n.type;
        }

        // if valuetype is a call, so we need to get called function
        // return type
        if (varnode.vardecl_n.value.type == NODE_VALUE_TYPE_CALL)
        {
                node_t *call = varnode.vardecl_n.value.n;
                node_t *fn = parser_find_function (p, call->call_n.name);
                if (!fn)
                        return (node_t){
                                .type = NODE_INVALID
                        }; // IMPORTANT TODO: Notice Error
                actual = fn->function_n.return_type;
        }

        if (!str_equals (expected, "Any") && !str_equals (expected, actual))
        {
                parser_error (p, valueTk,
                              "Type mismatch in declaration of '%s': "
                              "expected '%s', got '%s', raw: %d",
                              varname, expected, actual, valueTk->type);
        }
        printd ("parser::parser_parse_statement+154: Var(%s) = (%s)\n",
                varname, valueTk->text);
        return varnode;
}

static node_t
parser_parse_statement (parser_t *p)
{
        static int am = 0;
        am++;

        token_t *tk = parser_current (p, 0);
        printd (
            "parser::parser_parse_statement called %d times, cur token: %s\n",
            am, tk->text);

        if (tk->type == TOKEN_KEYWORD && str_equals (tk->text, "return"))
        {
                parser_consume (p, TOKEN_KEYWORD);
                token_t *arrow = parser_current (p, 0);
                if (arrow->type == TOKEN_RARROW || arrow->type == TOKEN_LARROW)
                {
                        parser_consume (p, arrow->type);
                }

                token_t *next = parser_current (p, 0);
                node_t rn = make_node (NODE_RETURN);

                if (next->type == TOKEN_BOOL)
                {
                        rn.return_n.b = str_equals (
                            parser_consume (p, TOKEN_BOOL)->text, "true");
                        rn.return_n.type = NODE_VALUE_TYPE_BOOL;
                }
                else if (next->type == TOKEN_INT)
                {
                        rn.return_n.i
                            = str_to_int (parser_consume (p, TOKEN_INT)->text);
                        rn.return_n.type = NODE_VALUE_TYPE_INT;
                }
                else if (next->type == TOKEN_FLOAT)
                {
                        rn.return_n.f = str_to_float (
                            parser_consume (p, TOKEN_FLOAT)->text);
                        rn.return_n.type = NODE_VALUE_TYPE_FLOAT;
                }
                else if (next->type == TOKEN_LONG)
                {
                        rn.return_n.l = str_to_long (
                            parser_consume (p, TOKEN_LONG)->text);
                        rn.return_n.type = NODE_VALUE_TYPE_LONG;
                }
                else if (next->type == TOKEN_STRING)
                {
                        rn.return_n.s
                            = strdup (parser_consume (p, TOKEN_STRING)->text);
                        rn.return_n.type = NODE_VALUE_TYPE_STRING;
                }
                else if (next->type == TOKEN_IDENTIFIER)
                {
                        token_t *id_token = next;
                        token_t *after = parser_current (p, 1);
                        if (after->type == TOKEN_RARROW
                            || after->type == TOKEN_LARROW
                            || after->type == TOKEN_LPAREN)
                        {
                                node_t call
                                    = parser_parse_call_node (p, id_token);
                                rn.return_n.n = node_copy (&call);
                                rn.return_n.type = NODE_VALUE_TYPE_CALL;
                        }
                        else
                        {
                                rn.return_n.s = strdup (
                                    parser_consume (p, TOKEN_IDENTIFIER)
                                        ->text);
                                rn.return_n.type = NODE_VALUE_TYPE_VAR;
                        }
                }
                else
                {
                        parser_error (
                            p, next,
                            "Unsupported value in typed return statement.");
                        return (node_t){ .type = NODE_INVALID };
                }
                return rn;
        }
        else if (tk->type == TOKEN_KEYWORD && str_equals (tk->text, "import"))
        {
                return parser_parse_import (p);
        }
        else if (tk->type == TOKEN_KEYWORD && str_equals (tk->text, "work"))
        {
                return parser_parse_function (p);
        }
        else if (tk->type == TOKEN_KEYWORD && str_equals (tk->text, "native"))
        {
                return parser_parse_native_decl (p);
        }
        else if (tk->type == TOKEN_KEYWORD && str_equals (tk->text, "let"))
        {
                return parser_parse_variable (p, tk);
        }
        else if (tk->type == TOKEN_IDENTIFIER)
        {
                token_t *next = parser_current (p, 1);
                if (next->type == TOKEN_LPAREN || next->type == TOKEN_LARROW
                    || next->type == TOKEN_RARROW)
                {
                        return parser_parse_call_node (p, tk);
                }
                return parser_parse_variable (p, tk);
        }
        parser_error (p, tk, "Unknown statement: %s", tk->text);
        return (node_t){ .type = NODE_INVALID };
}

parser_t *
parser_make (const char *filename, token_vector_t *tokens)
{
        parser_t *p = malloc (sizeof (*p));
        p->tokens = tokens;
        p->nodes = vector_make (sizeof (node_t));
        p->scope_body = NULL;
        p->pos = 0;
        p->filename = filename;
        return p;
}

void
parser_delete (parser_t *p)
{
        for (size_t i = 0; i < p->nodes->size; ++i)
        {
                node_t *n = (node_t *)vector_get (p->nodes, i);
                node_delete (n);
        }
        vector_delete (p->nodes);
        free (p);
}

node_vector_t *
parser_get_nodes (parser_t *p)
{
        return p->nodes;
}

void
parser_parse_program (parser_t *p)
{
        do
        {
                node_t n = parser_parse_statement (p);

                if (n.type != NODE_INVALID)
                {
                        vector_push_back (p->nodes, &n);
                }
        } while ((parser_current (p, 0))->type != TOKEN_EOF);
}
