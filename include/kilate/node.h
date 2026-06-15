#ifndef __NODE_H__
#define __NODE_H__

#include <malloc.h>
#include <string.h>

#include "kilate/bool.h"
#include "kilate/lexer.h"
#include "kilate/vector.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _Nullable
#define _Nullable
#endif

typedef enum node_kind_t
{
        NODE_INVALID = -1,
        NODE_FUNCTION,
        NODE_NATIVE_FUNCTION,
        NODE_CALL,
        NODE_ARG,
        NODE_RETURN,
        NODE_VARDECL,
        NODE_NATIVEDECL,
        NODE_IMPORT
} node_kind_t;

typedef enum node_value_kind_t
{
        NODE_VALUE_TYPE_INT,
        NODE_VALUE_TYPE_FLOAT,
        NODE_VALUE_TYPE_LONG,
        NODE_VALUE_TYPE_STRING,
        NODE_VALUE_TYPE_BOOL,
        NODE_VALUE_TYPE_VAR,
        NODE_VALUE_TYPE_FUNC,
        NODE_VALUE_TYPE_FUNC_OR_VAR,
        NODE_VALUE_TYPE_CALL,
        NODE_VALUE_TYPE_ANY
} node_value_kind_t;

typedef struct node_t node_t;

typedef node_t function_node_t;
typedef node_t native_function_node_t;
typedef node_t call_node_t;
typedef node_t arg_node_t;
typedef node_t return_node_t;
typedef node_t vardecl_node_t;
typedef node_t nativedecl_node_t;
typedef node_t import_node_t;
typedef node_t param_node_t;

typedef vector_t node_vector_t;
typedef node_vector_t node_arg_vector_t;
typedef node_vector_t node_param_vector_t;

typedef struct native_fndata_t native_fndata_t;
typedef return_node_t (*native_fn_t) (native_fndata_t *);

typedef struct interpreter_t interpreter_t;

typedef struct value_t
{
        node_value_kind_t type;
        union
        {
                int i;
                float f;
                long l;
                bool b;
                void *v;

                // if type is var, so this field may be the var NAME!!!
                char *s;

                // can be a function_node, native_function_node or a call_node
                node_t *n;
        };
} value_t;

typedef struct safe_value_t
{
        node_value_kind_t type;
        value_t value;
} safe_value_t;

struct __function_node
{
        bool native;

        char *name;
        char *return_type;
        node_param_vector_t *params;

        union
        {
                node_vector_t *body;   // if its a normal
                                       // function so its valid
                native_fn_t native_fn; // if its a native
                                       // function so its valid
        };
};

struct node_t
{
        node_kind_t type;

        union
        {
                struct __function_node function_n;
                struct __function_node nativedecl_n;

                struct
                {
                        char *name;
                        node_arg_vector_t *args;
                } call_n;

                value_t arg_n;
                value_t param_n;
                value_t return_n;

                struct
                {
                        char *name;
                        char *type;
                        value_t value;
                } vardecl_n;

                struct
                {
                        char *path;
                } import_n;
        };
};

static inline node_t
make_node (node_kind_t kind)
{
        node_t node;
        memset (&node, 0, sizeof node);
        node.type = kind;
        return node;
}

void node_delete (node_t *);

node_t *node_copy (node_t *);

const char *node_kind_tostr (node_kind_t);
const char *token_kind_to_str (token_kind_t);
const char *node_value_kind_to_str (node_value_kind_t);
node_value_kind_t str_to_node_value_kind (const char *);

safe_value_t get_safe_value (interpreter_t *, arg_node_t *);

int safe_to_int (safe_value_t);
float safe_to_float (safe_value_t);
char *safe_to_string (safe_value_t);

#ifdef __cplusplus
}
#endif

#endif
