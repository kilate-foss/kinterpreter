#include "kilate/string.h"

#include <stdlib.h>
#include <string.h>

#include "kilate/bool.h"
#include "kilate/error.h"

size_t str_length(const char *s)
{
        size_t len = 0;
        while (s[len] != '\0') {
                len++;
        }
        return len;
}

bool str_starts_with(const char *s, const char *startWith, size_t offset)
{
        if (strncmp(s + offset, startWith, str_length(startWith)) == 0) {
                return true;
        }
        return false;
}

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

bool str_equals(const char *s, const char *other)
{
        if (strcmp(s, other) == 0) {
                return true;
        }
        return false;
}

void str_concat(char *dest, const char *toConcat)
{
        strcat(dest, toConcat);
}

int str_to_int(const char *src)
{
        int num = 0;
        size_t i = 0;
        int sign = 1;

        if (src[0] == '-') {
                sign = -1;
                i = 1;
        }

        for (; i < str_length(src); ++i) {
                if (src[i] >= '0' && src[i] <= '9') {
                        num = num * 10 + (src[i] - '0');
                } else {
                        break;
                }
        }

        return sign * num;
}

float str_to_float(const char *s)
{
        if (s == NULL)
                return 0.0f;
        return strtof(s, NULL);
}

long str_to_long(const char *s)
{
        if (s == NULL)
                return 0;
        return strtol(s, NULL, 10);
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