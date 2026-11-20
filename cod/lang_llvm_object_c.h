/*===--- lang_llvm_object_c.h - Verified native object emission -----------*/
#ifndef COD_LANG_LLVM_OBJECT_C_H
#define COD_LANG_LLVM_OBJECT_C_H

#include <stddef.h>

#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/TargetMachine.h>

typedef enum cod_llvm_object_status {
  COD_LLVM_OBJECT_OK,
  COD_LLVM_OBJECT_TARGET_UNAVAILABLE,
  COD_LLVM_OBJECT_INVALID_MODULE,
  COD_LLVM_OBJECT_EMISSION_FAILED
} cod_llvm_object_status;

typedef struct cod_llvm_object_result {
  cod_llvm_object_status status;
  LLVMMemoryBufferRef object;
  char *diagnostic;
} cod_llvm_object_result;

/* The result owns its object bytes and any LLVM diagnostic. */
static inline void cod_llvm_object_release(cod_llvm_object_result *result) {
  if (result->object) LLVMDisposeMemoryBuffer(result->object);
  if (result->diagnostic) LLVMDisposeMessage(result->diagnostic);
  result->object = NULL;
  result->diagnostic = NULL;
}

/* The caller initializes the target and its assembly printer. A module that
   omits its triple uses LLVM's default target. Construct target-sensitive IR
   against a layout compatible with the selected target before calling this. */
static inline cod_llvm_object_result cod_llvm_emit_object(LLVMModuleRef module) {
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

#endif
