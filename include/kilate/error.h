#ifndef __ERROR_H__
#define __ERROR_H__

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void error_fatal(char *, ...);

#ifdef __cplusplus
}
#endif

#endif