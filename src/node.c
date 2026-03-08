#include "kilate/node.h"

#include <stdlib.h>
#include <string.h>

#include "kilate/string.h"
#include "kilate/vector.h"

void node_delete(node_t* n) {
  if (n == NULL)
    return;
  if (n->type == NODE_FUNCTION) {
    free(n->function_n.fn_name);
    if (n->function_n.fn_return_type != NULL) {
      free(n->function_n.fn_return_type);
    }
    // free body nodes
    for (size_t j = 0; j < n->function_n.fn_body->size; ++j) {
      node_t** body_nodePtr =
          (node_t**)vector_get(n->function_n.fn_body, j);
      if (body_nodePtr != NULL) {
        node_t* body_node = *body_nodePtr;
        node_delete(body_node);
      }
    }
    vector_delete(n->function_n.fn_body);
    // free param nodes
    for (size_t j = 0; j < n->function_n.fn_params->size; ++j) {
      node_fnparam_t* param =
          *(node_fnparam_t**)vector_get(n->function_n.fn_params, j);
      free(param->value);
      free(param);
    }
    vector_delete(n->function_n.fn_params);
  } else if (n->type == NODE_IMPORT) {
    free(n->import_n.import_path);
  } else if (n->type == NODE_CALL) {
    free(n->call_n.fn_call_name);
    node_delete_params(n->call_n.fn_call_params);
  } else if (n->type == NODE_VARDEC) {
    free(n->vardec_n.var_name);
    free(n->vardec_n.var_type);
  }
  free(n);
}

node_t *node_copy(node_t *n) {
  if (!n)
    return NULL;

  node_t* new = malloc(sizeof(node_t));
  if (!new)
    return NULL;

  new->type = n->type;

  if (n->type == NODE_FUNCTION) {
    new->function_n.fn_name = NULL;
    new->function_n.fn_return_type = NULL;
    if (n->function_n.fn_name != NULL) {
      new->function_n.fn_name = strdup(n->function_n.fn_name);
    }
    if (n->function_n.fn_return_type != NULL) {
      new->function_n.fn_return_type = strdup(n->function_n.fn_return_type);
    }

    new->function_n.fn_body = vector_make(sizeof(node_t*));
    for (size_t i = 0; i < n->function_n.fn_body->size; ++i) {
      node_t* child_node =
          *(node_t**)vector_get(n->function_n.fn_body, i);
      node_t* child_copy = node_copy(child_node);
      vector_push_back(new->function_n.fn_body, &child_copy);
    }

    new->function_n.fn_params = vector_make(sizeof(node_fnparam_t*));
    for (size_t i = 0; i < n->function_n.fn_params->size; ++i) {
      node_fnparam_t* param =
          *(node_fnparam_t**)vector_get(n->function_n.fn_params, i);
      node_fnparam_t* param_copy = node_fnparam_copy(param);
      vector_push_back(new->function_n.fn_params, &param_copy);
    }
  } else if (n->type == NODE_CALL) {
    new->call_n.fn_call_name = NULL;
    if (n->call_n.fn_call_name != NULL) {
      new->call_n.fn_call_name = strdup(n->call_n.fn_call_name);
    }
    new->call_n.fn_call_params = vector_make(sizeof(node_fnparam_t*));
    for (size_t i = 0; i < n->call_n.fn_call_params->size; ++i) {
      node_fnparam_t* param =
          *(node_fnparam_t**)vector_get(n->call_n.fn_call_params, i);
      node_fnparam_t* param_copy = node_fnparam_copy(param);
      vector_push_back(new->call_n.fn_call_params, &param_copy);
    }
  } else if (n->type == NODE_RETURN) {
    new->return_n.return_type = n->return_n.return_type;
    new->return_n.return_value = n->return_n.return_value;  // void*
  } else if (n->type == NODE_VARDEC) {
    new->vardec_n.var_name = NULL;
    new->vardec_n.var_type = NULL;
    if (n->vardec_n.var_name != NULL) {
      new->vardec_n.var_name = strdup(n->vardec_n.var_name);
    }
    if (n->vardec_n.var_type != NULL) {
      new->vardec_n.var_type = strdup(n->vardec_n.var_type);
    }
    new->vardec_n.var_value_type = n->vardec_n.var_value_type;
    new->vardec_n.var_value = n->vardec_n.var_value;  // void*
  } else if (n->type == NODE_IMPORT) {
    new->import_n.import_path = strdup(n->import_n.import_path);
  }
  return new;
}

node_fnparam_t* node_fnparam_copy(node_fnparam_t* param) {
  node_fnparam_t* new = malloc(sizeof(node_fnparam_t));
  new->value = strdup(param->value);
  new->type = param->type;
  return new;
}

void node_delete_params(node_fnparam_vector_t* params) {
  if (params == NULL)
    return;
  for (size_t i = 0; i < params->size; ++i) {
    node_fnparam_t* param = *(node_fnparam_t**)vector_get(params, i);
    free(param->value);
    free(param);
  }
  vector_delete(params);
}

node_t* function_node_make(const char *name,
                                 const char *return_type,
                                 node_vector_t* body,
                                 node_fnparam_vector_t* params) {
  node_t *n = malloc(sizeof(node_t));
  n->type = NODE_FUNCTION;
  n->function_n.fn_name = strdup(name);
  if (return_type != NULL) {
    n->function_n.fn_return_type = strdup(return_type);
  } else {
    n->function_n.fn_return_type = NULL;
  }
  n->function_n.fn_body = body;
  n->function_n.fn_params = params;
  return n;
}

node_t* call_node_make(const char *functionName,
                             node_fnparam_vector_t* functionParams) {
  node_t *n = malloc(sizeof(node_t));
  n->type = NODE_CALL;
  n->call_n.fn_call_name = strdup(functionName);
  n->call_n.fn_call_params = functionParams;
  return n;
}

node_t* return_node_make(node_value_kind_t return_type,
                               void* return_value) {
  node_t *n = malloc(sizeof(node_t));
  n->type = NODE_RETURN;
  n->return_n.return_type = return_type;
  n->return_n.return_value = return_value;
  return n;
}

node_t* var_dec_node_make(const char *name,
                                const char *type,
                                node_value_kind_t valueType,
                                void* value) {
  node_t *n = malloc(sizeof(node_t));
  n->type = NODE_VARDEC;
  n->vardec_n.var_name = strdup(name);
  n->vardec_n.var_type = strdup(type);
  n->vardec_n.var_value_type = valueType;
  n->vardec_n.var_value = value;
  return n;
}

node_t* import_node_make(const char *path) {
  node_t *n = malloc(sizeof(node_t));
  n->type = NODE_IMPORT;
  n->import_n.import_path = strdup(path);
  return n;
}
