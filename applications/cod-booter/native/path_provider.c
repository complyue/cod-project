#define _XOPEN_SOURCE 700

#include "path_owner.h"
#include "toolchain_path.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static cod_booter_path_owner *path_owner(cod_native_PathOwner_borrow loan) {
    if (!loan.payload || loan.payload != loan.owner || (uint64_t)(uintptr_t)loan.owner != loan.owner) return NULL;
    return (cod_booter_path_owner *)(uintptr_t)loan.owner;
}

static const cod_booter_path_owner *canonical_path(cod_native_PathOwner_borrow loan) {
    const cod_booter_path_owner *owner = path_owner(loan);
    return owner && owner->state == COD_BOOTER_PATH_CANONICAL ? owner : NULL;
}

cod_native_status cod_native_PathOwner_construct_impl(uint64_t *resource, cod_native_PathOwner_failure *error) {
    cod_booter_path_owner *owner = calloc(1, sizeof(*owner));
    if (!owner) {
        error->os_error = ENOMEM;
        return COD_NATIVE_FAILURE;
    }
    *resource = (uint64_t)(uintptr_t)owner;
    return COD_NATIVE_OK;
}

void cod_native_PathOwner_destroy_impl(uint64_t resource) {
    cod_booter_path_owner *owner = (cod_booter_path_owner *)(uintptr_t)resource;
    if (!owner || owner->state == COD_BOOTER_PATH_LAUNCHER) return;
    free((void *)owner->bytes);
    free(owner);
}

cod_native_status cod_native_Path_construct_impl(cod_native_PathOwner_borrow loan, uint64_t *resource,
                                                cod_native_Path_failure *error) {
    const cod_booter_path_owner *owner = canonical_path(loan);
    if (!owner) {
        error->status = 1;
        return COD_NATIVE_FAILURE;
    }
    *resource = (uint64_t)(uintptr_t)owner->bytes;
    return COD_NATIVE_OK;
}

static int publish_canonical(cod_booter_path_owner *destination, const char *path) {
    char *canonical = realpath(path, NULL);
    if (!canonical) return errno;
    *destination = (cod_booter_path_owner){canonical, strlen(canonical), COD_BOOTER_PATH_CANONICAL};
    return 0;
}

cod_native_status cod_native_prepare_root_impl(cod_native_PathOwner_borrow loan, cod_native_Path_borrow input,
                                               uint64_t *result) {
    cod_booter_path_owner *destination = path_owner(loan);
    const cod_booter_path_owner *root = cod_booter_path_selected(input);
    if (!destination || !root) return COD_NATIVE_BOUNDARY_ERROR;
    *result = (uint64_t)(destination->state == COD_BOOTER_PATH_EMPTY
        ? publish_canonical(destination, root->bytes) : EINVAL);
    return COD_NATIVE_OK;
}

static int prepare_child(cod_booter_path_owner *destination, const cod_booter_path_owner *parent,
                         cod_native_prepare_child_view2 decoded, uint64_t used) {
    if (destination->state != COD_BOOTER_PATH_EMPTY || parent->state != COD_BOOTER_PATH_CANONICAL ||
        !used || used > decoded.count || !decoded.base || used >= (uint64_t)PTRDIFF_MAX) return EINVAL;
    const size_t length = (size_t)used;
    if (memchr(decoded.base, 0, length)) return EINVAL;
    size_t prefix = 0;
    if (decoded.base[0] != '/') {
        const char *separator = strrchr(parent->bytes, '/');
        if (!separator) return EINVAL;
        prefix = (size_t)(separator - parent->bytes) + 1;
    }
    if (prefix > (size_t)PTRDIFF_MAX - length - 1) return EOVERFLOW;
    char *path = malloc(prefix + length + 1);
    if (!path) return ENOMEM;
    memcpy(path, parent->bytes, prefix);
    memcpy(path + prefix, decoded.base, length);
    path[prefix + length] = 0;
    const int error = publish_canonical(destination, path);
    free(path);
    return error;
}

cod_native_status cod_native_prepare_child_impl(cod_native_PathOwner_borrow loan, cod_native_Path_borrow base,
                                                cod_native_prepare_child_view2 decoded, uint64_t used, uint64_t *result) {
    cod_booter_path_owner *destination = path_owner(loan);
    const cod_booter_path_owner *parent = cod_booter_path_selected(base);
    if (!destination || !parent) return COD_NATIVE_BOUNDARY_ERROR;
    *result = (uint64_t)prepare_child(destination, parent, decoded, used);
    return COD_NATIVE_OK;
}

typedef struct {
    const uint8_t *base;
    size_t count;
} path_bytes;

static path_bytes bounded_path(const uint8_t *base, uint64_t count, uint64_t first, uint64_t past) {
    if (!base || first >= past || past > count || past > (uint64_t)PTRDIFF_MAX)
        return (path_bytes){0};
    return (path_bytes){base + (size_t)first, (size_t)(past - first)};
}

static int join_module_path(path_bytes root, path_bytes module, char **joined) {
    if (!root.count || root.base[0] != '/' || !module.count || module.base[0] == '/' ||
        memchr(root.base, 0, root.count) || memchr(module.base, 0, module.count))
        return EINVAL;
    const size_t separator = root.base[root.count - 1] == '/' ? 0 : 1;
    const size_t limit = PTRDIFF_MAX - separator - 1;
    if (module.count > limit || root.count > limit - module.count) return EOVERFLOW;
    char *path = malloc(root.count + separator + module.count + 1);
    if (!path) return ENOMEM;
    memcpy(path, root.base, root.count);
    if (separator) path[root.count] = '/';
    memcpy(path + root.count + separator, module.base, module.count);
    path[root.count + separator + module.count] = 0;
    *joined = path;
    return 0;
}

static uint64_t module_path_result(cod_booter_path_owner *destination, path_bytes root, path_bytes module) {
    char *path = NULL;
    const int error = join_module_path(root, module, &path);
    if (error) return (uint64_t)error;
    uint64_t result;
    if (destination) {
        result = (uint64_t)(destination->state == COD_BOOTER_PATH_EMPTY
            ? publish_canonical(destination, path) : EINVAL);
    } else {
        struct stat info;
        if (lstat(path, &info) == 0) result = 0;
        else if (errno == ENOENT || errno == ENOTDIR) result = UINT64_MAX;
        else result = (uint64_t)errno;
    }
    free(path);
    return result;
}

cod_native_status cod_native_probe_package_module_impl(
    cod_native_probe_package_module_view0 root, uint64_t root_first, uint64_t root_past,
    cod_native_probe_package_module_view3 module, uint64_t first, uint64_t past, uint64_t *result) {
    *result = module_path_result(NULL, bounded_path(root.base, root.count, root_first, root_past),
        bounded_path(module.base, module.count, first, past));
    return COD_NATIVE_OK;
}

cod_native_status cod_native_prepare_package_module_impl(
    cod_native_PathOwner_borrow loan, cod_native_prepare_package_module_view1 root, uint64_t root_first,
    uint64_t root_past, cod_native_prepare_package_module_view4 module, uint64_t first, uint64_t past,
    uint64_t *result) {
    cod_booter_path_owner *destination = path_owner(loan);
    if (!destination) return COD_NATIVE_BOUNDARY_ERROR;
    *result = module_path_result(destination, bounded_path(root.base, root.count, root_first, root_past),
        bounded_path(module.base, module.count, first, past));
    return COD_NATIVE_OK;
}

cod_native_status cod_native_BundledDirectory_construct_impl(
    uint64_t *resource, cod_native_BundledDirectory_failure *failure) {
    char *directory = cod_booter_toolchain_path("lib/cod", true);
    if (!directory) {
        failure->os_error = (uint64_t)errno;
        return COD_NATIVE_FAILURE;
    }
    *resource = (uint64_t)(uintptr_t)directory;
    return COD_NATIVE_OK;
}

void cod_native_BundledDirectory_destroy_impl(uint64_t resource) {
    free((void *)(uintptr_t)resource);
}

static const char *bundle_directory(cod_native_BundledDirectory_borrow loan) {
    if (!loan.owner || loan.payload != loan.owner || (uint64_t)(uintptr_t)loan.owner != loan.owner)
        return NULL;
    return (const char *)(uintptr_t)loan.owner;
}

int cod_native_use_bundled_directory_validate_impl(
    cod_native_BundledDirectory_borrow loan, const cod_native_use_bundled_directory_arrays *views) {
    const char *directory = bundle_directory(loan);
    return directory && views->array0.base == (const uint8_t *)directory &&
        views->array0.count == strlen(directory);
}

cod_native_status cod_native_use_bundled_directory_impl(
    cod_native_BundledDirectory_borrow loan, cod_native_use_bundled_directory_callback run,
    void *context, uint8_t *result) {
    const char *directory = bundle_directory(loan);
    if (!directory) return COD_NATIVE_BOUNDARY_ERROR;
    const cod_native_use_bundled_directory_arrays views = {{(const uint8_t *)directory, strlen(directory)}};
    const cod_native_here_step completion = run(context, &views, result);
    return completion == COD_NATIVE_HERE_INVALID ? COD_NATIVE_BOUNDARY_ERROR : COD_NATIVE_OK;
}

cod_native_status cod_native_path_ready_impl(cod_native_PathOwner_borrow loan, COD_BOOL *result) {
    if (!path_owner(loan)) return COD_NATIVE_BOUNDARY_ERROR;
    *result = canonical_path(loan) != NULL;
    return COD_NATIVE_OK;
}

int cod_native_path_key_loan_validate_impl(cod_native_PathOwner_borrow loan,
                                          const cod_native_path_key_loan_arrays *views) {
    const cod_booter_path_owner *path = canonical_path(loan);
    return path && views->array0.base == (const uint8_t *)path->bytes && views->array0.count == path->length;
}

cod_native_status cod_native_path_key_loan_impl(cod_native_PathOwner_borrow loan, cod_native_path_key_loan_callback run,
                                                void *context, uint8_t *result) {
    const cod_booter_path_owner *path = canonical_path(loan);
    if (!path) return COD_NATIVE_BOUNDARY_ERROR;
    const cod_native_path_key_loan_arrays views = {{(const uint8_t *)path->bytes, path->length}};
    const cod_native_here_step completion = run(context, &views, result);
    return completion == COD_NATIVE_HERE_INVALID ? COD_NATIVE_BOUNDARY_ERROR : COD_NATIVE_OK;
}
