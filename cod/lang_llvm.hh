//===--- lang_llvm.hh - CoD native artifact emission ----------------===//
#pragma once

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/TargetOptions.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Host.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace cod::language {

class LLVMEmission {
public:
  std::pair<std::string, std::string> empty_program(bool emit_text, bool emit_bitcode) {
    llvm::LLVMContext context;
    llvm::Module module("cod.program", context);
    const llvm::Triple triple(llvm::sys::getDefaultTargetTriple());
    clang::DiagnosticOptions diagnostic_options;
    clang::IgnoringDiagConsumer diagnostics_consumer;
    clang::DiagnosticsEngine diagnostics(llvm::makeIntrusiveRefCnt<clang::DiagnosticIDs>(), diagnostic_options,
                                         &diagnostics_consumer, false);
    clang::TargetOptions options;
    options.Triple = triple.str();
    std::unique_ptr<clang::TargetInfo> target(clang::TargetInfo::CreateTargetInfo(diagnostics, options));
    if (!target) throw std::runtime_error("cannot obtain the native target data layout");
    module.setTargetTriple(triple);
    module.setDataLayout(target->getDataLayoutString());
    auto *main = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false),
        llvm::GlobalValue::ExternalLinkage, "main", module);
    llvm::IRBuilder<> builder(llvm::BasicBlock::Create(context, "entry", main));
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0));

    auto *pointer = llvm::PointerType::getUnqual(context);
    auto *sites = llvm::Function::Create(
        llvm::FunctionType::get(pointer, {llvm::Type::getInt64Ty(context)}, false),
        llvm::GlobalValue::ExternalLinkage, "cod_lang_exception_site", module);
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "entry", sites));
    builder.CreateRet(llvm::ConstantPointerNull::get(pointer));

    std::string errors;
    llvm::raw_string_ostream diagnostics_out(errors);
    if (llvm::verifyModule(module, &diagnostics_out))
      throw std::logic_error("invalid emitted CoD module: " + errors);
    std::string text, bitcode;
    if (emit_text) {
      llvm::raw_string_ostream output(text);
      module.print(output, nullptr);
    }
    if (emit_bitcode) {
      llvm::raw_string_ostream output(bitcode);
      llvm::WriteBitcodeToFile(module, output);
    }
    return {std::move(text), std::move(bitcode)};
  }
};

} // namespace cod::language
