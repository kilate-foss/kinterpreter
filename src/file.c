#include "kilate/file.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilate/error.h"

// Opens a file
// After do all uses with file, close it
// with file_close
int
file_open (file_t *file, const char *filepath, file_mode_t mode)
{
        const char *fmode = NULL;

        if (mode == FILE_MODE_READ)
                fmode = "r";
        else if (mode == FILE_MODE_WRITE)
                fmode = "w";
        else if (mode == FILE_MODE_RW)
                fmode = "r+";
        else
                return -1;

        file->raw = fopen (filepath, fmode);

        if (!file->raw)
                return -1;

        return 0;
}

int
file_close (file_t *file)
{
        if (!file || !file->raw)
                return -1;

        fclose (file->raw);
        file->raw = NULL;

        return 0;
}

// Returns the length of file content.
size_t
file_get_length (file_t *file)
{
        if (!file || !file->raw)
                return 0;

        long current = ftell (file->raw);
        fseek (file->raw, 0, SEEK_END);

        long len = ftell (file->raw);

        fseek (file->raw, current, SEEK_SET);

        return (size_t)len;
}

// Reads the content of file.
// Result should be free.
char *
file_read_text (file_t *file)
{
        if (!file || !file->raw)
                return NULL;

        size_t len = file_get_length (file);

        char *buffer = malloc (len + 1);
        if (!buffer)
        {
                error_fatal ("Can't alloc memory for reading file. Errno: %s",
                             strerror (errno));
                return NULL;
        }

        rewind (file->raw);

        size_t readed = fread (buffer, 1, len, file->raw);
        if (readed != len && ferror (file->raw))
        {
                error_fatal ("Failed to read file. Errno: %s",
                             strerror (errno));
                free (buffer);
                return NULL;
        }

        buffer[readed] = '\0';
        return buffer;
}
