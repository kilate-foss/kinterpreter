#include "kilate/error.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


void error_fatal(char * fmt, ...) {
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "[ERROR] ");
  vprintf(fmt, args);
  printf("\n");
  va_end(args);
  exit(1);
}