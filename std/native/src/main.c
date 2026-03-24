#include <kilate/native.h>
#include <kilate/string.h>
#include <kilate/util/native.h>
#include <kilate/vector.h>

#include "io.h"

KILATE_NATIVE_REGISTER()
{
        {
                // Register native print method
                node_arg_vector_t *req_params = vector_make(sizeof(arg_node_t *));
                params_add(req_params, NODE_VALUE_TYPE_ANY, "value");
                native_register_fn(EXPORT_STD_PRINT_NAME, "Int",
                                   req_params, std_print);
        }
        {
                // Register native system method
                node_arg_vector_t *req_params = vector_make(sizeof(arg_node_t *));
                params_add(req_params, NODE_VALUE_TYPE_STRING, "cmd");
                native_register_fn(EXPORT_STD_SYSTEM_NAME, "Int",
                                   req_params, std_system);
        }
        {
                // Register native system method
                node_arg_vector_t *req_params = vector_make(sizeof(arg_node_t *));
                params_add(req_params, NODE_VALUE_TYPE_LONG, "ms");
                native_register_fn(EXPORT_STD_SLEEP_NAME, "Work",
                                   req_params, std_sleep);
        }
}
