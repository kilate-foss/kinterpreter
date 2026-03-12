#include "kilate/interpreter.h"

#include <alloca.h>
#include <stdio.h>
#include <string.h>

#include "kilate/debug.h"
#include "kilate/environment.h"
#include "kilate/error.h"
#include "kilate/hashmap.h"
#include "kilate/native.h"
#include "kilate/node.h"
#include "kilate/parser.h"

interpreter_t *interpreter_make(node_vector_t *nodes_vector,
                                node_vector_t *native_functions_nodes_vector)
{
        if (nodes_vector == NULL)
                error_fatal("Node's Vector is invalid.");
        if (native_functions_nodes_vector == NULL)
                error_fatal("Native Functions Node's Vector is invalid.");

        interpreter_t *interpreter = malloc(sizeof(interpreter_t));
        interpreter->functions = hash_map_make(sizeof(node_t *));

        // register all funcs
        for (size_t i = 0; i < nodes_vector->size; i++) {
                node_t **nodePtr = (node_t **)vector_get(nodes_vector, i);
                if (nodePtr != NULL) {
                        function_node_t *node = *nodePtr;
                        if (node->type == NODE_FUNCTION) {
                                hash_map_put(interpreter->functions,
                                             node->function_n.name, nodePtr);
                        }
                }
        }

        for (size_t i = 0; i < native_functions_nodes_vector->size; i++) {
                native_function_node_t **entryPtr =
                    (native_function_node_t **)vector_get(
                        native_functions_nodes_vector, i);
                if (entryPtr != NULL) {
                        function_node_t *entry = *entryPtr;
                        entry->function_n.native = true;
                        hash_map_put(interpreter->functions,
                                     entry->function_n.name, entryPtr);
                }
        }

        interpreter->env = env_make(NULL);

        return interpreter;
}

void interpreter_delete(interpreter_t *self)
{
        if (self == NULL)
                return;
        hash_map_delete(self->functions);
        env_destroy(self->env);
        free(self);
}

#define MAIN_FUNCTION_NAME "Main"
#define MAIN_FUNCTION_RETURN "Int"
interpreter_result_t interpreter_run(interpreter_t *self)
{
        if (self == NULL)
                error_fatal("Interpreter is invalid.");

        node_t **mainPtr =
            (node_t **)hash_map_get(self->functions, MAIN_FUNCTION_NAME);
        if (mainPtr == NULL) {
                error_fatal("Your program needs a " MAIN_FUNCTION_NAME
                            " function!");
        }
        node_t *main = *mainPtr;

        if (main->function_n.return_type == NULL ||
            !str_equals(main->function_n.return_type, MAIN_FUNCTION_RETURN)) {
                error_fatal(MAIN_FUNCTION_NAME
                            " function should return bool.");
        }

        return interpreter_run_fn(self, main, NULL);
}

interpreter_result_t interpreter_run_fnlow(interpreter_t *self, node_t *fn,
                                           node_arg_vector_t *args)
{
        if (!fn ||
            (fn->type != NODE_FUNCTION && fn->type != NODE_NATIVE_FUNCTION))
                goto def;

        if (fn->type == NODE_FUNCTION) {
                return interpreter_run_fn(self, fn, args);
        }

        if (fn->type == NODE_NATIVE_FUNCTION) {
                native_fndata_t *ndata = malloc(sizeof(*ndata));
                ndata->args = args;
                ndata->inter = self;

                native_fn_t n = fn->function_n.native_fn;
                return_node_t *nreturn = n(ndata);
                return (interpreter_result_t){ .type = IRT_RETURN,
                                               .value = nreturn->return_n };
        }
def:
        return (interpreter_result_t){ 0 };
}

interpreter_result_t interpreter_run_fn(interpreter_t *self, node_t *func,
                                        node_arg_vector_t *params)
{
        if (self == NULL)
                error_fatal("Interpreter is invalid.");

        if (func == NULL || func->type != NODE_FUNCTION) {
                error_fatal("Function Node Not is a Valid Function.");
        }

        printd("125: %s\n", (func->function_n.native) ? "true" : "false");

        if (!func->function_n.body) {
                error_fatal("Function body is not Valid.");
        }

        env_t *old = self->env;
        self->env = env_make(NULL);

        if (params != NULL && func->function_n.params != NULL) {
                for (size_t i = 0; i < params->size; i++) {
                        arg_node_t *param =
                            *(arg_node_t **)vector_get(params, i);
                        param_node_t *fnParam = *(param_node_t **)vector_get(
                            func->function_n.params, i);

                        safe_value_t svalue = get_safe_value(self, param);

                        if (fnParam->arg_n.type != NODE_VALUE_TYPE_ANY &&
                            fnParam->arg_n.type != svalue.type) {
                                error_fatal(
                                    "Argument %zu to function '%s' expected "
                                    "type '%s', but got '%s'",
                                    i + 1, func->function_n.name,
                                    parser_nodevaluetype_to_str(
                                        fnParam->arg_n.type),
                                    parser_nodevaluetype_to_str(svalue.type));
                        }

                        node_t *var = var_dec_node_make(
                            fnParam->arg_n.s,
                            parser_nodevaluetype_to_str(fnParam->arg_n.type),
                            svalue.value);
                        node_t *var_copy = node_copy(var);
                        env_definevar(self->env, var_copy->vardec_n.name,
                                      var_copy);
                }
        }

        printd("158: %s\n", func->function_n.name);
        for (size_t i = 0; i < func->function_n.body->size; i++) {
                node_t **stmtPtr =
                    (node_t **)vector_get(func->function_n.body, i);
                if (stmtPtr != NULL) {
                        node_t *stmt = *stmtPtr;
                        interpreter_result_t result =
                            interpreter_run_node(self, stmt);
                        if (result.type == IRT_RETURN) {
                                env_t *to_destroy = self->env;
                                self->env = old;
                                env_destroy(to_destroy);
                                return result;
                        }
                }
        }

        env_t *to_destroy = self->env;
        self->env = old;
        env_destroy(to_destroy);

        // default value
        return (interpreter_result_t){ .type = IRT_FUNC, .value.type = -1 };
}

interpreter_result_t interpreter_run_node(interpreter_t *self, node_t *n)
{
        if (self == NULL)
                error_fatal("Interpreter is invalid.");
        if (n == NULL)
                error_fatal("Node is invalid.");

        switch (n->type) {
        case NODE_CALL: {
                function_node_t **fnptr =
                    (node_t **)hash_map_get(self->functions, n->call_n.name);

                if (!fnptr) {
                        error_fatal("Function not found: %s", n->call_n.name);
                }

                function_node_t *fn = *fnptr;
                return interpreter_run_fnlow(self, fn, n->call_n.args);
        }

        case NODE_RETURN: {
                return (interpreter_result_t){ .type = IRT_RETURN,
                                               .value = n->return_n };
        }

        case NODE_VARDEC: {
                env_definevar(self->env, n->vardec_n.name, node_copy(n));
                return (interpreter_result_t){ .type = IRT_FUNC,
                                               .value.type = -1 };
        }

        default:
                error_fatal("Unknown node type %d", n->type);
        }
        return (interpreter_result_t){ .type = IRT_FUNC, .value.type = -1 };
}
