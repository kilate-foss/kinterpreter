#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <stdio.h>

#include "kilate/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef vector_t hashmap_vector_t;
typedef vector_t hashentry_vector_t;

typedef struct hashmap_t {
        hashentry_vector_t *itens;
        size_t itemSize;
        size_t capacity;
} hashmap_t;

typedef struct hashentry_t {
        char *key;
        void *value;
        void *next;
} hashentry_t;

hashmap_t *hash_map_make(size_t);

void hash_map_delete(hashmap_t *);

unsigned int hash_map_hash(hashmap_t *, char *);

void *hash_map_get(hashmap_t *, char *);

void hash_map_put(hashmap_t *, char *, void *);

#ifdef __cplusplus
}
#endif

#endif