/*===--- lang_llvm_c.h - LLVM operations for native CoD emission -------------*/
#ifndef COD_LANG_LLVM_C_H
#define COD_LANG_LLVM_C_H

#include <llvm-c/Core.h>

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
