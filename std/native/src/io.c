#include "io.h"

#include <kilate/native.h>
#include <kilate/node.h>
#include <kilate/util/native.h>
#include <stdlib.h>

#include "sys.h"

return_node_t *std_print(native_fndata_t *data) {
    for (size_t i = 0; i < data->args->size; ++i) {
        arg_node_t *param = *(arg_node_t **)vector_get(data->args, i);
        safe_value_t v = get_safe_value(data->inter, param);
        printf("%s", safe_to_string(v));
    }

    free(data);
    node_t *node = alloc_node(NODE_RETURN);
    node->return_n.type = NODE_VALUE_TYPE_INT;
    node->return_n.i = 0;
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
