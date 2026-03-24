#ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum file_mode_t {
        FILE_MODE_READ,
        FILE_MODE_WRITE,
        FILE_MODE_RW
} file_mode_t;

typedef struct file_t {
        int fd;
} file_t;

int file_open(file_t *, const char *, file_mode_t);

int file_close(file_t *);

size_t file_get_length(file_t *);

char *file_read_text(file_t *);

#ifdef __cplusplus
}
#endif

#endif
