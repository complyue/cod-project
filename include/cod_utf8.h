#ifndef COD_UTF8_H
#define COD_UTF8_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static inline bool cod_unicode_scalar(uint32_t value) {
    return value <= 0x10ffffu && (value < 0xd800u || value > 0xdfffu);
}

// Returns the encoded width, or zero on malformed or truncated input.
static inline size_t cod_utf8_decode(const uint8_t *bytes, size_t size, uint32_t *result) {
    if (!bytes || !size) return 0;
    const uint8_t lead = bytes[0];
    uint32_t value = lead;
    uint32_t minimum = 0;
    size_t width = 1;
    if (lead >= 0xc2u && lead <= 0xdfu) {
        width = 2;
        minimum = 0x80u;
        value &= 0x1fu;
    } else if (lead >= 0xe0u && lead <= 0xefu) {
        width = 3;
        minimum = 0x800u;
        value &= 0x0fu;
    } else if (lead >= 0xf0u && lead <= 0xf4u) {
        width = 4;
        minimum = 0x10000u;
        value &= 0x07u;
    } else if (lead >= 0x80u) return 0;
    if (size < width) return 0;
    for (size_t i = 1; i < width; ++i) {
        const uint8_t tail = bytes[i];
        if ((tail & 0xc0u) != 0x80u) return 0;
        value = (value << 6) | (tail & 0x3fu);
    }
    if (value < minimum || !cod_unicode_scalar(value)) return 0;
    if (result) *result = value;
    return width;
}

static inline bool cod_utf8_valid(const uint8_t *bytes, size_t size) {
    if (!bytes) return false;
    for (size_t consumed = 0; consumed < size;) {
        const size_t width = cod_utf8_decode(bytes + consumed, size - consumed, NULL);
        if (!width) return false;
        consumed += width;
    }
    return true;
}

// The output has four accessible bytes. The return value excludes any terminator.
static inline size_t cod_utf8_encode(uint32_t value, uint8_t *bytes) {
    if (!cod_unicode_scalar(value)) return 0;
    const size_t width = value < 0x80u ? 1 : value < 0x800u ? 2 : value < 0x10000u ? 3 : 4;
    if (width == 1) {
        bytes[0] = (uint8_t)value;
        return 1;
    }
    for (size_t i = width; i > 1; --i) {
        bytes[i - 1] = (uint8_t)(0x80u | (value & 0x3fu));
        value >>= 6;
    }
    static const uint8_t lead[] = {0, 0, 0xc0u, 0xe0u, 0xf0u};
    bytes[0] = (uint8_t)(lead[width] | value);
    return width;
}

#endif
