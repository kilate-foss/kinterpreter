#ifndef __STRING_H__
#define __STRING_H__

#include <stdarg.h>
#include <stdlib.h>

#include "kilate/bool.h"
#include "kilate/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef vector_t str_vector_t;

size_t str_length(const char *);

bool str_starts_with(const char *, const char *, size_t);

size_t str_index_of(const char *, char, size_t);

char * str_substring(const char *, size_t, size_t);

bool str_equals(const char *, const char *);

void str_concat(char *, const char *);

int str_to_int(const char *);

float str_to_float(const char *);

long str_to_long(const char *);

char * str_format(const char *, ...);

#ifdef __cplusplus
}
#endif

#endif