//===--- lang_llvm.hh - Direct LLVM execution lowering --------------===//
#pragma once

#include "lang_syntax.hh"

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/TargetOptions.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/SaveAndRestore.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Host.h>

#include <map>
#include <cmath>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace cod::language {

// The semantic checker owns identities, effects and regional lifetimes. This
// emitter owns only native values/control flow; it never interprets C++ text.
class LLVMEmission {
  std::unique_ptr<llvm::LLVMContext> context_;
  std::unique_ptr<llvm::Module> module_;
  std::unique_ptr<llvm::IRBuilder<>> builder_;
  llvm::Function *entry_ = nullptr;
  llvm::IntegerType *pointer_integer_ = nullptr;
  llvm::StructType *array_header_ = nullptr;
  std::map<size_t, llvm::StructType *> records_;
  std::map<size_t, std::vector<Type>> fields_;
  std::map<size_t, llvm::GlobalVariable *> slots_;
  const Location *source_location_ = nullptr;
  struct Cleanup {
    llvm::Value *address;
    llvm::Value *saved; // nullptr identifies a region-owner slot to release.
  };
  std::vector<Cleanup> cleanups_;
  std::vector<llvm::Value *> module_regions_;

  llvm::LLVMContext &context() { return *context_; }
  llvm::IRBuilder<> &b() { return *builder_; }
  llvm::Type *void_type() { return llvm::Type::getVoidTy(context()); }
  llvm::IntegerType *i64() { return llvm::Type::getInt64Ty(context()); }
  llvm::PointerType *pointer_type() { return llvm::PointerType::getUnqual(context()); }
  llvm::Value *source_context() {
    if (source_location_) return b().CreateGlobalString(location_text(*source_location_));
    return llvm::ConstantPointerNull::get(pointer_type());
  }
  static Type representation(Type type) {
    return type.kind == Type::unit ? Type(type.representation) : type;
  }
  static bool signed_integer(Type type) { return representation(type) == Type::integer; }
  llvm::ConstantInt *integer(llvm::IntegerType *type, uint64_t bits) {
    return llvm::ConstantInt::get(type, bits);
  }
  llvm::Value *call_runtime(const std::string &name, llvm::Type *result,
                            const std::vector<llvm::Value *> &args) {
    std::vector<llvm::Type *> types;
    for (const auto arg : args) types.push_back(arg->getType());
    auto function = module_->getOrInsertFunction(name, llvm::FunctionType::get(result, types, false));
    return b().CreateCall(function, args);
  }
  llvm::Value *slot(size_t id, Type type) {
    if (const auto found = slots_.find(id); found != slots_.end()) return found->second;
    auto *native = type_of(type);
    auto *global = new llvm::GlobalVariable(*module_, native, false, llvm::GlobalValue::InternalLinkage,
        llvm::Constant::getNullValue(native), "effect." + std::to_string(id));
    global->setThreadLocalMode(llvm::GlobalVariable::GeneralDynamicTLSModel);
    slots_.emplace(id, global);
    return global;
  }
  void require(llvm::Value *condition, const std::string &message) {
    auto *good = llvm::BasicBlock::Create(context(), "checked", entry_);
    auto *bad = llvm::BasicBlock::Create(context(), "failure", entry_);
    b().CreateCondBr(condition, good, bad);
    b().SetInsertPoint(bad);
    const auto diagnostic = source_location_ ? location_text(*source_location_) + ": " + message : message;
    call_runtime("cod_lang_fail", void_type(), {b().CreateGlobalString(diagnostic)});
    b().CreateUnreachable();
    b().SetInsertPoint(good);
  }
  llvm::Value *finite(llvm::Value *number) {
    auto *infinity = llvm::ConstantFP::getInfinity(number->getType());
    auto *absolute = b().CreateUnaryIntrinsic(llvm::Intrinsic::fabs, number);
    return b().CreateFCmpOLT(absolute, infinity);
  }
public:
  llvm::Value *binary(Type type, const std::string &op, llvm::Value *left, llvm::Value *right) {
    if (floating(representation(type))) {
      if (op == "+") return b().CreateFAdd(left, right);
      if (op == "-") return b().CreateFSub(left, right);
      if (op == "*") return b().CreateFMul(left, right);
      if (op == "/") return b().CreateFDiv(left, right);
      if (op == "==") return b().CreateFCmpOEQ(left, right);
      if (op == "!=") return b().CreateFCmpUNE(left, right);
      if (op == "<") return b().CreateFCmpOLT(left, right);
      if (op == "<=") return b().CreateFCmpOLE(left, right);
      if (op == ">") return b().CreateFCmpOGT(left, right);
      if (op == ">=") return b().CreateFCmpOGE(left, right);
    }
    const bool signed_value = signed_integer(type);
    if (op == "==") return b().CreateICmpEQ(left, right);
    if (op == "!=") return b().CreateICmpNE(left, right);
    if (op == "<") return signed_value ? b().CreateICmpSLT(left, right) : b().CreateICmpULT(left, right);
    if (op == "<=") return signed_value ? b().CreateICmpSLE(left, right) : b().CreateICmpULE(left, right);
    if (op == ">") return signed_value ? b().CreateICmpSGT(left, right) : b().CreateICmpUGT(left, right);
    if (op == ">=") return signed_value ? b().CreateICmpSGE(left, right) : b().CreateICmpUGE(left, right);
    const auto message_type = type_name(representation(type));
    if (op == "+" || op == "-" || op == "*") {
      const auto intrinsic = op == "+" ? (signed_value ? llvm::Intrinsic::sadd_with_overflow : llvm::Intrinsic::uadd_with_overflow) :
          op == "-" ? (signed_value ? llvm::Intrinsic::ssub_with_overflow : llvm::Intrinsic::usub_with_overflow) :
                      (signed_value ? llvm::Intrinsic::smul_with_overflow : llvm::Intrinsic::umul_with_overflow);
      auto *result = b().CreateIntrinsic(intrinsic, {left->getType()}, {left, right});
      const auto operation = op == "+" ? "addition" : op == "-" ? "subtraction" : "multiplication";
      require(b().CreateNot(b().CreateExtractValue(result, 1)), message_type + " " + operation + " overflow");
      return b().CreateExtractValue(result, 0);
    }
    auto *native_type = llvm::cast<llvm::IntegerType>(left->getType());
    require(b().CreateICmpNE(right, integer(native_type, 0)), op == "%" ? "remainder by zero" : "division by zero");
    if (signed_value) {
      auto *minimum = llvm::ConstantInt::get(context(), llvm::APInt::getSignedMinValue(native_type->getBitWidth()));
      auto *minus_one = llvm::ConstantInt::getSigned(native_type, -1);
      auto *overflow = b().CreateAnd(b().CreateICmpEQ(left, minimum), b().CreateICmpEQ(right, minus_one));
      if (op == "/") {
        require(b().CreateNot(overflow), message_type + " division overflow");
        return b().CreateSDiv(left, right);
      }
      // LLVM remainder must not evaluate its exceptional signed divisor pair.
      return b().CreateSRem(left, b().CreateSelect(overflow, integer(native_type, 1), right));
    }
    return op == "/" ? b().CreateUDiv(left, right) : b().CreateURem(left, right);
  }

public:
  auto source_location(const Location &at) {
    return llvm::SaveAndRestore<const Location *>(source_location_, &at);
  }
  LLVMEmission() {
    context_ = std::make_unique<llvm::LLVMContext>();
    module_ = std::make_unique<llvm::Module>("cod.program", context());
    const llvm::Triple triple(llvm::sys::getDefaultTargetTriple());
    clang::DiagnosticOptions diagnostic_options;
    clang::IgnoringDiagConsumer diagnostics_consumer;
    clang::DiagnosticsEngine diagnostics(llvm::makeIntrusiveRefCnt<clang::DiagnosticIDs>(), diagnostic_options,
                                         &diagnostics_consumer, false);
    clang::TargetOptions options;
    options.Triple = triple.str();
    std::unique_ptr<clang::TargetInfo> target(clang::TargetInfo::CreateTargetInfo(diagnostics, options));
    if (!target) throw std::runtime_error("cannot obtain the native target data layout");
    module_->setTargetTriple(triple);
    module_->setDataLayout(target->getDataLayoutString());
    pointer_integer_ = module_->getDataLayout().getIntPtrType(context());
    array_header_ = llvm::StructType::create(context(), {pointer_integer_, i64()}, "cod.array");
    entry_ = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(context()), false),
                                     llvm::GlobalValue::ExternalLinkage, "main", *module_);
    builder_ = std::make_unique<llvm::IRBuilder<>>(llvm::BasicBlock::Create(context(), "entry", entry_));
  }
  llvm::Type *type_of(Type type) {
    type = representation(type);
    switch (type.kind) {
    case Type::integer: case Type::uint64: case Type::reference: case Type::array:
    case Type::regional: case Type::null_ref: return i64();
    case Type::uint8: return llvm::Type::getInt8Ty(context());
    case Type::uint32: return llvm::Type::getInt32Ty(context());
    case Type::boolean: return llvm::Type::getInt1Ty(context());
    case Type::float32: return llvm::Type::getFloatTy(context());
    case Type::float64: return llvm::Type::getDoubleTy(context());
    case Type::region: return pointer_type();
    case Type::void_type: return void_type();
    case Type::product: return record_type(type);
    default: throw std::logic_error("unresolved LLVM type");
    }
  }
  llvm::StructType *record_type(Type type) {
    if (const auto found = records_.find(type.identity); found != records_.end()) return found->second;
    auto *record = llvm::StructType::create(context(), "record." + std::to_string(type.identity));
    records_.emplace(type.identity, record);
    return record;
  }
  void record(Type type, const std::vector<Type> &fields) {
    std::vector<llvm::Type *> native;
    for (const auto &field : fields)
      native.push_back(field.kind == Type::reference ? pointer_integer_ : type_of(field));
    if (native.empty()) native.push_back(llvm::Type::getInt8Ty(context()));
    record_type(type)->setBody(native);
    fields_[type.identity] = fields;
  }
  llvm::Value *bits(Type type, uint64_t bits) {
    return integer(llvm::cast<llvm::IntegerType>(type_of(type)), bits);
  }
  llvm::Value *floating_literal(Type type, const std::string &spelling) {
    return llvm::ConstantFP::get(type_of(type), spelling);
  }
  llvm::Value *read_symbol(size_t id, Type type) {
    return b().CreateLoad(type_of(type), slot(id, type));
  }
  void effect(size_t id, Type type, llvm::Value *source) {
    if (type == Type::region || type.kind == Type::procedure) return;
    auto *address = slot(id, type);
    cleanups_.push_back({address, b().CreateLoad(type_of(type), address)});
    b().CreateStore(source, address);
  }
  size_t scope() const { return cleanups_.size(); }
  void restore(size_t mark) {
    while (cleanups_.size() > mark) {
      auto cleanup = cleanups_.back();
      cleanups_.pop_back();
      if (cleanup.saved) b().CreateStore(cleanup.saved, cleanup.address);
      else call_runtime("cod_lang_region_free", void_type(), {b().CreateLoad(pointer_type(), cleanup.address)});
    }
  }
  llvm::Value *product(Type type, const std::vector<llvm::Value *> &args) {
    llvm::Value *result = llvm::Constant::getNullValue(type_of(type));
    for (size_t i = 0; i < args.size(); ++i) result = b().CreateInsertValue(result, args[i], unsigned(i));
    return result;
  }
  llvm::Value *field(llvm::Value *source, size_t field) {
    return b().CreateExtractValue(source, unsigned(field));
  }
  void print(Type type, llvm::Value *source) {
    auto *number = source;
    std::string suffix;
    if (type == Type::boolean) {
      suffix = "bool";
      number = b().CreateZExt(number, llvm::Type::getInt32Ty(context()));
    } else if (type == Type::integer) suffix = "i64";
    else if (type == Type::float32) suffix = "f32";
    else if (type == Type::float64) suffix = "f64";
    else {
      suffix = "u64";
      number = b().CreateZExtOrTrunc(number, i64());
    }
    call_runtime("cod_lang_print_" + suffix, void_type(), {number});
  }
  void assert_value(llvm::Value *source) {
    require(source, "CoD assertion failed");
  }
  llvm::Value *unary(Type type, const std::string &op, llvm::Value *source) {
    auto *number = source;
    if (op == "!") return b().CreateNot(number);
    if (floating(representation(type))) return b().CreateFNeg(number);
    return binary(type, "-", llvm::Constant::getNullValue(number->getType()), number);
  }
  llvm::Value *reference_equal(const std::string &op, llvm::Value *left, llvm::Value *right,
                                bool different_regions) {
    auto *a = left, *c = right;
    auto *equal = different_regions ? b().CreateAnd(b().CreateICmpEQ(a, integer(i64(), 0)),
                                                    b().CreateICmpEQ(c, integer(i64(), 0))) : b().CreateICmpEQ(a, c);
    return op == "==" ? equal : b().CreateNot(equal);
  }
  llvm::Value *cast(Type from, Type to, llvm::Value *number) {
    from = representation(from);
    to = representation(to);
    if (from == to) return number;
    auto *target = type_of(to);
    const bool source_integer = integral(from), target_integer = integral(to);
    const std::string error = "numeric conversion out of range";
    if (source_integer && target_integer) {
      const auto source_bits = number->getType()->getIntegerBitWidth();
      const auto target_bits = target->getIntegerBitWidth();
      auto *source_type = llvm::cast<llvm::IntegerType>(number->getType());
      const bool source_signed = signed_integer(from), target_signed = signed_integer(to);
      if (source_signed && !target_signed)
        require(b().CreateICmpSGE(number, integer(source_type, 0)), error);
      if (source_signed && target_signed && target_bits < source_bits) {
        auto minimum = llvm::APInt::getSignedMinValue(target_bits).sext(source_bits);
        require(b().CreateICmpSGE(number, llvm::ConstantInt::get(context(), minimum)), error);
      }
      if (target_bits < source_bits || (!source_signed && target_signed && target_bits == source_bits)) {
        auto maximum = target_signed ? llvm::APInt::getSignedMaxValue(target_bits) : llvm::APInt::getMaxValue(target_bits);
        auto *bound = llvm::ConstantInt::get(context(), maximum.zextOrTrunc(source_bits));
        require(source_signed ? b().CreateICmpSLE(number, bound) : b().CreateICmpULE(number, bound), error);
      }
      return b().CreateIntCast(number, target, source_signed);
    }
    if (source_integer)
      return signed_integer(from) ? b().CreateSIToFP(number, target) : b().CreateUIToFP(number, target);
    if (target_integer) {
      const bool target_signed = signed_integer(to);
      const auto digits = target->getIntegerBitWidth() - (target_signed ? 1 : 0);
      const auto upper = std::ldexp(1.0, digits);
      auto *truncated = b().CreateUnaryIntrinsic(llvm::Intrinsic::trunc, number);
      auto *lower_bound = llvm::ConstantFP::get(number->getType(), target_signed ? -upper : 0.0);
      auto *upper_bound = llvm::ConstantFP::get(number->getType(), upper);
      require(b().CreateAnd(b().CreateFCmpOGE(truncated, lower_bound), b().CreateFCmpOLT(truncated, upper_bound)), error);
      return target_signed ? b().CreateFPToSI(truncated, target) : b().CreateFPToUI(truncated, target);
    }
    if (to == Type::float64) return b().CreateFPExt(number, target);
    auto *result = b().CreateFPTrunc(number, target);
    require(b().CreateOr(b().CreateNot(finite(number)), finite(result)), error);
    return result;
  }

  struct Branch {
    llvm::BasicBlock *otherwise = nullptr, *merge = nullptr, *first_end = nullptr;
    llvm::Value *first_value = nullptr;
  };
  struct ConversionPoint {
    llvm::BasicBlock *before, *after;
  };
  ConversionPoint conversion_point() {
    auto *before = b().GetInsertBlock();
    auto *after = llvm::BasicBlock::Create(context(), "argument.next", entry_);
    b().CreateBr(after);
    b().SetInsertPoint(after);
    return {before, after};
  }
  llvm::IRBuilderBase::InsertPoint begin_conversion(const ConversionPoint &point) {
    auto resume = b().saveIP();
    point.before->getTerminator()->eraseFromParent();
    b().SetInsertPoint(point.before);
    return resume;
  }
  void end_conversion(const ConversionPoint &point, llvm::IRBuilderBase::InsertPoint resume) {
    b().CreateBr(point.after);
    b().restoreIP(resume);
  }
  Branch branch(llvm::Value *condition, bool invert = false) {
    auto *yes = llvm::BasicBlock::Create(context(), "then", entry_);
    auto *no = llvm::BasicBlock::Create(context(), "else", entry_);
    auto *merge = llvm::BasicBlock::Create(context(), "merge", entry_);
    auto *test = condition;
    b().CreateCondBr(invert ? b().CreateNot(test) : test, yes, no);
    b().SetInsertPoint(yes);
    return {no, merge, nullptr, nullptr};
  }
  void otherwise(Branch &branch, llvm::Value *first = nullptr) {
    branch.first_end = b().GetInsertBlock();
    branch.first_value = first;
    b().CreateBr(branch.merge);
    b().SetInsertPoint(branch.otherwise);
  }
  llvm::Value *merge(Branch &branch, Type type, llvm::Value *second = nullptr) {
    auto *second_end = b().GetInsertBlock();
    b().CreateBr(branch.merge);
    b().SetInsertPoint(branch.merge);
    if (type == Type::void_type) return nullptr;
    auto *result = b().CreatePHI(type_of(type), 2);
    result->addIncoming(branch.first_value, branch.first_end);
    result->addIncoming(second, second_end);
    return result;
  }

  struct Loop {
    llvm::BasicBlock *header = nullptr, *exit = nullptr;
    llvm::PHINode *index = nullptr, *accumulator = nullptr;
  };
  Loop loop(Type index_type, llvm::Value *first, llvm::Value *last, llvm::Value *initial = nullptr) {
    auto *before = b().GetInsertBlock();
    auto *header = llvm::BasicBlock::Create(context(), "loop", entry_);
    auto *body = llvm::BasicBlock::Create(context(), "body", entry_);
    auto *exit = llvm::BasicBlock::Create(context(), "done", entry_);
    b().CreateBr(header);
    b().SetInsertPoint(header);
    auto *index = b().CreatePHI(type_of(index_type), 2, "index");
    index->addIncoming(first, before);
    llvm::PHINode *accumulator = nullptr;
    if (initial) {
      accumulator = b().CreatePHI(initial->getType(), 2, "accumulator");
      accumulator->addIncoming(initial, before);
    }
    auto *condition = signed_integer(index_type) ? b().CreateICmpSLT(index, last) :
                                                  b().CreateICmpULT(index, last);
    b().CreateCondBr(condition, body, exit);
    b().SetInsertPoint(body);
    return {header, exit, index, accumulator};
  }
  void end_loop(Loop &loop, llvm::Value *next = nullptr) {
    auto *end = b().GetInsertBlock();
    auto *index = b().CreateAdd(loop.index, integer(llvm::cast<llvm::IntegerType>(loop.index->getType()), 1));
    loop.index->addIncoming(index, end);
    if (loop.accumulator) loop.accumulator->addIncoming(next, end);
    b().CreateBr(loop.header);
    b().SetInsertPoint(loop.exit);
  }

private:
  llvm::Value *address(llvm::Value *region, llvm::Value *offset) {
    return b().CreateGEP(llvm::Type::getInt8Ty(context()), region, offset);
  }
  llvm::Value *checked_address(llvm::Value *owner, llvm::Value *offset) {
    auto *position = offset;
    require(b().CreateICmpNE(position, integer(i64(), 0)), "null regional reference");
    return address(owner, position);
  }
  llvm::Value *offset_of(llvm::Value *region, llvm::Value *target) {
    auto *offset = b().CreateSub(b().CreatePtrToInt(target, pointer_integer_), b().CreatePtrToInt(region, pointer_integer_));
    return b().CreateZExtOrTrunc(offset, i64());
  }
  llvm::Value *allocate(llvm::Value *owner, llvm::Value *bytes, uint64_t alignment) {
    return call_runtime("cod_lang_region_alloc", i64(), {owner, bytes, integer(i64(), alignment), source_context()});
  }
  void store_reference(llvm::Value *region, llvm::Value *field, llvm::Value *offset) {
    auto *target = address(region, offset);
    auto *relative = b().CreateSub(b().CreatePtrToInt(target, pointer_integer_), b().CreatePtrToInt(field, pointer_integer_));
    auto *stored = b().CreateSelect(b().CreateICmpEQ(offset, integer(i64(), 0)), integer(pointer_integer_, 0), relative);
    b().CreateStore(stored, field);
  }
  llvm::Value *read_reference(llvm::Value *region, llvm::Value *field) {
    auto *relative = b().CreateLoad(pointer_integer_, field);
    auto *target = b().CreateIntToPtr(b().CreateAdd(b().CreatePtrToInt(field, pointer_integer_), relative), pointer_type());
    auto *offset = offset_of(region, target);
    return b().CreateSelect(b().CreateICmpEQ(relative, integer(pointer_integer_, 0)), integer(i64(), 0), offset);
  }
  llvm::Value *array_address(llvm::Value *owner, llvm::Value *array,
                              Type index_type, llvm::Value *index, Type element) {
    auto *header = checked_address(owner, array);
    auto *length = b().CreateLoad(i64(), b().CreateStructGEP(array_header_, header, 1));
    auto *position = cast(index_type, Type::uint64, index);
    require(b().CreateICmpULT(position, length), "regional array index out of bounds");
    auto *offset = read_reference(owner, b().CreateStructGEP(array_header_, header, 0));
    return b().CreateGEP(type_of(element), address(owner, offset), position);
  }

public:
  llvm::Value *region_new(Type capacity_type, llvm::Value *capacity) {
    auto *bytes = cast(capacity_type, Type::uint64, capacity);
    return call_runtime("cod_lang_region_new", pointer_type(), {bytes, source_context()});
  }
  void hold_region(llvm::Value *region, bool module_lifetime) {
    llvm::IRBuilder<> allocations(&entry_->getEntryBlock(), entry_->getEntryBlock().begin());
    auto *slot = allocations.CreateAlloca(pointer_type(), nullptr, "region.owner");
    b().CreateStore(region, slot);
    if (module_lifetime) module_regions_.push_back(slot);
    else cleanups_.push_back({slot, nullptr});
  }
  llvm::Value *region_size(llvm::Value *owner, bool capacity) {
    return call_runtime(capacity ? "cod_lang_region_capacity" : "cod_lang_region_used", i64(), {owner});
  }
  llvm::Value *record_new(llvm::Value *owner, Type type, const std::vector<llvm::Value *> &args) {
    auto *record = record_type(type);
    const auto &layout = module_->getDataLayout();
    auto *offset = allocate(owner, integer(i64(), layout.getTypeAllocSize(record).getFixedValue()), layout.getABITypeAlign(record).value());
    auto *storage = address(owner, offset);
    const auto &fields = fields_.at(type.identity);
    for (size_t i = 0; i < args.size(); ++i) {
      auto *field = b().CreateStructGEP(record, storage, unsigned(i));
      if (fields[i].kind == Type::reference) store_reference(owner, field, args[i]);
      else b().CreateStore(args[i], field);
    }
    return offset;
  }
  llvm::Value *array_new(llvm::Value *owner, Type element, Type count_type,
                          llvm::Value *count, llvm::Value *initial) {
    const auto &layout = module_->getDataLayout();
    auto *length = cast(count_type, Type::uint64, count);
    auto *offset = allocate(owner, integer(i64(), layout.getTypeAllocSize(array_header_).getFixedValue()), layout.getABITypeAlign(array_header_).value());
    auto *header = address(owner, offset);
    auto *pointer_field = b().CreateStructGEP(array_header_, header, 0);
    b().CreateStore(integer(pointer_integer_, 0), pointer_field);
    b().CreateStore(length, b().CreateStructGEP(array_header_, header, 1));
    auto *allocate_block = llvm::BasicBlock::Create(context(), "array.allocate", entry_);
    auto *done = llvm::BasicBlock::Create(context(), "array.ready", entry_);
    b().CreateCondBr(b().CreateICmpNE(length, integer(i64(), 0)), allocate_block, done);
    b().SetInsertPoint(allocate_block);
    auto *element_type = type_of(element);
    auto *bytes = binary(Type::uint64, "*", length, integer(i64(), layout.getTypeAllocSize(element_type).getFixedValue()));
    auto *data_offset = allocate(owner, bytes, layout.getABITypeAlign(element_type).value());
    auto *data = address(owner, data_offset);
    store_reference(owner, pointer_field, data_offset);
    auto fill = loop(Type::uint64, integer(i64(), 0), length);
    b().CreateStore(initial, b().CreateGEP(element_type, data, fill.index));
    end_loop(fill);
    b().CreateBr(done);
    b().SetInsertPoint(done);
    return offset;
  }
  llvm::Value *dereference(llvm::Value *owner, llvm::Value *source, Type type) {
    auto *storage = checked_address(owner, source);
    return type.kind == Type::array || type.kind == Type::regional ? source : b().CreateLoad(type_of(type), storage);
  }
  llvm::Value *regional_field(llvm::Value *owner, llvm::Value *source,
                               Type record, size_t index, Type field_type) {
    auto *field = b().CreateStructGEP(record_type(record), checked_address(owner, source), unsigned(index));
    return field_type.kind == Type::reference ? read_reference(owner, field) : b().CreateLoad(type_of(field_type), field);
  }
  void set_field(llvm::Value *owner, llvm::Value *source, Type record,
                 size_t index, Type field_type, llvm::Value *input) {
    auto *field = b().CreateStructGEP(record_type(record), checked_address(owner, source), unsigned(index));
    if (field_type.kind == Type::reference) store_reference(owner, field, input);
    else b().CreateStore(input, field);
  }
  llvm::Value *array_length(llvm::Value *owner, llvm::Value *source) {
    return b().CreateLoad(i64(), b().CreateStructGEP(array_header_, checked_address(owner, source), 1));
  }
  llvm::Value *array_element(llvm::Value *owner, llvm::Value *source, Type index_type,
                              llvm::Value *index, Type element, bool reference) {
    auto *location = array_address(owner, source, index_type, index, element);
    return reference ? offset_of(owner, location) : b().CreateLoad(type_of(element), location);
  }
  llvm::Value *array_store(llvm::Value *owner, llvm::Value *source, Type index_type,
                            llvm::Value *index, Type element, llvm::Value *input) {
    b().CreateStore(input, array_address(owner, source, index_type, index, element));
    return input;
  }
  void scalar_store(llvm::Value *owner, llvm::Value *source, llvm::Value *input) {
    b().CreateStore(input, checked_address(owner, source));
  }

  std::pair<std::string, std::string> finish() {
    if (!cleanups_.empty()) throw std::logic_error("unclosed LLVM resource scope");
    for (auto region = module_regions_.rbegin(); region != module_regions_.rend(); ++region)
      call_runtime("cod_lang_region_free", void_type(), {b().CreateLoad(pointer_type(), *region)});
    b().CreateRet(integer(llvm::Type::getInt32Ty(context()), 0));
    std::string errors;
    llvm::raw_string_ostream diagnostics(errors);
    if (llvm::verifyModule(*module_, &diagnostics))
      throw std::runtime_error("invalid emitted LLVM IR: " + errors);
    std::string text, bitcode;
    llvm::raw_string_ostream text_stream(text), bitcode_stream(bitcode);
    module_->print(text_stream, nullptr);
    llvm::WriteBitcodeToFile(*module_, bitcode_stream);
    return {std::move(text), std::move(bitcode)};
  }
};

} // namespace cod::language
