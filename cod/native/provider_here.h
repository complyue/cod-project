#ifndef COD_NATIVE_PROVIDER_HERE_H
#define COD_NATIVE_PROVIDER_HERE_H

#include "provider_text.h"
#include <inttypes.h>

enum { cod_provider_here_value, cod_provider_here_protected, cod_provider_here_invalid };

typedef struct {
    const char *scalar;
    bool read_only;
} cod_provider_view;

typedef struct {
    const cod_provider_field *fields;
    const uint64_t *offsets;
    size_t count;
    uint64_t size;
    uint64_t alignment;
} cod_provider_row;

static inline void cod_provider_here_preamble(cod_provider_text *header) {
    cod_provider_print(header, "typedef uint32_t cod_native_here_step;\nenum {\n"
        "    COD_NATIVE_HERE_VALUE = %d,\n    COD_NATIVE_HERE_PROTECTED = %d,\n"
        "    COD_NATIVE_HERE_INVALID = %d\n};\n\n",
        cod_provider_here_value, cod_provider_here_protected, cod_provider_here_invalid);
}

static inline void cod_provider_element(cod_provider_text *output, const char *name, size_t index, cod_provider_view view) {
    if (view.scalar) cod_provider_print(output, "%s", view.scalar);
    else cod_provider_print(output, "cod_native_%s_row%zu", name, index);
}

static inline void cod_provider_invalid_view(cod_provider_text *output, const char *site, const char *access,
                                             const char *name, size_t index, cod_provider_view view) {
    cod_provider_print(output, "%s%scount > (uint64_t)PTRDIFF_MAX / sizeof(*%s%sbase)"
        " || (%s%scount && (!%s%sbase || (uintptr_t)%s%sbase %% _Alignof(",
        site, access, site, access, site, access, site, access, site, access);
    cod_provider_element(output, name, index, view);
    cod_provider_print(output, ")))");
}

static inline void cod_provider_write_view(cod_provider_text *header, cod_provider_text *stubs, const char *name,
                                           size_t index, cod_provider_view view, const cod_provider_row *row) {
    if (!view.scalar) {
        cod_provider_print(header, "typedef struct cod_native_%s_row%zu {\n", name, index);
        for (size_t i = 0; i < row->count; ++i)
            cod_provider_print(header, "    %s %s;\n", row->fields[i].type, row->fields[i].name);
        cod_provider_print(header, "} cod_native_%s_row%zu;\n", name, index);
        cod_provider_print(stubs, "_Static_assert(sizeof(cod_native_%s_row%zu) == %" PRIu64
            " && _Alignof(cod_native_%s_row%zu) == %" PRIu64 ", \"unexpected native row layout\");\n",
            name, index, row->size, name, index, row->alignment);
        for (size_t i = 0; i < row->count; ++i)
            cod_provider_print(stubs, "_Static_assert(offsetof(cod_native_%s_row%zu, %s) == %" PRIu64
                ", \"unexpected native row offset\");\n", name, index, row->fields[i].name, row->offsets[i]);
    }
    cod_provider_print(header, "typedef struct cod_native_%s_view%zu {\n    %s",
                       name, index, view.read_only ? "const " : "");
    cod_provider_element(header, name, index, view);
    cod_provider_print(header, " *base;\n    uint64_t count;\n} cod_native_%s_view%zu;\n", name, index);
    cod_provider_print(stubs, "_Static_assert(sizeof(cod_native_%s_view%zu) == 2 * sizeof(uint64_t)"
        " && offsetof(cod_native_%s_view%zu, count) == sizeof(uint64_t), \"unexpected native view layout\");\n",
        name, index, name, index);
}

static inline void cod_provider_here_signature(cod_provider_text *output, const char *name, const char *handle) {
    cod_provider_print(output, "cod_native_status cod_native_%s_call(const cod_native_%s_borrow *handle, "
        "cod_native_%s_callback body, void *context, cod_native_here_step *completion, uint8_t *value)",
        name, handle, name);
}

static inline void cod_provider_write_here(cod_provider_text *header, cod_provider_text *stubs, const char *name,
                                           const char *handle, const cod_provider_view *views, size_t count) {
    cod_provider_print(header, "typedef struct cod_native_%s_arrays {\n", name);
    for (size_t i = 0; i < count; ++i)
        cod_provider_print(header, "    cod_native_%s_view%zu array%zu;\n", name, i, i);
    cod_provider_print(header, "} cod_native_%s_arrays;\n"
        "typedef cod_native_here_step (*cod_native_%s_callback)(void *context, const cod_native_%s_arrays *views, uint8_t *feedback);\n",
        name, name, name);
    for (size_t i = 0; i < count; ++i)
        cod_provider_print(stubs, "_Static_assert(offsetof(cod_native_%s_arrays, array%zu) == "
            "%zu * 2 * sizeof(uint64_t), \"unexpected native array offset\");\n", name, i, i);

    cod_provider_here_signature(header, name, handle);
    cod_provider_print(header, ";\n"
        "cod_native_status cod_native_%s_impl(cod_native_%s_borrow handle, cod_native_%s_callback run, void *loan, uint8_t *value);\n"
        "int cod_native_%s_validate_impl(cod_native_%s_borrow handle, const cod_native_%s_arrays *views);\n\n",
        name, handle, name, name, handle, name);

    cod_provider_print(stubs, "typedef struct cod_native_%s_state {\n    cod_native_%s_borrow handle;\n"
        "    cod_native_%s_callback body;\n    void *context;\n    cod_native_here_step completion;\n} cod_native_%s_state;\n\n"
        "static cod_native_here_step cod_native_%s_run(void *context, const cod_native_%s_arrays *views, uint8_t *feedback) {\n"
        "    cod_native_%s_state *state = context;\n"
        "    if (state->completion != COD_NATIVE_HERE_VALUE) return state->completion;\n"
        "    if (!views || !feedback) return state->completion = COD_NATIVE_HERE_INVALID;\n",
        name, handle, name, name, name, name, name);
    for (size_t i = 0; i < count; ++i) {
        char site[sizeof("views->array") + 3 * sizeof(size_t)];
        snprintf(site, sizeof(site), "views->array%zu", i);
        cod_provider_print(stubs, "    if (");
        cod_provider_invalid_view(stubs, site, ".", name, i, views[i]);
        cod_provider_print(stubs, ") return state->completion = COD_NATIVE_HERE_INVALID;\n");
    }
    cod_provider_print(stubs, "    if (!cod_native_%s_validate_impl(state->handle, views)) "
        "return state->completion = COD_NATIVE_HERE_INVALID;\n"
        "    return state->completion = state->body(state->context, views, feedback);\n}\n\n", name);
    cod_provider_here_signature(stubs, name, handle);
    cod_provider_print(stubs, " {\n"
        "    if (!handle || !body || !context || !completion || !value)\n"
        "        return COD_NATIVE_BOUNDARY_ERROR;\n"
        "    cod_native_%s_state state = {*handle, body, context, COD_NATIVE_HERE_VALUE};\n"
        "    uint8_t result = 0;\n"
        "    const cod_native_status status = cod_native_%s_impl(*handle, cod_native_%s_run, &state, &result);\n"
        "    *completion = state.completion;\n"
        "    if (state.completion == COD_NATIVE_HERE_VALUE && status == COD_NATIVE_OK) *value = result;\n"
        "    return state.completion == COD_NATIVE_HERE_INVALID ? COD_NATIVE_BOUNDARY_ERROR : status;\n}\n\n",
        name, name, name);
}

#endif
