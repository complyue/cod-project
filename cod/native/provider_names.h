#ifndef COD_NATIVE_PROVIDER_NAMES_H
#define COD_NATIVE_PROVIDER_NAMES_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

static inline bool cod_provider_member_name(const char *name) {
    static const char *const keywords[] = {
#define COD_PROVIDER_EXACT(spelling) spelling,
#define COD_PROVIDER_PREFIX(spelling)
#include "provider_member_names.def"
#undef COD_PROVIDER_PREFIX
#undef COD_PROVIDER_EXACT
    };
    static const char *const prefixes[] = {
#define COD_PROVIDER_EXACT(spelling)
#define COD_PROVIDER_PREFIX(spelling) spelling,
#include "provider_member_names.def"
#undef COD_PROVIDER_PREFIX
#undef COD_PROVIDER_EXACT
    };
    for (size_t i = 0; i < sizeof(prefixes) / sizeof(*prefixes); ++i)
        if (strncmp(name, prefixes[i], strlen(prefixes[i])) == 0) return false;
    for (size_t i = 0; i < sizeof(keywords) / sizeof(*keywords); ++i)
        if (strcmp(name, keywords[i]) == 0) return false;
    return true;
}

#endif
