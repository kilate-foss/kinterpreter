#ifndef __IO_H__
#define __IO_H__

#include <kilate/native.h>
#include <kilate/node.h>

node_t* std_print(native_fndata_t*);

node_t* std_system(native_fndata_t*);

node_t* std_sleep(native_fndata_t*);

#endif