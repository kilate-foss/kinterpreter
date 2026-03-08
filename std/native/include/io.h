#ifndef __IO_H__
#define __IO_H__

#include <kilate/native.h>
#include <kilate/node.h>

#define EXPORT_STD_PRINT_NAME "Print"
node_t *std_print(native_fndata_t *);

#define EXPORT_STD_SYSTEM_NAME "System"
node_t *std_system(native_fndata_t *);

#define EXPORT_STD_SLEEP_NAME "Sleep"
node_t *std_sleep(native_fndata_t *);

#endif