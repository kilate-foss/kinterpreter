#include "kilate/string.h"

#include <stdlib.h>
#include <string.h>

#include "kilate/bool.h"
#include "kilate/error.h"

size_t str_index_of(const char *s, char ch, size_t offset)
{
        char *ptr = strchr(s + offset, ch);
        if (ptr == NULL) {
                printf("Failed to get index of string %s\n", s);
                return SIZE_MAX;
        }
        return ptr - s;
}

char *str_substring(const char *s, size_t start, size_t end)
{
        if (!s || start > end || end > str_length(s)) {
                return NULL;
        }

        size_t len = end - start;
        char *result = malloc(len + 1);
        if (!result)
                return NULL;

        memcpy(result, s + start, len);
        result[len] = '\0';
        return result;
}

char *str_format(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);

        size_t len = vsnprintf(NULL, 0, fmt, args);
        va_end(args);

        char *buffer = malloc(len + 1);
        if (!buffer)
                return NULL;

        va_start(args, fmt);
        vsnprintf(buffer, len + 1, fmt, args);
        va_end(args);

        return buffer;
}