#include "kilate/debug.h"

#include <stdio.h>
#include <stdlib.h>

void printd(const char *fmt, ...)
{
#ifdef DEBUG
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
#else
        (void)fmt;
#endif
}
