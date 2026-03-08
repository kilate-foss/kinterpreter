#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

#include "kilate/bool.h"
#include "kilate/node.h"
#include "kilate/string.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct env_entry_t {
  char* name;
  node_t* value;
  struct env_entry_t* next;
} env_entry_t;

typedef struct env_t {
  env_entry_t* variables;
  struct env_t * parent;
} env_t;

env_t * env_make(env_t *);

void env_destroy(env_t *);

bool env_definevar(env_t *,
                   const char *,
                   void*);

node_t * env_getvar(env_t *, const char *);

bool env_setvar(env_t *, const char *, void *);

#ifdef __cplusplus
}
#endif

#endif