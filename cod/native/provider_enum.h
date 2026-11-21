#ifndef COD_NATIVE_PROVIDER_ENUM_H
#define COD_NATIVE_PROVIDER_ENUM_H

#include "provider_text.h"
#include <inttypes.h>
#include <string.h>

typedef struct {
    const char *spelling;
    uint64_t code;
    bool canonical;
} cod_provider_enum_member;

static inline char *cod_provider_escape_scope(char *out, unsigned char byte) {
    static const char hex[] = "0123456789abcdef";
    if ((byte >= '0' && byte <= '9') || (byte >= 'A' && byte <= 'Z') ||
        (byte >= 'a' && byte <= 'z' && byte != 'x')) {
        *out++ = (char)byte;
    } else {
        *out++ = 'x';
        *out++ = hex[byte >> 4];
        *out++ = hex[byte & 15];
    }
    return out;
}

/* Use the canonical source path relative to the entry source's directory. */
static inline char *cod_provider_enum_name(const char *entry, const char *module, const char *type) {
    size_t common = 0;
    for (size_t i = 0; entry[i] && entry[i] == module[i]; ++i)
        if (entry[i] == '/') common = i + 1;
    size_t up = 0;
    for (const char *at = entry + common; *at; ++at)
        if (*at == '/') ++up;
    const size_t module_length = strlen(module + common), type_length = strlen(type);
    const size_t overhead = sizeof("cod_enum__") + 3 * sizeof(size_t) + 2;
    if (type_length > SIZE_MAX - overhead || up > (SIZE_MAX - overhead - type_length) / 9) return NULL;
    const size_t fixed = overhead + type_length + 9 * up;
    if (module_length > (SIZE_MAX - fixed) / 3) return NULL;
    const size_t capacity = fixed + 3 * module_length;
    char *name = (char *)malloc(capacity);
    if (!name) return NULL;
    char *out = name;
    memcpy(out, "cod_enum_", sizeof("cod_enum_") - 1);
    out += sizeof("cod_enum_") - 1;
    for (size_t i = 0; i < up; ++i) {
        memcpy(out, "x2ex2ex2f", 9);
        out += 9;
    }
    for (const char *at = module + common; *at; ++at)
        out = cod_provider_escape_scope(out, (unsigned char)*at);
    /* A length delimiter keeps the type separate from generated member names. */
    snprintf(out, capacity - (size_t)(out - name), "_%zu_%s", type_length, type);
    return name;
}

static inline void cod_provider_enum_code(cod_provider_text *out, cod_provider_scalar backing, uint64_t code) {
    if (backing == cod_provider_i64 && code > INT64_MAX) {
        if (code == (UINT64_C(1) << 63)) cod_provider_print(out, "INT64_MIN");
        else cod_provider_print(out, "(-INT64_C(%" PRIu64 "))", UINT64_C(0) - code);
    } else {
        const char *macro = backing == cod_provider_i64 ? "INT64_C" : backing == cod_provider_u64 ? "UINT64_C"
            : backing == cod_provider_u32 ? "UINT32_C" : "UINT8_C";
        cod_provider_print(out, "%s(%" PRIu64 ")", macro, code);
    }
}

static inline void cod_provider_write_enum(cod_provider_text *header, cod_provider_text *stubs, const char *name,
                                           cod_provider_scalar backing, const cod_provider_enum_member *members,
                                           size_t count) {
    cod_provider_print(header, "typedef %s %s;\n", cod_provider_scalar_spelling(backing), name);
    for (size_t i = 0; i < count; ++i) {
        cod_provider_print(header, "#define %s_member_%s ", name, members[i].spelling);
        cod_provider_enum_code(header, backing, members[i].code);
        cod_provider_print(header, "\n");
    }
    cod_provider_print(header, "\n");
    cod_provider_print(stubs, "static int %s_valid(%s value) {\n    switch (value) {\n", name, name);
    for (size_t i = 0; i < count; ++i)
        if (members[i].canonical)
            cod_provider_print(stubs, "    case %s_member_%s:\n", name, members[i].spelling);
    cod_provider_print(stubs, "        return 1;\n    default:\n        return 0;\n    }\n}\n\n");
}

#endif
