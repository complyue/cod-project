#ifndef COD_NATIVE_PROVIDER_TEXT_H
#define COD_NATIVE_PROVIDER_TEXT_H

#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define COD_PROVIDER_HEADER_NAME "cod_providers.h"
#define COD_PROVIDER_STUBS_NAME "cod_providers.c"

enum { cod_provider_native_ok, cod_provider_native_failure, cod_provider_native_boundary_error };

typedef enum {
    cod_provider_u8, cod_provider_u32, cod_provider_u64, cod_provider_i64, cod_provider_f32, cod_provider_f64,
} cod_provider_scalar;

static inline const char *cod_provider_scalar_spelling(cod_provider_scalar scalar) {
    switch (scalar) {
    case cod_provider_u8: return "uint8_t";
    case cod_provider_u32: return "uint32_t";
    case cod_provider_u64: return "uint64_t";
    case cod_provider_i64: return "int64_t";
    case cod_provider_f32: return "float";
    case cod_provider_f64: return "double";
    }
    return NULL;
}

typedef struct {
    bool (*write)(void *context, const char *text, size_t length);
    void *context;
    bool failed;
} cod_provider_text;

typedef enum { cod_provider_argument_scalar, cod_provider_argument_handle, cod_provider_argument_view } cod_provider_argument_kind;

typedef struct {
    const char *type;
    cod_provider_argument_kind kind;
    bool boolean;
    uint64_t owner_role;
} cod_provider_argument;

typedef struct {
    const char *type;
    const char *name;
    bool boolean;
} cod_provider_field;

typedef struct {
    const char *name;
    const char *parent;
    const cod_provider_argument *inputs;
    size_t input_count;
    const char *failure;
    const cod_provider_field *fields;
    size_t field_count;
} cod_provider_constructor;

static inline void cod_provider_print(cod_provider_text *output, const char *format, ...) {
    if (output->failed) return;
    char local[256];
    va_list args, copy;
    va_start(args, format);
    va_copy(copy, args);
    const int length = vsnprintf(local, sizeof(local), format, args);
    va_end(args);
    if (length < 0) {
        output->failed = true;
    } else if ((size_t)length < sizeof(local)) {
        output->failed = !output->write(output->context, local, (size_t)length);
    } else {
        char *text = (char *)malloc((size_t)length + 1);
        output->failed = !text;
        if (text) {
            output->failed = vsnprintf(text, (size_t)length + 1, format, copy) != length ||
                             !output->write(output->context, text, (size_t)length);
            free(text);
        }
    }
    va_end(copy);
}

static inline void cod_provider_begin(cod_provider_text *header, cod_provider_text *stubs) {
    cod_provider_print(header,
        "#ifndef COD_GENERATED_PROVIDERS_H\n#define COD_GENERATED_PROVIDERS_H\n\n#include <stdint.h>\n\n"
        "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n"
        "typedef uint32_t cod_native_status;\nenum {\n"
        "    COD_NATIVE_OK = %d,\n    COD_NATIVE_FAILURE = %d,\n    COD_NATIVE_BOUNDARY_ERROR = %d\n};\n\n",
        cod_provider_native_ok, cod_provider_native_failure, cod_provider_native_boundary_error);
    cod_provider_print(stubs, "#include \"" COD_PROVIDER_HEADER_NAME "\"\n#include <stddef.h>\n\n");
}

static inline void cod_provider_end(cod_provider_text *header) {
    cod_provider_print(header, "#ifdef __cplusplus\n}\n#endif\n\n#endif\n");
}

static inline void cod_provider_borrow(cod_provider_text *header, cod_provider_text *stubs, const char *name) {
    cod_provider_print(header, "typedef struct cod_native_%s_borrow {\n"
        "    uint64_t payload;\n    uint64_t owner;\n} cod_native_%s_borrow;\n\n", name, name);
    cod_provider_print(stubs, "_Static_assert(sizeof(cod_native_%s_borrow) == 2 * sizeof(uint64_t) &&\n"
        "               offsetof(cod_native_%s_borrow, owner) == sizeof(uint64_t), \"invalid cod_native_%s_borrow layout\");\n",
        name, name, name);
}

static inline void cod_provider_constructor_signature(cod_provider_text *out, const cod_provider_constructor *shape,
                                                      bool implementation) {
    cod_provider_print(out, "cod_native_status cod_native_%s_construct%s(", shape->name, implementation ? "_impl" : "");
    if (shape->parent) {
        cod_provider_print(out, "%scod_native_%s_borrow %sowner, ",
                           implementation ? "" : "const ", shape->parent, implementation ? "" : "*");
    }
    for (size_t i = 0; i < shape->input_count; ++i) {
        const cod_provider_argument *input = &shape->inputs[i];
        if (input->kind == cod_provider_argument_handle) {
            cod_provider_print(out, "%scod_native_%s_borrow %sarg%zu, ", implementation ? "" : "const ",
                               input->type, implementation ? "" : "*", i);
        } else {
            cod_provider_print(out, "%s arg%zu, ", input->type, i);
        }
    }
    cod_provider_print(out, "uint64_t *resource");
    if (shape->failure) cod_provider_print(out, ", cod_native_%s_failure *error", shape->name);
    cod_provider_print(out, ")");
}

static inline void cod_provider_write_constructor(cod_provider_text *header, cod_provider_text *stubs,
                                                  const cod_provider_constructor *shape) {
    const char *name = shape->name;
    if (shape->failure) {
        cod_provider_print(header, "/* Payload of the checked %s exception. */\ntypedef struct cod_native_%s_failure {\n",
                           shape->failure, name);
        for (size_t i = 0; i < shape->field_count; ++i)
            cod_provider_print(header, "    %s %s;\n", shape->fields[i].type, shape->fields[i].name);
        if (!shape->field_count) cod_provider_print(header, "    uint8_t reserved;\n");
        cod_provider_print(header, "} cod_native_%s_failure;\n\n", name);
    }

    cod_provider_constructor_signature(header, shape, false);
    cod_provider_print(header, ";\n");
    if (!shape->parent) cod_provider_print(header, "void cod_native_%s_destroy(uint64_t resource);\n", name);
    cod_provider_constructor_signature(header, shape, true);
    cod_provider_print(header, ";\n");
    if (!shape->parent) cod_provider_print(header, "void cod_native_%s_destroy_impl(uint64_t resource);\n", name);
    cod_provider_print(header, "\n");

    cod_provider_constructor_signature(stubs, shape, false);
    cod_provider_print(stubs, " {\n    if (!resource");
    if (shape->parent) cod_provider_print(stubs, " || !owner");
    if (shape->failure) cod_provider_print(stubs, " || !error");
    for (size_t i = 0; i < shape->input_count; ++i) {
        const cod_provider_argument *input = &shape->inputs[i];
        if (input->kind == cod_provider_argument_handle) cod_provider_print(stubs, " || !arg%zu", i);
        else if (input->boolean) cod_provider_print(stubs, " || arg%zu > 1", i);
    }
    cod_provider_print(stubs, ") return COD_NATIVE_BOUNDARY_ERROR;\n");
    for (size_t i = 0; i < shape->input_count; ++i) {
        const cod_provider_argument *input = &shape->inputs[i];
        if (input->kind != cod_provider_argument_handle) continue;
        for (size_t j = 0; j < i; ++j) {
            const cod_provider_argument *earlier = &shape->inputs[j];
            if (earlier->kind == cod_provider_argument_handle && earlier->owner_role == input->owner_role) {
                cod_provider_print(stubs, "    if (arg%zu->owner != arg%zu->owner) return COD_NATIVE_BOUNDARY_ERROR;\n", j, i);
                break;
            }
        }
    }

    cod_provider_print(stubs, "    uint64_t acquired = 0;\n");
    if (shape->failure) cod_provider_print(stubs, "    cod_native_%s_failure reported = {0};\n", name);
    cod_provider_print(stubs, "    const cod_native_status status = cod_native_%s_construct_impl(", name);
    if (shape->parent) cod_provider_print(stubs, "*owner, ");
    for (size_t i = 0; i < shape->input_count; ++i)
        cod_provider_print(stubs, "%sarg%zu, ", shape->inputs[i].kind == cod_provider_argument_handle ? "*" : "", i);
    cod_provider_print(stubs, "&acquired%s);\n"
        "    if (status == COD_NATIVE_OK) {\n        *resource = acquired;\n        return status;\n    }\n",
        shape->failure ? ", &reported" : "");
    if (shape->failure) {
        cod_provider_print(stubs, "    if (status == COD_NATIVE_FAILURE) {\n");
        for (size_t i = 0; i < shape->field_count; ++i)
            if (shape->fields[i].boolean)
                cod_provider_print(stubs, "        if (reported.%s > 1) return COD_NATIVE_BOUNDARY_ERROR;\n", shape->fields[i].name);
        cod_provider_print(stubs, "        *error = reported;\n        return status;\n    }\n");
    }
    cod_provider_print(stubs, "    return COD_NATIVE_BOUNDARY_ERROR;\n}\n\n");
    if (!shape->parent) cod_provider_print(stubs, "void cod_native_%s_destroy(uint64_t resource) {\n"
        "    cod_native_%s_destroy_impl(resource);\n}\n\n", name, name);
}

#endif
