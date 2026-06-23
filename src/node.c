#include "kilate/node.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "kilate/bool.h"
#include "kilate/debug.h"
#include "kilate/environment.h"
#include "kilate/interpreter.h"
#include "kilate/vector.h"

void
node_delete (node_t *n)
{
        if (n == NULL)
                return;

        if (n->type == NODE_FUNCTION)
        {
                if (n->function_n.name)
                        free (n->function_n.name);

                if (n->function_n.return_type != NULL)
                        free (n->function_n.return_type);

                if (n->function_n.body)
                {
                        for (size_t j = 0; j < n->function_n.body->size; ++j)
                        {
                                node_t *body_node = (node_t *)vector_get (
                                    n->function_n.body, j);

                                if (body_node != NULL)
                                {
                                        node_delete (body_node);
                                }
                        }
                        vector_delete (n->function_n.body);
                }

                if (n->function_n.params)
                {
                        for (size_t i = 0; i < n->function_n.params->size; ++i)
                        {
                                param_node_t *param
                                    = (param_node_t *)vector_get (
                                        n->function_n.params, i);
                                node_delete (param);
                        }

                        vector_delete (n->function_n.params);
                }
        }
        else if (n->type == NODE_IMPORT)
        {
                if (n->import_n.path)
                        free (n->import_n.path);
        }
        else if (n->type == NODE_CALL)
        {
                if (n->call_n.name)
                        free (n->call_n.name);

                if (n->call_n.args)
                {
                        for (size_t i = 0; i < n->call_n.args->size; ++i)
                        {
                                arg_node_t *arg = (arg_node_t *)vector_get (
                                    n->call_n.args, i);
                                node_delete (arg);
                        }
                        vector_delete (n->call_n.args);
                }
        }
        else if (n->type == NODE_VARDECL)
        {
                if (n->vardecl_n.name)
                        free (n->vardecl_n.name);
                if (n->vardecl_n.type)
                        free (n->vardecl_n.type);
        }
}

node_t *
node_copy (node_t *n)
{
        if (!n)
                return NULL;

        node_t *new = malloc (sizeof (node_t));
        if (!new)
                return NULL;

        memset (new, 0, sizeof *new);
        new->type = n->type;

        if (n->type == NODE_FUNCTION)
        {
                new->function_n.name
                    = n->function_n.name ? strdup (n->function_n.name) : NULL;

                new->function_n.return_type
                    = n->function_n.return_type
                          ? strdup (n->function_n.return_type)
                          : NULL;

                if (n->function_n.body)
                {
                        new->function_n.body = vector_make (sizeof (node_t));
                        vector_reserve (new->function_n.body,
                                        n->function_n.body->size);
                        for (size_t i = 0; i < n->function_n.body->size; ++i)
                        {
                                node_t *child = (node_t *)vector_get (
                                    n->function_n.body, i);
                                vector_push_back (new->function_n.body,
                                                  node_copy (child));
                        }
                }

                if (n->function_n.params)
                {
                        new->function_n.params
                            = vector_make (sizeof (param_node_t));
                        vector_reserve (new->function_n.params,
                                        n->function_n.params->size);
                        for (size_t i = 0; i < n->function_n.params->size; ++i)
                        {
                                param_node_t *param
                                    = (param_node_t *)vector_get (
                                        n->function_n.params, i);
                                vector_push_back (new->function_n.params,
                                                  node_copy (param));
                        }
                }
        }
        else if (n->type == NODE_CALL)
        {
                new->call_n.name
                    = n->call_n.name ? strdup (n->call_n.name) : NULL;

                if (n->call_n.args)
                {
                        new->call_n.args = vector_make (sizeof (arg_node_t));
                        vector_reserve (new->call_n.args,
                                        n->call_n.args->size);
                        for (size_t i = 0; i < n->call_n.args->size; ++i)
                        {
                                arg_node_t *arg = (arg_node_t *)vector_get (
                                    n->call_n.args, i);

                                arg_node_t *arg_copy = node_copy (arg);
                                vector_push_back (new->call_n.args, arg_copy);
                        }
                }
        }
        else if (n->type == NODE_RETURN)
        {
                new->return_n = n->return_n;
        }
        else if (n->type == NODE_VARDECL)
        {
                new->vardecl_n.name
                    = n->vardecl_n.name ? strdup (n->vardecl_n.name) : NULL;

                new->vardecl_n.type
                    = n->vardecl_n.type ? strdup (n->vardecl_n.type) : NULL;

                new->vardecl_n.value.type = n->vardecl_n.value.type;
                new->vardecl_n.value = n->vardecl_n.value;
        }
        else if (n->type == NODE_IMPORT)
        {
                new->import_n.path = strdup (n->import_n.path);
        }
        else if (n->type == NODE_ARG)
        {
                new->arg_n = n->arg_n;
        }
        else if (n->type == NODE_PARAM)
        {
                new->param_n.kind = n->param_n.kind;
                new->param_n.name
                    = (n->param_n.name) ? strdup (n->param_n.name) : NULL;
        }

        return new;
}

const char *
node_kind_tostr (node_kind_t k)
{
        switch (k)
        {
        case NODE_INVALID:
                return "Invalid";
        case NODE_FUNCTION:
                return "Function";
        case NODE_NATIVE_FUNCTION:
                return "NativeFunction";
        case NODE_CALL:
                return "Call";
        case NODE_RETURN:
                return "Return";
        case NODE_IMPORT:
                return "Import";
        case NODE_ARG:
                return "Arg";
        case NODE_PARAM:
                return "Param";
        case NODE_VARDECL:
                return "VarDecl";
        case NODE_NATIVEDECL:
                return "NativeDecl";
        };
}

const char *
node_value_kind_to_str (node_value_kind_t type)
{
        switch (type)
        {
        case NODE_VALUE_TYPE_INT:
                return "Int";
        case NODE_VALUE_TYPE_UINT:
                return "UInt";
        case NODE_VALUE_TYPE_FLOAT:
                return "Float";
        case NODE_VALUE_TYPE_LONG:
                return "Long";
        case NODE_VALUE_TYPE_STRING:
                return "String";
        case NODE_VALUE_TYPE_BOOL:
                return "Bool";
        case NODE_VALUE_TYPE_VAR:
                return "Var";
        case NODE_VALUE_TYPE_FUNC:
                return "Func";
        case NODE_VALUE_TYPE_FUNC_OR_VAR:
                return "FuncOrVar";
        case NODE_VALUE_TYPE_CALL:
                return "Call";
        default:
                return "Any";
        }
}

node_value_kind_t
str_to_node_value_kind (const char *value)
{
#ifndef ct
#define ct(str) str_equals (value, str)
#endif

        if (ct ("String"))
        {
                return NODE_VALUE_TYPE_STRING;
        }
        else if (ct ("Bool"))
        {
                return NODE_VALUE_TYPE_BOOL;
        }
        else if (ct ("Int"))
        {
                return NODE_VALUE_TYPE_INT;
        }
        else if (ct ("UInt"))
        {
                return NODE_VALUE_TYPE_UINT;
        }
        else if (ct ("Float"))
        {
                return NODE_VALUE_TYPE_FLOAT;
        }
        else if (ct ("Long"))
        {
                return NODE_VALUE_TYPE_LONG;
        }
        else
        {
                return NODE_VALUE_TYPE_ANY;
        }

#ifdef ct
#undef ct
#endif
}

static bool
is_lit (node_value_kind_t kind)
{
        switch (kind)
        {
        case NODE_VALUE_TYPE_BOOL:
        case NODE_VALUE_TYPE_INT:
        case NODE_VALUE_TYPE_UINT:
        case NODE_VALUE_TYPE_FLOAT:
        case NODE_VALUE_TYPE_LONG:
        case NODE_VALUE_TYPE_STRING:
                return true;
        default:
                return false;
        };
}

const char *
value_to_str (value_t value)
{
        safe_value_t v = { .value = value, .type = value.type };
        return safe_to_string (v);
}

safe_value_t
get_safe_value (interpreter_t *inter, arg_node_t *arg)
{
        safe_value_t result = { 0 };

        printd ("node::get_safe_value+3: %s\n",
                node_value_kind_to_str (arg->arg_n.type));
        if (arg->arg_n.type == NODE_VALUE_TYPE_VAR)
        {
                node_t *var = env_getvar (inter->env, arg->arg_n.s);
                if (var)
                {
                        printd (
                            "node::get_safe_value+7: var(%s) = value(%s)\n",
                            var->vardecl_n.name,
                            node_value_kind_to_str (
                                var->vardecl_n.value.type));
                        if (!is_lit (var->vardecl_n.value.type))
                        {
                                arg_node_t arg
                                    = { .arg_n = var->vardecl_n.value };
                                safe_value_t sv = get_safe_value (inter, &arg);
                                result = sv;
                        }
                        else
                        {
                                result.type = var->vardecl_n.value.type;
                                result.value = var->vardecl_n.value;
                        }
                }
        }
        else if (arg->arg_n.type == NODE_VALUE_TYPE_CALL)
        {
                call_node_t *call = arg->arg_n.n;
                if (call)
                {
                        printd ("node::get_safe_value+23: fn(%s)\n",
                                node_kind_tostr (call->type));
                        interpreter_result_t ret
                            = interpreter_run_node (inter, call);
                        result.type = ret.value.type;
                        result.value = ret.value;
                }
        }
        else
        {
                printd ("node::get_safe_value::342: %s\n",
                        node_value_kind_to_str (arg->arg_n.type));
                result.type = arg->arg_n.type;
                result.value = arg->arg_n;
                if (arg->arg_n.type == NODE_VALUE_TYPE_UINT)
                {
                        printd ("node::get_safe_value::348: %u\n",
                                arg->arg_n.u);
                }
        }

        return result;
}

int32_t
safe_to_int (safe_value_t v)
{
        switch (v.type)
        {
        case NODE_VALUE_TYPE_INT:
                return v.value.i;
        case NODE_VALUE_TYPE_LONG:
                return (int32_t)v.value.l;
        case NODE_VALUE_TYPE_FLOAT:
                return (int32_t)v.value.f;
        case NODE_VALUE_TYPE_BOOL:
                return v.value.b ? 1 : 0;
        default:
                return 0;
        }
}

uint32_t
safe_to_uint (safe_value_t v)
{
        switch (v.type)
        {
        case NODE_VALUE_TYPE_INT:
                return (uint32_t)v.value.i;
        case NODE_VALUE_TYPE_UINT:
                return v.value.u;
        case NODE_VALUE_TYPE_LONG:
                return (uint32_t)v.value.l;
        case NODE_VALUE_TYPE_FLOAT:
                return (uint32_t)v.value.f;
        case NODE_VALUE_TYPE_BOOL:
                return v.value.b ? 1 : 0;
        default:
                return 0;
        }
}

float
safe_to_float (safe_value_t v)
{
        switch (v.type)
        {
        case NODE_VALUE_TYPE_FLOAT:
                return v.value.f;
        case NODE_VALUE_TYPE_INT:
                return (float)v.value.i;
        case NODE_VALUE_TYPE_LONG:
                return (float)v.value.l;
        case NODE_VALUE_TYPE_BOOL:
                return v.value.b ? 1.0f : 0.0f;
        default:
                return 0.0f;
        }
}

char *
safe_to_string (safe_value_t v)
{
        switch (v.type)
        {
        case NODE_VALUE_TYPE_STRING:
                return v.value.s;
        case NODE_VALUE_TYPE_INT:
        {
                static char buf[32];
                snprintf (buf, sizeof (buf), "%d", v.value.i);
                return buf;
        }
        case NODE_VALUE_TYPE_UINT:
        {
                static char buf[32];
                snprintf (buf, sizeof (buf), "%d", v.value.u);
                return buf;
        }
        case NODE_VALUE_TYPE_FLOAT:
        {
                static char buf[32];
                snprintf (buf, sizeof (buf), "%f", v.value.f);
                return buf;
        }
        case NODE_VALUE_TYPE_BOOL:
                return v.value.b ? "true" : "false";
        default:
        {
                static char buf[2];
                snprintf (buf, sizeof (buf), "%d", v.type);
                buf[1] = '\0';
                return buf;
        }
        };
}
