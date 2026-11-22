#define _XOPEN_SOURCE 700

#include "cod_providers.h"
#include "cod_region_storage.h"
#include "lang_intrinsic_catalog.h"
#include "lang_llvm_commands_c.h"
#include "module_graph.h"
#include "provider_output.h"

#include <errno.h>
#include <stdbool.h>
#include <string.h>

enum { declaration_arrays = 7, frame_arrays = 11, max_active_modules = 128 };
enum { invalid_boundary = 70 };

typedef cod_enum_CompilerStatusx2ecod_14_CompilerStatus compiler_status;
enum {
    complete = cod_enum_CompilerStatusx2ecod_14_CompilerStatus_member_Complete,
    scratch_failed = cod_enum_CompilerStatusx2ecod_14_CompilerStatus_member_ScratchFailed,
    source_open_failed = cod_enum_CompilerStatusx2ecod_14_CompilerStatus_member_SourceOpenFailed,
    form_unsupported = cod_enum_CompilerStatusx2ecod_14_CompilerStatus_member_FormUnsupported,
};

typedef cod_native_graph_frame_use_row5 export_row;
typedef cod_native_graph_frame_use_row9 interface_row;

#define CATALOG_FIELD(field) \
    _Static_assert(offsetof(export_row, field) == offsetof(cod_lang_intrinsic_export, field), "graph catalog row offset")
_Static_assert(sizeof(export_row) == sizeof(cod_lang_intrinsic_export), "graph catalog row size");
CATALOG_FIELD(module_first);
CATALOG_FIELD(module_past);
CATALOG_FIELD(name_first);
CATALOG_FIELD(name_past);
CATALOG_FIELD(identity);
CATALOG_FIELD(kind);
CATALOG_FIELD(checked);
CATALOG_FIELD(publishing_module);
CATALOG_FIELD(origin_module);
CATALOG_FIELD(owner_module);
CATALOG_FIELD(owner_identity);
CATALOG_FIELD(shape);
#undef CATALOG_FIELD

typedef struct {
    char *path;
    bool checked;
    cod_llvm_command_unit unit;
    unsigned char *literal_bytes;
} loaded_module;

typedef struct {
    uint64_t site, first, past;
    char *path;
} dependency;

typedef struct {
    loaded_module *modules;
    size_t module_count, module_capacity, active_count;
    unsigned char *spellings;
    size_t spelling_count, spelling_capacity;
    export_row *exports;
    size_t export_count, export_capacity;
    interface_row *interfaces;
    size_t interface_count, interface_capacity;
    size_t function_count;
    bool native, collected;
} module_graph;

typedef struct {
    module_graph *graph;
    size_t module;
    const char *input;
    int path_error;
    dependency *edges;
    size_t edge_count, edge_capacity;
    cod_region_storage scratch;
    cod_region_extent extents[frame_arrays];
    size_t tokens, nodes, commands;
    cod_native_graph_frame_use_row6 progress;
    bool views_live;
} module_frame;

static const struct {
    size_t size, alignment;
} frame_layout[frame_arrays] = {
#define ROW(i) {sizeof(cod_native_graph_frame_use_row##i), _Alignof(cod_native_graph_frame_use_row##i)}
    ROW(0), ROW(1), ROW(2), ROW(3), ROW(4), ROW(5), ROW(9), ROW(11), ROW(12), ROW(13), ROW(14),
#undef ROW
};

_Static_assert(sizeof(cod_native_graph_frame_use_row14) == sizeof(cod_llvm_command), "graph command row size");
#define COMMAND_FIELD(field) \
    _Static_assert(offsetof(cod_native_graph_frame_use_row14, field) == offsetof(cod_llvm_command, field), \
                   "graph command row offset")
COMMAND_FIELD(opcode);
COMMAND_FIELD(a);
COMMAND_FIELD(b);
COMMAND_FIELD(c);
COMMAND_FIELD(d);
#undef COMMAND_FIELD

static void *reserve(void *storage, size_t *capacity, size_t needed, size_t width) {
    if (needed <= *capacity) return storage;
    const size_t limit = PTRDIFF_MAX / width;
    if (needed > limit) return NULL;
    size_t next = *capacity ? *capacity : 8;
    while (next < needed && next <= limit / 2) next *= 2;
    if (next < needed || next > limit) next = needed;
    void *grown = realloc(storage, next * width);
    if (grown) *capacity = next;
    return grown;
}

static module_frame *frame_from_handle(cod_native_ModuleFrame_borrow handle) {
    return handle.payload == handle.owner ? (module_frame *)(uintptr_t)handle.payload : NULL;
}

static void *frame_array(module_frame *frame, size_t index) {
    return cod_region_storage_address(&frame->scratch, frame->extents[index]);
}

cod_native_status cod_native_prepare_graph_frame_impl(cod_native_ModuleFrame_borrow handle, uint64_t tokens,
                                                     uint64_t nodes, uint64_t commands, uint8_t *status) {
    module_frame *frame = frame_from_handle(handle);
    if (!frame || frame->scratch.bytes) return COD_NATIVE_BOUNDARY_ERROR;
    loaded_module *module = &frame->graph->modules[frame->module];
    if (!module->path) {
        /* The root's SourceRevision has already been acquired by CoD. */
        module->path = realpath(frame->input, NULL);
        if (!module->path) {
            frame->path_error = errno;
            *status = errno == ENOMEM ? COD_REGION_STORAGE_NO_MEMORY : COD_REGION_STORAGE_INVALID;
            return COD_NATIVE_OK;
        }
    }
    frame->tokens = (size_t)tokens;
    frame->nodes = (size_t)nodes;
    frame->commands = frame->graph->native ? (size_t)commands : 0;
    if ((uint64_t)frame->tokens != tokens || (uint64_t)frame->nodes != nodes ||
        (frame->graph->native && (uint64_t)frame->commands != commands)) {
        *status = COD_REGION_STORAGE_TOO_LARGE;
        return COD_NATIVE_OK;
    }
    size_t capacity = 0;
    const size_t arrays = frame->graph->native ? frame_arrays : declaration_arrays;
    for (size_t i = 0; i < arrays; ++i) {
        const size_t count = i == 0 ? frame->tokens : i == frame_arrays - 1 ? frame->commands : frame->nodes;
        *status = cod_region_layout_reserve(&capacity, PTRDIFF_MAX, count,
                                            frame_layout[i].size, frame_layout[i].alignment, &frame->extents[i]);
        if (*status != COD_REGION_STORAGE_OK) return COD_NATIVE_OK;
    }
    *status = cod_region_storage_open_layout(&frame->scratch, capacity);
    if (*status == COD_REGION_STORAGE_OK) memset(frame->scratch.bytes, 0, capacity);
    return COD_NATIVE_OK;
}

static cod_native_graph_frame_use_arrays frame_views(module_frame *frame) {
    module_graph *graph = frame->graph;
    return (cod_native_graph_frame_use_arrays){
        {frame_array(frame, 0), frame->tokens},
#define ARRAY(i) {frame_array(frame, i), frame->nodes}
        ARRAY(1), ARRAY(2), ARRAY(3), ARRAY(4), ARRAY(5),
#undef ARRAY
        {&frame->progress, 1},
        {graph->spellings, graph->spelling_count},
        {(const void *)graph->exports, graph->export_count},
        {frame_array(frame, 6), frame->nodes},
        {(const void *)graph->interfaces, graph->interface_count},
#define NATIVE(i, count) {graph->native ? frame_array(frame, i) : NULL, graph->native ? (count) : 0}
        NATIVE(7, frame->nodes), NATIVE(8, frame->nodes), NATIVE(9, frame->nodes), NATIVE(10, frame->commands),
#undef NATIVE
    };
}

int cod_native_graph_frame_use_validate_impl(cod_native_ModuleFrame_borrow handle,
                                             const cod_native_graph_frame_use_arrays *offered) {
    module_frame *frame = frame_from_handle(handle);
    if (!frame || !frame->views_live) return 0;
    const cod_native_graph_frame_use_arrays expected = frame_views(frame);
#define MATCH(i) (offered->array##i.base == expected.array##i.base && offered->array##i.count == expected.array##i.count)
    return MATCH(0) && MATCH(1) && MATCH(2) && MATCH(3) && MATCH(4) &&
           MATCH(5) && MATCH(6) && MATCH(7) && MATCH(8) && MATCH(9) && MATCH(10) &&
           MATCH(11) && MATCH(12) && MATCH(13) && MATCH(14);
#undef MATCH
}

cod_native_status cod_native_graph_frame_use_impl(cod_native_ModuleFrame_borrow handle,
                                                 cod_native_graph_frame_use_callback run, void *loan, compiler_status *value) {
    module_frame *frame = frame_from_handle(handle);
    if (!frame || !frame->scratch.bytes || frame->views_live) return COD_NATIVE_BOUNDARY_ERROR;
    const cod_native_graph_frame_use_arrays views = frame_views(frame);
    frame->views_live = true;
    const cod_native_here_step result = run(loan, &views, value);
    frame->views_live = false;
    return result == COD_NATIVE_HERE_INVALID ? COD_NATIVE_BOUNDARY_ERROR : COD_NATIVE_OK;
}

/* The parser supplied a quoted path span in the live requesting revision. */
static char *dependency_path(const char *receiver, const uint8_t *bytes, size_t width) {
    if (width < 3 || bytes[0] != '"' || bytes[width - 1] != '"') {
        errno = EINVAL;
        return NULL;
    }
    const size_t directory = bytes[1] == '/' ? 0 : (size_t)(strrchr(receiver, '/') - receiver) + 1;
    if (width > PTRDIFF_MAX - directory) {
        errno = ENOMEM;
        return NULL;
    }
    char *name = malloc(directory + width);
    if (!name) return NULL;
    memcpy(name, receiver, directory);
    size_t size = directory;
    for (size_t at = 1; at < width - 1; ++at) {
        if (bytes[at] == '\\') {
            ++at;
            if (at >= width - 1 || (bytes[at] != '\\' && bytes[at] != '"')) {
                free(name);
                errno = EINVAL;
                return NULL;
            }
        }
        if (bytes[at] == 0) {
            free(name);
            errno = EINVAL;
            return NULL;
        }
        name[size++] = (char)bytes[at];
    }
    name[size] = 0;
    char *path = realpath(name, NULL);
    const int error = errno;
    free(name);
    errno = error;
    return path;
}

cod_native_status cod_native_remember_graph_dependency_impl(cod_native_ModuleFrame_borrow handle,
    cod_native_remember_graph_dependency_view1 source, uint64_t site, uint64_t first, uint64_t past, uint8_t *status) {
    module_frame *frame = frame_from_handle(handle);
    if (!frame || !frame->views_live || site >= frame->progress.used || first >= past || past > source.count)
        return COD_NATIVE_BOUNDARY_ERROR;
    char *path = dependency_path(frame->graph->modules[frame->module].path, source.base + first, (size_t)(past - first));
    if (!path) {
        *status = errno == ENOMEM ? module_graph_storage_failed : module_source_unavailable;
        return COD_NATIVE_OK;
    }
    dependency *edges = reserve(frame->edges, &frame->edge_capacity, frame->edge_count + 1, sizeof(*edges));
    if (!edges) {
        free(path);
        *status = module_graph_storage_failed;
        return COD_NATIVE_OK;
    }
    frame->edges = edges;
    frame->edges[frame->edge_count++] = (dependency){site, first, past, path};
    *status = 0;
    return COD_NATIVE_OK;
}

static size_t find_module(const module_graph *graph, const char *path) {
    for (size_t i = 0; i < graph->module_count; ++i)
        if (strcmp(graph->modules[i].path, path) == 0) return i;
    return graph->module_count;
}

static uint8_t enter_module(module_graph *graph, const char *input_path, char *path, cod_booter_diagnostic *diagnostic) {
    const size_t module = graph->module_count;
    loaded_module *modules = reserve(graph->modules, &graph->module_capacity, module + 1, sizeof(*modules));
    if (!modules) {
        free(path);
        diagnostic->stage = 7;
        diagnostic->code = COD_REGION_STORAGE_NO_MEMORY;
        return scratch_failed;
    }
    graph->modules = modules;
    graph->modules[graph->module_count++] = (loaded_module){.path = path};
    module_frame frame = {
        .graph = graph, .module = module, .input = input_path,
        .progress = {.identity = module + 1, .native = graph->native, .entry = module == 0},
    };
    const cod_native_loan paths = {input_path, (uint64_t)(uintptr_t)input_path};
    const cod_native_loan work = {&frame, (uint64_t)(uintptr_t)&frame};
    const cod_native_loan destination = {diagnostic, (uint64_t)(uintptr_t)diagnostic};
    const cod_native_Path_borrow input = {paths.owner, paths.owner};
    const cod_native_ModuleFrame_borrow module_work = {work.owner, work.owner};
    const cod_native_DiagnosticDestination_borrow result = {destination.owner, destination.owner};
    compiler_status status;
    ++graph->active_count;
    const cod_native_status entered =
        cod_native_check_graph_module_entry(&input, &module_work, &result, &paths, &work, &destination, &status);
    --graph->active_count;
    for (size_t i = 0; i < frame.edge_count; ++i) free(frame.edges[i].path);
    free(frame.edges);
    cod_region_storage_release(&frame.scratch);
    if (entered == COD_NATIVE_OK && frame.path_error && frame.path_error != ENOMEM) {
        *diagnostic = (cod_booter_diagnostic){.os_error = (uint64_t)frame.path_error};
        return source_open_failed;
    }
    graph->modules[module].checked = entered == COD_NATIVE_OK && status == complete;
    return entered == COD_NATIVE_OK ? status : invalid_boundary;
}

cod_native_status cod_native_follow_graph_dependencies_impl(cod_native_ModuleFrame_borrow handle, uint8_t *status) {
    module_frame *frame = frame_from_handle(handle);
    if (!frame || !frame->scratch.bytes || frame->views_live) return COD_NATIVE_BOUNDARY_ERROR;
    module_graph *graph = frame->graph;
    cod_native_graph_frame_use_row2 *selections = frame_array(frame, 2);
    *status = 0;
    for (size_t i = 0; i < frame->edge_count; ++i) {
        dependency *edge = &frame->edges[i];
        frame->progress.failed_first = edge->first;
        frame->progress.failed_past = edge->past;
        const size_t target = find_module(graph, edge->path);
        if (target != graph->module_count) {
            if (!graph->modules[target].checked) {
                *status = module_cycle_unproved;
                return COD_NATIVE_OK;
            }
        } else {
            if (graph->active_count == max_active_modules) {
                *status = module_depth_unproved;
                return COD_NATIVE_OK;
            }
            /* The child records locations in its own revision. The parent's
               public diagnostic stays at its requesting path span. */
            cod_booter_diagnostic child_diagnostic = {0};
            char *path = edge->path;
            edge->path = NULL;
            const uint8_t entered = enter_module(graph, path, path, &child_diagnostic);
            if (entered) {
                if (entered == form_unsupported) {
                    *status = module_dependency_unsupported;
                    return COD_NATIVE_OK;
                }
                *status = child_diagnostic.stage == 2 &&
                          (child_diagnostic.code == module_cycle_unproved || child_diagnostic.code == module_depth_unproved)
                        ? (uint8_t)child_diagnostic.code : module_dependency_rejected;
                return COD_NATIVE_OK;
            }
        }
        selections[edge->site].dependency_module = target + 1;
    }
    if (graph->native) {
        const uint64_t width = frame->progress.used;
        if (!width || width > SIZE_MAX - graph->function_count) {
            *status = module_graph_storage_failed;
            return COD_NATIVE_OK;
        }
        frame->progress.function_base = graph->function_count;
        graph->function_count += (size_t)width;
    }
    return COD_NATIVE_OK;
}

static cod_region_storage_status stage_native_unit(module_frame *frame,
    cod_native_commit_graph_exports_view1 source) {
    const size_t count = (size_t)frame->progress.command_count;
    if (!count) return COD_REGION_STORAGE_OK;
    if (frame->progress.command_count > frame->commands ||
        (uint64_t)(size_t)frame->progress.labels != frame->progress.labels ||
        (uint64_t)(size_t)frame->progress.slots != frame->progress.slots ||
        (uint64_t)(size_t)source.count != source.count ||
        count > PTRDIFF_MAX / sizeof(cod_llvm_command)) return COD_REGION_STORAGE_TOO_LARGE;
    cod_llvm_command *commands = malloc(count * sizeof(*commands));
    if (!commands) return COD_REGION_STORAGE_NO_MEMORY;
    memcpy(commands, frame_array(frame, 10), count * sizeof(*commands));
    size_t literal_count = 0;
    size_t literal_bytes = 0;
    for (size_t i = 0; i < count; ++i) {
        const cod_llvm_command *command = &commands[i];
        if (command->opcode != COD_LLVM_TEXT_LITERAL) continue;
        if (command->b > command->c || command->c > source.count ||
            command->c - command->b < 2 || command->c - command->b > PTRDIFF_MAX - literal_bytes) {
            free(commands);
            return COD_REGION_STORAGE_INVALID;
        }
        ++literal_count;
        literal_bytes += (size_t)(command->c - command->b);
    }
    cod_llvm_literal_spelling *literals = literal_count ? calloc(literal_count, sizeof(*literals)) : NULL;
    unsigned char *bytes = literal_bytes ? malloc(literal_bytes) : NULL;
    if (literal_count && (!literals || !bytes)) {
        free(bytes);
        free(literals);
        free(commands);
        return COD_REGION_STORAGE_NO_MEMORY;
    }
    size_t index = 0;
    size_t offset = 0;
    for (size_t i = 0; i < count; ++i) {
        cod_llvm_command *command = &commands[i];
        if (command->opcode != COD_LLVM_TEXT_LITERAL) continue;
        const size_t width = (size_t)(command->c - command->b);
        memcpy(bytes + offset, source.base + command->b, width);
        literals[index++] = (cod_llvm_literal_spelling){bytes + offset, width};
        command->d = index;
        offset += width;
    }
    loaded_module *module = &frame->graph->modules[frame->module];
    module->literal_bytes = bytes;
    module->unit = (cod_llvm_command_unit){
        .records = commands, .count = count,
        .labels = (size_t)frame->progress.labels, .slots = (size_t)frame->progress.slots,
        .source_size = (size_t)source.count, .source_name = module->path,
        .literals = literals, .literal_count = literal_count,
    };
    return COD_REGION_STORAGE_OK;
}

static void append_spelling(module_graph *graph, const uint8_t *source, uint64_t *first, uint64_t *past) {
    const size_t width = (size_t)(*past - *first);
    if (width == 0) return;
    memcpy(graph->spellings + graph->spelling_count, source + *first, width);
    *first = graph->spelling_count;
    graph->spelling_count += width;
    *past = graph->spelling_count;
}

cod_native_status cod_native_commit_graph_exports_impl(cod_native_ModuleFrame_borrow handle,
    cod_native_commit_graph_exports_view1 source, uint8_t *status) {
    module_frame *frame = frame_from_handle(handle);
    if (!frame || frame->views_live || frame->progress.count > frame->nodes ||
        frame->progress.interface_count > frame->nodes) return COD_NATIVE_BOUNDARY_ERROR;
    const export_row *staged = frame_array(frame, 5);
    const interface_row *staged_interfaces = frame_array(frame, 6);
    module_graph *graph = frame->graph;
    size_t spelling_end = graph->spelling_count;
    const size_t count = (size_t)frame->progress.count;
    const size_t interface_count = (size_t)frame->progress.interface_count;
    *status = COD_REGION_STORAGE_TOO_LARGE;
    if (count > SIZE_MAX - graph->export_count || interface_count > SIZE_MAX - graph->interface_count) {
        return COD_NATIVE_OK;
    }
    for (size_t i = 0; i < count; ++i) {
        if (staged[i].publishing_module != frame->module + 1 || staged[i].name_first > staged[i].name_past ||
            staged[i].name_past > source.count) return COD_NATIVE_BOUNDARY_ERROR;
        const size_t width = (size_t)(staged[i].name_past - staged[i].name_first);
        if (width > SIZE_MAX - spelling_end) return COD_NATIVE_OK;
        spelling_end += width;
    }
    for (size_t i = 0; i < interface_count; ++i) {
        if (staged_interfaces[i].name_first > staged_interfaces[i].name_past ||
            staged_interfaces[i].name_past > source.count) return COD_NATIVE_BOUNDARY_ERROR;
        const size_t width = (size_t)(staged_interfaces[i].name_past - staged_interfaces[i].name_first);
        if (width > SIZE_MAX - spelling_end) return COD_NATIVE_OK;
        spelling_end += width;
    }
    *status = COD_REGION_STORAGE_NO_MEMORY;
    unsigned char *spellings = reserve(graph->spellings, &graph->spelling_capacity, spelling_end, 1);
    if (!spellings) return COD_NATIVE_OK;
    graph->spellings = spellings;
    export_row *exports = reserve(graph->exports, &graph->export_capacity, graph->export_count + count, sizeof(*exports));
    if (!exports) return COD_NATIVE_OK;
    graph->exports = exports;
    if (interface_count) {
        interface_row *interfaces = reserve(graph->interfaces, &graph->interface_capacity,
                                                graph->interface_count + interface_count, sizeof(*interfaces));
        if (!interfaces) return COD_NATIVE_OK;
        graph->interfaces = interfaces;
    }
    for (size_t i = 0; i < count; ++i) {
        export_row row = staged[i];
        append_spelling(graph, source.base, &row.name_first, &row.name_past);
        graph->exports[graph->export_count++] = row;
    }
    for (size_t i = 0; i < interface_count; ++i) {
        interface_row row = staged_interfaces[i];
        append_spelling(graph, source.base, &row.name_first, &row.name_past);
        graph->interfaces[graph->interface_count++] = row;
    }
    *status = graph->native ? stage_native_unit(frame, source) : COD_REGION_STORAGE_OK;
    return COD_NATIVE_OK;
}

static bool initialize_catalog(module_graph *graph, const void *spellings, size_t spelling_count,
                               const void *exports, size_t export_count) {
    if (!spellings || !spelling_count || !exports || !export_count ||
        export_count > PTRDIFF_MAX / sizeof(*graph->exports)) return false;
    graph->spellings = malloc(spelling_count);
    graph->exports = malloc(export_count * sizeof(*graph->exports));
    if (!graph->spellings || !graph->exports) return false;
    memcpy(graph->spellings, spellings, spelling_count);
    memcpy(graph->exports, exports, export_count * sizeof(*graph->exports));
    graph->spelling_count = graph->spelling_capacity = spelling_count;
    graph->export_count = graph->export_capacity = export_count;
    return true;
}

static void release_graph(module_graph *graph) {
    for (size_t i = 0; i < graph->module_count; ++i) {
        const loaded_module *module = &graph->modules[i];
        free((void *)module->unit.literals);
        free(module->literal_bytes);
        free((void *)module->unit.records);
        free(module->path);
    }
    free(graph->modules);
    free(graph->exports);
    free(graph->interfaces);
    free(graph->spellings);
}

static uint8_t check_graph_input(module_graph *graph, const char *input, cod_booter_diagnostic *diagnostic) {
    *diagnostic = (cod_booter_diagnostic){0};
    return enter_module(graph, input, NULL, diagnostic);
}

static uint8_t run_module_graph(const char *input, const char *directory, cod_booter_diagnostic *diagnostic) {
    module_graph graph = {0};
    uint8_t status = scratch_failed;
    if (initialize_catalog(&graph, &cod_lang_intrinsic_names, sizeof(cod_lang_intrinsic_names),
            cod_lang_intrinsic_exports, sizeof(cod_lang_intrinsic_exports) / sizeof(cod_lang_intrinsic_exports[0]))) {
        status = check_graph_input(&graph, input, diagnostic);
    } else {
        *diagnostic = (cod_booter_diagnostic){0};
        diagnostic->stage = 7;
        diagnostic->code = COD_REGION_STORAGE_NO_MEMORY;
    }
    if (!status && directory) {
        const char **paths = calloc(graph.module_count, sizeof(*paths));
        uint32_t emitted = provider_output_memory;
        if (paths) {
            for (size_t i = 0; i < graph.module_count; ++i) paths[i] = graph.modules[i].path;
            emitted = cod_booter_write_c_providers(directory, graph.interfaces, graph.interface_count,
                                                  graph.spellings, graph.spelling_count, paths, graph.module_count);
            free(paths);
        }
        if (emitted) {
            diagnostic->stage = provider_output_stage;
            diagnostic->code = emitted;
            status = 1;
        }
    }
    release_graph(&graph);
    return status;
}

uint8_t cod_booter_check_declarations(const char *input, cod_booter_diagnostic *diagnostic) {
    return run_module_graph(input, NULL, diagnostic);
}

uint8_t cod_booter_emit_c_providers(const char *input, const char *directory, cod_booter_diagnostic *diagnostic) {
    return run_module_graph(input, directory, diagnostic);
}

cod_native_status cod_native_NativeGraph_construct_impl(uint64_t *resource, cod_native_NativeGraph_failure *failure) {
    module_graph *graph = calloc(1, sizeof(*graph));
    if (!graph) {
        failure->status = COD_REGION_STORAGE_NO_MEMORY;
        return COD_NATIVE_FAILURE;
    }
    graph->native = true;
    *resource = (uint64_t)(uintptr_t)graph;
    return COD_NATIVE_OK;
}

void cod_native_NativeGraph_destroy_impl(uint64_t resource) {
    module_graph *graph = (module_graph *)(uintptr_t)resource;
    release_graph(graph);
    free(graph);
}

static module_graph *native_graph(cod_native_NativeGraph_borrow graph) {
    return graph.payload == graph.owner ? (module_graph *)(uintptr_t)graph.payload : NULL;
}

cod_native_status cod_native_collect_native_graph_impl(cod_native_NativeGraph_borrow handle, cod_native_Path_borrow input,
    cod_native_DiagnosticDestination_borrow result, cod_native_collect_native_graph_view3 spellings,
    cod_native_collect_native_graph_view4 exports, compiler_status *value) {
    module_graph *graph = native_graph(handle);
    const char *path = (const char *)(uintptr_t)input.payload;
    cod_booter_diagnostic *diagnostic = (cod_booter_diagnostic *)(uintptr_t)result.payload;
    if (!graph || graph->collected || !path || !diagnostic || result.payload != result.owner)
        return COD_NATIVE_BOUNDARY_ERROR;
    graph->collected = true;
    if ((uint64_t)(size_t)spellings.count != spellings.count || (uint64_t)(size_t)exports.count != exports.count ||
        !initialize_catalog(graph, spellings.base, (size_t)spellings.count, exports.base, (size_t)exports.count)) {
        *diagnostic = (cod_booter_diagnostic){.stage = 7, .code = COD_REGION_STORAGE_NO_MEMORY};
        *value = scratch_failed;
        return COD_NATIVE_OK;
    }
    *value = check_graph_input(graph, path, diagnostic);
    return *value == invalid_boundary ? COD_NATIVE_BOUNDARY_ERROR : COD_NATIVE_OK;
}

cod_native_status cod_native_construct_native_graph_impl(cod_native_NativeGraph_borrow handle,
    cod_native_LLVMSession_borrow resource, uint8_t *status) {
    const module_graph *graph = native_graph(handle);
    cod_llvm_session *session = (cod_llvm_session *)(uintptr_t)resource.payload;
    if (!graph || !graph->module_count || !graph->modules[0].checked || !session || resource.payload != resource.owner)
        return COD_NATIVE_BOUNDARY_ERROR;
    cod_llvm_command_unit *units = calloc(graph->module_count, sizeof(*units));
    *status = llvm_construction_failed;
    if (!units) return COD_NATIVE_OK;
    size_t count = 0;
    for (size_t i = 0; i < graph->module_count; ++i)
        if (graph->modules[i].unit.count) units[count++] = graph->modules[i].unit;
    if (cod_llvm_construct_units(session, units, count, graph->function_count)) *status = llvm_ok;
    free(units);
    return COD_NATIVE_OK;
}
