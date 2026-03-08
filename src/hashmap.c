#include "kilate/hashmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilate/error.h"
#include "kilate/string.h"

hashmap_t* hash_map_make(size_t itemSize) {
  hashmap_t* hashMap = malloc(sizeof(hashmap_t));
  hashMap->itemSize = itemSize;
  hashMap->capacity = 64;
  hashMap->itens = vector_make(sizeof(hashentry_t*));
  for (size_t i = 0; i < hashMap->capacity; i++) {
    hashentry_t* null_ptr = NULL;
    vector_push_back(hashMap->itens, &null_ptr);
  }
  return hashMap;
}

void hash_map_delete(hashmap_t* self) {
  for (size_t i = 0; i < self->itens->size; ++i) {
    hashentry_t* item = *(hashentry_t**)vector_get(self->itens, i);
    if (item != NULL) {
      free(item->key);
      free(item->value);
      free(item);
    }
  }
  vector_delete(self->itens);
  free(self);
}

unsigned int hash_map_hash(hashmap_t* self, char* key) {
  if (self == NULL)
    error_fatal("Hashmap is null.");
  if (key == NULL)
    error_fatal("Key is null.");

  unsigned int hash = 5381;
  int c;
  while ((c = *key++)) {
    hash = ((hash << 5) + hash) + c;  // hash * 33 + c
  }
  return hash % self->capacity;
}
void* hash_map_get(hashmap_t* self, char* key) {
  if (self == NULL)
    error_fatal("Hashmap is null.");
  if (key == NULL)
    error_fatal("Key is null.");

  unsigned int index = hash_map_hash(self, key);

  hashentry_t** itemPtr = (hashentry_t**)vector_get(self->itens, index);
  hashentry_t* item = *itemPtr;

  while (item) {
    if (str_equals(item->key, key)) {
      return item->value;
    }
    item = item->next;
  }
  return NULL;
}

void hash_map_put(hashmap_t* self, char * key, void* value) {
  if (self == NULL)
    error_fatal("Hashmap is null.");
  if (key == NULL)
    error_fatal("Key is null.");

  unsigned int index = hash_map_hash(self, key);
  hashentry_t** headPtr = (hashentry_t**)vector_get(self->itens, index);
  hashentry_t* head = *headPtr;

  hashentry_t* item = head;
  while (item) {
    if (str_equals(item->key, key)) {
      memcpy(item->value, value, self->itemSize);
      return;
    }
    item = item->next;
  }

  hashentry_t* newItem = malloc(sizeof(hashentry_t));
  newItem->key = strdup(key);
  newItem->value = malloc(self->itemSize);
  memcpy(newItem->value, value, self->itemSize);
  newItem->next = head;

  *headPtr = newItem;
}
