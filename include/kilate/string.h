#ifndef __STRING_H__
#define __STRING_H__

#include <stdarg.h>
#include <stdlib.h>

#include "kilate/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef vector_t str_vector_t;

#define str_length(s) strlen(s)
#define str_equals(s, e) (strcmp(s, e) == 0)
#define str_concat(s, e) strcat(s, e)
#define str_to_int(s) atoi(s)
#define str_to_float(s) strtof(s, NULL)
#define str_to_long(s) strtol(s, NULL, 10)
#define str_starts_with(s, startWith, offset) (strncmp(s + offset, startWith, str_length(startWith)) == 0)

size_t str_index_of(const char *, char, size_t);

char *str_substring(const char *, size_t, size_t);

char *str_format(const char *, ...);

#ifdef __cplusplus
}
#endif

#endif