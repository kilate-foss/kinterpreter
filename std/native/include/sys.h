#ifndef __SYS_H__
#define __SYS_H__

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#define sys_sleep(ms) Sleep(ms)
#else
#include <unistd.h>
#define sys_sleep(ms) usleep(ms * 1000)
#endif

#endif