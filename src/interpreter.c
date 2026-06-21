#include "kilate/interpreter.h"

#include <alloca.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "kilate/debug.h"
#include "kilate/environment.h"
#include "kilate/error.h"
#include "kilate/hashmap.h"
#include "kilate/native.h"
#include "kilate/node.h"
#include "kilate/string.h"
#include "kilate/vector.h"

static const char *
irk_to_str (interpreter_result_kind_t irk)
{
        switch (irk)
        {
        case IRT_FUNC:
                return "irk::func";
        case IRT_RETURN:
                return "irk::return";
        }
        return "irk::<?>";
}

static node_t
interpret_variable_value (interpreter_t *self, vardecl_node_t *var)
{
        printd ("interpreter::interpret_variable_value+1: var(%s, %s) = "
                "value(%s)\n",
                var->vardecl_n.name, var->vardecl_n.type,
                node_value_kind_to_str (var->vardecl_n.value.type));

        arg_node_t arg;
        arg.arg_n = var->vardecl_n.value;
        safe_value_t sv = get_safe_value (self, &arg);

        printd ("interpreter::interpret_variable_value+10: sv(%s) = "
                "value(%s).u(%d)\n",
                node_value_kind_to_str (sv.type),
                node_value_kind_to_str (sv.value.type), sv.value.u);

        vardecl_node_t nvar = make_node (NODE_VARDECL);
        nvar.vardecl_n.name = strdup (var->vardecl_n.name);
        nvar.vardecl_n.type = strdup (var->vardecl_n.type);
        nvar.vardecl_n.value = sv.value;

        printd ("interpreter::interpret_variable_value+20: "
                "stored_value(%s).u(%d)\n",
                node_value_kind_to_str (nvar.vardecl_n.value.type),
                nvar.vardecl_n.value.u);

        // node_delete (var);
        return nvar;
}

static value_t
interpret_return_value (interpreter_t *self, return_node_t *ret)
{
        printd (
            "interpreter::interpret_return_value+1: return() = value(%s)\n",
            node_value_kind_to_str (ret->return_n.type));

        arg_node_t arg;
        arg.arg_n = ret->return_n;
        safe_value_t sv = get_safe_value (self, &arg);
        return sv.value;
}

static interpreter_result_t
_interpreter_run_fn (interpreter_t *self, node_t *func,
                     node_arg_vector_t *params)
{
        if (self == NULL)
        {
                error_fatal ("error[?]: Interpreter is invalid.");
        }

        if (func == NULL || func->type != NODE_FUNCTION)
        {
                error_fatal (
                    "error[%s]: Function Node Not is a Valid Function.",
                    self->filename);
        }

        printd ("interpreter::interpreter_run_fn+8: %s\n",
                (func->function_n.native) ? "true" : "false");

        if (!func->function_n.body)
        {
                error_fatal ("error[%s]: Function body is not Valid.",
                             self->filename);
        }

        env_t *old = self->env;
        self->env = env_make (NULL);

        printd ("func=%p\n", func);
        printd ("func->name=%s\n", func->function_n.name);
        printd ("func->params=%p\n", func->function_n.params);
        if (func->function_n.params)
                printd ("func->params->size=%zu\n",
                        func->function_n.params->size);

        printd ("params=%p\n", params);
        if (params)
                printd ("params->size=%zu\n", params->size);

        if (params != NULL && func->function_n.params != NULL)
        {
                for (size_t i = 0; i < params->size; i++)
                {
                        arg_node_t *param
                            = (arg_node_t *)vector_get (params, i);
                        param_node_t *fnParam = (param_node_t *)vector_get (
                            func->function_n.params, i);

                        safe_value_t svalue = get_safe_value (self, param);

                        if (fnParam->arg_n.type != NODE_VALUE_TYPE_ANY
                            && fnParam->arg_n.type != svalue.type)
                        {
                                error_fatal (
                                    "error[%s]: Argument %zu to function '%s' "
                                    "expected "
                                    "type '%s', but got '%s'",
                                    self->filename, i + 1,
                                    func->function_n.name,
                                    node_value_kind_to_str (
                                        fnParam->arg_n.type),
                                    node_value_kind_to_str (svalue.type));
                        }

                        vardecl_node_t var = make_node (NODE_VARDECL);
                        var.vardecl_n.name = strdup (fnParam->arg_n.s);
                        var.vardecl_n.type = strdup (
                            node_value_kind_to_str (fnParam->arg_n.type));
                        var.vardecl_n.value = svalue.value;
                        env_definevar (self->env, var.vardecl_n.name, &var);
                }
        }

        printd ("interpreter::interpreter_run_fn+47: %s\n",
                func->function_n.name);
        for (size_t i = 0; i < func->function_n.body->size; i++)
        {
                node_t *stmt = (node_t *)vector_get (func->function_n.body, i);
                if (stmt != NULL)
                {
                        interpreter_result_t result
                            = interpreter_run_node (self, stmt);
                        if (result.type == IRT_RETURN)
                        {
                                env_t *to_destroy = self->env;
                                self->env = old;
                                env_destroy (to_destroy);
                                return result;
                        }
                }
        }

        env_t *to_destroy = self->env;
        self->env = old;
        env_destroy (to_destroy);

        // default value
        return (interpreter_result_t){ .type = IRT_FUNC, .value.type = -1 };
}

static void
valid_native_decl (interpreter_t *self, nativedecl_node_t *decl,
                   node_vector_t *native_functions_nodes_vector)
{
        bool found = false;

        struct __function_node d = decl->nativedecl_n;

        for (size_t i = 0; i < native_functions_nodes_vector->size; ++i)
        {
                native_function_node_t *fn
                    = (native_function_node_t *)vector_get (
                        native_functions_nodes_vector, i);

                struct __function_node f = fn->function_n;

                if (str_equals (f.name, d.name))
                {
                        found = true;
                        hm_put (self->decls, d.name, decl);
                        break;
                }
        }

        if (!found)
        {
                error_fatal (
                    "error[%s]: native function '%s' declared but not found",
                    self->filename, d.name);
        }
}

static void
valid_native_decls (interpreter_t *self, node_vector_t *nodes,
                    node_vector_t *native_functions_nodes_vector)
{
        for (size_t i = 0; i < nodes->size; ++i)
        {
                node_t *node = (node_t *)vector_get (nodes, i);
                if (!node)
                        continue;

                if (node->type == NODE_NATIVEDECL)
                {
                        valid_native_decl (self, node,
                                           native_functions_nodes_vector);
                }
        }
}

interpreter_t *
interpreter_make (const char *filename, node_vector_t *nodes_vector,
                  node_vector_t *native_functions_nodes_vector)
{
        if (nodes_vector == NULL)
                error_fatal ("error[%s]: Node's Vector is invalid.", filename);
        if (native_functions_nodes_vector == NULL)
                error_fatal (
                    "error[%s]: Native Functions Node's Vector is invalid.",
                    filename);

        interpreter_t *interpreter = malloc (sizeof (interpreter_t));
        interpreter->functions = hm_make (sizeof (node_t));
        interpreter->decls = hm_make (sizeof (node_t));
        interpreter->filename = filename;

        // register all funcs
        for (size_t i = 0; i < nodes_vector->size; i++)
        {
                node_t *node = (node_t *)vector_get (nodes_vector, i);
                if (node != NULL)
                {
                        if (node->type == NODE_FUNCTION)
                        {
                                hm_put (interpreter->functions,
                                        node->function_n.name, node);
                        }
                }
        }

        for (size_t i = 0; i < native_functions_nodes_vector->size; i++)
        {
                native_function_node_t *entry
                    = (native_function_node_t *)vector_get (
                        native_functions_nodes_vector, i);
                if (entry != NULL)
                {
                        entry->function_n.native = true;
                        hm_put (interpreter->functions, entry->function_n.name,
                                entry);
                }
        }

        valid_native_decls (interpreter, nodes_vector,
                            native_functions_nodes_vector);

        interpreter->env = env_make (NULL);

        return interpreter;
}

void
interpreter_delete (interpreter_t *self)
{
        if (self == NULL)
                return;
        hm_delete (self->functions);
        env_destroy (self->env);
        free (self);
}

#define MAIN_FUNCTION_NAME "Main"
#define MAIN_FUNCTION_RETURN "Int"
interpreter_result_t
interpreter_run (interpreter_t *self)
{
        if (self == NULL)
                error_fatal ("error[?]: Interpreter is invalid.");

        node_t *main = (node_t *)hm_get (self->functions, MAIN_FUNCTION_NAME);
        if (main == NULL)
        {
                error_fatal (
                    "error[%s]: Your program needs a " MAIN_FUNCTION_NAME
                    " function!",
                    self->filename);
        }

        if (main->function_n.return_type == NULL
            || !str_equals (main->function_n.return_type,
                            MAIN_FUNCTION_RETURN))
        {
                error_fatal ("error[%s]: " MAIN_FUNCTION_NAME
                             " function should return " MAIN_FUNCTION_RETURN,
                             self->filename);
        }

        node_arg_vector_t *main_args
            = vector_make (sizeof (node_arg_vector_t));

        vector_push_back (
            main_args,
            &(node_t){
                .arg_n = (value_t){ .i = 0, .type = NODE_VALUE_TYPE_INT },
            });

        vector_push_back (
            main_args,
            &(node_t){
                .arg_n = (value_t){ .s = strdup ("[\"Argv should be here\"]"),
                                    .type = NODE_VALUE_TYPE_STRING },
            });

        return interpreter_run_fn (self, main, main_args);
}

interpreter_result_t
interpreter_run_fn (interpreter_t *self, node_t *fn, node_arg_vector_t *args)
{
        if (!fn
            || (fn->type != NODE_FUNCTION && fn->type != NODE_NATIVE_FUNCTION))
                goto def;

        if (fn->type == NODE_FUNCTION && !fn->function_n.native)
        {
                return _interpreter_run_fn (self, fn, args);
        }

        if (fn->type == NODE_NATIVE_FUNCTION && fn->function_n.native)
        {
                native_fndata_t *ndata = malloc (sizeof *ndata);
                ndata->args = args;
                ndata->inter = self;

                native_fn_t n = fn->function_n.native_fn;
                return_node_t nreturn = n (ndata);
                return (interpreter_result_t){ .type = IRT_RETURN,
                                               .value = nreturn.return_n };
        }
def:
        return (interpreter_result_t){ 0 };
}

interpreter_result_t
interpreter_run_node (interpreter_t *self, node_t *n)
{
        if (self == NULL)
                error_fatal ("error[?]: Interpreter is invalid.");
        if (n == NULL)
                error_fatal ("error[%s]: Node is invalid.", self->filename);

        switch (n->type)
        {
        case NODE_CALL:
        {
                function_node_t *fn
                    = (node_t *)hm_get (self->functions, n->call_n.name);

                if (!fn
                    || (fn->type == NODE_NATIVE_FUNCTION
                        && !(hm_get (self->decls, n->call_n.name))))
                {
                        error_fatal ("error[%s]: Function not declared or not "
                                     "exists: %s",
                                     self->filename, n->call_n.name);
                }

                printd (
                    "interpreter::interpreter_run_node+16: exec_fnlow(%s)\n",
                    fn->function_n.name);
                interpreter_result_t r
                    = interpreter_run_fn (self, fn, n->call_n.args);

                printd (
                    "interpreter::interpreter_run_node+21: call_result(%s, "
                    "%s) "
                    "= value(%s, %s)\n",
                    fn->function_n.name, irk_to_str (r.type),
                    node_value_kind_to_str (r.value.type),
                    value_to_str (r.value));

                return (interpreter_result_t){ .type = IRT_FUNC,
                                               .value = r.value };
        }

        case NODE_RETURN:
        {

                return (interpreter_result_t){
                        .type = IRT_RETURN,
                        .value = interpret_return_value (self, n)
                };
        }

        case NODE_VARDECL:
        {
                vardecl_node_t var = interpret_variable_value (self, n);
                env_definevar (self->env, n->vardecl_n.name, node_copy (&var));
                return (interpreter_result_t){ .type = IRT_FUNC,
                                               .value.type = -1 };
        }

        default:
                error_fatal ("error[%s]: Unknown node type %d", self->filename,
                             n->type);
        }
        return (interpreter_result_t){ .type = IRT_FUNC, .value.type = -1 };
}
