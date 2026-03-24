#ifndef __CONFIG__
#define __CONFIG__

#include "kilate/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

extern vector_t *files;
extern vector_t *libs_directories;
extern vector_t *libs_native_directories;

void config_init();
void config_end();

#ifdef __cplusplus
}
#endif

#endif