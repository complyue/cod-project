#define _POSIX_C_SOURCE 200809L

#include "cod_providers.h"

#include "lang_llvm_commands_c.h"
#include "lang_llvm_object_c.h"

#include <unistd.h>

_Static_assert(UINTPTR_MAX <= UINT64_MAX, "LLVM handles require a pointer that fits in a handle word");
_Static_assert(sizeof(cod_native_llvm_construct_row1) == sizeof(cod_llvm_command), "unexpected command width");
#define COMMAND_OFFSET(field) \
  _Static_assert(offsetof(cod_native_llvm_construct_row1, field) == offsetof(cod_llvm_command, field), \
                 "unexpected command field position")
COMMAND_OFFSET(opcode);
COMMAND_OFFSET(a);
COMMAND_OFFSET(b);
COMMAND_OFFSET(c);
COMMAND_OFFSET(d);
#undef COMMAND_OFFSET

enum {
  llvm_ok, llvm_invalid_command_range, llvm_construction_failed, llvm_verification_failed,
  llvm_emission_failed, llvm_publication_failed
};

cod_native_status cod_native_LLVMSession_construct_impl(uint64_t *resource, cod_native_LLVMSession_failure *failure) {
  cod_llvm_session *session = calloc(1, sizeof(*session));
  if (!session || !cod_llvm_session_open(session, "cod-booter")) {
    free(session);
    failure->status = llvm_construction_failed;
    return COD_NATIVE_FAILURE;
  }
  *resource = (uint64_t)(uintptr_t)session;
  return COD_NATIVE_OK;
}

void cod_native_LLVMSession_destroy_impl(uint64_t resource) {
  cod_llvm_session *session = (cod_llvm_session *)(uintptr_t)resource;
  cod_llvm_session_release(session);
  free(session);
}

static cod_llvm_session *llvm_session(cod_native_LLVMSession_borrow handle) {
  return handle.payload == handle.owner ? (cod_llvm_session *)(uintptr_t)handle.payload : NULL;
}

cod_native_status cod_native_llvm_construct_impl(cod_native_LLVMSession_borrow resource,
                                                cod_native_llvm_construct_view1 commands,
                                                uint64_t used, uint64_t labels, uint64_t slots, uint8_t *status) {
  cod_llvm_session *session = llvm_session(resource);
  if (!session) return COD_NATIVE_BOUNDARY_ERROR;
  if (used > commands.count || (uint64_t)(size_t)labels != labels || (uint64_t)(size_t)slots != slots) {
    *status = llvm_invalid_command_range;
    return COD_NATIVE_OK;
  }
  *status = cod_llvm_construct_word_records(session, commands.base, (size_t)used, (size_t)labels, (size_t)slots)
      ? llvm_ok : llvm_construction_failed;
  return COD_NATIVE_OK;
}

cod_native_status cod_native_llvm_verify_impl(cod_native_LLVMSession_borrow resource, uint8_t *status) {
  const cod_llvm_session *session = llvm_session(resource);
  if (!session) return COD_NATIVE_BOUNDARY_ERROR;
  char *message = NULL;
  *status = LLVMVerifyModule(session->module, LLVMReturnStatusAction, &message) ? llvm_verification_failed : llvm_ok;
  if (message) LLVMDisposeMessage(message);
  return COD_NATIVE_OK;
}

static bool publish_object(LLVMMemoryBufferRef object, const char *path) {
  static const char suffix[] = ".tmp.XXXXXX";
  const size_t length = strlen(path);
  if (length > SIZE_MAX - sizeof(suffix)) return false;
  char *temporary = malloc(length + sizeof(suffix));
  if (!temporary) return false;
  memcpy(temporary, path, length);
  memcpy(temporary + length, suffix, sizeof(suffix));
  const int fd = mkstemp(temporary);
  bool published = false;
  if (fd >= 0) {
    FILE *stream = fdopen(fd, "wb");
    if (stream) {
      const size_t size = LLVMGetBufferSize(object);
      const bool written = fwrite(LLVMGetBufferStart(object), 1, size, stream) == size;
      const bool closed = fclose(stream) == 0;
      published = written && closed && rename(temporary, path) == 0;
    } else close(fd);
    if (!published) unlink(temporary);
  }
  free(temporary);
  return published;
}

/* The launcher registers the native target and its assembly printer. */
cod_native_status cod_native_llvm_emit_object_impl(cod_native_LLVMSession_borrow resource, cod_native_Path_borrow output,
                                                  uint8_t *status) {
  cod_llvm_session *session = llvm_session(resource);
  const char *path = (const char *)(uintptr_t)output.payload;
  if (!session || !path) return COD_NATIVE_BOUNDARY_ERROR;
  cod_llvm_object_result result = cod_llvm_emit_object(session->module);
  *status = llvm_emission_failed;
  if (result.status == COD_LLVM_OBJECT_OK)
    *status = publish_object(result.object, path) ? llvm_ok : llvm_publication_failed;
  cod_llvm_object_release(&result);
  return COD_NATIVE_OK;
}
