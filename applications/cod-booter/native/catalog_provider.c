#include "cod_providers.h"

#include "lang_intrinsic_catalog.h"

typedef cod_native_catalog_use_row1 catalog_row;
_Static_assert(sizeof(catalog_row) == sizeof(cod_lang_intrinsic_export) &&
               _Alignof(catalog_row) == _Alignof(cod_lang_intrinsic_export), "invalid catalog row layout");
#define CHECK_FIELD(field) \
    _Static_assert(offsetof(catalog_row, field) == offsetof(cod_lang_intrinsic_export, field), "invalid catalog row offset")
CHECK_FIELD(module_first);
CHECK_FIELD(module_past);
CHECK_FIELD(name_first);
CHECK_FIELD(name_past);
CHECK_FIELD(identity);
CHECK_FIELD(kind);
CHECK_FIELD(checked);
#undef CHECK_FIELD
_Static_assert(UINTPTR_MAX <= UINT64_MAX, "catalog handles require a pointer that fits in a handle word");

static uint64_t catalog_word(void) {
    return (uint64_t)(uintptr_t)&cod_lang_intrinsic_names;
}

static cod_native_catalog_use_arrays catalog_views(void) {
    return (cod_native_catalog_use_arrays){
        {(const uint8_t *)&cod_lang_intrinsic_names, sizeof(cod_lang_intrinsic_names)},
        {(const void *)cod_lang_intrinsic_exports, sizeof(cod_lang_intrinsic_exports) / sizeof(cod_lang_intrinsic_exports[0])},
    };
}

cod_native_status cod_native_IntrinsicCatalog_construct_impl(uint64_t *resource) {
    *resource = catalog_word();
    return COD_NATIVE_OK;
}

void cod_native_IntrinsicCatalog_destroy_impl(uint64_t resource) {
    // The catalog borrows static immutable backing.
    (void)resource;
}

static int live_catalog(cod_native_IntrinsicCatalog_borrow resource) {
    return resource.payload == catalog_word() && resource.owner == resource.payload;
}

int cod_native_catalog_use_validate_impl(cod_native_IntrinsicCatalog_borrow resource,
                                        const cod_native_catalog_use_arrays *views) {
    const cod_native_catalog_use_arrays expected = catalog_views();
    if (!live_catalog(resource) || views->array0.base != expected.array0.base || views->array0.count != expected.array0.count ||
        views->array1.base != expected.array1.base || views->array1.count != expected.array1.count) return 0;
    for (size_t i = 0; i < expected.array1.count; ++i) {
        const cod_lang_intrinsic_export *row = &cod_lang_intrinsic_exports[i];
        if (row->module_first > row->module_past || row->module_past > expected.array0.count ||
            row->name_first > row->name_past || row->name_past > expected.array0.count) return 0;
    }
    return 1;
}

cod_native_status cod_native_catalog_use_impl(cod_native_IntrinsicCatalog_borrow resource, cod_native_catalog_use_callback run,
                                             void *loan, uint8_t *status) {
    if (!live_catalog(resource)) return COD_NATIVE_BOUNDARY_ERROR;
    const cod_native_catalog_use_arrays views = catalog_views();
    return run(loan, &views, status) == COD_NATIVE_HERE_INVALID ? COD_NATIVE_BOUNDARY_ERROR : COD_NATIVE_OK;
}
