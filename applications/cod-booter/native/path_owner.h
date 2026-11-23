#ifndef COD_BOOTER_PATH_OWNER_H
#define COD_BOOTER_PATH_OWNER_H

#include "cod_providers.h"

#include <stddef.h>

typedef enum {
    COD_BOOTER_PATH_EMPTY,
    COD_BOOTER_PATH_LAUNCHER,
    COD_BOOTER_PATH_CANONICAL,
} cod_booter_path_state;

typedef struct {
    const char *bytes;
    size_t length;
    cod_booter_path_state state;
} cod_booter_path_owner;

_Static_assert(UINTPTR_MAX <= UINT64_MAX, "path handles require a pointer that fits in a handle word");

static inline const cod_booter_path_owner *cod_booter_path_selected(cod_native_Path_borrow path) {
    if (!path.owner || (uint64_t)(uintptr_t)path.owner != path.owner) return NULL;
    const cod_booter_path_owner *owner = (const cod_booter_path_owner *)(uintptr_t)path.owner;
    if (owner->state != COD_BOOTER_PATH_LAUNCHER && owner->state != COD_BOOTER_PATH_CANONICAL) return NULL;
    if (!owner->bytes || path.payload != (uint64_t)(uintptr_t)owner->bytes) return NULL;
    return owner;
}

#endif
