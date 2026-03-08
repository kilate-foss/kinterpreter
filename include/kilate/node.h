#ifndef __NODE_H__
#define __NODE_H__

#include "kilate/bool.h"
#include "kilate/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum node_kind_t {
        NODE_FUNCTION,
        NODE_CALL,
        NODE_RETURN,
        NODE_VARDEC,
        NODE_IMPORT
} node_kind_t;

typedef enum node_value_kind_t {
        NODE_VALUE_TYPE_INT,
        NODE_VALUE_TYPE_FLOAT,
        NODE_VALUE_TYPE_LONG,
        NODE_VALUE_TYPE_STRING,
        NODE_VALUE_TYPE_BOOL,
        NODE_VALUE_TYPE_VAR,
        NODE_VALUE_TYPE_FUNC,
        NODE_VALUE_TYPE_CALL,
        NODE_VALUE_TYPE_ANY
} node_value_kind_t;

typedef struct node_t node_t;
typedef node_t function_node_t;
typedef node_t call_node_t;
typedef node_t return_node_t;
typedef node_t vardec_node_t;
typedef node_t import_node_t;
typedef vector_t node_vector_t;

typedef struct node_fnparam_t {
        char *value;
        node_value_kind_t type;
} node_fnparam_t;

typedef vector_t node_fnparam_vector_t;

typedef struct value_t {
        node_value_kind_t type;
        union {
                int i;
                float f;
                long l;
                char *s; // if return_type is var, so this field may be the var NAME!!!
                bool b;
                node_t *n; // can be a function_node or a call_node.
                void *v;
        };
} value_t;

struct node_t {
        node_kind_t type;

        union {
                struct {
                        char *fn_name;
                        char *fn_return_type;
                        node_vector_t *fn_body;
                        node_fnparam_vector_t *fn_params;
                } function_n;

                struct {
                        char *fn_call_name;
                        node_fnparam_vector_t *fn_call_params;
                } call_n;

                struct {
                        char *var_name;
                        char *var_type;
                        node_value_kind_t var_value_type;
                        void *var_value;
                } vardec_n;

                value_t return_n;

                struct {
                        char *import_path;
                } import_n;
        };
};

void node_delete(node_t *);

node_t *node_copy(node_t *);

node_fnparam_t *node_fnparam_copy(node_fnparam_t *);

void node_delete_params(node_fnparam_vector_t *);

function_node_t *function_node_make(const char *, const char *,
                                    node_vector_t *, node_fnparam_vector_t *);

call_node_t *call_node_make(const char *, node_fnparam_vector_t *);

vardec_node_t *var_dec_node_make(const char *, const char *, node_value_kind_t,
                                 void *);

import_node_t *import_node_make(const char *);

#ifdef __cplusplus
}
#endif

#endif
