#include "cod_providers.h"

#include "compile_diagnostic.h"

_Static_assert(UINTPTR_MAX <= UINT64_MAX, "diagnostic handles require a pointer that fits in a handle word");

cod_native_status cod_native_write_diagnostic_impl(cod_native_DiagnosticDestination_borrow destination,
    uint8_t stage, uint8_t code, uint64_t first_line, uint64_t first_column, uint64_t past_line, uint64_t past_column,
    uint64_t related_line, uint64_t related_column, uint64_t os_error, uint8_t *status) {
    if (!destination.payload || destination.payload != destination.owner) return COD_NATIVE_BOUNDARY_ERROR;
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
