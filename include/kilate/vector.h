#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
        void *data;
        size_t capacity;
        size_t size;
        size_t itemSize;
} vector_t;

vector_t *vector_make(size_t);

void vector_delete(vector_t *);

const void *vector_get(vector_t *, size_t);

void vector_set(vector_t *, const size_t, const void *);

void vector_reserve(vector_t *, const size_t);

void vector_insert(vector_t *, const size_t, const void *);

void vector_push_back(vector_t *, const void *);

void vector_remove(vector_t *, size_t);

#ifdef __cplusplus
}
#endif

#endif