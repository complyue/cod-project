#include "cod_providers.h"

#include "cod_source.h"

_Static_assert(sizeof(cod_source_span) == sizeof(cod_native_source_use_row1), "source index row size");
_Static_assert(_Alignof(cod_source_span) == _Alignof(cod_native_source_use_row1), "source index row alignment");
_Static_assert(offsetof(cod_source_span, first) == offsetof(cod_native_source_use_row1, first), "source index start");
_Static_assert(offsetof(cod_source_span, past) == offsetof(cod_native_source_use_row1, past), "source index end");

static const cod_source_revision *source_revision(cod_native_SourceRevision_borrow revision) {
  return revision.payload == revision.owner ? (const cod_source_revision *)(uintptr_t)revision.payload : NULL;
}

int cod_native_source_use_validate_impl(cod_native_SourceRevision_borrow revision,
                                       const cod_native_source_use_arrays *views) {
  const cod_source_revision *source = source_revision(revision);
  if (!source) return 0;
  const cod_source_view canonical = cod_source_borrow(source);
  return views->array0.base == canonical.bytes && views->array0.count == canonical.size &&
      (const void *)views->array1.base == (const void *)canonical.lines && views->array1.count == canonical.line_count;
}

cod_native_status cod_native_source_use_impl(cod_native_SourceRevision_borrow revision, cod_native_source_use_callback run,
                                             void *loan, uint8_t *value) {
  const cod_source_revision *source = source_revision(revision);
  if (!source) return COD_NATIVE_BOUNDARY_ERROR;

  const cod_source_view view = cod_source_borrow(source);
  const cod_native_source_use_arrays arrays = {
      {view.bytes, view.size},
      {(const cod_native_source_use_row1 *)view.lines, view.line_count},
  };
  const cod_native_here_step completion = run(loan, &arrays, value);
  return completion == COD_NATIVE_HERE_INVALID ? COD_NATIVE_BOUNDARY_ERROR : COD_NATIVE_OK;
}
