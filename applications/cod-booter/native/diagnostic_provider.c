#include "cod_providers.h"

#include "compile_diagnostic.h"

#include <stdlib.h>
#include <string.h>

_Static_assert(UINTPTR_MAX <= UINT64_MAX, "diagnostic handles require a pointer that fits in a handle word");

_Static_assert(offsetof(cod_booter_diagnostic_sink, primary) > 0,
    "a diagnostic snapshot must be distinguishable from a primary-only record");

static cod_booter_diagnostic_sink *snapshot_destination(cod_native_DiagnosticDestination_borrow destination) {
    const size_t offset = offsetof(cod_booter_diagnostic_sink, primary);
    if (!destination.owner || destination.owner > UINTPTR_MAX - offset ||
        destination.payload != destination.owner + offset ||
        destination.owner % _Alignof(cod_booter_diagnostic_sink)) return NULL;
    return (cod_booter_diagnostic_sink *)(uintptr_t)destination.owner;
}

cod_native_status cod_native_diagnostic_limit_impl(cod_native_DiagnosticDestination_borrow destination,
    uint64_t *result) {
    const cod_booter_diagnostic_sink *sink = snapshot_destination(destination);
    if (!sink && (!destination.payload || destination.payload > UINTPTR_MAX ||
                  destination.payload != destination.owner)) return COD_NATIVE_BOUNDARY_ERROR;
    *result = sink ? sink->finding_limit : 0;
    return COD_NATIVE_OK;
}

cod_native_status cod_native_write_diagnostic_impl(cod_native_DiagnosticDestination_borrow destination,
    uint8_t stage, uint32_t code, uint64_t first_line, uint64_t first_column, uint64_t past_line, uint64_t past_column,
    uint64_t related_line, uint64_t related_column, uint64_t os_error, uint8_t *status) {
    if (!destination.payload || destination.payload > UINTPTR_MAX) return COD_NATIVE_BOUNDARY_ERROR;
    cod_booter_diagnostic_sink *sink = snapshot_destination(destination);
    if (destination.payload != destination.owner && !sink) return COD_NATIVE_BOUNDARY_ERROR;
    if (sink) sink->primary_retained = false;
    cod_booter_diagnostic *record = (cod_booter_diagnostic *)(uintptr_t)destination.payload;
    *record = (cod_booter_diagnostic){
        .stage = stage,
        .code = code,
        .first = {first_line, first_column},
        .past = {past_line, past_column},
        .related = {related_line, related_column},
        .os_error = os_error,
    };
    *status = 0;
    return COD_NATIVE_OK;
}

typedef struct {
    const uint8_t *bytes;
    size_t size;
} diagnostic_bytes;

static bool diagnostic_range(const uint8_t *base, uint64_t count, uint64_t first, uint64_t past,
                             diagnostic_bytes *bytes) {
    if (first > past || past > count || past > (uint64_t)PTRDIFF_MAX || (!base && past)) return false;
    if (first == past) {
        *bytes = (diagnostic_bytes){0};
        return true;
    }
    *bytes = (diagnostic_bytes){base + (size_t)first, (size_t)(past - first)};
    return memchr(bytes->bytes, 0, bytes->size) == NULL;
}

static uint64_t finding_count(const cod_booter_diagnostic_event *event) {
    return event->kind == cod_booter_source_reports
        ? ((const cod_booter_source_diagnostics *)event)->count : 1;
}

static bool valid_finding_range(const cod_booter_diagnostic_sink *sink, uint64_t first, uint64_t count) {
    if (!count || count > UINT64_MAX - first) return false;
    const cod_booter_diagnostic_event *last = sink->last_event;
    return first == (last ? last->first_finding + finding_count(last) : 0);
}

static void append_event(cod_booter_diagnostic_sink *sink, cod_booter_diagnostic_event *event) {
    if (sink->last_event) sink->last_event->next = event;
    else sink->first_event = event;
    sink->last_event = event;
}

cod_native_status cod_native_append_package_diagnostic_impl(
    cod_native_DiagnosticDestination_borrow destination, uint8_t kind, uint64_t os_error,
    uint64_t first_line, uint64_t first_column, uint64_t past_line, uint64_t past_column,
    cod_native_append_package_diagnostic_view7 importer, uint64_t import_first, uint64_t import_past,
    cod_native_append_package_diagnostic_view10 spelling, uint64_t first, uint64_t past,
    cod_native_append_package_diagnostic_view13 candidate, cod_native_append_package_diagnostic_view14 other,
    uint64_t finding, uint8_t *status) {
    cod_booter_diagnostic_sink *sink = snapshot_destination(destination);
    diagnostic_bytes ranges[4];
    if (!sink || !valid_finding_range(sink, finding, 1) || !first_line || !first_column || !past_line || !past_column ||
        past_line < first_line || (past_line == first_line && past_column < first_column) ||
        kind < package_shadow || kind > invalid_module_path ||
        !diagnostic_range(importer.base, importer.count, import_first, import_past, &ranges[0]) ||
        !diagnostic_range(spelling.base, spelling.count, first, past, &ranges[1]) ||
        !diagnostic_range(candidate.base, candidate.count, 0, candidate.count, &ranges[2]) ||
        !diagnostic_range(other.base, other.count, 0, other.count, &ranges[3]) ||
        !ranges[0].size || !ranges[1].size)
        return COD_NATIVE_BOUNDARY_ERROR;

    const bool two_origins = kind == package_shadow || kind == package_ambiguity;
    if ((two_origins && (!ranges[2].size || !ranges[3].size)) || (!two_origins && ranges[3].size))
        return COD_NATIVE_BOUNDARY_ERROR;
    size_t size = sizeof(cod_booter_package_diagnostic);
    for (size_t i = 0; i < sizeof(ranges) / sizeof(ranges[0]); ++i) {
        if (ranges[i].size > (size_t)PTRDIFF_MAX - size) {
            sink->package_delivery_failed = true;
            *status = 1;
            return COD_NATIVE_OK;
        }
        size += ranges[i].size;
    }
    cod_booter_package_diagnostic *report = malloc(size);
    if (!report) {
        sink->package_delivery_failed = true;
        *status = 1;
        return COD_NATIVE_OK;
    }
    *report = (cod_booter_package_diagnostic){
        .event = {.kind = cod_booter_package_report, .first_finding = finding},
        .kind = kind,
        .os_error = os_error,
        .first = {first_line, first_column},
        .past = {past_line, past_column},
        .importer_size = ranges[0].size,
        .spelling_size = ranges[1].size,
        .candidate_size = ranges[2].size,
        .other_size = ranges[3].size,
    };
    unsigned char *copied = report->bytes;
    for (size_t i = 0; i < sizeof(ranges) / sizeof(ranges[0]); ++i) {
        if (ranges[i].size) memcpy(copied, ranges[i].bytes, ranges[i].size);
        copied += ranges[i].size;
    }
    append_event(sink, &report->event);
    *status = 0;
    return COD_NATIVE_OK;
}

cod_native_status cod_native_append_symbol_diagnostic_impl(cod_native_DiagnosticDestination_borrow destination,
    uint8_t stage, uint8_t code, uint64_t first_line, uint64_t first_column,
    uint64_t past_line, uint64_t past_column,
    cod_native_append_symbol_diagnostic_view7 keys, uint64_t key_first, uint64_t key_past,
    cod_native_append_symbol_diagnostic_view10 spelling, uint64_t name_first, uint64_t name_past,
    cod_native_append_symbol_diagnostic_view13 module_key, uint64_t module_first, uint64_t module_past,
    cod_native_append_symbol_diagnostic_view16 definition, uint64_t definition_first, uint64_t definition_past,
    uint64_t finding, uint8_t *status) {
    cod_booter_diagnostic_sink *sink = snapshot_destination(destination);
    diagnostic_bytes ranges[4];
    if (!sink || !valid_finding_range(sink, finding, 1) || !stage || !code ||
        !first_line || !first_column || !past_line || !past_column ||
        past_line < first_line || (past_line == first_line && past_column <= first_column) ||
        !diagnostic_range(keys.base, keys.count, key_first, key_past, &ranges[0]) ||
        !diagnostic_range(spelling.base, spelling.count, name_first, name_past, &ranges[1]) ||
        !diagnostic_range(module_key.base, module_key.count, module_first, module_past, &ranges[2]) ||
        !diagnostic_range(definition.base, definition.count, definition_first, definition_past, &ranges[3]))
        return COD_NATIVE_BOUNDARY_ERROR;

    size_t size = sizeof(cod_booter_symbol_diagnostic);
    for (size_t i = 0; i < sizeof(ranges) / sizeof(ranges[0]); ++i) {
        if (!ranges[i].size) return COD_NATIVE_BOUNDARY_ERROR;
        if (ranges[i].size > (size_t)PTRDIFF_MAX - size) {
            sink->snapshot_failed = true;
            *status = 1;
            return COD_NATIVE_OK;
        }
        size += ranges[i].size;
    }
    cod_booter_symbol_diagnostic *retained = malloc(size);
    if (!retained) {
        sink->snapshot_failed = true;
        *status = 1;
        return COD_NATIVE_OK;
    }
    *retained = (cod_booter_symbol_diagnostic){
        .event = {.kind = cod_booter_symbol_report, .first_finding = finding},
        .location = {.stage = stage, .code = code, .first = {first_line, first_column},
                     .past = {past_line, past_column}},
        .importer_size = ranges[0].size,
        .spelling_size = ranges[1].size,
        .module_size = ranges[2].size,
        .definition_size = ranges[3].size,
    };
    unsigned char *copied = retained->bytes;
    for (size_t i = 0; i < sizeof(ranges) / sizeof(ranges[0]); ++i) {
        memcpy(copied, ranges[i].bytes, ranges[i].size);
        copied += ranges[i].size;
    }
    append_event(sink, &retained->event);
    if (finding == 0 && sink->primary.stage == stage && sink->primary.code == code &&
        sink->primary.first.line == first_line && sink->primary.first.column == first_column)
        sink->primary_retained = true;
    *status = 0;
    return COD_NATIVE_OK;
}

cod_native_status cod_native_write_source_reports_impl(cod_native_DiagnosticDestination_borrow destination,
    cod_native_write_source_reports_view1 rows, uint64_t used,
    cod_native_write_source_reports_view3 path, uint64_t first, uint64_t past, uint8_t stage,
    uint64_t first_finding, uint8_t *status) {
    cod_booter_diagnostic_sink *sink = snapshot_destination(destination);
    if (!sink || !rows.base || used > rows.count ||
        used > SIZE_MAX / sizeof(cod_booter_diagnostic) ||
        !valid_finding_range(sink, first_finding, used) ||
        !path.base || first >= past || past > path.count || past > SIZE_MAX || past - first >= SIZE_MAX)
        return COD_NATIVE_BOUNDARY_ERROR;

    const size_t path_size = (size_t)(past - first);
    const size_t header_size = sizeof(cod_booter_source_diagnostics);
    const size_t bytes_limit = (size_t)PTRDIFF_MAX;
    if (past > bytes_limit || path_size >= bytes_limit - header_size ||
        used > (bytes_limit - header_size - path_size - 1) / sizeof(cod_booter_diagnostic)) {
        sink->snapshot_failed = true;
        *status = 1;
        return COD_NATIVE_OK;
    }
    const uint8_t *path_bytes = path.base + (size_t)first;
    if (memchr(path_bytes, '\0', path_size)) return COD_NATIVE_BOUNDARY_ERROR;
    for (size_t i = 0; i < (size_t)used; ++i) {
        const cod_native_write_source_reports_row1 *row = &rows.base[i];
        const bool located = row->first_line && row->first_column && row->past_line && row->past_column &&
            (row->past_line > row->first_line ||
                (row->past_line == row->first_line && row->past_column >= row->first_column));
        const bool unlocated = !row->first_line && !row->first_column && !row->past_line && !row->past_column;
        if (!row->code || (!located && !unlocated) || (!row->related_line != !row->related_column))
            return COD_NATIVE_BOUNDARY_ERROR;
    }

    cod_booter_source_diagnostics *batch =
        malloc(header_size + (size_t)used * sizeof(cod_booter_diagnostic) + path_size + 1);
    if (!batch) {
        sink->snapshot_failed = true;
        *status = 1;
        return COD_NATIVE_OK;
    }
    *batch = (cod_booter_source_diagnostics){
        .event = {.kind = cod_booter_source_reports, .first_finding = first_finding},
        .count = (size_t)used,
    };
    char *copied_path = (char *)(batch->reports + batch->count);
    memcpy(copied_path, path_bytes, path_size);
    copied_path[path_size] = '\0';
    for (size_t i = 0; i < batch->count; ++i) {
        const cod_native_write_source_reports_row1 *row = &rows.base[i];
        batch->reports[i] = (cod_booter_diagnostic){
            .stage = stage,
            .code = row->code,
            .first = {row->first_line, row->first_column},
            .past = {row->past_line, row->past_column},
            .related = {row->related_line, row->related_column},
        };
    }
    append_event(sink, &batch->event);
    const cod_booter_diagnostic *first_report = &batch->reports[0];
    if (first_finding == 0 && sink->primary.stage == first_report->stage && sink->primary.code == first_report->code &&
        sink->primary.first.line == first_report->first.line && sink->primary.first.column == first_report->first.column)
        sink->primary_retained = true;
    *status = 0;
    return COD_NATIVE_OK;
}


static cod_booter_diagnostic_event *retained_finding(cod_booter_diagnostic_sink *sink, uint64_t finding) {
    if (finding == UINT64_MAX) return NULL;
    for (cod_booter_diagnostic_event *event = sink->first_event; event; event = event->next) {
        if (finding < event->first_finding) continue;
        if (finding - event->first_finding < finding_count(event)) return event;
    }
    return NULL;
}

cod_native_status cod_native_append_dependency_note_impl(cod_native_DiagnosticDestination_borrow destination,
    uint64_t finding, cod_native_append_dependency_note_view2 path, uint64_t first, uint64_t past,
    uint64_t first_line, uint64_t first_column, uint64_t past_line, uint64_t past_column, uint8_t *status) {
    cod_booter_diagnostic_sink *sink = snapshot_destination(destination);
    if (!sink || !first_line || !first_column || !past_line || !past_column || past_line < first_line ||
        (past_line == first_line && past_column < first_column)) return COD_NATIVE_BOUNDARY_ERROR;
    diagnostic_bytes importer;
    cod_booter_diagnostic_event *event = retained_finding(sink, finding);
    if (!event || !diagnostic_range(path.base, path.count, first, past, &importer) || !importer.size)
        return COD_NATIVE_BOUNDARY_ERROR;
    if (importer.size >= (size_t)PTRDIFF_MAX - sizeof(cod_booter_dependency_note)) {
        sink->snapshot_failed = true;
        *status = 1;
        return COD_NATIVE_OK;
    }
    cod_booter_dependency_note *note = malloc(sizeof(*note) + importer.size + 1);
    if (!note) {
        sink->snapshot_failed = true;
        *status = 1;
        return COD_NATIVE_OK;
    }
    *note = (cod_booter_dependency_note){
        .finding = finding,
        .first = {first_line, first_column},
        .past = {past_line, past_column},
    };
    memcpy(note->path, importer.bytes, importer.size);
    note->path[importer.size] = 0;
    if (event->last_note) event->last_note->next = note;
    else event->first_note = note;
    event->last_note = note;
    *status = 0;
    return COD_NATIVE_OK;
}
