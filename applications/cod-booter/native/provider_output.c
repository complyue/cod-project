#define _POSIX_C_SOURCE 200809L

#include "provider_output.h"
#include "native/provider_names.h"
#include "native/provider_call.h"
#include "native/provider_enum.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

/* Module interface value and declaration kinds from NameResolution.cod. */
enum {
    bits_u64 = 8, result_void = 9, bits_bool = 11, handle_type = 12, record_type = 13,
    constructor = 14, operation = 15, entry = 16, bits_u8 = 17, bits_u32 = 18,
    bits_i64 = 19, bits_f32 = 20, bits_f64 = 21, array_type = 22, enum_type = 26,
    string_type = 29, c_string_type = 30,
};
typedef cod_native_graph_frame_use_row9 interface_row;

typedef struct {
    const interface_row *rows;
    size_t count;
    char **names;
    char **enum_names;
    const char *const *module_paths;
    size_t module_count;
    bool *borrowed;
    cod_provider_argument *inputs;
    cod_provider_field *fields;
    cod_provider_view *views;
    uint64_t *offsets;
    cod_provider_enum_member *members;
} provider_interfaces;

typedef struct {
    cod_provider_scalar scalar;
    uint64_t size;
    uint64_t alignment;
} scalar_layout;

static const scalar_layout *layout_for(uint8_t kind) {
    static const scalar_layout layouts[] = {
#define SCALAR(kind, scalar, type) [kind] = {scalar, sizeof(type), _Alignof(type)}
        SCALAR(bits_bool, cod_provider_u8, uint8_t),
        SCALAR(bits_u8, cod_provider_u8, uint8_t),
        SCALAR(bits_u32, cod_provider_u32, uint32_t),
        SCALAR(bits_u64, cod_provider_u64, uint64_t),
        SCALAR(bits_i64, cod_provider_i64, int64_t),
        SCALAR(bits_f32, cod_provider_f32, float),
        SCALAR(bits_f64, cod_provider_f64, double),
#undef SCALAR
    };
    if (kind >= sizeof(layouts) / sizeof(*layouts) || !layouts[kind].size) return NULL;
    return &layouts[kind];
}

static const char *scalar_spelling(uint8_t kind) {
    const scalar_layout *layout = layout_for(kind);
    return layout ? cod_provider_scalar_spelling(layout->scalar) : NULL;
}

static size_t next_interface(const interface_row *rows, size_t at) {
    return at + 1 + (size_t)rows[at].entry_count;
}

static uint32_t validate_rows(const interface_row *rows, size_t count, size_t spelling_count, size_t *name_bytes) {
    for (size_t i = 0; i < count;) {
        const interface_row *row = &rows[i];
        if (row->value_kind || row->entry_count > count - i - 1) return provider_output_invalid;
        if ((row->interface_kind == operation && row->has_here && row->entry_count < 2) ||
            (row->interface_kind == entry && row->owner_count > row->entry_count)) return provider_output_invalid;
        i = next_interface(rows, i);
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
    for (size_t i = 0; i < interfaces->count; i = next_interface(interfaces->rows, i))
        if (interfaces->rows[i].defining_module == module && interfaces->rows[i].declaration == declaration) return i;
    return interfaces->count;
}

static uint32_t declare_enum(const provider_interfaces *interfaces, const interface_row *ref,
                             cod_provider_text *header, cod_provider_text *stubs) {
    const size_t id = find_declaration(interfaces, ref->type_module, ref->type_identity);
    if (id == interfaces->count || !interfaces->names[id]) return provider_output_invalid;
    if (interfaces->enum_names[id]) return 0;
    const interface_row *domain = &interfaces->rows[id];
    const uint8_t kind = domain->element_kind;
    if (domain->interface_kind != enum_type || !domain->entry_count ||
        !domain->defining_module || domain->defining_module > interfaces->module_count ||
        (kind != bits_u8 && kind != bits_u32 && kind != bits_u64 && kind != bits_i64)) return provider_output_invalid;
    for (size_t i = 0; i < domain->entry_count; ++i) {
        const interface_row *member = domain + 1 + i;
        const char *spelling = interfaces->names[id + 1 + i];
        if (member->value_kind != enum_type || !spelling) return provider_output_invalid;
        bool canonical = true;
        for (size_t j = 0; j < i; ++j)
            if (domain[1 + j].declaration == member->declaration) {
                canonical = false;
                break;
            }
        interfaces->members[i] = (cod_provider_enum_member){spelling, member->enum_code, canonical};
    }
    char *name = cod_provider_enum_name(interfaces->module_paths[0],
        interfaces->module_paths[domain->defining_module - 1], interfaces->names[id]);
    if (!name) return provider_output_memory;
    interfaces->enum_names[id] = name;
    cod_provider_write_enum(header, stubs, name, layout_for(kind)->scalar, interfaces->members, (size_t)domain->entry_count);
    return 0;
}

static const char *enum_name(const provider_interfaces *interfaces, const interface_row *ref) {
    const size_t id = find_declaration(interfaces, ref->type_module, ref->type_identity);
    return id == interfaces->count ? NULL : interfaces->enum_names[id];
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

static uint32_t describe_argument(const provider_interfaces *interfaces, const interface_row *input,
                                  cod_provider_argument *argument, cod_provider_text *header, cod_provider_text *stubs) {
    *argument = (cod_provider_argument){.boolean = input->value_kind == bits_bool, .owner_role = input->owner_role};
    if (input->value_kind == handle_type) {
        argument->kind = cod_provider_argument_handle;
        return describe_borrow(interfaces, input, &argument->type, header, stubs);
    }
    if (input->value_kind == array_type) {
        argument->kind = cod_provider_argument_view;
        return 0;
    }
    if (input->value_kind == string_type || input->value_kind == c_string_type) {
        argument->kind = input->value_kind == string_type ? cod_provider_argument_text : cod_provider_argument_c_string;
        return 0;
    }
    argument->enum_value = input->value_kind == enum_type;
    argument->type = argument->enum_value ? enum_name(interfaces, input) : scalar_spelling(input->value_kind);
    return argument->type ? 0 : provider_output_invalid;
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
        const uint32_t status = describe_argument(interfaces, row + 1 + i, &interfaces->inputs[i], header, stubs);
        if (status) return status;
        if (interfaces->inputs[i].kind == cod_provider_argument_view || cod_provider_text_argument(&interfaces->inputs[i]))
            return provider_output_invalid;
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

/* The booter emits for its own host target. The generated C checks the
   calculated sizes, alignments and offsets against the consumer compiler. */
static uint32_t describe_product(const provider_interfaces *interfaces, const interface_row *array, cod_provider_row *row) {
    const size_t id = find_declaration(interfaces, array->type_module, array->type_identity);
    if (id == interfaces->count || interfaces->rows[id].interface_kind != record_type ||
        !interfaces->rows[id].entry_count) return provider_output_invalid;
    *row = (cod_provider_row){.fields = interfaces->fields, .offsets = interfaces->offsets,
        .count = (size_t)interfaces->rows[id].entry_count, .alignment = 1};
    for (size_t i = 0; i < row->count; ++i) {
        const size_t field = id + 1 + i;
        const uint8_t kind = interfaces->rows[field].value_kind;
        const scalar_layout *layout = layout_for(kind);
        if (kind == bits_bool || !layout || !interfaces->names[field]) return provider_output_invalid;
        if (!cod_provider_member_name(interfaces->names[field])) return provider_output_reserved_name;
        interfaces->fields[i] = (cod_provider_field){cod_provider_scalar_spelling(layout->scalar), interfaces->names[field], false};
        const uint64_t padding = (layout->alignment - row->size % layout->alignment) % layout->alignment;
        if (row->size > UINT64_MAX - padding - layout->size) return provider_output_invalid;
        interfaces->offsets[i] = row->size + padding;
        row->size = interfaces->offsets[i] + layout->size;
        if (row->alignment < layout->alignment) row->alignment = layout->alignment;
    }
    const uint64_t padding = (row->alignment - row->size % row->alignment) % row->alignment;
    if (row->size > UINT64_MAX - padding) return provider_output_invalid;
    row->size += padding;
    return 0;
}

static uint32_t write_view(const provider_interfaces *interfaces, const interface_row *input, const char *name,
                            size_t index, cod_provider_text *header, cod_provider_text *stubs) {
    const size_t id = find_declaration(interfaces, input->type_module, input->type_identity);
    if (input->value_kind != array_type || id == interfaces->count || interfaces->rows[id].interface_kind != array_type)
        return provider_output_invalid;
    const interface_row *array = &interfaces->rows[id];
    const uint8_t kind = array->element_kind;
    cod_provider_view view = {.scalar = scalar_spelling(kind), .read_only = input->target_readonly != 0};
    cod_provider_row layout = {0};
    if (kind == record_type) {
        const uint32_t status = describe_product(interfaces, array, &layout);
        if (status) return status;
    } else if (!view.scalar || kind == bits_bool || !view.read_only) {
        return provider_output_invalid;
    }
    cod_provider_write_view(header, stubs, name, index, view, &layout);
    interfaces->views[index] = view;
    return 0;
}

static uint32_t write_here_operation(const provider_interfaces *interfaces, size_t at,
                                     cod_provider_text *header, cod_provider_text *stubs) {
    const interface_row *row = &interfaces->rows[at];
    const char *name = interfaces->names[at];
    const char *handle;
    if (row[1].value_kind != handle_type ||
        (row->result_kind != bits_u8 && row->result_kind != enum_type)) return provider_output_invalid;
    const char *enum_type_name = row->result_kind == enum_type ? enum_name(interfaces, row) : NULL;
    if (row->result_kind == enum_type && !enum_type_name) return provider_output_invalid;
    uint32_t status = describe_borrow(interfaces, row + 1, &handle, header, stubs);
    if (status) return status;
    const size_t count = (size_t)row->entry_count - 1;
    for (size_t i = 0; i < count; ++i) {
        status = write_view(interfaces, row + 2 + i, name, i, header, stubs);
        if (status) return status;
    }
    cod_provider_write_here(header, stubs, name, handle, interfaces->views, count, enum_type_name);
    return 0;
}

static uint32_t write_callable(const provider_interfaces *interfaces, size_t at,
                               cod_provider_text *header, cod_provider_text *stubs) {
    const interface_row *row = &interfaces->rows[at];
    const bool inbound = row->interface_kind == entry;
    const bool enum_result = row->result_kind == enum_type;
    const cod_provider_callable shape = {.name = interfaces->names[at], .inputs = interfaces->inputs,
        .count = (size_t)row->entry_count,
        .result = enum_result ? enum_name(interfaces, row) : scalar_spelling(row->result_kind),
        .owner_count = (size_t)row->owner_count, .enum_result = enum_result};
    if ((!shape.result && row->result_kind != result_void) ||
        (!inbound && shape.result && row->result_kind != bits_u8 && !enum_result)) return provider_output_invalid;
    for (size_t i = 0; i < shape.count; ++i) {
        const interface_row *input = row + 1 + i;
        cod_provider_argument *argument = interfaces->inputs + i;
        uint32_t status = describe_argument(interfaces, input, argument, header, stubs);
        if (status) return status;
        if (argument->kind == cod_provider_argument_view) {
            if (inbound || !input->target_readonly) return provider_output_invalid;
            status = write_view(interfaces, input, shape.name, i, header, stubs);
            if (status) return status;
        }
        if (inbound && argument->kind == cod_provider_argument_handle && argument->owner_role >= shape.owner_count)
            return provider_output_invalid;
        if (cod_provider_text_argument(argument) && (!input->target_readonly || !input->owner_readonly ||
                (inbound && argument->owner_role >= shape.owner_count)))
            return provider_output_invalid;
    }
    if (inbound) cod_provider_write_entry(header, stubs, &shape);
    else cod_provider_write_call(header, stubs, &shape, interfaces->views);
    return 0;
}

static bool unique_name(const provider_interfaces *interfaces, size_t at) {
    const char *name = interfaces->names[at];
    for (size_t i = 0; i < at; i = next_interface(interfaces->rows, i))
        if (interfaces->rows[i].interface_kind == interfaces->rows[at].interface_kind &&
            strcmp(interfaces->names[i], name) == 0) return false;
    return true;
}

static uint32_t collect_borrows(const provider_interfaces *interfaces, size_t at,
                                 cod_provider_text *header, cod_provider_text *stubs, bool *any_borrow) {
    const interface_row *row = &interfaces->rows[at];
    if (!interfaces->names[at]) return provider_output_invalid;
    if (!unique_name(interfaces, at)) return provider_output_conflict;
    const char *handle;
    if (row->interface_kind == constructor && row->type_module) {
        const uint32_t status = describe_borrow(interfaces, row, &handle, header, stubs);
        if (status) return status;
        *any_borrow = true;
    }
    for (size_t i = 0; i < row->entry_count; ++i) {
        const interface_row *input = row + 1 + i;
        if (input->value_kind != handle_type) continue;
        const uint32_t status = describe_borrow(interfaces, input, &handle, header, stubs);
        if (status) return status;
        *any_borrow = true;
    }
    return 0;
}

static uint32_t write_interface(const provider_interfaces *interfaces, size_t at,
                                cod_provider_text *header, cod_provider_text *stubs) {
    const interface_row *row = &interfaces->rows[at];
    if (row->interface_kind == constructor) {
        cod_provider_constructor shape;
        const uint32_t status = describe_constructor(interfaces, at, &shape, header, stubs);
        if (status) return status;
        cod_provider_write_constructor(header, stubs, &shape);
        return 0;
    }
    if (row->has_here) return write_here_operation(interfaces, at, header, stubs);
    return write_callable(interfaces, at, header, stubs);
}

static uint32_t format_files(const provider_interfaces *interfaces, cod_provider_text *header, cod_provider_text *stubs) {
    cod_provider_begin(header, stubs);
    bool here = false, loans = false, incoming_text = false, bounded_output = false, c_string_output = false;
    for (size_t i = 0; i < interfaces->count; i = next_interface(interfaces->rows, i)) {
        const interface_row *row = &interfaces->rows[i];
        here |= row->interface_kind == operation && row->has_here;
        loans |= row->interface_kind == entry && row->owner_count;
        for (size_t j = 0; j < row->entry_count; ++j) {
            const uint8_t kind = row[1 + j].value_kind;
            if (kind == string_type || kind == c_string_type) {
                incoming_text |= row->interface_kind == entry;
                bounded_output |= row->interface_kind == operation && kind == string_type;
                c_string_output |= row->interface_kind == operation && kind == c_string_type;
            }
        }
    }
    if (here) cod_provider_here_preamble(header);
    if (loans) cod_provider_loan_preamble(header);
    cod_provider_text_preamble(header, stubs, incoming_text, bounded_output, c_string_output);

    static const uint8_t groups[] = {constructor, entry, operation};
    for (size_t group = 0; group < sizeof(groups) / sizeof(*groups); ++group) {
        for (size_t i = 0; i < interfaces->count; i = next_interface(interfaces->rows, i)) {
            const interface_row *row = &interfaces->rows[i];
            if (row->interface_kind != groups[group]) continue;
            for (size_t j = 0; j < row->entry_count; ++j) {
                if (row[1 + j].value_kind != enum_type) continue;
                const uint32_t status = declare_enum(interfaces, row + 1 + j, header, stubs);
                if (status) return status;
            }
            if (row->result_kind == enum_type) {
                const uint32_t status = declare_enum(interfaces, row, header, stubs);
                if (status) return status;
            }
        }
    }
    bool any_borrow = false;
    for (size_t group = 0; group < sizeof(groups) / sizeof(*groups); ++group) {
        for (size_t i = 0; i < interfaces->count; i = next_interface(interfaces->rows, i)) {
            if (interfaces->rows[i].interface_kind != groups[group]) continue;
            const uint32_t status = collect_borrows(interfaces, i, header, stubs, &any_borrow);
            if (status) return status;
        }
    }
    if (any_borrow) cod_provider_print(stubs, "\n");
    for (size_t group = 0; group < sizeof(groups) / sizeof(*groups); ++group) {
        for (size_t i = 0; i < interfaces->count; i = next_interface(interfaces->rows, i)) {
            if (interfaces->rows[i].interface_kind != groups[group]) continue;
            const uint32_t status = write_interface(interfaces, i, header, stubs);
            if (status) return status;
        }
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
                                    const unsigned char *spellings, size_t spelling_count,
                                    const char *const *module_paths, size_t module_count) {
    size_t name_bytes;
    uint32_t status = validate_rows(rows, row_count, spelling_count, &name_bytes);
    if (status) return status;
    const size_t capacity = row_count ? row_count : 1;
    provider_interfaces interfaces = {.rows = rows, .count = row_count,
        .module_paths = module_paths, .module_count = module_count,
        .names = calloc(capacity, sizeof(*interfaces.names)), .borrowed = calloc(capacity, sizeof(*interfaces.borrowed)),
        .enum_names = calloc(capacity, sizeof(*interfaces.enum_names)),
        .members = calloc(capacity, sizeof(*interfaces.members)),
        .inputs = calloc(capacity, sizeof(*interfaces.inputs)), .fields = calloc(capacity, sizeof(*interfaces.fields)),
        .views = calloc(capacity, sizeof(*interfaces.views)), .offsets = calloc(capacity, sizeof(*interfaces.offsets))};
    char *storage = malloc(name_bytes);
    if (interfaces.names && interfaces.enum_names && interfaces.members && interfaces.borrowed && interfaces.inputs && interfaces.fields &&
        interfaces.views && interfaces.offsets && storage) {
        copy_names(&interfaces, spellings, storage);
        status = write_files(directory, &interfaces);
    } else {
        status = provider_output_memory;
    }
    free(storage);
    if (interfaces.enum_names)
        for (size_t i = 0; i < row_count; ++i) free(interfaces.enum_names[i]);
    free(interfaces.members);
    free(interfaces.enum_names);
    free(interfaces.offsets);
    free(interfaces.views);
    free(interfaces.fields);
    free(interfaces.inputs);
    free(interfaces.borrowed);
    free(interfaces.names);
    return status;
}
