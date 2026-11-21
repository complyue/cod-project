/*===--- lang_llvm_object_c.h - Verified native object emission -----------*/
#ifndef COD_LANG_LLVM_OBJECT_C_H
#define COD_LANG_LLVM_OBJECT_C_H

#include <stdbool.h>

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

#ifdef __cplusplus
extern "C" {
#endif

/* The result owns its object bytes and any LLVM diagnostic. */
void cod_llvm_object_release(cod_llvm_object_result *result);
/* The caller initializes the target and its assembly printer. A module that
   omits its triple uses LLVM's default target. Construct target-sensitive IR
   against a layout compatible with the selected target before calling this. */
cod_llvm_object_result cod_llvm_emit_object(LLVMModuleRef module);
/* Publish the complete buffer by replacing the destination on the same filesystem. */
bool cod_llvm_publish_object(LLVMMemoryBufferRef object, const char *path);

#ifdef __cplusplus
}
#endif

#endif
