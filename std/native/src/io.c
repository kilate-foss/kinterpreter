#include "io.h"

#include <kilate/native.h>
#include <kilate/node.h>
#include <kilate/util/native.h>
#include <stdlib.h>

#include "sys.h"

return_node_t *std_print(native_fndata_t *data)
{
        int printf_ret = 0;
        for (size_t i = 0; i < data->params->size; ++i) {
                node_fnparam_t *param =
                    *(node_fnparam_t **)vector_get(data->params, i);
                if (param->type == NODE_VALUE_TYPE_VAR) {
                        node_t *var = env_getvar(data->env, param->value);
                        void *value = var->vardec_n.var_value;
                        switch (var->vardec_n.var_value_type) {
                        case NODE_VALUE_TYPE_INT: {
                                printf_ret =
                                    printf("%d", (int)(intptr_t)value);
                                break;
                        }
                        case NODE_VALUE_TYPE_FLOAT: {
                                printf_ret = printf("%f", *(float *)value);
                                break;
                        }
                        case NODE_VALUE_TYPE_LONG: {
                                printf_ret =
                                    printf("%ld", (long)(intptr_t)value);
                                break;
                        }
                        case NODE_VALUE_TYPE_STRING:
                                printf_ret = printf("%s", (char *)value);
                                break;
                        case NODE_VALUE_TYPE_BOOL:
                                printf_ret = printf(
                                    "%s",
                                    (bool)(intptr_t)value ? "true" : "false");
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
                printf_ret = printf("%s", param->value);
        }
        free(data);

        node_t *node = alloc_node(NODE_RETURN);
        node->return_n.type = NODE_VALUE_TYPE_INT;
        node->return_n.i = printf_ret;
        return node;
}

node_t *std_system(native_fndata_t *data)
{
        node_t *node = alloc_node(NODE_RETURN);
        node->return_n.type = NODE_VALUE_TYPE_INT;
        char *cmd = native_fndata_getstr(data, 0);
        if (cmd == NULL) {
                node->return_n.i = -1;
                return node;
        }
        node->return_n.i = system(cmd);
        return node;
}

node_t *std_sleep(native_fndata_t *data)
{
        bool ok;
        long time = native_fndata_getlong(data, 0, &ok);
        if (ok)
                sys_sleep(time);
        return NULL;
}
