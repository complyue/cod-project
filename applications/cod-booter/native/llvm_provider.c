#include "cod_providers.h"

#include "compile_diagnostic.h"
#include "lang_llvm_commands_c.h"
#include "lang_llvm_object_c.h"

#include <stdlib.h>

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
                                                cod_native_llvm_construct_view2 source,
                                                uint64_t used, uint64_t labels, uint64_t slots, uint8_t *status) {
  cod_llvm_session *session = llvm_session(resource);
  if (!session) return COD_NATIVE_BOUNDARY_ERROR;
  if (used > commands.count || (uint64_t)(size_t)labels != labels || (uint64_t)(size_t)slots != slots ||
      (uint64_t)(size_t)source.count != source.count) {
    *status = llvm_invalid_command_range;
    return COD_NATIVE_OK;
  }
  *status = cod_llvm_construct_source_records(session, commands.base, (size_t)used, (size_t)labels, (size_t)slots,
                                              source.base, (size_t)source.count)
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

/* The launcher registers the native target and its assembly printer. */
cod_native_status cod_native_llvm_emit_object_impl(cod_native_LLVMSession_borrow resource, const char *path,
                                                  uint8_t *status) {
  cod_llvm_session *session = llvm_session(resource);
  if (!session || !path) return COD_NATIVE_BOUNDARY_ERROR;
  cod_llvm_object_result result = cod_llvm_emit_object(session->module);
  *status = llvm_emission_failed;
  if (result.status == COD_LLVM_OBJECT_OK)
    *status = cod_llvm_publish_object(result.object, path) ? llvm_ok : llvm_publication_failed;
  cod_llvm_object_release(&result);
  return COD_NATIVE_OK;
}
