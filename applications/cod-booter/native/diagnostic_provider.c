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

cod_native_status cod_native_write_diagnostic_impl(cod_native_DiagnosticDestination_borrow destination,
    uint8_t stage, uint32_t code, uint64_t first_line, uint64_t first_column, uint64_t past_line, uint64_t past_column,
    uint64_t related_line, uint64_t related_column, uint64_t os_error, uint8_t *status) {
    if (!destination.payload || destination.payload > UINTPTR_MAX) return COD_NATIVE_BOUNDARY_ERROR;
    if (destination.payload != destination.owner && !snapshot_destination(destination))
        return COD_NATIVE_BOUNDARY_ERROR;
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

cod_native_status cod_native_append_package_diagnostic_impl(
    cod_native_DiagnosticDestination_borrow destination, uint8_t kind, uint64_t os_error,
    uint64_t first_line, uint64_t first_column, uint64_t past_line, uint64_t past_column,
    cod_native_append_package_diagnostic_view7 importer, uint64_t import_first, uint64_t import_past,
    cod_native_append_package_diagnostic_view10 spelling, uint64_t first, uint64_t past,
    cod_native_append_package_diagnostic_view13 candidate, cod_native_append_package_diagnostic_view14 other,
    uint8_t *status) {
    cod_booter_diagnostic_sink *sink = snapshot_destination(destination);
    diagnostic_bytes ranges[4];
    if (!sink || !first_line || !first_column || !past_line || !past_column ||
        past_line < first_line || (past_line == first_line && past_column < first_column) ||
        kind < package_shadow || kind > package_process_failure ||
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
    if (sink->last_package_report) sink->last_package_report->next = report;
    else sink->package_reports = report;
    sink->last_package_report = report;
    *status = 0;
    return COD_NATIVE_OK;
}

cod_native_status cod_native_write_parse_reports_impl(cod_native_DiagnosticDestination_borrow destination,
    cod_native_write_parse_reports_view1 rows, uint64_t used,
    cod_native_write_parse_reports_view3 path, uint64_t first, uint64_t past, uint8_t stage, uint8_t *status) {
    cod_booter_diagnostic_sink *sink = snapshot_destination(destination);
    if (!sink || !rows.base || !used || used > rows.count ||
        used > SIZE_MAX / sizeof(cod_booter_diagnostic) ||
        !path.base || first >= past || past > path.count || past > SIZE_MAX || past - first >= SIZE_MAX)
        return COD_NATIVE_BOUNDARY_ERROR;

    const size_t path_size = (size_t)(past - first);
    const uint8_t *path_bytes = path.base + (size_t)first;
    if (memchr(path_bytes, '\0', path_size)) return COD_NATIVE_BOUNDARY_ERROR;
    char *copied_path = malloc(path_size + 1);
    cod_booter_diagnostic *copied = malloc((size_t)used * sizeof(*copied));
    if (!copied || !copied_path) {
        free(copied_path);
        free(copied);
        sink->snapshot_failed = true;
        *status = 1;
        return COD_NATIVE_OK;
    }

    memcpy(copied_path, path_bytes, path_size);
    copied_path[path_size] = '\0';
    for (size_t i = 0; i < (size_t)used; ++i) {
        const cod_native_write_parse_reports_row1 *row = &rows.base[i];
        copied[i] = (cod_booter_diagnostic){
            .stage = stage,
            .code = row->code,
            .first = {row->first_line, row->first_column},
            .past = {row->past_line, row->past_column},
        };
    }
    free(sink->path);
    free(sink->reports);
    sink->path = copied_path;
    sink->reports = copied;
    sink->count = (size_t)used;
    sink->snapshot_failed = false;
    *status = 0;
    return COD_NATIVE_OK;
}
