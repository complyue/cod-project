#ifndef COD_NATIVE_PROVIDER_CALL_H
#define COD_NATIVE_PROVIDER_CALL_H

#include "provider_here.h"

typedef struct {
    const char *name;
    const cod_provider_argument *inputs;
    size_t count;
    const char *result;
    size_t owner_count;
} cod_provider_callable;

static inline void cod_provider_loan_preamble(cod_provider_text *header) {
    cod_provider_print(header, "/* The supplying native activation keeps the lent backing live through the call. */\n"
        "typedef struct cod_native_loan {\n    const void *activation;\n    uint64_t owner;\n} cod_native_loan;\n\n");
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
    }
}

static inline void cod_provider_call_signature(cod_provider_text *out, const cod_provider_callable *shape, bool implementation) {
    cod_provider_print(out, "cod_native_status cod_native_%s_%s(", shape->name, implementation ? "impl" : "call");
    for (size_t i = 0; i < shape->count; ++i) {
        if (i) cod_provider_print(out, ", ");
        cod_provider_callable_input(out, shape, i, implementation ? "" : "*");
    }
    if (shape->result) cod_provider_print(out, "%s%s *value", shape->count ? ", " : "", shape->result);
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
        } else if (input->boolean) cod_provider_print(stubs, "    if (arg%zu > 1) return COD_NATIVE_BOUNDARY_ERROR;\n", i);
    }
    if (shape->result) cod_provider_print(stubs, "    %s result = 0;\n", shape->result);
    cod_provider_print(stubs, "    const cod_native_status status = cod_native_%s_impl(", shape->name);
    for (size_t i = 0; i < shape->count; ++i)
        cod_provider_print(stubs, "%s%sarg%zu", i ? ", " : "",
                           shape->inputs[i].kind == cod_provider_argument_scalar ? "" : "*", i);
    if (shape->result) cod_provider_print(stubs, "%s&result", shape->count ? ", " : "");
    cod_provider_print(stubs, ");\n    if (status != COD_NATIVE_OK) return COD_NATIVE_BOUNDARY_ERROR;\n");
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
    }
    cod_provider_print(stubs, "    cod_native_%s_entry_body(", shape->name);
    for (size_t i = 0; i < shape->count; ++i) cod_provider_print(stubs, "%sarg%zu", i ? ", " : "", i);
    for (size_t i = 0; i < shape->owner_count; ++i)
        cod_provider_print(stubs, "%sloan%zu->activation", i || shape->count ? ", " : "", i);
    if (shape->result) cod_provider_print(stubs, "%svalue", shape->count || shape->owner_count ? ", " : "");
    cod_provider_print(stubs, ");\n    return COD_NATIVE_OK;\n}\n\n");
}

#endif
