#define _POSIX_C_SOURCE 200809L

#include "../../lang_llvm_object_c.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* The result owns its object bytes and any LLVM diagnostic. */
void cod_llvm_object_release(cod_llvm_object_result *result) {
  if (result->object) LLVMDisposeMemoryBuffer(result->object);
  if (result->diagnostic) LLVMDisposeMessage(result->diagnostic);
  result->object = NULL;
  result->diagnostic = NULL;
}

/* The caller initializes the target and its assembly printer. A module that
   omits its triple uses LLVM's default target. Construct target-sensitive IR
   against a layout compatible with the selected target before calling this. */
cod_llvm_object_result cod_llvm_emit_object(LLVMModuleRef module) {
  cod_llvm_object_result result = {COD_LLVM_OBJECT_TARGET_UNAVAILABLE, NULL, NULL};
  const char *triple = LLVMGetTarget(module);
  char *default_triple = NULL;
  if (!*triple) {
    default_triple = LLVMGetDefaultTargetTriple();
    if (!default_triple) return result;
    triple = default_triple;
  }

  LLVMTargetRef target = NULL;
  LLVMTargetMachineRef machine = NULL;
  if (!LLVMGetTargetFromTriple(triple, &target, &result.diagnostic))
    machine = LLVMCreateTargetMachine(target, triple, "generic", "",
        LLVMCodeGenLevelDefault, LLVMRelocPIC, LLVMCodeModelDefault);
  if (default_triple) {
    if (machine) LLVMSetTarget(module, default_triple);
    LLVMDisposeMessage(default_triple);
  }
  if (!machine) return result;

  LLVMTargetDataRef layout = LLVMCreateTargetDataLayout(machine);
  LLVMSetModuleDataLayout(module, layout);
  LLVMDisposeTargetData(layout);

  result.status = COD_LLVM_OBJECT_INVALID_MODULE;
  if (!LLVMVerifyModule(module, LLVMReturnStatusAction, &result.diagnostic)) {
    if (result.diagnostic) LLVMDisposeMessage(result.diagnostic);
    result.diagnostic = NULL;
    LLVMMemoryBufferRef object = NULL;
    if (LLVMTargetMachineEmitToMemoryBuffer(machine, module, LLVMObjectFile, &result.diagnostic, &object) || !object) {
      if (object) LLVMDisposeMemoryBuffer(object);
      result.status = COD_LLVM_OBJECT_EMISSION_FAILED;
    } else {
      result.object = object;
      result.status = COD_LLVM_OBJECT_OK;
    }
  }
  LLVMDisposeTargetMachine(machine);
  return result;
}

bool cod_llvm_publish_object(LLVMMemoryBufferRef object, const char *path) {
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
