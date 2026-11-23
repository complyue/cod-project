#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "path_owner.h"
#include "toolchain_path.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <spawn.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

typedef struct {
    uint8_t *bytes;
    size_t count;
    size_t capacity;
} byte_capture;

typedef struct {
    byte_capture output;
    byte_capture diagnostics;
    int wait_status;
} package_capture;

static void release_capture(package_capture *capture) {
    if (!capture) return;
    free(capture->diagnostics.bytes);
    free(capture->output.bytes);
    free(capture);
}

static const package_capture *selected_capture(cod_native_PackageInputCapture_borrow loan) {
    if (!loan.owner || loan.payload != loan.owner || (uint64_t)(uintptr_t)loan.owner != loan.owner)
        return NULL;
    return (const package_capture *)(uintptr_t)loan.owner;
}


static void close_pipe(int endpoints[2]) {
    for (size_t end = 0; end < 2; ++end) {
        if (endpoints[end] >= 0) close(endpoints[end]);
        endpoints[end] = -1;
    }
}

static int open_capture_pipe(int endpoints[2]) {
    if (pipe2(endpoints, O_CLOEXEC) < 0) return errno;
    for (size_t end = 0; end < 2; ++end) {
        if (endpoints[end] > STDERR_FILENO) continue;
        const int fd = fcntl(endpoints[end], F_DUPFD_CLOEXEC, STDERR_FILENO + 1);
        if (fd < 0) {
            const int error = errno;
            close_pipe(endpoints);
            return error;
        }
        close(endpoints[end]);
        endpoints[end] = fd;
    }
    return 0;
}

static int start_producer(const char *tool, const char *entry, int output, int diagnostics,
                          pid_t *child) {
    posix_spawn_file_actions_t actions;
    int status = posix_spawn_file_actions_init(&actions);
    if (status != 0) return status;
    status = posix_spawn_file_actions_adddup2(&actions, output, STDOUT_FILENO);
    if (status == 0)
        status = posix_spawn_file_actions_adddup2(&actions, diagnostics, STDERR_FILENO);
    if (status == 0) {
        char *arguments[] = {(char *)tool, "compiler-inputs", (char *)entry, NULL};
        status = posix_spawn(child, tool, &actions, NULL, arguments, environ);
    }
    posix_spawn_file_actions_destroy(&actions);
    return status;
}

static int append_bytes(byte_capture *capture, const uint8_t *bytes, size_t count) {
    const size_t limit = PTRDIFF_MAX;
    if (count > limit - capture->count) return EOVERFLOW;
    const size_t next_count = capture->count + count;
    if (next_count > capture->capacity) {
        size_t capacity = capture->capacity ? capture->capacity : 4096;
        while (capacity < next_count)
            capacity = capacity > limit / 2 ? limit : capacity * 2;
        uint8_t *next = realloc(capture->bytes, capacity);
        if (!next) return ENOMEM;
        capture->bytes = next;
        capture->capacity = capacity;
    }
    memcpy(capture->bytes + capture->count, bytes, count);
    capture->count = next_count;
    return 0;
}

static int collect_streams(int output, int diagnostics, package_capture *capture) {
    struct pollfd descriptors[] = {{output, POLLIN, 0}, {diagnostics, POLLIN, 0}};
    byte_capture *targets[] = {&capture->output, &capture->diagnostics};
    uint8_t bytes[16384];
    size_t remaining = 2;
    while (remaining) {
        if (poll(descriptors, 2, -1) < 0) {
            if (errno == EINTR) continue;
            return errno;
        }
        for (size_t i = 0; i < 2; ++i) {
            const short events = descriptors[i].revents;
            if (events & (POLLERR | POLLNVAL)) return EIO;
            if (!(events & (POLLIN | POLLHUP))) continue;
            const ssize_t count = read(descriptors[i].fd, bytes, sizeof(bytes));
            if (count < 0) {
                if (errno == EINTR) continue;
                return errno;
            }
            if (count == 0) {
                descriptors[i].fd = -1;
                --remaining;
                continue;
            }
            const int status = append_bytes(targets[i], bytes, (size_t)count);
            if (status != 0) return status;
        }
    }
    return 0;
}

static pid_t wait_producer(pid_t child, int *status, int options) {
    pid_t result;
    do {
        result = waitpid(child, status, options);
    } while (result < 0 && errno == EINTR);
    return result;
}

static int capture_inputs(const char *tool, const char *entry, package_capture *capture) {
    int output[2] = {-1, -1};
    int diagnostics[2] = {-1, -1};
    pid_t child;
    int status = open_capture_pipe(output);
    if (status == 0) status = open_capture_pipe(diagnostics);
    if (status == 0) status = start_producer(tool, entry, output[1], diagnostics[1], &child);
    if (status != 0) {
        close_pipe(diagnostics);
        close_pipe(output);
        return status;
    }
    close(output[1]);
    output[1] = -1;
    close(diagnostics[1]);
    diagnostics[1] = -1;
    status = collect_streams(output[0], diagnostics[0], capture);
    close_pipe(diagnostics);
    close_pipe(output);
    if (status == 0)
        return wait_producer(child, &capture->wait_status, 0) == child ? 0 : errno;
    if (wait_producer(child, &capture->wait_status, WNOHANG) == 0) {
        kill(child, SIGKILL);
        wait_producer(child, &capture->wait_status, 0);
    }
    return status;
}

cod_native_status cod_native_PackageInputCapture_construct_impl(
    cod_native_Path_borrow entry, uint64_t *resource, cod_native_PackageInputCapture_failure *failure) {
    const cod_booter_path_owner *path = cod_booter_path_selected(entry);
    if (!path) return COD_NATIVE_BOUNDARY_ERROR;
    char *tool = cod_booter_toolchain_path("codp", false);
    if (!tool) {
        failure->os_error = (uint64_t)errno;
        return COD_NATIVE_FAILURE;
    }
    package_capture *capture = calloc(1, sizeof(*capture));
    const int status = capture ? capture_inputs(tool, path->bytes, capture) : ENOMEM;
    free(tool);
    if (status != 0) {
        release_capture(capture);
        failure->os_error = (uint64_t)status;
        return COD_NATIVE_FAILURE;
    }
    *resource = (uint64_t)(uintptr_t)capture;
    return COD_NATIVE_OK;
}

void cod_native_PackageInputCapture_destroy_impl(uint64_t resource) {
    release_capture((package_capture *)(uintptr_t)resource);
}

cod_native_status cod_native_package_capture_status_impl(
    cod_native_PackageInputCapture_borrow loan, uint64_t *result) {
    const package_capture *capture = selected_capture(loan);
    if (!capture) return COD_NATIVE_BOUNDARY_ERROR;
    *result = (uint64_t)(unsigned)capture->wait_status;
    return COD_NATIVE_OK;
}

int cod_native_use_package_capture_validate_impl(
    cod_native_PackageInputCapture_borrow loan, const cod_native_use_package_capture_arrays *views) {
    const package_capture *capture = selected_capture(loan);
    return capture && views->array0.base == capture->output.bytes &&
        views->array0.count == capture->output.count &&
        views->array1.base == capture->diagnostics.bytes &&
        views->array1.count == capture->diagnostics.count;
}

cod_native_status cod_native_use_package_capture_impl(
    cod_native_PackageInputCapture_borrow loan, cod_native_use_package_capture_callback run,
    void *context, uint8_t *result) {
    const package_capture *capture = selected_capture(loan);
    if (!capture) return COD_NATIVE_BOUNDARY_ERROR;
    const cod_native_use_package_capture_arrays views = {
        {capture->output.bytes, capture->output.count},
        {capture->diagnostics.bytes, capture->diagnostics.count},
    };
    return run(context, &views, result) == COD_NATIVE_HERE_INVALID
        ? COD_NATIVE_BOUNDARY_ERROR : COD_NATIVE_OK;
}
