#define _POSIX_C_SOURCE 200809L

#include "provider_output.h"
#include "path_owner.h"
#include "native/provider_text.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

static bool write_text(FILE *file, cod_native_text text) {
    const size_t size = (size_t)text.size;
    return !size || fwrite(text.bytes, 1, size, file) == size;
}

static FILE *open_staging_file(char *path) {
    const int fd = mkstemp(path);
    if (fd < 0) return NULL;
    FILE *file = fdopen(fd, "w");
    if (!file) {
        close(fd);
        unlink(path);
    }
    return file;
}

static bool regular_output(const char *path, bool *exists) {
    struct stat state;
    *exists = lstat(path, &state) == 0;
    return *exists ? S_ISREG(state.st_mode) : errno == ENOENT;
}

static bool publish_pair(const char *header_path, const char *stubs_path, const char *header_stage,
                         const char *stubs_stage, char *backup) {
    bool exists;
    if (!regular_output(stubs_path, &exists) || !regular_output(header_path, &exists)) return false;
    if (exists) {
        const int fd = mkstemp(backup);
        if (fd < 0) return false;
        close(fd);
        if (unlink(backup) != 0 || link(header_path, backup) != 0) return false;
    }
    if (rename(header_stage, header_path) != 0) {
        if (exists) unlink(backup);
        return false;
    }
    if (rename(stubs_stage, stubs_path) != 0) {
        if (exists) {
            /* Leave the backup available if a new I/O error prevents restoration. */
            (void)rename(backup, header_path);
        } else {
            (void)unlink(header_path);
        }
        return false;
    }
    if (exists) unlink(backup);
    return true;
}

static uint32_t write_files(const char *directory, size_t length, cod_native_text header_text, cod_native_text stubs_text) {
    const size_t suffix = sizeof("/.cod_providers.previous.XXXXXX");
    if (length > SIZE_MAX / 5 - suffix) return provider_output_memory;
    const size_t width = length + suffix;
    char *paths = malloc(5 * width);
    if (!paths) return provider_output_memory;
    char *header_path = paths, *stubs_path = paths + width;
    char *header_stage = paths + 2 * width, *stubs_stage = paths + 3 * width;
    char *backup = paths + 4 * width;
    snprintf(header_path, width, "%s/" COD_PROVIDER_HEADER_NAME, directory);
    snprintf(stubs_path, width, "%s/" COD_PROVIDER_STUBS_NAME, directory);
    snprintf(header_stage, width, "%s/." COD_PROVIDER_HEADER_NAME ".XXXXXX", directory);
    snprintf(stubs_stage, width, "%s/." COD_PROVIDER_STUBS_NAME ".XXXXXX", directory);
    snprintf(backup, width, "%s/.cod_providers.previous.XXXXXX", directory);
    uint32_t status = provider_output_io;
    FILE *header = NULL, *stubs = NULL;
    const int directory_fd = (mkdir(directory, 0777) == 0 || errno == EEXIST)
        ? open(directory, O_RDONLY | O_DIRECTORY | O_CLOEXEC) : -1;
    if (directory_fd >= 0 && flock(directory_fd, LOCK_EX) == 0) {
        header = open_staging_file(header_stage);
        if (header) stubs = open_staging_file(stubs_stage);
        if (stubs && write_text(header, header_text) && write_text(stubs, stubs_text)) status = 0;
    }
    const bool header_staged = header != NULL, stubs_staged = stubs != NULL;
    if (header_staged && fclose(header) != 0 && !status) status = provider_output_io;
    if (stubs_staged && fclose(stubs) != 0 && !status) status = provider_output_io;
    if (!status && !publish_pair(header_path, stubs_path, header_stage, stubs_stage, backup)) status = provider_output_io;
    if (header_staged) unlink(header_stage);
    if (stubs_staged) unlink(stubs_stage);
    if (directory_fd >= 0) close(directory_fd);
    free(paths);
    return status;
}

cod_native_status cod_native_publish_provider_pair_impl(cod_native_Path_borrow directory,
    const cod_native_text *header, const cod_native_text *stubs, uint64_t *status) {
    const cod_booter_path_owner *path = cod_booter_path_selected(directory);
    if (!path || path->length > PTRDIFF_MAX || path->bytes[path->length] != 0 ||
        memchr(path->bytes, 0, path->length) || !header || !stubs ||
        header->size != (size_t)header->size || stubs->size != (size_t)stubs->size ||
        (header->size && !header->bytes) || (stubs->size && !stubs->bytes))
        return COD_NATIVE_BOUNDARY_ERROR;
    *status = write_files(path->bytes, path->length, *header, *stubs);
    return COD_NATIVE_OK;
}
