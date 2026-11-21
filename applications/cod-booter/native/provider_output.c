#define _POSIX_C_SOURCE 200809L

#include "provider_output.h"
#include "native/provider_names.h"
#include "native/provider_text.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

/* Module interface value and declaration kinds from NameResolution.cod. */
enum {
    bits_u64 = 8, bits_bool = 11, handle_type = 12, record_type = 13,
    constructor = 14, operation = 15, entry = 16, bits_u8 = 17, bits_u32 = 18,
    bits_i64 = 19, bits_f32 = 20, bits_f64 = 21,
};
typedef cod_native_graph_frame_use_row9 interface_row;

typedef struct {
    const interface_row *rows;
    size_t count;
    char **names;
    bool *borrowed;
    cod_provider_argument *inputs;
    cod_provider_field *fields;
} provider_interfaces;

static const char *scalar_spelling(uint8_t kind) {
    switch (kind) {
    case bits_bool: case bits_u8: return cod_provider_scalar_spelling(cod_provider_u8);
    case bits_u32: return cod_provider_scalar_spelling(cod_provider_u32);
    case bits_u64: return cod_provider_scalar_spelling(cod_provider_u64);
    case bits_i64: return cod_provider_scalar_spelling(cod_provider_i64);
    case bits_f32: return cod_provider_scalar_spelling(cod_provider_f32);
    case bits_f64: return cod_provider_scalar_spelling(cod_provider_f64);
    default: return NULL;
    }
}

static uint32_t validate_rows(const interface_row *rows, size_t count, size_t spelling_count, size_t *name_bytes) {
    for (size_t i = 0; i < count;) {
        const interface_row *row = &rows[i];
        if (row->value_kind || row->entry_count > count - i - 1) return provider_output_invalid;
        if (row->interface_kind == operation || row->interface_kind == entry) return provider_output_unsupported;
        i += 1 + (size_t)row->entry_count;
    }
    *name_bytes = 1;
    for (size_t i = 0; i < count; ++i) {
        if (rows[i].name_past < rows[i].name_first || rows[i].name_past > spelling_count)
            return provider_output_invalid;
        const size_t width = (size_t)(rows[i].name_past - rows[i].name_first);
        if (!width) continue;
        if (width >= SIZE_MAX - *name_bytes) return provider_output_memory;
        *name_bytes += width + 1;
    }
    return 0;
}

static void copy_names(const provider_interfaces *interfaces, const unsigned char *spellings, char *storage) {
    for (size_t i = 0; i < interfaces->count; ++i) {
        const interface_row *row = &interfaces->rows[i];
        const size_t width = (size_t)(row->name_past - row->name_first);
        if (!width) continue;
        interfaces->names[i] = storage;
        memcpy(storage, spellings + row->name_first, width);
        storage[width] = 0;
        storage += width + 1;
    }
}

static size_t find_declaration(const provider_interfaces *interfaces, uint64_t module, uint64_t declaration) {
    for (size_t i = 0; i < interfaces->count; i += 1 + (size_t)interfaces->rows[i].entry_count)
        if (interfaces->rows[i].defining_module == module && interfaces->rows[i].declaration == declaration) return i;
    return interfaces->count;
}

static uint32_t describe_borrow(const provider_interfaces *interfaces, const interface_row *reference,
                                const char **type, cod_provider_text *header, cod_provider_text *stubs) {
    const size_t id = find_declaration(interfaces, reference->type_module, reference->type_identity);
    if (id == interfaces->count || !interfaces->names[id]) return provider_output_invalid;
    const uint8_t kind = interfaces->rows[id].interface_kind;
    if (kind != handle_type && kind != constructor) return provider_output_invalid;
    *type = interfaces->names[id];
    if (interfaces->borrowed[id]) return 0;
    for (size_t i = 0; i < interfaces->count; ++i)
        if (interfaces->borrowed[i] && strcmp(interfaces->names[i], *type) == 0) return provider_output_conflict;
    cod_provider_borrow(header, stubs, *type);
    interfaces->borrowed[id] = true;
    return 0;
}

static uint32_t describe_constructor(const provider_interfaces *interfaces, size_t at, cod_provider_constructor *shape,
                                     cod_provider_text *header, cod_provider_text *stubs) {
    const interface_row *row = &interfaces->rows[at];
    *shape = (cod_provider_constructor){.name = interfaces->names[at], .inputs = interfaces->inputs,
        .input_count = (size_t)row->entry_count, .fields = interfaces->fields};
    if (!shape->name) return provider_output_invalid;
    if (row->type_module) {
        const uint32_t status = describe_borrow(interfaces, row, &shape->parent, header, stubs);
        if (status) return status;
    }
    for (size_t i = 0; i < shape->input_count; ++i) {
        const interface_row *input = row + 1 + i;
        cod_provider_argument *argument = &interfaces->inputs[i];
        *argument = (cod_provider_argument){.borrowed = input->value_kind == handle_type,
            .boolean = input->value_kind == bits_bool, .owner_role = input->owner_role};
        if (argument->borrowed) {
            const uint32_t status = describe_borrow(interfaces, input, &argument->type, header, stubs);
            if (status) return status;
        } else {
            argument->type = scalar_spelling(input->value_kind);
            if (!argument->type) return provider_output_invalid;
        }
    }
    if (row->failure_module) {
        const size_t failure = find_declaration(interfaces, row->failure_module, row->failure_identity);
        if (failure == interfaces->count || !interfaces->names[failure] ||
            interfaces->rows[failure].interface_kind != record_type) return provider_output_invalid;
        shape->failure = interfaces->names[failure];
        shape->field_count = (size_t)interfaces->rows[failure].entry_count;
        for (size_t i = 0; i < shape->field_count; ++i) {
            const size_t field = failure + 1 + i;
            const uint8_t kind = interfaces->rows[field].value_kind;
            interfaces->fields[i] = (cod_provider_field){scalar_spelling(kind), interfaces->names[field], kind == bits_bool};
            if (!interfaces->fields[i].type || !interfaces->fields[i].name) return provider_output_invalid;
            if (!cod_provider_member_name(interfaces->fields[i].name)) return provider_output_reserved_name;
        }
    }
    return 0;
}

static uint32_t format_files(const provider_interfaces *interfaces, cod_provider_text *header, cod_provider_text *stubs) {
    cod_provider_begin(header, stubs);
    bool any_borrow = false;
    for (size_t i = 0; i < interfaces->count; i += 1 + (size_t)interfaces->rows[i].entry_count) {
        if (interfaces->rows[i].interface_kind != constructor) continue;
        const char *name = interfaces->names[i];
        if (!name) return provider_output_invalid;
        for (size_t j = 0; j < i; j += 1 + (size_t)interfaces->rows[j].entry_count)
            if (interfaces->rows[j].interface_kind == constructor && strcmp(interfaces->names[j], name) == 0)
                return provider_output_conflict;
        cod_provider_constructor shape;
        const uint32_t status = describe_constructor(interfaces, i, &shape, header, stubs);
        if (status) return status;
        any_borrow |= shape.parent != NULL;
        for (size_t j = 0; j < shape.input_count; ++j) any_borrow |= shape.inputs[j].borrowed;
    }
    if (any_borrow) cod_provider_print(stubs, "\n");
    for (size_t i = 0; i < interfaces->count; i += 1 + (size_t)interfaces->rows[i].entry_count) {
        if (interfaces->rows[i].interface_kind != constructor) continue;
        cod_provider_constructor shape;
        const uint32_t status = describe_constructor(interfaces, i, &shape, header, stubs);
        if (status) return status;
        cod_provider_write_constructor(header, stubs, &shape);
    }
    cod_provider_end(header);
    return header->failed || stubs->failed ? provider_output_io : 0;
}

static bool stream_write(void *context, const char *text, size_t length) {
    return fwrite(text, 1, length, context) == length;
}

static FILE *open_staging_file(char *path) {
    const int fd = mkstemp(path);
    if (fd < 0) return NULL;
    FILE *file = fdopen(fd, "w");
    if (!file) {
        close(fd);
        unlink(path);
    }
    return file;
}

static bool regular_output(const char *path, bool *exists) {
    struct stat state;
    *exists = lstat(path, &state) == 0;
    return *exists ? S_ISREG(state.st_mode) : errno == ENOENT;
}

static bool publish_pair(const char *header_path, const char *stubs_path, const char *header_stage,
                         const char *stubs_stage, char *backup) {
    bool exists;
    if (!regular_output(stubs_path, &exists) || !regular_output(header_path, &exists)) return false;
    if (exists) {
        const int fd = mkstemp(backup);
        if (fd < 0) return false;
        close(fd);
        if (unlink(backup) != 0 || link(header_path, backup) != 0) return false;
    }
    if (rename(header_stage, header_path) != 0) {
        if (exists) unlink(backup);
        return false;
    }
    if (rename(stubs_stage, stubs_path) != 0) {
        if (exists) {
            /* Leave the backup available if a new I/O error prevents restoration. */
            (void)rename(backup, header_path);
        } else {
            (void)unlink(header_path);
        }
        return false;
    }
    if (exists) unlink(backup);
    return true;
}

static uint32_t write_files(const char *directory, const provider_interfaces *interfaces) {
    const size_t length = strlen(directory);
    const size_t suffix = sizeof("/.cod_providers.previous.XXXXXX");
    if (length > SIZE_MAX / 5 - suffix) return provider_output_memory;
    const size_t width = length + suffix;
    char *paths = malloc(5 * width);
    if (!paths) return provider_output_memory;
    char *header_path = paths, *stubs_path = paths + width;
    char *header_stage = paths + 2 * width, *stubs_stage = paths + 3 * width;
    char *backup = paths + 4 * width;
    snprintf(header_path, width, "%s/" COD_PROVIDER_HEADER_NAME, directory);
    snprintf(stubs_path, width, "%s/" COD_PROVIDER_STUBS_NAME, directory);
    snprintf(header_stage, width, "%s/." COD_PROVIDER_HEADER_NAME ".XXXXXX", directory);
    snprintf(stubs_stage, width, "%s/." COD_PROVIDER_STUBS_NAME ".XXXXXX", directory);
    snprintf(backup, width, "%s/.cod_providers.previous.XXXXXX", directory);
    uint32_t status = provider_output_io;
    FILE *header = NULL, *stubs = NULL;
    const int directory_fd = (mkdir(directory, 0777) == 0 || errno == EEXIST)
        ? open(directory, O_RDONLY | O_DIRECTORY | O_CLOEXEC) : -1;
    if (directory_fd >= 0 && flock(directory_fd, LOCK_EX) == 0) {
        header = open_staging_file(header_stage);
        if (header) stubs = open_staging_file(stubs_stage);
        if (stubs) {
            cod_provider_text header_text = {stream_write, header, false};
            cod_provider_text stubs_text = {stream_write, stubs, false};
            status = format_files(interfaces, &header_text, &stubs_text);
        }
    }
    const bool header_staged = header != NULL, stubs_staged = stubs != NULL;
    if (header_staged && fclose(header) != 0 && !status) status = provider_output_io;
    if (stubs_staged && fclose(stubs) != 0 && !status) status = provider_output_io;
    if (!status && !publish_pair(header_path, stubs_path, header_stage, stubs_stage, backup)) status = provider_output_io;
    if (header_staged) unlink(header_stage);
    if (stubs_staged) unlink(stubs_stage);
    if (directory_fd >= 0) close(directory_fd);
    free(paths);
    return status;
}

uint32_t cod_booter_write_c_providers(const char *directory, const interface_row *rows, size_t row_count,
                                    const unsigned char *spellings, size_t spelling_count) {
    size_t name_bytes;
    uint32_t status = validate_rows(rows, row_count, spelling_count, &name_bytes);
    if (status) return status;
    const size_t capacity = row_count ? row_count : 1;
    provider_interfaces interfaces = {.rows = rows, .count = row_count,
        .names = calloc(capacity, sizeof(*interfaces.names)), .borrowed = calloc(capacity, sizeof(*interfaces.borrowed)),
        .inputs = calloc(capacity, sizeof(*interfaces.inputs)), .fields = calloc(capacity, sizeof(*interfaces.fields))};
    char *storage = malloc(name_bytes);
    if (interfaces.names && interfaces.borrowed && interfaces.inputs && interfaces.fields && storage) {
        copy_names(&interfaces, spellings, storage);
        status = write_files(directory, &interfaces);
    } else {
        status = provider_output_memory;
    }
    free(storage);
    free(interfaces.fields);
    free(interfaces.inputs);
    free(interfaces.borrowed);
    free(interfaces.names);
    return status;
}
