#include "io.h"

#include <kilate/native.h>
#include <kilate/util/native.h>
#include <kilate/node.h>

#include "sys.h"

node_t* std_print(native_fndata_t* data) {
  for (size_t i = 0; i < data->params->size; ++i) {
    node_fnparam_t* param =
        *(node_fnparam_t**)vector_get(data->params, i);
    if (param->type == NODE_VALUE_TYPE_VAR) {
      node_t* var = env_getvar(data->env, param->value);
      void* value = var->vardec_n.var_value;
      switch (var->vardec_n.var_value_type) {
        case NODE_VALUE_TYPE_INT: {
          printf("%d", (int)(intptr_t)value);
          break;
        }
        case NODE_VALUE_TYPE_FLOAT: {
          printf("%f", *(float*)value);
          break;
        }
        case NODE_VALUE_TYPE_LONG: {
          printf("%ld", (long)(intptr_t)value);
          break;
        }
        case NODE_VALUE_TYPE_STRING:
          printf("%s", (char *)value);
          break;
        case NODE_VALUE_TYPE_BOOL:
          printf("%s", (bool)(intptr_t)value ? "true" : "false");
          break;
        case NODE_VALUE_TYPE_FUNC:
          // Does nothing for now
          break;
        case NODE_VALUE_TYPE_VAR:
          // Does nothing for now
          break;
        default:
          // Does nothing for now
          break;
      }
      continue;
    }
    printf("%s", param->value);
  }
  free(data);
  return NULL;
}

node_t* std_system(native_fndata_t* data) {
  char * cmd = native_fndata_getstr(data, 0);
  if (cmd != NULL)
    system(cmd);
  return NULL;
}

node_t* std_sleep(native_fndata_t* data) {
  bool ok;
  long time = native_fndata_getlong(data, 0, &ok);
  if (ok)
    sys_sleep(time);
  return NULL;
}