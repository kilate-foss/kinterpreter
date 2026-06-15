#ifndef kilate_interpreter_h
#define kilate_interpreter_h

#include "kilate/environment.h"
#include "kilate/hashmap.h"
#include "kilate/node.h"
#include "kilate/string.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct interpreter_t
{
        hashmap_t *functions;
        hashmap_t *decls;
        env_t *env;
        const char *filename;
} interpreter_t;

typedef enum interpreter_result_kind_t
{
        IRT_FUNC,
        IRT_RETURN
} interpreter_result_kind_t;

typedef struct interpreter_result_t
{
        interpreter_result_kind_t type;
        value_t value;
} interpreter_result_t;

interpreter_t *interpreter_make (const char *, node_vector_t *, node_vector_t *);

void interpreter_delete (interpreter_t *);

// Start interpreting.
interpreter_result_t interpreter_run (interpreter_t *);

// Executes a Function Node
// it can run KilateFunctions either NativeFunctions
interpreter_result_t interpreter_run_fn (interpreter_t *, node_t *,
                                         node_arg_vector_t *);

// Executes a Node
interpreter_result_t interpreter_run_node (interpreter_t *, node_t *);

#ifdef __cplusplus
}
#endif

#endif
