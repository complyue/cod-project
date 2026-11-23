#ifndef COD_NATIVE_PROVIDER_CALL_H
#define COD_NATIVE_PROVIDER_CALL_H

#include "provider_here.h"

typedef struct {
    const char *name;
    const cod_provider_argument *inputs;
    size_t count;
    const char *result;
    size_t owner_count;
    bool enum_result;
    bool boolean_result;
} cod_provider_callable;

static inline void cod_provider_loan_preamble(cod_provider_text *header) {
    cod_provider_print(header, "/* The supplying native activation keeps the lent backing live through the call. */\n"
        "typedef struct cod_native_loan {\n    const void *activation;\n    uint64_t owner;\n} cod_native_loan;\n\n");
}

static inline bool cod_provider_text_argument(const cod_provider_argument *input) {
    return input->kind == cod_provider_argument_text || input->kind == cod_provider_argument_c_string;
}

static inline void cod_provider_text_preamble(cod_provider_text *header, cod_provider_text *stubs,
                                             bool incoming, bool bounded_output, bool c_string_output) {
    if (!incoming && !bounded_output && !c_string_output) return;
    cod_provider_print(header, "typedef struct cod_native_text {\n"
        "    const uint8_t *bytes;\n    uint64_t size;\n    uint64_t extent;\n} cod_native_text;\n\n");
    if (incoming || c_string_output) cod_provider_print(stubs, "#include <stdlib.h>\n#include <string.h>\n");
    if (incoming) cod_provider_print(stubs, "#include <cod_utf8.h>\n");
    cod_provider_print(stubs, "\nstatic int cod_text_has_extent(const cod_native_text *text) {\n"
        "    return text && text->bytes && text->size <= text->extent && text->extent <= PTRDIFF_MAX &&\n"
        "           text->size < PTRDIFF_MAX;\n}\n\n");
    if (incoming) cod_provider_print(stubs,
        "static int cod_text_snapshot(const cod_native_text *input, int c_string, cod_native_text *output) {\n"
        "    if (!cod_text_has_extent(input)) return 0;\n"
        "    const size_t size = (size_t)input->size;\n"
        "    if (c_string && (input->extent == input->size || input->bytes[size] != 0)) return 0;\n"
        "    uint8_t *bytes = malloc(size + 1);\n"
        "    if (!bytes) return 0;\n"
        "    memcpy(bytes, input->bytes, size);\n    bytes[size] = 0;\n"
        "    if (!cod_utf8_valid(bytes, size) || (c_string && memchr(bytes, 0, size))) { free(bytes); return 0; }\n"
        "    *output = (cod_native_text){bytes, size, size + 1};\n    return 1;\n}\n\n");
    if (c_string_output) cod_provider_print(stubs,
        "static const char *cod_text_c_string(const cod_native_text *text, char **temporary) {\n"
        "    if (!cod_text_has_extent(text)) return NULL;\n"
        "    const size_t size = (size_t)text->size;\n"
        "    if (text->extent > text->size && text->bytes[size] == 0) return (const char *)text->bytes;\n"
        "    *temporary = malloc(size + 1);\n    if (!*temporary) return NULL;\n"
        "    memcpy(*temporary, text->bytes, size);\n    (*temporary)[size] = 0;\n    return *temporary;\n}\n\n");
}

static inline void cod_provider_callable_input(cod_provider_text *out, const cod_provider_callable *shape,
                                               size_t i, const char *pointer) {
    const cod_provider_argument *input = shape->inputs + i;
    const char *qualifier = *pointer ? "const " : "";
    switch (input->kind) {
    case cod_provider_argument_handle:
        cod_provider_print(out, "%scod_native_%s_borrow %sarg%zu",
            qualifier, input->type, pointer, i);
        break;
    case cod_provider_argument_view:
        cod_provider_print(out, "%scod_native_%s_view%zu %sarg%zu",
            qualifier, shape->name, i, pointer, i);
        break;
    case cod_provider_argument_scalar:
        cod_provider_print(out, "%s arg%zu", input->type, i);
        break;
    case cod_provider_argument_text:
        cod_provider_print(out, "const cod_native_text *arg%zu", i);
        break;
    case cod_provider_argument_c_string:
        cod_provider_print(out, *pointer ? "const cod_native_text *arg%zu" : "const char *arg%zu", i);
        break;
    }
}

static inline void cod_provider_call_signature(cod_provider_text *out, const cod_provider_callable *shape, bool implementation) {
    cod_provider_print(out, "cod_native_status cod_native_%s_%s(", shape->name, implementation ? "impl" : "call");
    for (size_t i = 0; i < shape->count; ++i) {
        if (i) cod_provider_print(out, ", ");
        cod_provider_callable_input(out, shape, i, implementation ? "" : "*");
    }
    if (shape->result) cod_provider_print(out, "%s%s *%s", shape->count ? ", " : "", shape->result,
                                          implementation && shape->boolean_result ? "result" : "value");
    else if (!shape->count) cod_provider_print(out, "void");
    cod_provider_print(out, ")");
}

static inline void cod_provider_write_call(cod_provider_text *header, cod_provider_text *stubs,
                                           const cod_provider_callable *shape, const cod_provider_view *views) {
    cod_provider_call_signature(header, shape, false);
    cod_provider_print(header, ";\n");
    cod_provider_call_signature(header, shape, true);
    cod_provider_print(header, ";\n\n");
    cod_provider_call_signature(stubs, shape, false);
    cod_provider_print(stubs, " {\n");
    if (shape->result) cod_provider_print(stubs, "    if (!value) return COD_NATIVE_BOUNDARY_ERROR;\n");
    for (size_t i = 0; i < shape->count; ++i) {
        const cod_provider_argument *input = shape->inputs + i;
        if (input->kind == cod_provider_argument_view) {
            char site[sizeof("arg") + 3 * sizeof(size_t)];
            snprintf(site, sizeof(site), "arg%zu", i);
            cod_provider_print(stubs, "    if (!%s || ", site);
            cod_provider_invalid_view(stubs, site, "->", shape->name, i, views[i]);
            cod_provider_print(stubs, ") return COD_NATIVE_BOUNDARY_ERROR;\n");
        } else if (input->kind == cod_provider_argument_handle) {
            cod_provider_print(stubs, "    if (!arg%zu", i);
            for (size_t j = 0; j < i; ++j) {
                const cod_provider_argument *prior = shape->inputs + j;
                if (prior->kind == cod_provider_argument_handle && prior->owner_role == input->owner_role) {
                    cod_provider_print(stubs, " || arg%zu->owner != arg%zu->owner", i, j);
                    break;
                }
            }
            cod_provider_print(stubs, ") return COD_NATIVE_BOUNDARY_ERROR;\n");
        } else if (input->kind == cod_provider_argument_text)
            cod_provider_print(stubs, "    if (!cod_text_has_extent(arg%zu)) return COD_NATIVE_BOUNDARY_ERROR;\n", i);
        else if (input->boolean) cod_provider_print(stubs, "    if (arg%zu > 1) return COD_NATIVE_BOUNDARY_ERROR;\n", i);
        else if (input->enum_value)
            cod_provider_print(stubs, "    if (!%s_valid(arg%zu)) return COD_NATIVE_BOUNDARY_ERROR;\n", input->type, i);
    }
    for (size_t i = 0; i < shape->count; ++i) if (shape->inputs[i].kind == cod_provider_argument_c_string) {
        cod_provider_print(stubs, "    char *text_backing%zu = NULL;\n"
            "    const char *text%zu = cod_text_c_string(arg%zu, &text_backing%zu);\n"
            "    if (!text%zu) {\n", i, i, i, i, i);
        for (size_t j = 0; j < i; ++j) if (shape->inputs[j].kind == cod_provider_argument_c_string)
            cod_provider_print(stubs, "        free(text_backing%zu);\n", j);
        cod_provider_print(stubs, "        return COD_NATIVE_BOUNDARY_ERROR;\n    }\n");
    }
    if (shape->result) cod_provider_print(stubs, "    %s result = 0;\n", shape->result);
    cod_provider_print(stubs, "    const cod_native_status status = cod_native_%s_impl(", shape->name);
    for (size_t i = 0; i < shape->count; ++i) {
        if (shape->inputs[i].kind == cod_provider_argument_c_string) cod_provider_print(stubs, "%stext%zu", i ? ", " : "", i);
        else cod_provider_print(stubs, "%s%sarg%zu", i ? ", " : "",
                               shape->inputs[i].kind == cod_provider_argument_scalar ||
                               shape->inputs[i].kind == cod_provider_argument_text ? "" : "*", i);
    }
    if (shape->result) cod_provider_print(stubs, "%s&result", shape->count ? ", " : "");
    cod_provider_print(stubs, ");\n");
    for (size_t i = 0; i < shape->count; ++i) if (shape->inputs[i].kind == cod_provider_argument_c_string)
        cod_provider_print(stubs, "    free(text_backing%zu);\n", i);
    cod_provider_print(stubs, "    if (status != COD_NATIVE_OK) return COD_NATIVE_BOUNDARY_ERROR;\n");
    if (shape->enum_result)
        cod_provider_print(stubs, "    if (!%s_valid(result)) return COD_NATIVE_BOUNDARY_ERROR;\n", shape->result);
    if (shape->boolean_result)
        cod_provider_print(stubs, "    if (result > 1) return COD_NATIVE_BOUNDARY_ERROR;\n");
    if (shape->result) cod_provider_print(stubs, "    *value = result;\n");
    cod_provider_print(stubs, "    return COD_NATIVE_OK;\n}\n\n");
}

static inline void cod_provider_entry_signature(cod_provider_text *out, const cod_provider_callable *shape, bool body) {
    cod_provider_print(out, "%s cod_native_%s_entry%s(", body ? "void" : "cod_native_status", shape->name, body ? "_body" : "");
    for (size_t i = 0; i < shape->count; ++i) {
        if (i) cod_provider_print(out, ", ");
        cod_provider_callable_input(out, shape, i, "* ");
    }
    for (size_t i = 0; i < shape->owner_count; ++i)
        cod_provider_print(out, "%s%s loan%zu", i || shape->count ? ", " : "",
                           body ? "const void *" : "const cod_native_loan *", i);
    const bool has_inputs = shape->count || shape->owner_count;
    if (shape->result) cod_provider_print(out, "%s%s * value", has_inputs ? ", " : "", shape->result);
    else if (!has_inputs) cod_provider_print(out, "void");
    cod_provider_print(out, ")");
}

static inline void cod_provider_write_entry(cod_provider_text *header, cod_provider_text *stubs,
                                            const cod_provider_callable *shape) {
    cod_provider_entry_signature(header, shape, false);
    cod_provider_print(header, ";\n\n");
    cod_provider_print(stubs, "extern ");
    cod_provider_entry_signature(stubs, shape, true);
    cod_provider_print(stubs, ";\n\n");
    cod_provider_entry_signature(stubs, shape, false);
    cod_provider_print(stubs, " {\n");
    if (shape->result) cod_provider_print(stubs, "    if (!value) return COD_NATIVE_BOUNDARY_ERROR;\n");
    for (size_t i = 0; i < shape->owner_count; ++i)
        cod_provider_print(stubs, "    if (!loan%zu || !loan%zu->activation) return COD_NATIVE_BOUNDARY_ERROR;\n", i, i);
    for (size_t i = 0; i < shape->count; ++i) {
        const cod_provider_argument *input = shape->inputs + i;
        if (input->kind == cod_provider_argument_handle)
            cod_provider_print(stubs, "    if (!arg%zu || arg%zu->owner != loan%" PRIu64 "->owner) return COD_NATIVE_BOUNDARY_ERROR;\n",
                               i, i, input->owner_role);
        else if (input->boolean) cod_provider_print(stubs, "    if (arg%zu > 1) return COD_NATIVE_BOUNDARY_ERROR;\n", i);
        else if (input->enum_value)
            cod_provider_print(stubs, "    if (!%s_valid(arg%zu)) return COD_NATIVE_BOUNDARY_ERROR;\n", input->type, i);
    }
    for (size_t i = 0; i < shape->count; ++i) if (cod_provider_text_argument(shape->inputs + i)) {
        cod_provider_print(stubs, "    cod_native_text text%zu;\n"
            "    if (!cod_text_snapshot(arg%zu, %u, &text%zu)) {\n", i, i,
            shape->inputs[i].kind == cod_provider_argument_c_string ? 1u : 0u, i);
        for (size_t j = 0; j < i; ++j) if (cod_provider_text_argument(shape->inputs + j))
            cod_provider_print(stubs, "        free((void *)text%zu.bytes);\n", j);
        cod_provider_print(stubs, "        return COD_NATIVE_BOUNDARY_ERROR;\n    }\n");
    }
    if (shape->enum_result) cod_provider_print(stubs, "    %s result = 0;\n", shape->result);
    cod_provider_print(stubs, "    cod_native_%s_entry_body(", shape->name);
    for (size_t i = 0; i < shape->count; ++i)
        cod_provider_print(stubs, cod_provider_text_argument(shape->inputs + i) ? "%s&text%zu" : "%sarg%zu", i ? ", " : "", i);
    for (size_t i = 0; i < shape->owner_count; ++i)
        cod_provider_print(stubs, "%sloan%zu->activation", i || shape->count ? ", " : "", i);
    if (shape->result) cod_provider_print(stubs, "%s%s", shape->count || shape->owner_count ? ", " : "",
                                          shape->enum_result ? "&result" : "value");
    cod_provider_print(stubs, ");\n");
    for (size_t i = 0; i < shape->count; ++i) if (cod_provider_text_argument(shape->inputs + i))
        cod_provider_print(stubs, "    free((void *)text%zu.bytes);\n", i);
    if (shape->enum_result)
        cod_provider_print(stubs, "    if (!%s_valid(result)) return COD_NATIVE_BOUNDARY_ERROR;\n    *value = result;\n", shape->result);
    cod_provider_print(stubs, "    return COD_NATIVE_OK;\n}\n\n");
}

#endif
