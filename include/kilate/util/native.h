#ifndef __NATIVE_UTIL_H__
#define __NATIVE_UTIL_H__

#include <stddef.h>

#include "kilate/native.h"

#ifdef __cplusplus
extern "C" {
#endif

/** These methods also checks the 'var' type. */
/** Returns a string value from a param in native_fndata */
char * native_fndata_getstr(native_fndata_t* data, size_t index);

/** Returns a int value from a param in native_fndata */
int native_fndata_getint(native_fndata_t* data, size_t index, bool* ok);

/** Returns a float value from a param in native_fndata */
float native_fndata_getfloat(native_fndata_t* data, size_t index, bool* ok);

/** Returns a long value from a param in native_fndata */
long native_fndata_getlong(native_fndata_t* data, size_t index, bool* ok);

/** Returns a bool value from a param in native_fndata */
bool native_fndata_getbool(native_fndata_t* data, size_t index, bool* ok);

/** Adds a param in params vector */
void params_add(str_vector_t* params, const char * param);

/** Creates a native_fnentry */
native_fnentry_t* native_fnentry_make(const char * name,
                                  str_vector_t* reqParams,
                                  native_fn_t fn);

#ifdef __cplusplus
}
#endif

#endif