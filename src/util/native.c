#include "kilate/util/native.h"

#include <stdlib.h>
#include <string.h>

#include "kilate/native.h"

char *native_fndata_getstr(native_fndata_t *data, size_t index)
{
        if (index >= data->args->size)
                return "";

        param_node_t *param = *(param_node_t **)vector_get(data->args, index);
        if (param->arg_n.type == NODE_VALUE_TYPE_STRING) {
                return param->arg_n.s;
        }

        node_t *var = env_getvar(data->inter->env, param->arg_n.s);
        if (!var || var->vardec_n.value.type != NODE_VALUE_TYPE_STRING)
                return NULL;

        if (!var->vardec_n.value.s)
                return NULL;

        return var->vardec_n.value.s;
}

int native_fndata_getint(native_fndata_t *data, size_t index, bool *ok)
{
        if (index >= data->args->size) {
                if (ok)
                        *ok = false;
                return 0;
        }

        param_node_t *param = *(param_node_t **)vector_get(data->args, index);
        if (param->arg_n.type == NODE_VALUE_TYPE_INT) {
                if (ok)
                        *ok = true;
                return param->arg_n.i;
        }

        node_t *var = env_getvar(data->inter->env, param->arg_n.s);
        if (!var || var->vardec_n.value.type != NODE_VALUE_TYPE_INT) {
                if (ok)
                        *ok = false;
                return 0;
        }

        if (ok)
                *ok = true;
        return var->vardec_n.value.i;
}

float native_fndata_getfloat(native_fndata_t *data, size_t index, bool *ok)
{
        if (index >= data->args->size) {
                if (ok)
                        *ok = false;
                return 0;
        }

        param_node_t *param = *(param_node_t **)vector_get(data->args, index);
        if (param->arg_n.type == NODE_VALUE_TYPE_FLOAT) {
                if (ok)
                        *ok = true;
                return param->arg_n.f;
                ;
        }

        node_t *var = env_getvar(data->inter->env, param->arg_n.s);
        if (!var || var->vardec_n.value.f != NODE_VALUE_TYPE_FLOAT) {
                if (ok)
                        *ok = false;
                return 0;
        }

        if (ok)
                *ok = true;
        return var->vardec_n.value.f;
}

long native_fndata_getlong(native_fndata_t *data, size_t index, bool *ok)
{
        if (index >= data->args->size) {
                if (ok)
                        *ok = false;
                return 0;
        }

        param_node_t *param = *(param_node_t **)vector_get(data->args, index);
        if (param->arg_n.type == NODE_VALUE_TYPE_LONG) {
                if (ok)
                        *ok = true;
                return param->arg_n.l;
        }

        node_t *var = env_getvar(data->inter->env, param->arg_n.s);
        if (!var || var->vardec_n.value.type != NODE_VALUE_TYPE_LONG) {
                if (ok)
                        *ok = false;
                return 0;
        }

        if (ok)
                *ok = true;
        return var->vardec_n.value.l;
}

bool native_fndata_getbool(native_fndata_t *data, size_t index, bool *ok)
{
        if (index >= data->args->size) {
                if (ok)
                        *ok = false;
                return false;
        }

        param_node_t *param = *(param_node_t **)vector_get(data->args, index);
        if (param->arg_n.type == NODE_VALUE_TYPE_BOOL) {
                if (ok)
                        *ok = true;
                return param->arg_n.b;
        }

        node_t *var = env_getvar(data->inter->env, param->arg_n.s);
        if (!var || var->vardec_n.value.type != NODE_VALUE_TYPE_BOOL) {
                if (ok)
                        *ok = false;
                return false;
        }

        if (ok)
                *ok = true;
        return var->vardec_n.value.b;
}

void params_add(node_param_vector_t *params, node_value_kind_t type,
                const char *param)
{
        param_node_t *n = alloc_node(NODE_ARG);
        n->arg_n.type = type;
        n->arg_n.s = (char *)param;
        vector_push_back(params, n);
}