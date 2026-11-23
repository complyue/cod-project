#ifndef COD_BOOTER_TOOLCHAIN_PATH_H
#define COD_BOOTER_TOOLCHAIN_PATH_H

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static inline char *cod_booter_toolchain_path(const char *suffix, bool relative_to_prefix) {
    const size_t length = strlen(suffix) + 1;
    size_t capacity = 256;
    char *path = NULL;
    for (;;) {
        char *grown = realloc(path, capacity);
        if (!grown) {
            free(path);
            errno = ENOMEM;
            return NULL;
        }
        path = grown;
        const ssize_t size = readlink("/proc/self/exe", path, capacity - 1);
        if (size < 0) {
            const int error = errno;
            free(path);
            errno = error;
            return NULL;
        }
        if ((size_t)size < capacity - 1) {
            path[size] = 0;
            char *separator = strrchr(path, '/');
            if (!separator || path[0] != '/') {
                free(path);
                errno = EINVAL;
                return NULL;
            }
            if (relative_to_prefix && separator != path) {
                *separator = 0;
                separator = strrchr(path, '/');
            }
            const size_t prefix = (size_t)(separator + 1 - path);
            if (length <= capacity - prefix) {
                memcpy(path + prefix, suffix, length);
                return path;
            }
        }
        if (capacity > (size_t)PTRDIFF_MAX / 2) {
            free(path);
            errno = EOVERFLOW;
            return NULL;
        }
        capacity *= 2;
    }
}

#endif
