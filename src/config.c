#include <stdio.h>
#include <stdlib.h>

#include "kilate/config.h"
#include "kilate/vector.h"

vector_t* files = NULL;
vector_t* libs_directories = NULL;
vector_t* libs_native_directories = NULL;

void config_init() {
  files = vector_make(sizeof(char *));
  libs_directories = vector_make(sizeof(char *));
  libs_native_directories = vector_make(sizeof(char *));
}

void config_end() {
  for (size_t i = 0; i < files->size; ++i) {
    char * filename = *(char **)vector_get(files, i);
    free(filename);
  }

  for (size_t i = 0; i < libs_directories->size; ++i) {
    char * lib = *(char **)vector_get(libs_directories, i);
    free(lib);
  }

  for (size_t i = 0; i < libs_native_directories->size; ++i) {
    char * lib = *(char **)vector_get(libs_native_directories, i);
    free(lib);
  }

  vector_delete(libs_directories);
  vector_delete(libs_native_directories);
  vector_delete(files);
}