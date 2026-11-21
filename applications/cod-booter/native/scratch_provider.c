#include "cod_providers.h"

#include "cod_region_storage.h"

#include <string.h>

_Static_assert(UINTPTR_MAX <= UINT64_MAX, "scratch handles require a pointer that fits in a handle word");

enum { scratch_arrays = 9 };

typedef struct {
  cod_region_storage storage;
  cod_region_extent extents[scratch_arrays];
  size_t counts[scratch_arrays];
} scratch_region;

static const struct {
  size_t width;
  size_t alignment;
} scratch_rows[scratch_arrays] = {
#define ROW(index) {sizeof(cod_native_scratch_use_row##index), _Alignof(cod_native_scratch_use_row##index)}
    ROW(0), ROW(1), ROW(2), ROW(3), ROW(4), ROW(5), ROW(6), ROW(7), ROW(8),
#undef ROW
};

static cod_region_storage_status plan_scratch(scratch_region *scratch, size_t *capacity,
                                              uint64_t tokens, uint64_t nodes, uint64_t commands) {
  const uint64_t counts[scratch_arrays] = {tokens, nodes, nodes, nodes, nodes, nodes, nodes, nodes, commands};
  for (size_t i = 0; i < scratch_arrays; ++i) {
    scratch->counts[i] = (size_t)counts[i];
    if ((uint64_t)scratch->counts[i] != counts[i]) return COD_REGION_STORAGE_TOO_LARGE;
    const cod_region_storage_status status = cod_region_layout_reserve(
        capacity, PTRDIFF_MAX, scratch->counts[i], scratch_rows[i].width, scratch_rows[i].alignment, &scratch->extents[i]);
    if (status != COD_REGION_STORAGE_OK) return status;
  }
  return COD_REGION_STORAGE_OK;
}

cod_native_status cod_native_Scratch_construct_impl(uint64_t tokens, uint64_t nodes, uint64_t commands,
                                                    uint64_t *resource, cod_native_Scratch_failure *failure) {
  scratch_region layout = {0};
  size_t capacity = 0;
  cod_region_storage_status status = plan_scratch(&layout, &capacity, tokens, nodes, commands);
  if (status != COD_REGION_STORAGE_OK) {
    failure->status = (uint32_t)status;
    return COD_NATIVE_FAILURE;
  }
  scratch_region *scratch = malloc(sizeof(*scratch));
  if (!scratch) {
    failure->status = COD_REGION_STORAGE_NO_MEMORY;
    return COD_NATIVE_FAILURE;
  }
  *scratch = layout;
  status = cod_region_storage_open_layout(&scratch->storage, capacity);
  if (status != COD_REGION_STORAGE_OK) {
    free(scratch);
    failure->status = (uint32_t)status;
    return COD_NATIVE_FAILURE;
  }
  memset(scratch->storage.bytes, 0, capacity);
  *resource = (uint64_t)(uintptr_t)scratch;
  return COD_NATIVE_OK;
}

void cod_native_Scratch_destroy_impl(uint64_t resource) {
  scratch_region *scratch = (scratch_region *)(uintptr_t)resource;
  cod_region_storage_release(&scratch->storage);
  free(scratch);
}

static cod_native_scratch_use_arrays scratch_views(scratch_region *scratch) {
  return (cod_native_scratch_use_arrays){
#define VIEW(index) {cod_region_storage_address(&scratch->storage, scratch->extents[index]), scratch->counts[index]}
      VIEW(0), VIEW(1), VIEW(2), VIEW(3), VIEW(4), VIEW(5), VIEW(6), VIEW(7), VIEW(8),
#undef VIEW
  };
}

static scratch_region *scratch_from_handle(cod_native_Scratch_borrow resource) {
  return resource.payload == resource.owner ? (scratch_region *)(uintptr_t)resource.payload : NULL;
}

int cod_native_scratch_use_validate_impl(cod_native_Scratch_borrow resource, const cod_native_scratch_use_arrays *views) {
  scratch_region *scratch = scratch_from_handle(resource);
  if (!scratch) return 0;
  const cod_native_scratch_use_arrays expected = scratch_views(scratch);
#define MATCH(index) (views->array##index.base == expected.array##index.base && views->array##index.count == expected.array##index.count)
  return MATCH(0) && MATCH(1) && MATCH(2) && MATCH(3) && MATCH(4) && MATCH(5) && MATCH(6) && MATCH(7) && MATCH(8);
#undef MATCH
}

cod_native_status cod_native_scratch_use_impl(cod_native_Scratch_borrow resource, cod_native_scratch_use_callback run,
                                            void *loan, uint8_t *value) {
  scratch_region *scratch = scratch_from_handle(resource);
  if (!scratch) return COD_NATIVE_BOUNDARY_ERROR;
  const cod_native_scratch_use_arrays views = scratch_views(scratch);
  const cod_native_here_step completion = run(loan, &views, value);
  return completion == COD_NATIVE_HERE_INVALID ? COD_NATIVE_BOUNDARY_ERROR : COD_NATIVE_OK;
}
