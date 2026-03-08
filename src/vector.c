#include "kilate/vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilate/error.h"

vector_t* vector_make(size_t itemSize) {
  vector_t* self = malloc(sizeof(vector_t));
  if (itemSize == 0)
    error_fatal("Item size cant be 0");
  if (self == NULL) {
    printf("Failed to create vector, Out Of Memory.\n");
    return NULL;
  }
  self->itemSize = itemSize;
  self->size = 0;
  self->capacity = 1;
  self->data = calloc(self->itemSize, self->capacity);
  return self;
}

void vector_delete(vector_t* self) {
  if (self == NULL)
    error_fatal("Vector is null");
  if (self == NULL)
    return;
  free(self->data);
  free(self);
}

const void* vector_get(vector_t* self, size_t index) {
  if (self == NULL)
    error_fatal("Vector is null");
  if (index >= self->size)
    error_fatal("Index can't be higher or equal vector size.");

  return (char*)(self->data) + index * self->itemSize;
}

void vector_reserve(vector_t* self, const size_t size) {
  if (self == NULL)
    error_fatal("Vector is null");
  if (self->capacity <= size) {
    self->data = realloc(self->data, size * self->itemSize);
    memset((char*)self->data + self->capacity * self->itemSize, 0,
           (size - self->capacity) * self->itemSize);
    self->capacity = size;
  }
}

void vector_set(vector_t* self, size_t index, const void* item) {
  if (self == NULL)
    error_fatal("Vector is null");
  if (index >= self->size)
    error_fatal("Index can't be higher or equal vector size.");

  memcpy((char*)self->data + index * self->itemSize, item, self->itemSize);
}

void vector_insert(vector_t* self, size_t index, const void* item) {
  if (self == NULL)
    error_fatal("Vector is null");
  if (!(index <= self->size))
    error_fatal("Index can't be higher than vector size");

  if (self->capacity <= self->size) {
    vector_reserve(self, 2 * self->capacity);
  }
  memmove((char*)self->data + (index + 1) * self->itemSize,
          (char*)self->data + index * self->itemSize,
          (self->size - index) * self->itemSize);

  self->size++;
  vector_set(self, index, item);
}

void vector_push_back(vector_t* self, const void* item) {
  vector_insert(self, self->size, item);
}

void vector_remove(vector_t* self, size_t index) {
  if (self == NULL)
    error_fatal("Vector is null");
  if (index >= self->size)
    error_fatal("Index can't be higher or equal vector size.");

  if (index < self->size - 1) {
    memmove((char*)self->data + index * self->itemSize,
            (char*)self->data + (index + 1) * self->itemSize,
            (self->size - index - 1) * self->itemSize);
  }
  self->size--;
}