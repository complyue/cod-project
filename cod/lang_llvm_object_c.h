/*===--- lang_llvm_object_c.h - Verified native object emission -----------*/
#ifndef COD_LANG_LLVM_OBJECT_C_H
#define COD_LANG_LLVM_OBJECT_C_H

#include <stdbool.h>
#include <stdint.h>

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

typedef enum cod_llvm_abi_repr {
  COD_LLVM_ABI_I8, COD_LLVM_ABI_I32, COD_LLVM_ABI_I64, COD_LLVM_ABI_F32, COD_LLVM_ABI_F64,
  COD_LLVM_ABI_REPR_COUNT
} cod_llvm_abi_repr;

typedef struct cod_llvm_abi_layout {
  uint64_t size[COD_LLVM_ABI_REPR_COUNT];
  uint64_t alignment[COD_LLVM_ABI_REPR_COUNT];
  uint64_t struct_alignment;
} cod_llvm_abi_layout;

#ifdef __cplusplus
extern "C" {
#endif

/* The result owns its object bytes and any LLVM diagnostic. */
void cod_llvm_object_release(cod_llvm_object_result *result);
/* The caller initializes the target and its assembly printer. A module that
   omits its triple uses LLVM's default target. Construct target-sensitive IR
   against a layout compatible with the selected target before calling this. */
cod_llvm_object_result cod_llvm_emit_object(LLVMModuleRef module);
/* Measure the fixed primitive and unpacked-struct ABI facts for a layout. */
bool cod_llvm_measure_abi(LLVMTargetDataRef target, cod_llvm_abi_layout *result);
/* Requires native target registration. No object module or assembler is needed. */
bool cod_llvm_measure_native_abi(cod_llvm_abi_layout *result);
/* Publish the complete buffer by replacing the destination on the same filesystem. */
bool cod_llvm_publish_object(LLVMMemoryBufferRef object, const char *path);

#ifdef __cplusplus
}
#endif

#endif
