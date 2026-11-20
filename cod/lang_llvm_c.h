/*===--- lang_llvm_c.h - LLVM operations for native CoD emission -------------*/
#ifndef COD_LANG_LLVM_C_H
#define COD_LANG_LLVM_C_H

#include <stdbool.h>
#include <stddef.h>

#include <llvm-c/Core.h>

typedef struct cod_llvm_session {
  LLVMContextRef context;
  LLVMModuleRef module;
  LLVMBuilderRef builder;
} cod_llvm_session;

static inline void cod_llvm_session_release(cod_llvm_session *session) {
  if (session->builder) LLVMDisposeBuilder(session->builder);
  if (session->module) LLVMDisposeModule(session->module);
  if (session->context) LLVMContextDispose(session->context);
  session->builder = NULL;
  session->module = NULL;
  session->context = NULL;
}

/* Publish a complete session into an empty owner. Acquisition retains and
   releases the partial session locally on failure. */
static inline bool cod_llvm_session_open(cod_llvm_session *session, const char *module_name) {
  if (session->context || session->module || session->builder) return false;
  cod_llvm_session acquired = {LLVMContextCreate(), NULL, NULL};
  if (acquired.context) acquired.module = LLVMModuleCreateWithNameInContext(module_name, acquired.context);
  if (acquired.module) acquired.builder = LLVMCreateBuilderInContext(acquired.context);
  if (!acquired.builder) {
    cod_llvm_session_release(&acquired);
    return false;
  }
  *session = acquired;
  return true;
}

/* Form the LLVM {result, overflow} value for the selected integer intrinsic.
   The caller supplies equal integer operand types and a builder in the same
   module. The caller decides how to propagate the overflow flag. */
static inline LLVMValueRef cod_llvm_build_checked_integer(LLVMBuilderRef builder, LLVMModuleRef module,
    unsigned intrinsic, LLVMValueRef left, LLVMValueRef right) {
  LLVMTypeRef operand_type = LLVMTypeOf(left);
  LLVMValueRef callee = LLVMGetIntrinsicDeclaration(module, intrinsic, &operand_type, 1);
  LLVMValueRef operands[] = {left, right};
  return LLVMBuildCall2(builder, LLVMGlobalGetValueType(callee), callee, operands, 2, "");
}

#endif
