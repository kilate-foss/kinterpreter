#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <stdio.h>

#include "kilate/vector.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef vector_t hashmap_vector_t;
typedef vector_t hm_entry_vector_t;

typedef struct hashmap_t
{
        hm_entry_vector_t *itens;
        size_t item_size;
        size_t capacity;
} hashmap_t;

typedef struct hm_entry_t
{
        char *key;
        void *value;
        void *next;
} hm_entry_t;

hashmap_t *hm_make (size_t);

void hm_delete (hashmap_t *);

unsigned int hm_hash (hashmap_t *, char *);

void *hm_get (hashmap_t *, char *);

void hm_put (hashmap_t *, char *, void *);

#ifdef __cplusplus
}
#endif

#endif