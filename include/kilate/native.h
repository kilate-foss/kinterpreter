#ifndef __NATIVE_H__
#define __NATIVE_H__

#include "kilate/environment.h"
#include "kilate/lexer.h"
#include "kilate/node.h"
#include "kilate/parser.h"
#include "kilate/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  env_t* env;
  node_fnparam_vector_t* params;
} native_fndata_t;

typedef node_t* (*native_fn_t)(native_fndata_t*);

typedef struct {
  char *name;
  str_vector_t* requiredParams;
  native_fn_t fn;
} native_fnentry_t;

#define KILATE_NATIVE_REGISTER() void KILATE_NATIVE_REGISTER()

extern node_vector_t* native_functions;

void native_init();

void native_load_extern();

void native_end();

void native_register_fnentry(native_fnentry_t*);

void native_register_fn(const char *, str_vector_t*, native_fn_t);

native_fnentry_t* native_find_function(const char *);

void native_register_all_functions();

#ifdef __cplusplus
}
#endif

#endif