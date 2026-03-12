#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__

#include "kilate/environment.h"
#include "kilate/hashmap.h"
#include "kilate/node.h"
#include "kilate/string.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct interpreter_t {
        hashmap_t *functions;
        env_t *env;
} interpreter_t;

typedef enum interpreter_result_kind_t {
        IRT_FUNC,
        IRT_RETURN
} interpreter_result_kind_t;

typedef struct interpreter_result_t {
        interpreter_result_kind_t type;
        value_t value;
} interpreter_result_t;

interpreter_t *interpreter_make(node_vector_t *, node_vector_t *);

void interpreter_delete(interpreter_t *);

interpreter_result_t interpreter_run(interpreter_t *);

// Low function node runner
// it will call interpreter_run_fn for Kilate Functions.
// and runs directly for native functions.
interpreter_result_t interpreter_run_fnlow(interpreter_t *, node_t *, node_arg_vector_t *);

// Runs a Kilate Node
interpreter_result_t interpreter_run_fn(interpreter_t *, node_t *,
                                        node_arg_vector_t *);

interpreter_result_t interpreter_run_node(interpreter_t *, node_t *);

#ifdef __cplusplus
}
#endif

#endif
