/*===--- cod_source.h - Read-only indexed source revisions ----------------===*/
#ifndef COD_SOURCE_H
#define COD_SOURCE_H

#include "cod_region_storage.h"

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cod_source_span {
  size_t first;
  size_t past;
} cod_source_span;

/* A view borrows both buffers from one revision. Keep the revision live and
   its file contents stable until all uses of the view are finished. */
typedef struct cod_source_view {
  const unsigned char *bytes;
  size_t size;
  const cod_source_span *lines;
  size_t line_count;
} cod_source_view;

typedef enum cod_source_status {
  COD_SOURCE_OK,
  COD_SOURCE_SYSTEM_ERROR,
  COD_SOURCE_NOT_REGULAR,
  COD_SOURCE_TOO_LARGE,
  COD_SOURCE_NO_MEMORY,
  COD_SOURCE_CHANGED
} cod_source_status;

typedef struct cod_source_error {
  cod_source_status status;
  unsigned long system_error;
} cod_source_error;

typedef struct cod_source_revision cod_source_revision;

struct cod_source_revision {
  cod_source_view view;
  cod_region_storage line_storage;
};

static inline void cod_source_release(cod_source_revision *source) {
  if (!source) return;
  if (source->view.bytes) {
#ifdef _WIN32
    UnmapViewOfFile(source->view.bytes);
#else
    munmap((void *)source->view.bytes, source->view.size);
#endif
  }
  cod_region_storage_release(&source->line_storage);
  free(source);
}

static inline cod_source_status cod_source_map_file(cod_source_revision *source, const char *path,
                                                     unsigned long *system_error) {
#ifdef _WIN32
  const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, NULL, 0);
  if (!length) {
    *system_error = GetLastError();
    return COD_SOURCE_SYSTEM_ERROR;
  }
  if ((size_t)length > SIZE_MAX / sizeof(wchar_t)) return COD_SOURCE_TOO_LARGE;
  wchar_t *filename = (wchar_t *)malloc((size_t)length * sizeof(wchar_t));
  if (!filename) return COD_SOURCE_NO_MEMORY;
  if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, filename, length)) {
    *system_error = GetLastError();
    free(filename);
    return COD_SOURCE_SYSTEM_ERROR;
  }
  HANDLE file = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                            NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
  if (file == INVALID_HANDLE_VALUE) *system_error = GetLastError();
  free(filename);
  if (file == INVALID_HANDLE_VALUE) return COD_SOURCE_SYSTEM_ERROR;

  cod_source_status result = COD_SOURCE_OK;
  BY_HANDLE_FILE_INFORMATION info;
  LARGE_INTEGER size;
  if (GetFileType(file) != FILE_TYPE_DISK) result = COD_SOURCE_NOT_REGULAR;
  else if (!GetFileInformationByHandle(file, &info)) result = COD_SOURCE_SYSTEM_ERROR;
  else if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) result = COD_SOURCE_NOT_REGULAR;
  else if (!GetFileSizeEx(file, &size)) result = COD_SOURCE_SYSTEM_ERROR;
  else if (size.QuadPart < 0 || (uintmax_t)size.QuadPart > SIZE_MAX) result = COD_SOURCE_TOO_LARGE;
  else if (size.QuadPart) {
    HANDLE mapping = CreateFileMappingW(file, NULL, PAGE_READONLY, 0, 0, NULL);
    if (mapping) {
      source->view.bytes = (const unsigned char *)MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, (SIZE_T)size.QuadPart);
      if (source->view.bytes) source->view.size = (size_t)size.QuadPart;
      else {
        result = COD_SOURCE_SYSTEM_ERROR;
        *system_error = GetLastError();
      }
      CloseHandle(mapping);
    } else result = COD_SOURCE_SYSTEM_ERROR;
  }
  if (result == COD_SOURCE_SYSTEM_ERROR && !*system_error) *system_error = GetLastError();
  CloseHandle(file);
  return result;
#else
  /* Opening needs to remain nonblocking even if the path names a FIFO. The
     descriptor check applies to the same file that will be mapped. */
  int flags = O_RDONLY | O_NONBLOCK;
#ifdef O_CLOEXEC
  flags |= O_CLOEXEC;
#endif
  int file;
  do { file = open(path, flags); } while (file < 0 && errno == EINTR);
  if (file < 0) {
    *system_error = (unsigned long)errno;
    return COD_SOURCE_SYSTEM_ERROR;
  }
#ifndef O_CLOEXEC
  if (fcntl(file, F_SETFD, FD_CLOEXEC) == -1) {
    *system_error = (unsigned long)errno;
    close(file);
    return COD_SOURCE_SYSTEM_ERROR;
  }
#endif

  cod_source_status result = COD_SOURCE_OK;
  struct stat info;
  if (fstat(file, &info) != 0) result = COD_SOURCE_SYSTEM_ERROR;
  else if (!S_ISREG(info.st_mode)) result = COD_SOURCE_NOT_REGULAR;
  else if (info.st_size < 0 || (uintmax_t)info.st_size > SIZE_MAX) result = COD_SOURCE_TOO_LARGE;
  else if (info.st_size) {
    const size_t size = (size_t)info.st_size;
    void *mapping = mmap(NULL, size, PROT_READ, MAP_PRIVATE, file, 0);
    if (mapping == MAP_FAILED) result = COD_SOURCE_SYSTEM_ERROR;
    else if (!mapping) {
      munmap(mapping, size);
      close(file);
      *system_error = EFAULT;
      return COD_SOURCE_SYSTEM_ERROR;
    }
    else {
      source->view.bytes = (const unsigned char *)mapping;
      source->view.size = size;
    }
  }
  if (result == COD_SOURCE_SYSTEM_ERROR) *system_error = (unsigned long)errno;
  close(file);
  return result;
#endif
}

static inline cod_source_status cod_source_index_lines(cod_source_revision *source) {
  const unsigned char *bytes = source->view.bytes;
  const size_t size = source->view.size;
  if (size > (size_t)PTRDIFF_MAX) return COD_SOURCE_TOO_LARGE;
  size_t count = 1;
  for (size_t offset = 0; offset < size; ++offset) count += bytes[offset] == '\n';
  if (count > (size_t)PTRDIFF_MAX / sizeof(cod_source_span)) return COD_SOURCE_TOO_LARGE;

  const cod_region_storage_status opened = cod_region_storage_open(&source->line_storage, count * sizeof(cod_source_span));
  if (opened == COD_REGION_STORAGE_NO_MEMORY) return COD_SOURCE_NO_MEMORY;
  if (opened != COD_REGION_STORAGE_OK) return COD_SOURCE_TOO_LARGE;
#ifdef __cplusplus
  const size_t alignment = alignof(cod_source_span);
#else
  const size_t alignment = _Alignof(cod_source_span);
#endif
  cod_region_extent index;
  if (cod_region_storage_reserve(&source->line_storage, count, sizeof(cod_source_span), alignment, &index) !=
      COD_REGION_STORAGE_OK) return COD_SOURCE_TOO_LARGE;
  cod_source_span *lines = (cod_source_span *)cod_region_storage_address(&source->line_storage, index);

  size_t first = 0;
  for (size_t line = 0; line < count; ++line) {
    const unsigned char *lf = first < size ? (const unsigned char *)memchr(bytes + first, '\n', size - first) : NULL;
    if ((lf != NULL) != (line + 1 < count)) return COD_SOURCE_CHANGED;
    const size_t past = lf ? (size_t)(lf - bytes) : size;
    lines[line].first = first;
    lines[line].past = past;
    if (lf) first = past + 1;
  }
  source->view.lines = lines;
  source->view.line_count = count;
  return COD_SOURCE_OK;
}

/* Open a regular file by UTF-8 path and build the index before publishing
   the revision. The empty file and the position after a final LF each have
   an empty line. Every line span excludes its LF. */
static inline cod_source_revision *cod_source_open_readonly(const char *path, cod_source_error *error) {
  error->status = COD_SOURCE_OK;
  error->system_error = 0;
  cod_source_revision *source = (cod_source_revision *)malloc(sizeof(*source));
  if (!source) {
    error->status = COD_SOURCE_NO_MEMORY;
    return NULL;
  }
  cod_source_view empty = {NULL, 0, NULL, 0};
  source->view = empty;
  cod_region_storage empty_storage = {NULL, 0, 0};
  source->line_storage = empty_storage;
  error->status = cod_source_map_file(source, path, &error->system_error);
  if (error->status == COD_SOURCE_OK) error->status = cod_source_index_lines(source);
  if (error->status == COD_SOURCE_OK) return source;
  cod_source_release(source);
  return NULL;
}

static inline cod_source_view cod_source_borrow(const cod_source_revision *source) {
  return source->view;
}

/* Diagnostics use one-based lines and byte columns, including at EOF. */
static inline int cod_source_line_column(cod_source_view source, size_t offset, size_t *line, size_t *column) {
  if (offset > source.size || !source.line_count) return 0;
  size_t first = 0;
  size_t past = source.line_count;
  while (past - first > 1) {
    const size_t middle = first + (past - first) / 2;
    if (source.lines[middle].first <= offset) first = middle;
    else past = middle;
  }
  *line = first + 1;
  *column = offset - source.lines[first].first + 1;
  return 1;
}

#ifdef __cplusplus
}
#endif
#endif
