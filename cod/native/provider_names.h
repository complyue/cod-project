#ifndef COD_NATIVE_PROVIDER_NAMES_H
#define COD_NATIVE_PROVIDER_NAMES_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

static inline bool cod_provider_member_name(const char *name) {
    static const char *const keywords[] = {
        "alignas", "alignof", "auto", "bool", "break", "case", "char", "const", "constexpr", "continue",
        "default", "do", "double", "else", "enum", "extern", "false", "float", "for", "goto", "if",
        "inline", "int", "long", "nullptr", "register", "restrict", "return", "short", "signed", "sizeof",
        "static", "static_assert", "struct", "switch", "thread_local", "true", "typedef", "typeof", "typeof_unqual",
        "union", "unsigned", "void", "volatile", "while",
    };
    /* Generated provider headers use the C keyword and standard macro namespaces. */
    static const char *const prefixes[] = {
        "_", "INT", "UINT", "PTRDIFF_", "SIZE_", "WCHAR_", "WINT_", "SIG_ATOMIC_",
    };
    if (strcmp(name, "NULL") == 0) return false;
    for (size_t i = 0; i < sizeof(prefixes) / sizeof(*prefixes); ++i)
        if (strncmp(name, prefixes[i], strlen(prefixes[i])) == 0) return false;
    for (size_t i = 0; i < sizeof(keywords) / sizeof(*keywords); ++i)
        if (strcmp(name, keywords[i]) == 0) return false;
    return true;
}

#endif
