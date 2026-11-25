#define _POSIX_C_SOURCE 200809L

#include "cod_providers.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
    char **names;
    size_t count;
    size_t selected;
    bool ready;
} directory_snapshot;

static directory_snapshot *snapshot_owner(cod_native_DirectorySnapshot_borrow loan) {
    if (!loan.owner || loan.owner != loan.payload || (uint64_t)(uintptr_t)loan.owner != loan.owner) return NULL;
    return (directory_snapshot *)(uintptr_t)loan.owner;
}

static int copy_directory_path(const uint8_t *base, uint64_t count, uint64_t used, char **path) {
    if (!base || !used || used > count || used >= (uint64_t)PTRDIFF_MAX || base[0] != '/' ||
        memchr(base, 0, (size_t)used)) return EINVAL;
    char *copy = malloc((size_t)used + 1);
    if (!copy) return ENOMEM;
    memcpy(copy, base, (size_t)used);
    copy[used] = 0;
    *path = copy;
    return 0;
}

cod_native_status cod_native_probe_directory_impl(cod_native_probe_directory_view0 input, uint64_t used,
                                                  uint64_t *result) {
    char *path = NULL;
    const int error = copy_directory_path(input.base, input.count, used, &path);
    if (error) {
        *result = (uint64_t)error;
        return COD_NATIVE_OK;
    }
    struct stat info;
    if (lstat(path, &info) != 0) {
        *result = errno == ENOENT || errno == ENOTDIR ? UINT64_MAX : (uint64_t)errno;
    } else if (S_ISLNK(info.st_mode) && stat(path, &info) != 0) {
        *result = (uint64_t)errno;
    } else {
        *result = S_ISDIR(info.st_mode) ? 0 : ENOTDIR;
    }
    free(path);
    return COD_NATIVE_OK;
}

cod_native_status cod_native_DirectorySnapshot_construct_impl(uint64_t *resource,
                                                              cod_native_DirectorySnapshot_failure *failure) {
    directory_snapshot *snapshot = calloc(1, sizeof(*snapshot));
    if (!snapshot) {
        failure->os_error = ENOMEM;
        return COD_NATIVE_FAILURE;
    }
    snapshot->selected = SIZE_MAX;
    *resource = (uint64_t)(uintptr_t)snapshot;
    return COD_NATIVE_OK;
}

static void release_names(directory_snapshot *snapshot) {
    for (size_t index = 0; index < snapshot->count; ++index) free(snapshot->names[index]);
    free(snapshot->names);
}

void cod_native_DirectorySnapshot_destroy_impl(uint64_t resource) {
    directory_snapshot *snapshot = (directory_snapshot *)(uintptr_t)resource;
    if (!snapshot) return;
    release_names(snapshot);
    free(snapshot);
}

static int retain_name(directory_snapshot *snapshot, size_t *capacity, const char *name) {
    if (snapshot->count == *capacity) {
        const size_t limit = PTRDIFF_MAX / sizeof(*snapshot->names);
        if (*capacity >= limit) return EOVERFLOW;
        const size_t next = !*capacity ? 8 : *capacity > limit / 2 ? limit : *capacity * 2;
        char **buffer = realloc(snapshot->names, next * sizeof(*buffer));
        if (!buffer) return ENOMEM;
        snapshot->names = buffer;
        *capacity = next;
    }
    char *copy = strdup(name);
    if (!copy) return ENOMEM;
    snapshot->names[snapshot->count++] = copy;
    return 0;
}

static int compare_names(const void *left, const void *right) {
    return strcmp(*(char *const *)left, *(char *const *)right);
}

static int collect_names(DIR *directory, directory_snapshot *staged) {
    size_t capacity = 0;
    for (;;) {
        errno = 0;
        struct dirent *entry = readdir(directory);
        if (!entry) return errno;
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
        struct stat info;
        if (fstatat(dirfd(directory), entry->d_name, &info, 0) != 0) {
            const int error = errno;
            if ((error == ENOENT || error == ENOTDIR) &&
                fstatat(dirfd(directory), entry->d_name, &info, AT_SYMLINK_NOFOLLOW) == 0 &&
                S_ISLNK(info.st_mode)) continue;
            return error;
        }
        if (S_ISDIR(info.st_mode)) {
            const int error = retain_name(staged, &capacity, entry->d_name);
            if (error) return error;
        }
    }
}

cod_native_status cod_native_read_directory_impl(cod_native_DirectorySnapshot_borrow loan,
                                                 cod_native_read_directory_view1 input, uint64_t used,
                                                 uint64_t *result) {
    directory_snapshot *snapshot = snapshot_owner(loan);
    if (!snapshot || snapshot->ready) return COD_NATIVE_BOUNDARY_ERROR;
    char *path = NULL;
    int error = copy_directory_path(input.base, input.count, used, &path);
    if (error) {
        *result = (uint64_t)error;
        return COD_NATIVE_OK;
    }
    DIR *directory = opendir(path);
    error = directory ? 0 : errno;
    free(path);
    if (error) {
        *result = (uint64_t)error;
        return COD_NATIVE_OK;
    }

    directory_snapshot staged = {.selected = SIZE_MAX};
    error = collect_names(directory, &staged);
    if (closedir(directory) != 0 && !error) error = errno;
    if (error) {
        release_names(&staged);
    } else {
        if (staged.count > 1) qsort(staged.names, staged.count, sizeof(*staged.names), compare_names);
        staged.ready = true;
        *snapshot = staged;
    }
    *result = (uint64_t)error;
    return COD_NATIVE_OK;
}

cod_native_status cod_native_directory_name_count_impl(cod_native_DirectorySnapshot_borrow loan, uint64_t *value) {
    const directory_snapshot *snapshot = snapshot_owner(loan);
    if (!snapshot || !snapshot->ready) return COD_NATIVE_BOUNDARY_ERROR;
    *value = snapshot->count;
    return COD_NATIVE_OK;
}

cod_native_status cod_native_directory_name_size_impl(cod_native_DirectorySnapshot_borrow loan,
                                                      uint64_t index, uint64_t *value) {
    const directory_snapshot *snapshot = snapshot_owner(loan);
    if (!snapshot || !snapshot->ready || index >= snapshot->count) return COD_NATIVE_BOUNDARY_ERROR;
    *value = strlen(snapshot->names[index]);
    return COD_NATIVE_OK;
}

cod_native_status cod_native_select_directory_name_impl(cod_native_DirectorySnapshot_borrow loan,
                                                        uint64_t index, uint64_t *value) {
    directory_snapshot *snapshot = snapshot_owner(loan);
    if (!snapshot || !snapshot->ready) return COD_NATIVE_BOUNDARY_ERROR;
    if (index >= snapshot->count) {
        *value = EINVAL;
    } else {
        snapshot->selected = (size_t)index;
        *value = 0;
    }
    return COD_NATIVE_OK;
}

static const char *selected_name(cod_native_DirectorySnapshot_borrow loan) {
    const directory_snapshot *snapshot = snapshot_owner(loan);
    return snapshot && snapshot->ready && snapshot->selected < snapshot->count
        ? snapshot->names[snapshot->selected] : NULL;
}

int cod_native_use_directory_name_loan_validate_impl(cod_native_DirectorySnapshot_borrow loan,
                                                const cod_native_use_directory_name_loan_arrays *views) {
    const char *name = selected_name(loan);
    return name && views->array0.base == (const uint8_t *)name && views->array0.count == strlen(name);
}

cod_native_status cod_native_use_directory_name_loan_impl(cod_native_DirectorySnapshot_borrow loan,
                                                     cod_native_use_directory_name_loan_callback run, void *context,
                                                     cod_enum_CompilerStatusx2ecod_14_CompilerStatus *value) {
    const char *name = selected_name(loan);
    if (!name) return COD_NATIVE_BOUNDARY_ERROR;
    const cod_native_use_directory_name_loan_arrays views = {{(const uint8_t *)name, strlen(name)}};
    return run(context, &views, value) == COD_NATIVE_HERE_INVALID ? COD_NATIVE_BOUNDARY_ERROR : COD_NATIVE_OK;
}
