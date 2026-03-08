#ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>

#define FILE_MODE_READ "r"
#define FILE_MODE_WRITE "w"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct file_t {
  FILE *std_file;
} file_t;

file_t* file_open(const char *, const char *);

void file_close(file_t*);

size_t file_get_length(file_t*);

char * file_read_text(file_t*);

#ifdef __cplusplus
}
#endif

#endif
