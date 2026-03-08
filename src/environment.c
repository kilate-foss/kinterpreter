#include "kilate/environment.h"

#include <stdlib.h>
#include <string.h>

#include "kilate/bool.h"
#include "kilate/error.h"
#include "kilate/string.h"

env_t* env_make(env_t* parent) {
  env_t* env = malloc(sizeof(env_t));
  if (env == NULL)
    error_fatal("Failed to allocate environment");

  env->variables = NULL;
  env->parent = parent;
  return env;
}

void env_destroy(env_t* env) {
  if (env == NULL)
    return;

  env_entry_t* current = env->variables;
  while (current) {
    env_entry_t* next = current->next;
    free(current->name);
    if (current->value) {
      node_delete((node_t*)current->value);
    }
    free(current);
    current = next;
  }

  free(env);
}

bool env_definevar(env_t* env,
                                   const char * name,
                                   void* value) {
  if (env == NULL || name == NULL)
    error_fatal("Environment or name is null.");

  env_entry_t* current = env->variables;
  while (current) {
    if (str_equals(current->name, name)) {
      return false;
    }
    current = current->next;
  }

  env_entry_t* new_entry = malloc(sizeof(env_entry_t));
  if (!new_entry)
    return false;

  new_entry->name = strdup(name);
  new_entry->value = value;
  new_entry->next = env->variables;

  env->variables = new_entry;
  return true;
}

node_t* env_getvar(env_t* env, const char* name) {
  if (env == NULL || name == NULL)
    error_fatal("Environment or name is null.");

  env_t* current_env = env;
  while (current_env) {
    env_entry_t* current = current_env->variables;
    while (current) {
      if (strcmp(current->name, name) == 0) {
        return current->value;
      }
      current = current->next;
    }
    current_env = current_env->parent;
  }

  return NULL;
}

bool env_setvar(env_t* env,
                                const char* name,
                                void* value) {
  if (env == NULL || name == NULL)
    error_fatal("Environment or name is null.");

  env_t* current_env = env;
  while (current_env) {
    env_entry_t* current = current_env->variables;
    while (current) {
      if (strcmp(current->name, name) == 0) {
        current->value = value;
        return true;
      }
      current = current->next;
    }
    current_env = current_env->parent;
  }

  return false;
}