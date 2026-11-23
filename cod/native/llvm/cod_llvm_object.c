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

static LLVMTargetMachineRef cod_llvm_target_machine(const char *triple, char **default_triple, char **diagnostic) {
  if (!*triple) {
    *default_triple = LLVMGetDefaultTargetTriple();
    if (!*default_triple) return NULL;
    triple = *default_triple;
  }
  LLVMTargetRef target = NULL;
  if (LLVMGetTargetFromTriple(triple, &target, diagnostic)) return NULL;
  return LLVMCreateTargetMachine(target, triple, "generic", "",
      LLVMCodeGenLevelDefault, LLVMRelocPIC, LLVMCodeModelDefault);
}

bool cod_llvm_measure_abi(LLVMTargetDataRef target, cod_llvm_abi_layout *result) {
  if (!target || !result) return false;
  LLVMContextRef context = LLVMContextCreate();
  if (!context) return false;
  LLVMTypeRef types[COD_LLVM_ABI_REPR_COUNT] = {
    [COD_LLVM_ABI_I8] = LLVMInt8TypeInContext(context),
    [COD_LLVM_ABI_I32] = LLVMInt32TypeInContext(context),
    [COD_LLVM_ABI_I64] = LLVMInt64TypeInContext(context),
    [COD_LLVM_ABI_F32] = LLVMFloatTypeInContext(context),
    [COD_LLVM_ABI_F64] = LLVMDoubleTypeInContext(context)
  };
  cod_llvm_abi_layout measured = {0};
  for (size_t i = 0; i < COD_LLVM_ABI_REPR_COUNT; ++i) {
    measured.size[i] = LLVMABISizeOfType(target, types[i]);
    measured.alignment[i] = LLVMABIAlignmentOfType(target, types[i]);
  }
  LLVMTypeRef empty_record = LLVMStructTypeInContext(context, NULL, 0, false);
  measured.struct_alignment = LLVMABIAlignmentOfType(target, empty_record);
  LLVMContextDispose(context);
  *result = measured;
  return true;
}

bool cod_llvm_measure_native_abi(cod_llvm_abi_layout *result) {
  char *triple = NULL, *diagnostic = NULL;
  LLVMTargetMachineRef machine = cod_llvm_target_machine("", &triple, &diagnostic);
  if (triple) LLVMDisposeMessage(triple);
  if (diagnostic) LLVMDisposeMessage(diagnostic);
  if (!machine) return false;
  LLVMTargetDataRef layout = LLVMCreateTargetDataLayout(machine);
  const bool success = cod_llvm_measure_abi(layout, result);
  if (layout) LLVMDisposeTargetData(layout);
  LLVMDisposeTargetMachine(machine);
  return success;
}

/* The caller initializes the target and its assembly printer. A module that
   omits its triple uses LLVM's default target. Construct target-sensitive IR
   against a layout compatible with the selected target before calling this. */
cod_llvm_object_result cod_llvm_emit_object(LLVMModuleRef module) {
  cod_llvm_object_result result = {COD_LLVM_OBJECT_TARGET_UNAVAILABLE, NULL, NULL};
  char *default_triple = NULL;
  LLVMTargetMachineRef machine = cod_llvm_target_machine(LLVMGetTarget(module), &default_triple, &result.diagnostic);
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
