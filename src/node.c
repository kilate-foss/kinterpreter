#include "kilate/node.h"

#include <stdlib.h>
#include <string.h>

#include "kilate/debug.h"
#include "kilate/environment.h"
#include "kilate/interpreter.h"
#include "kilate/vector.h"

void node_delete(node_t *n)
{
        if (n == NULL)
                return;

        if (n->type == NODE_FUNCTION) {
                free(n->function_n.name);

                if (n->function_n.return_type != NULL)
                        free(n->function_n.return_type);

                for (size_t j = 0; j < n->function_n.body->size; ++j) {
                        node_t **body_nodePtr =
                            (node_t **)vector_get(n->function_n.body, j);

                        if (body_nodePtr != NULL) {
                                node_t *body_node = *body_nodePtr;
                                node_delete(body_node);
                        }
                }

                vector_delete(n->function_n.body);

                for (size_t i = 0; i < n->function_n.params->size; ++i) {
                        param_node_t *param = *(param_node_t **)vector_get(
                            n->function_n.params, i);
                        node_delete(param);
                }

                vector_delete(n->function_n.params);
        } else if (n->type == NODE_IMPORT) {
                free(n->import_n.path);
        } else if (n->type == NODE_CALL) {
                free(n->call_n.name);

                for (size_t i = 0; i < n->call_n.args->size; ++i) {
                        arg_node_t *arg = *(arg_node_t **)vector_get(
                            n->call_n.args, i);
                        node_delete(arg);
                }

                vector_delete(n->call_n.args);
        } else if (n->type == NODE_VARDEC) {
                free(n->vardec_n.name);
                free(n->vardec_n.type);
        }

        free(n);
}

const char *node_kind_tostr(node_kind_t k)
{
        switch (k) {
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
        case NODE_VARDEC:
                return "VarDecl";
        };
}

node_t *node_copy(node_t *n)
{
        if (!n)
                return NULL;

        node_t *new = malloc(sizeof(node_t));
        if (!new)
                return NULL;

        new->type = n->type;

        if (n->type == NODE_FUNCTION) {
                new->function_n.name = n->function_n.name ?
                                              strdup(n->function_n.name) :
                                              NULL;

                new->function_n.return_type =
                    n->function_n.return_type ?
                        strdup(n->function_n.return_type) :
                        NULL;

                new->function_n.body = vector_make(sizeof(node_t *));
                for (size_t i = 0; i < n->function_n.body->size; ++i) {
                        node_t *child =
                            *(node_t **)vector_get(n->function_n.body, i);

                        node_t *copy = node_copy(child);

                        vector_push_back(new->function_n.body, &copy);
                }

                new->function_n.params =
                    vector_make(sizeof(param_node_t *));
                for (size_t i = 0; i < n->function_n.params->size; ++i) {

                        param_node_t *param = *(param_node_t **)vector_get(
                            n->function_n.params, i);

                        param_node_t *param_copy =
                            (param_node_t *)node_copy((node_t *)param);

                        vector_push_back(new->function_n.params,
                                         &param_copy);
                }
        } else if (n->type == NODE_CALL) {
                new->call_n.name = n->call_n.name ?
                                               strdup(n->call_n.name) :
                                               NULL;

                new->call_n.args = vector_make(sizeof(arg_node_t *));
                for (size_t i = 0; i < n->call_n.args->size; ++i) {

                        arg_node_t *arg = *(arg_node_t **)vector_get(
                            n->call_n.args, i);

                        arg_node_t *arg_copy =
                            (arg_node_t *)node_copy((node_t *)arg);

                        vector_push_back(new->call_n.args,
                                         &arg_copy);
                }
        } else if (n->type == NODE_RETURN) {
                new->return_n = n->return_n;
        } else if (n->type == NODE_VARDEC) {
                new->vardec_n.name =
                    n->vardec_n.name ? strdup(n->vardec_n.name) : NULL;

                new->vardec_n.type =
                    n->vardec_n.type ? strdup(n->vardec_n.type) : NULL;

                new->vardec_n.value.type = n->vardec_n.value.type;
                new->vardec_n.value = n->vardec_n.value;
        } else if (n->type == NODE_IMPORT) {
                new->import_n.path = strdup(n->import_n.path);
        }

        return new;
}

call_node_t *call_node_make(const char *name,
                            node_arg_vector_t *args)
{
        node_t *n = alloc_node(NODE_CALL);
        n->call_n.name = strdup(name);
        n->call_n.args = args;
        return n;
}

vardec_node_t *var_dec_node_make(const char *name, const char *type, value_t v)
{
        node_t *n = alloc_node(NODE_VARDEC);
        n->vardec_n.name = strdup(name);
        n->vardec_n.type = strdup(type);
        n->vardec_n.value = v;
        return n;
}

import_node_t *import_node_make(const char *path)
{
        node_t *n = alloc_node(NODE_IMPORT);
        n->import_n.path = strdup(path);
        return n;
}

safe_value_t get_safe_value(interpreter_t *inter, arg_node_t *arg)
{
        safe_value_t result = { 0 };

        printd("213: %d\n", arg->arg_n.type);
        if (arg->arg_n.type == NODE_VALUE_TYPE_VAR) {
                node_t *var = env_getvar(inter->env, arg->arg_n.s);
                if (var) {
                        result.type = var->vardec_n.value.type;
                        result.value = var->vardec_n.value;
                }
        } else if (arg->arg_n.type == NODE_VALUE_TYPE_CALL) {
                node_t *fn = arg->arg_n.n;
                if (fn) {
                        interpreter_result_t ret = interpreter_run_fnlow(
                            inter, fn, arg->call_n.args);
                        result.type = ret.value.type;
                        result.value = ret.value;
                }
        } else {
                result.type = arg->arg_n.type;
                result.value = arg->arg_n;
        }

        return result;
}

int safe_to_int(safe_value_t v)
{
        switch (v.type) {
        case NODE_VALUE_TYPE_INT:
                return v.value.i;
        case NODE_VALUE_TYPE_LONG:
                return (int)v.value.l;
        case NODE_VALUE_TYPE_FLOAT:
                return (int)v.value.f;
        case NODE_VALUE_TYPE_BOOL:
                return v.value.b ? 1 : 0;
        default:
                return 0;
        }
}

float safe_to_float(safe_value_t v)
{
        switch (v.type) {
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

char *safe_to_string(safe_value_t v)
{
        switch (v.type) {
        case NODE_VALUE_TYPE_STRING:
                return v.value.s;
        case NODE_VALUE_TYPE_INT: {
                static char buf[32];
                snprintf(buf, sizeof(buf), "%d", v.value.i);
                return buf;
        }
        case NODE_VALUE_TYPE_FLOAT: {
                static char buf[32];
                snprintf(buf, sizeof(buf), "%f", v.value.f);
                return buf;
        }
        case NODE_VALUE_TYPE_BOOL:
                return v.value.b ? "true" : "false";
        default: {
                static char buf[2];
                snprintf(buf, sizeof(buf), "%d", v.type);
                buf[1] = '\0';
                return buf;
        }
        };
}
