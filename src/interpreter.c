#include "kilate/interpreter.h"

#include <stdio.h>
#include <string.h>

#include "kilate/environment.h"
#include "kilate/error.h"
#include "kilate/file.h"
#include "kilate/hashmap.h"
#include "kilate/node.h"
#include "kilate/parser.h"

interpreter_t* interpreter_make(
    node_vector_t* nodes_vector,
    node_vector_t* native_functions_nodes_vector) {
  if (nodes_vector == NULL)
    error_fatal("Node's Vector is invalid.");
  if (native_functions_nodes_vector == NULL)
    error_fatal("Native Functions Node's Vector is invalid.");

  interpreter_t* interpreter = malloc(sizeof(interpreter_t));
  interpreter->functions = hash_map_make(sizeof(node_t*));
  interpreter->native_functions = hash_map_make(sizeof(native_fn_t));

  // register all funcs
  for (size_t i = 0; i < nodes_vector->size; i++) {
    node_t** nodePtr = (node_t**)vector_get(nodes_vector, i);
    if (nodePtr != NULL) {
      node_t* node = *nodePtr;
      if (node->type == NODE_FUNCTION) {
        hash_map_put(interpreter->functions, node->function_n.fn_name,
                         nodePtr);
      }
    }
  }

  for (size_t i = 0; i < native_functions_nodes_vector->size; i++) {
    native_fnentry_t** entryPtr =
        (native_fnentry_t**)vector_get(native_functions_nodes_vector, i);
    if (entryPtr != NULL) {
      native_fnentry_t* entry = *entryPtr;
      hash_map_put(interpreter->native_functions, entry->name, entryPtr);
    }
  }

  interpreter->env = env_make(NULL);

  return interpreter;
}

void interpreter_delete(interpreter_t* self) {
  if (self == NULL)
    return;
  hash_map_delete(self->functions);
  hash_map_delete(self->native_functions);
  env_destroy(self->env);
  free(self);
}

interpreter_result_t interpreter_run(interpreter_t* self) {
  if (self == NULL)
    error_fatal("Interpreter is invalid.");

  node_t** mainPtr = (node_t**)hash_map_get(self->functions, "main");
  if (mainPtr == NULL) {
    error_fatal("Your program needs a main function!");
  }
  node_t* main = *mainPtr;

  if (main->function_n.fn_return_type == NULL ||
      !str_equals(main->function_n.fn_return_type, "bool")) {
    error_fatal("Main function should return bool.");
  }

  return interpreter_run_fn(self, main, NULL);
}

interpreter_result_t interpreter_run_fn(interpreter_t* self,
                                              node_t* func,
                                              node_fnparam_vector_t* params) {
  if (self == NULL)
    error_fatal("Interpreter is invalid.");

  if (func == NULL || func->type != NODE_FUNCTION) {
    error_fatal("Functin Node Not is a Valid Function");
  }

  env_t* old = self->env;
  self->env = env_make(NULL);

  if (params != NULL && func->function_n.fn_params != NULL) {
    for (size_t i = 0; i < params->size; i++) {
      node_fnparam_t* param = *(node_fnparam_t**)vector_get(params, i);
      node_fnparam_t* fnParam =
          *(node_fnparam_t**)vector_get(func->function_n.fn_params, i);

      node_value_kind_t actualType = param->type;
      void* actualValue = param->value;

      if (param->type == NODE_VALUE_TYPE_VAR) {
        node_t* real_var = env_getvar(old, (char *)param->value);
        if (real_var == NULL) {
          error_fatal("Variable not defined: %s", (char *)param->value);
        }
        actualType =
            parser_str_to_nodevaluetype(real_var->vardec_n.var_type);
        actualValue = real_var->vardec_n.var_value;
      }

      if (fnParam->type != NODE_VALUE_TYPE_ANY && fnParam->type != actualType) {
        error_fatal(
            "Argument %zu to function '%s' expected type '%s', but got '%s'",
            i + 1, func->function_n.fn_name,
            parser_nodevaluetype_to_str(fnParam->type),
            parser_nodevaluetype_to_str(actualType));
      }

      node_t* var = var_dec_node_make(
          fnParam->value, parser_nodevaluetype_to_str(fnParam->type),
          actualType, actualValue);
      node_t* var_copy = node_copy(var);
      env_definevar(self->env, var_copy->vardec_n.var_name,
                                var_copy);
    }
  }

  for (size_t i = 0; i < func->function_n.fn_body->size; i++) {
    node_t** stmtPtr =
        (node_t**)vector_get(func->function_n.fn_body, i);
    if (stmtPtr != NULL) {
      node_t* stmt = *stmtPtr;
      interpreter_result_t result = interpreter_run_node(self, stmt);
      if (result.type == IRT_RETURN) {
        env_t* to_destroy = self->env;
        self->env = old;
        env_destroy(to_destroy);
        return result;
      }
    }
  }

  env_t* to_destroy = self->env;
  self->env = old;
  env_destroy(to_destroy);

  // default value
  return (interpreter_result_t){.type = IRT_FUNC, .data = NULL};
}

interpreter_result_t interpreter_run_node(interpreter_t* self,
                                                node_t* n) {
  if (self == NULL)
    error_fatal("Interpreter is invalid.");
  if (n == NULL)
    error_fatal("Node is invalid.");

  switch (n->type) {
    case NODE_CALL: {
      node_t** calledPtr = (node_t**)hash_map_get(
          self->functions, n->call_n.fn_call_name);
      native_fnentry_t** nativeFnEntryPtr =
          (native_fnentry_t**)hash_map_get(self->native_functions,
                                                 n->call_n.fn_call_name);

      // if ptr not null, so its a fn
      if (calledPtr != NULL) {
        node_t* called = *calledPtr;
        interpreter_result_t result =
            interpreter_run_fn(self, called, n->call_n.fn_call_params);
        return result;
      } else if (nativeFnEntryPtr != NULL) {
        // else if native ptr is not null, so its native fn
        native_fndata_t* nativeFnData = malloc(sizeof(native_fndata_t));
        nativeFnData->params = n->call_n.fn_call_params;
        nativeFnData->env = self->env;

        native_fnentry_t* nativeFnEntry = *nativeFnEntryPtr;
        native_fn_t nativeFn = *nativeFnEntry->fn;
        node_t* nativeFnResult = nativeFn(nativeFnData);
        interpreter_result_t result =
            (interpreter_result_t){.data = nativeFnResult, .type = IRT_FUNC};
        return result;
      } else {
        // else not found
        error_fatal("Function not found: %s", n->call_n.fn_call_name);
      }
    }

    case NODE_RETURN: {
      void* value = NULL;
      if (n->return_n.return_value != NULL) {
        value = n->return_n.return_value;  // or evaluate this node if needed
      }
      return (interpreter_result_t){.type = IRT_RETURN, .data = value};
    }

    case NODE_VARDEC: {
      env_definevar(self->env, n->vardec_n.var_name,
                                node_copy(n));
      return (interpreter_result_t){.type = IRT_FUNC, .data = NULL};
    }

    default:
      error_fatal("Unknown node type %d", n->type);
  }
  return (interpreter_result_t){.type = IRT_FUNC, .data = NULL};
}
