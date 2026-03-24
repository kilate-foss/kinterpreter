#include "kilate/file.h"

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "kilate/error.h"
#include "kilate/string.h"

// Opens a file
// After do all uses with file, close it
// with file_close
int file_open(file_t *file, const char *filepath, file_mode_t mode)
{
        int fd = -1;
        if (mode == FILE_MODE_READ) {
                fd = open(filepath, O_RDONLY);
        } else if (mode == FILE_MODE_WRITE) {
                fd = open(filepath, O_WRONLY);
        } else if (mode == FILE_MODE_RW) {
                fd = open(filepath, O_RDWR);
        }

        if (fd < 0) {
                perror("Failed to open file");
                return -1;
        }

        file->fd = fd;
        return 0;
}

int file_close(file_t *file)
{
        if (!file || file->fd < 0)
                return -1;

        close(file->fd);
        return 0;
}

// Returns the length of file content.
size_t file_get_length(file_t *file)
{
        if (!file || file->fd < 0)
                return 0;
        size_t len = lseek(file->fd, 0, SEEK_END);
        lseek(file->fd, 0, SEEK_SET);
        return len;
}

// Reads the content of file.
// Result should be free.
char *file_read_text(file_t *file)
{
        if (!file || file->fd < 0)
                return 0;

        size_t len = file_get_length(file);
        if (len < 0) {
                error_fatal("Can't read a empty file. Errno: %s",
                            strerror(errno));
                return NULL;
        }

        char *buffer = malloc(len + 1);
        if (!buffer) {
                error_fatal("Can't alloc memory for reading file. Errno: %s",
                            strerror(errno));
                return NULL;
        }

        if (read(file->fd, buffer, len) < 0) {
                error_fatal("Failed to read file. Errno: %s", strerror(errno));
                free(buffer);
                return NULL;
        }

        buffer[len] = '\0';
        return buffer;
}
