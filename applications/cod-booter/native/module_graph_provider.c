#define _XOPEN_SOURCE 700

#include "cod_providers.h"
#include "cod_region_storage.h"
#include "lang_intrinsic_catalog.h"
#include "module_graph.h"
#include "provider_output.h"

#include <errno.h>
#include <stdbool.h>
#include <string.h>

enum { frame_arrays = 7, max_active_modules = 128 };
enum { scratch_failed = 6, source_open_failed = 7, invalid_boundary = 70 };

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
} module_graph;

typedef struct {
    module_graph *graph;
    size_t module;
    dependency *edges;
    size_t edge_count, edge_capacity;
    cod_region_storage scratch;
    cod_region_extent extents[frame_arrays];
    size_t tokens, nodes;
    cod_native_graph_frame_use_row6 progress;
    bool views_live;
} module_frame;

static const struct {
    size_t size, alignment;
} frame_layout[frame_arrays] = {
#define ROW(i) {sizeof(cod_native_graph_frame_use_row##i), _Alignof(cod_native_graph_frame_use_row##i)}
    ROW(0), ROW(1), ROW(2), ROW(3), ROW(4), ROW(5), ROW(9),
#undef ROW
};

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
                                                     uint64_t nodes, uint8_t *status) {
    module_frame *frame = frame_from_handle(handle);
    if (!frame || frame->scratch.bytes) return COD_NATIVE_BOUNDARY_ERROR;
    frame->tokens = (size_t)tokens;
    frame->nodes = (size_t)nodes;
    if ((uint64_t)frame->tokens != tokens || (uint64_t)frame->nodes != nodes) {
        *status = COD_REGION_STORAGE_TOO_LARGE;
        return COD_NATIVE_OK;
    }
    size_t capacity = 0;
    for (size_t i = 0; i < frame_arrays; ++i) {
        *status = cod_region_layout_reserve(&capacity, PTRDIFF_MAX, i == 0 ? frame->tokens : frame->nodes,
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
    };
}

int cod_native_graph_frame_use_validate_impl(cod_native_ModuleFrame_borrow handle,
                                             const cod_native_graph_frame_use_arrays *offered) {
    module_frame *frame = frame_from_handle(handle);
    if (!frame || !frame->views_live) return 0;
    const cod_native_graph_frame_use_arrays expected = frame_views(frame);
#define MATCH(i) (offered->array##i.base == expected.array##i.base && offered->array##i.count == expected.array##i.count)
    return MATCH(0) && MATCH(1) && MATCH(2) && MATCH(3) && MATCH(4) &&
           MATCH(5) && MATCH(6) && MATCH(7) && MATCH(8) && MATCH(9) && MATCH(10);
#undef MATCH
}

cod_native_status cod_native_graph_frame_use_impl(cod_native_ModuleFrame_borrow handle,
                                                 cod_native_graph_frame_use_callback run, void *loan, uint8_t *value) {
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

static uint8_t enter_module(module_graph *graph, char *path, cod_booter_diagnostic *diagnostic) {
    const size_t module = graph->module_count;
    loaded_module *modules = reserve(graph->modules, &graph->module_capacity, module + 1, sizeof(*modules));
    if (!modules) {
        free(path);
        diagnostic->stage = 7;
        diagnostic->code = COD_REGION_STORAGE_NO_MEMORY;
        return scratch_failed;
    }
    graph->modules = modules;
    graph->modules[graph->module_count++] = (loaded_module){path, false};
    module_frame frame = {.graph = graph, .module = module, .progress.identity = module + 1};
    const cod_native_loan paths = {path, (uint64_t)(uintptr_t)path};
    const cod_native_loan work = {&frame, (uint64_t)(uintptr_t)&frame};
    const cod_native_loan destination = {diagnostic, (uint64_t)(uintptr_t)diagnostic};
    const cod_native_Path_borrow input = {paths.owner, paths.owner};
    const cod_native_ModuleFrame_borrow module_work = {work.owner, work.owner};
    const cod_native_DiagnosticDestination_borrow result = {destination.owner, destination.owner};
    uint8_t status = 0;
    ++graph->active_count;
    if (cod_native_check_graph_module_entry(&input, &module_work, &result, &paths, &work, &destination, &status) != COD_NATIVE_OK)
        status = invalid_boundary;
    --graph->active_count;
    for (size_t i = 0; i < frame.edge_count; ++i) free(frame.edges[i].path);
    free(frame.edges);
    cod_region_storage_release(&frame.scratch);
    graph->modules[module].checked = status == 0;
    return status;
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
            const uint8_t entered = enter_module(graph, path, &child_diagnostic);
            if (entered) {
                *status = child_diagnostic.stage == 2 &&
                          (child_diagnostic.code == module_cycle_unproved || child_diagnostic.code == module_depth_unproved)
                        ? (uint8_t)child_diagnostic.code : module_dependency_rejected;
                return COD_NATIVE_OK;
            }
        }
        selections[edge->site].dependency_module = target + 1;
    }
    return COD_NATIVE_OK;
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
    *status = COD_REGION_STORAGE_OK;
    return COD_NATIVE_OK;
}

static uint8_t run_module_graph(const char *input, const char *directory, cod_booter_diagnostic *diagnostic) {
    *diagnostic = (cod_booter_diagnostic){0};
    char *path = realpath(input, NULL);
    if (!path) {
        diagnostic->os_error = errno;
        return source_open_failed;
    }
    module_graph graph = {
        .spelling_count = sizeof(cod_lang_intrinsic_names),
        .spelling_capacity = sizeof(cod_lang_intrinsic_names),
        .export_count = sizeof(cod_lang_intrinsic_exports) / sizeof(cod_lang_intrinsic_exports[0]),
        .export_capacity = sizeof(cod_lang_intrinsic_exports) / sizeof(cod_lang_intrinsic_exports[0]),
    };
    graph.spellings = malloc(sizeof(cod_lang_intrinsic_names));
    graph.exports = malloc(sizeof(cod_lang_intrinsic_exports));
    uint8_t status;
    if (graph.spellings && graph.exports) {
        memcpy(graph.spellings, &cod_lang_intrinsic_names, sizeof(cod_lang_intrinsic_names));
        memcpy(graph.exports, cod_lang_intrinsic_exports, sizeof(cod_lang_intrinsic_exports));
        status = enter_module(&graph, path, diagnostic);
    } else {
        free(path);
        diagnostic->stage = 7;
        diagnostic->code = COD_REGION_STORAGE_NO_MEMORY;
        status = scratch_failed;
    }
    if (!status && directory) {
        const uint32_t emitted = cod_booter_write_c_providers(directory, graph.interfaces, graph.interface_count,
                                                               graph.spellings, graph.spelling_count);
        if (emitted) {
            diagnostic->stage = provider_output_stage;
            diagnostic->code = emitted;
            status = emitted == provider_output_unsupported ? 2 : 1;
        }
    }
    for (size_t i = 0; i < graph.module_count; ++i) free(graph.modules[i].path);
    free(graph.modules);
    free(graph.exports);
    free(graph.interfaces);
    free(graph.spellings);
    return status;
}

uint8_t cod_booter_check_declarations(const char *input, cod_booter_diagnostic *diagnostic) {
    return run_module_graph(input, NULL, diagnostic);
}

uint8_t cod_booter_emit_c_providers(const char *input, const char *directory, cod_booter_diagnostic *diagnostic) {
    return run_module_graph(input, directory, diagnostic);
}
