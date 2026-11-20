#include "cod_providers.h"

#include "cod_source.h"

_Static_assert(UINTPTR_MAX <= UINT64_MAX, "source handles require a pointer that fits in a handle word");

cod_native_status cod_native_SourceRevision_construct_impl(cod_native_Path_borrow path, uint64_t *resource,
                                                            cod_native_SourceRevision_failure *failure) {
  const char *filename = (const char *)(uintptr_t)path.payload;
  if (!filename) return COD_NATIVE_BOUNDARY_ERROR;

  cod_source_error error;
  cod_source_revision *revision = cod_source_open_readonly(filename, &error);
  if (!revision) {
    failure->os_error = error.system_error;
    return COD_NATIVE_FAILURE;
  }
  *resource = (uint64_t)(uintptr_t)revision;
  return COD_NATIVE_OK;
}

void cod_native_SourceRevision_destroy_impl(uint64_t resource) {
  cod_source_release((cod_source_revision *)(uintptr_t)resource);
}
