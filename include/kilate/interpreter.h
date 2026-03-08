#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__

#include "kilate/environment.h"
#include "kilate/hashmap.h"
#include "kilate/node.h"
#include "kilate/string.h"
#include "kilate/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  hashmap_t * functions;
  hashmap_t * native_functions;
  env_t * env;
} interpreter_t;

typedef enum { IRT_FUNC, IRT_RETURN } interpreter_result_kind_t;

typedef struct {
  interpreter_result_kind_t type;
  void* data;
} interpreter_result_t;

interpreter_t * interpreter_make(node_vector_t*, node_vector_t*);

void interpreter_delete(interpreter_t *);

interpreter_result_t interpreter_run(interpreter_t *);

interpreter_result_t interpreter_run_fn(interpreter_t *,
                                              node_t*,
                                              str_vector_t*);

interpreter_result_t interpreter_run_node(interpreter_t *, node_t*);

#ifdef __cplusplus
}
#endif

#endif