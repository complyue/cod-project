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

/* Open into an empty owner. Release the session even if opening stops
   after acquiring only some of its handles. */
static inline bool cod_llvm_session_open(cod_llvm_session *session, const char *module_name) {
  session->context = LLVMContextCreate();
  if (!session->context) return false;
  session->module = LLVMModuleCreateWithNameInContext(module_name, session->context);
  if (!session->module) return false;
  session->builder = LLVMCreateBuilderInContext(session->context);
  return session->builder != NULL;
}

static inline void cod_llvm_session_release(cod_llvm_session *session) {
  if (session->builder) LLVMDisposeBuilder(session->builder);
  if (session->module) LLVMDisposeModule(session->module);
  if (session->context) LLVMContextDispose(session->context);
  session->builder = NULL;
  session->module = NULL;
  session->context = NULL;
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
