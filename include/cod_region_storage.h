/*===--- cod_region_storage.h - Fixed-capacity native working storage -------*/
#ifndef COD_REGION_STORAGE_H
#define COD_REGION_STORAGE_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum cod_region_storage_status {
  COD_REGION_STORAGE_OK,
  COD_REGION_STORAGE_INVALID,
  COD_REGION_STORAGE_TOO_LARGE,
  COD_REGION_STORAGE_NO_MEMORY,
  COD_REGION_STORAGE_FULL
} cod_region_storage_status;

/* Initialize the control record to zero before first use. One caller owns
   the backing allocation. Payload locators are byte offsets in that owner. */
typedef struct cod_region_storage {
  unsigned char *bytes;
  size_t capacity;
  size_t used;
} cod_region_storage;

typedef struct cod_region_extent {
  size_t offset;
  size_t size;
} cod_region_extent;

static inline cod_region_storage_status cod_region_storage_open(cod_region_storage *region, size_t capacity) {
  if (region->bytes) return COD_REGION_STORAGE_INVALID;
  if (capacity > (size_t)PTRDIFF_MAX) return COD_REGION_STORAGE_TOO_LARGE;
  unsigned char *bytes = (unsigned char *)malloc(capacity ? capacity : 1);
  if (!bytes) return COD_REGION_STORAGE_NO_MEMORY;
  region->bytes = bytes;
  region->capacity = capacity;
  region->used = 0;
  return COD_REGION_STORAGE_OK;
}

/* Open exact backing for the extents reserved in a layout. Its bytes are
   uninitialized. Keep the plan's extents paired with this owner. */
static inline cod_region_storage_status cod_region_storage_open_layout(cod_region_storage *region, size_t layout_size) {
  const cod_region_storage_status status = cod_region_storage_open(region, layout_size);
  if (status == COD_REGION_STORAGE_OK) region->used = layout_size;
  return status;
}

static inline void cod_region_storage_release(cod_region_storage *region) {
  free(region->bytes);
  region->bytes = NULL;
  region->capacity = 0;
  region->used = 0;
}

/* Reserve an aligned byte extent without allocating backing. Failure leaves
   the layout size and the output extent unchanged. */
static inline cod_region_storage_status cod_region_layout_reserve(size_t *used, size_t capacity,
    size_t count, size_t element_size, size_t alignment, cod_region_extent *extent) {
#ifdef __cplusplus
  const size_t max_alignment = alignof(max_align_t);
#else
  const size_t max_alignment = _Alignof(max_align_t);
#endif
  if (capacity > (size_t)PTRDIFF_MAX) return COD_REGION_STORAGE_TOO_LARGE;
  if (*used > capacity || !element_size || !alignment || alignment > max_alignment ||
      (alignment & (alignment - 1))) return COD_REGION_STORAGE_INVALID;
  if (count > SIZE_MAX / element_size) return COD_REGION_STORAGE_TOO_LARGE;
  const size_t size = count * element_size;
  if (!size) {
    extent->offset = 0;
    extent->size = 0;
    return COD_REGION_STORAGE_OK;
  }
  const size_t padding = (alignment - (*used & (alignment - 1))) & (alignment - 1);
  const size_t remaining = capacity - *used;
  if (padding > remaining || size > remaining - padding) return COD_REGION_STORAGE_FULL;
  extent->offset = *used + padding;
  extent->size = size;
  *used = extent->offset + size;
  return COD_REGION_STORAGE_OK;
}

/* A live owner uses the same layout rules as a preflight reservation. */
static inline cod_region_storage_status cod_region_storage_reserve(cod_region_storage *region,
    size_t count, size_t element_size, size_t alignment, cod_region_extent *extent) {
  if (!region->bytes) return COD_REGION_STORAGE_INVALID;
  return cod_region_layout_reserve(&region->used, region->capacity, count, element_size, alignment, extent);
}

/* Resolve an extent while its owner is live. This checks the byte range.
   The caller supplies the extent's provenance, type and initialization. */
static inline void *cod_region_storage_address(cod_region_storage *region, cod_region_extent extent) {
  if (!region->bytes || extent.offset > region->used || extent.size > region->used - extent.offset) return NULL;
  return region->bytes + extent.offset;
}

#ifdef __cplusplus
}
#endif
#endif
