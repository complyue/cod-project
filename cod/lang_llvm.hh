//===--- lang_llvm.hh - Direct LLVM execution lowering --------------===//
#pragma once

#include "lang_syntax.hh"
#include "lang_exceptions.hh"

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/TargetOptions.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/SaveAndRestore.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Transforms/Utils/CodeExtractor.h>
#include <llvm/Transforms/Utils/Local.h>

#include <algorithm>
#include <map>
#include <cmath>
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
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
    llvm::AllocaInst *saved; // nullptr identifies an owned image/region slot.
    bool callable = false;
    std::shared_ptr<std::vector<llvm::StoreInst *>> effect_writes = {};
    std::shared_ptr<std::vector<llvm::Instruction *>> protection = {};
    llvm::Value *active = nullptr;
    size_t owner_scope = 0;
  };
  size_t cleanup_owner_ = 0;
  struct Continuation {
    llvm::WeakTrackingVH selected, shared, resume;
    std::vector<llvm::WeakTrackingVH> others;
  };
  std::vector<Continuation> continuations_;
  struct CallableProtection {
    llvm::Value *source, *image, *slot;
    llvm::StoreInst *installed;
    llvm::CondBrInst *failure;
    std::shared_ptr<std::vector<llvm::Instruction *>> cleanup;
    std::vector<llvm::CallBase *> calls;
    bool required = false;
  };
  std::vector<CallableProtection> callable_protections_;
  std::vector<size_t> active_protections_;
  std::set<llvm::Function *> callable_writers_;
  std::vector<Cleanup> cleanups_;
  struct UncaughtSite {
    llvm::BasicBlock *block;
    size_t category;
    llvm::Value *payload;
    std::vector<Cleanup> cleanups;
    std::vector<llvm::Value *> regions, callables;
  };
  std::vector<UncaughtSite> uncaught_sites_;
  std::vector<llvm::Value *> module_regions_;
  std::vector<llvm::Value *> module_callables_;
  std::vector<llvm::Value *> exception_sites_;
  std::function<void(size_t, llvm::Value *)> exception_handler_;
  size_t callable_depth_ = 0;
  std::vector<std::vector<llvm::BasicBlock *>> callable_bodies_;
  std::set<llvm::Function *> callable_entries_;

  llvm::LLVMContext &context() { return *context_; }
  llvm::IRBuilder<> &b() { return *builder_; }
  llvm::Type *void_type() { return llvm::Type::getVoidTy(context()); }
  llvm::IntegerType *i64() { return llvm::Type::getInt64Ty(context()); }
  llvm::PointerType *pointer_type() { return llvm::PointerType::getUnqual(context()); }
  llvm::StructType *exception_payload_type() {
    return llvm::StructType::get(context(), {i64(), i64(), i64(), i64()});
  }
  llvm::AllocaInst *entry_slot(llvm::Type *type, const char *name, bool initialize = false) {
    llvm::IRBuilder<> storage(&entry_->getEntryBlock(), entry_->getEntryBlock().begin());
    auto *slot = storage.CreateAlloca(type, nullptr, name);
    if (initialize) storage.CreateStore(llvm::Constant::getNullValue(type), slot);
    return slot;
  }
  void release_slot(llvm::Value *slot, bool callable,
                    const std::shared_ptr<std::vector<llvm::Instruction *>> &protection = {}) {
    auto *release = call_runtime(callable ? "cod_lang_callable_free" : "cod_lang_region_free",
                                 void_type(), {b().CreateLoad(pointer_type(), slot)});
    auto *cleared = b().CreateStore(llvm::ConstantPointerNull::get(pointer_type()), slot);
    if (protection) {
      protection->push_back(llvm::cast<llvm::Instruction>(release));
      protection->push_back(cleared);
    }
  }
  void emit_cleanup(const Cleanup &cleanup) {
    if (!cleanup.saved) {
      release_slot(cleanup.address, cleanup.callable, cleanup.protection);
      return;
    }
    auto *restore = label("effect.restore"), *done = label("effect.restored");
    auto *active = b().CreateLoad(llvm::Type::getInt1Ty(context()), cleanup.active);
    b().CreateCondBr(active, restore, done);
    enter(restore);
    auto *saved = b().CreateLoad(cleanup.saved->getAllocatedType(), cleanup.saved);
    auto *store = b().CreateStore(saved, cleanup.address);
    if (cleanup.effect_writes) cleanup.effect_writes->push_back(store);
    b().CreateStore(llvm::ConstantInt::getFalse(context()), cleanup.active);
    jump(done);
    enter(done);
  }
  llvm::Value *source_context() {
    if (source_location_) return b().CreateGlobalString(location_text(*source_location_));
    return llvm::ConstantPointerNull::get(pointer_type());
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
  llvm::Value *checked_service(const std::string &name, llvm::Type *result,
                              std::vector<llvm::Value *> args, llvm::Value *requested,
                              std::initializer_list<ExceptionCategory> failures) {
    auto *status_type = llvm::ArrayType::get(i64(), 2);
    auto *status = entry_slot(status_type, "service.status");
    b().CreateStore(llvm::ConstantAggregateZero::get(status_type), status);
    args.push_back(status);
    auto *value = call_runtime(name, result, args);
    auto *code = b().CreateLoad(i64(), status);
    auto *available = b().CreateLoad(i64(), b().CreateConstGEP2_32(status_type, status, 0, 1));
    const auto site = exception_sites_.size();
    exception_sites_.push_back(source_context());
    auto *payload = exception_payload(integer(i64(), site), requested, available);
    for (auto failure : failures) {
      const auto category = exception_id(failure);
      auto *bad = label("exception.raise");
      auto *next = label("service.continue");
      b().CreateCondBr(b().CreateICmpEQ(code, integer(i64(), category)), bad, next);
      enter(bad);
      exception_handler_(category, payload);
      enter(next);
    }
    return value;
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
  struct Place { Type type; llvm::Value *owner, *address; };
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
    case Type::region: case Type::procedure: return pointer_type();
    case Type::void_type: return void_type();
    case Type::exception: return exception_payload_type();
    default: throw std::logic_error("unresolved LLVM type");
    }
  }
  llvm::Type *storage_type(Type type) {
    if (type.kind == Type::exception && !type.identity)
      return llvm::StructType::get(context(), {exception_payload_type(), i64()});
    return type_of(type);
  }
  llvm::Value *store_exception(llvm::Value *payload, llvm::Value *category) {
    llvm::Value *stored = llvm::PoisonValue::get(llvm::StructType::get(context(), {exception_payload_type(), i64()}));
    stored = b().CreateInsertValue(stored, payload, {0});
    return b().CreateInsertValue(stored, category, {1});
  }
  std::pair<llvm::Value *, llvm::Value *> load_exception(llvm::Value *stored) {
    return {b().CreateExtractValue(stored, {0}), b().CreateExtractValue(stored, {1})};
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
      native.push_back(field.kind == Type::reference ? pointer_integer_ : storage_type(field));
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
  using EffectWrites = std::vector<llvm::StoreInst *>;
  void effect(size_t id, Type type, llvm::Value *source, std::shared_ptr<EffectWrites> writes = {}) {
    if (type.kind == Type::region || (type.kind == Type::procedure && !source)) return;
    // WPO fixes each slot's type. Save the payload for scoped restoration;
    // bits never borrow a lexical cell, and object views retain region provenance.
    auto *address = slot(id, type);
    auto *saved = entry_slot(type_of(type), "effect.saved");
    auto *active = entry_slot(llvm::Type::getInt1Ty(context()), "effect.active", true);
    b().CreateStore(b().CreateLoad(type_of(type), address), saved);
    b().CreateStore(llvm::ConstantInt::getTrue(context()), active);
    cleanups_.push_back({address, saved, false, writes, {}, active, cleanup_owner_});
    auto *store = b().CreateStore(source, address);
    if (writes) writes->push_back(store);
  }
  void captured_procedure(size_t id, llvm::Value *image, const std::shared_ptr<EffectWrites> &writes) {
    auto *holder = entry_slot(pointer_type(), "capture.holder");
    writes->push_back(b().CreateStore(image, holder));
    effect(id, Type::procedure, holder, writes);
  }
  size_t scope() const { return cleanups_.size(); }
  llvm::AllocaInst *native_slot(llvm::Type *type, const char *name) {
    return entry_slot(type, name, true);
  }
  void write_native(llvm::Value *slot, llvm::Value *value) { b().CreateStore(value, slot); }
  llvm::Value *read_native(llvm::AllocaInst *slot) { return b().CreateLoad(slot->getAllocatedType(), slot); }
  llvm::AllocaInst *deferred_stack() { return entry_slot(pointer_type(), "defer.head", true); }
  llvm::StructType *deferred_layout(const std::vector<llvm::Value *> &values) {
    std::vector<llvm::Type *> fields{pointer_type(), i64()};
    for (auto *value : values) fields.push_back(value->getType());
    return llvm::StructType::get(context(), fields);
  }
  llvm::Value *deferred_field(llvm::StructType *layout, llvm::Value *record, size_t index) {
    return b().CreateStructGEP(layout, record, unsigned(index + 2));
  }
  llvm::Value *read_deferred_field(llvm::StructType *layout, llvm::Value *record, size_t index) {
    return b().CreateLoad(layout->getElementType(unsigned(index + 2)), deferred_field(layout, record, index));
  }
  void register_deferred(llvm::Value *head, llvm::StructType *layout, size_t site,
                         const std::vector<llvm::Value *> &values) {
    auto *bytes = integer(i64(), module_->getDataLayout().getTypeAllocSize(layout));
    auto *record = checked_service("cod_lang_defer_allocate", pointer_type(), {bytes}, bytes,
                                  {ExceptionCategory::out_of_memory});
    b().CreateStore(b().CreateLoad(pointer_type(), head), b().CreateStructGEP(layout, record, 0));
    b().CreateStore(integer(i64(), site), b().CreateStructGEP(layout, record, 1));
    for (size_t i = 0; i < values.size(); ++i) b().CreateStore(values[i], deferred_field(layout, record, i));
    b().CreateStore(record, head);
  }
  llvm::Value *pop_deferred(llvm::Value *head, llvm::Value *record) {
    auto *header = llvm::StructType::get(context(), {pointer_type(), i64()});
    b().CreateStore(b().CreateLoad(pointer_type(), b().CreateStructGEP(header, record, 0)), head);
    return b().CreateLoad(i64(), b().CreateStructGEP(header, record, 1));
  }
  void free_deferred(llvm::Value *record) { call_runtime("cod_lang_defer_free", void_type(), {record}); }
  size_t &cleanup_owner() { return cleanup_owner_; }
  void unwind(size_t mark, std::optional<size_t> preserve = {}) {
    for (size_t i = cleanups_.size(); i > mark; --i)
      if (cleanups_[i - 1].owner_scope != preserve) emit_cleanup(cleanups_[i - 1]);
  }
  void restore(size_t mark) {
    if (reachable()) unwind(mark);
    cleanups_.resize(mark);
  }
  bool reachable() const { return !builder_->GetInsertBlock()->hasTerminator(); }
  llvm::BasicBlock *here() { return b().GetInsertBlock(); }
  llvm::BasicBlock *label(const char *name) { return llvm::BasicBlock::Create(context(), name, entry_); }
  void enter(llvm::BasicBlock *block) { b().SetInsertPoint(block); }
  void jump(llvm::BasicBlock *block) { b().CreateBr(block); }
  void choose(llvm::Value *condition, llvm::BasicBlock *yes, llvm::BasicBlock *no) {
    b().CreateCondBr(condition, yes, no);
  }
  void unreachable() { b().CreateUnreachable(); }
  llvm::Value *phi(llvm::Type *type, const std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> &edges) {
    auto *result = llvm::PHINode::Create(type, unsigned(edges.size()), "", here()->getFirstNonPHIIt());
    for (const auto &[value, from] : edges) result->addIncoming(value, from);
    return result;
  }
  void continuation(llvm::BasicBlock *selected, const std::vector<llvm::BasicBlock *> &inputs,
                    llvm::BasicBlock *shared, llvm::BasicBlock *resume) {
    Continuation continuation{selected, shared, resume, {}};
    for (auto *input : inputs) if (input != selected) continuation.others.emplace_back(input);
    continuations_.push_back(std::move(continuation));
  }
  void finish_continuations() {
    // A shared finally executes for distinct pending completions. Carry values
    // available on just one input through that block before their uses in the
    // matching continuation. All uses, including native context stores, are
    // known here; unrelated invalid SSA remains a verifier error.
    for (const auto &continuation : continuations_) {
      if (!continuation.selected || !continuation.shared || !continuation.resume ||
          std::any_of(continuation.others.begin(), continuation.others.end(), [](const auto &other) { return !other; })) continue;
      auto *selected = llvm::cast<llvm::BasicBlock>(continuation.selected);
      auto *shared = llvm::cast<llvm::BasicBlock>(continuation.shared);
      auto *resume = llvm::cast<llvm::BasicBlock>(continuation.resume);
      auto *function = shared->getParent();
      llvm::DominatorTree dominance(*function);
      for (auto &block : *function) for (auto &value : block) {
        if (value.getType()->isVoidTy() || !dominance.dominates(&value, selected->getTerminator()) ||
            dominance.dominates(&value, &*shared->getFirstNonPHIIt())) continue;
        llvm::SmallVector<llvm::Use *> uses;
        for (auto &use : value.uses()) {
          auto *user = llvm::dyn_cast<llvm::Instruction>(use.getUser());
          if (!user || user->getFunction() != function || dominance.dominates(&value, use)) continue;
          auto *from = user->getParent();
          if (auto *phi = llvm::dyn_cast<llvm::PHINode>(user)) from = phi->getIncomingBlock(use);
          if (dominance.dominates(resume, from)) uses.push_back(&use);
        }
        if (uses.empty()) continue;
        auto *carried = llvm::PHINode::Create(value.getType(), 1 + continuation.others.size(), "completion.value", shared->getFirstNonPHIIt());
        carried->addIncoming(&value, selected);
        for (const auto &other : continuation.others)
          carried->addIncoming(llvm::PoisonValue::get(value.getType()), llvm::cast<llvm::BasicBlock>(other));
        for (auto *use : uses) use->set(carried);
      }
    }
  }
  void on_exception(std::function<void(size_t, llvm::Value *)> handler) {
    exception_handler_ = std::move(handler);
  }
  llvm::BasicBlock *begin_callable() {
    if (callable_depth_++) return nullptr;
    auto *body = label("callable.body");
    jump(body);
    enter(body);
    return body;
  }
  void end_callable(llvm::BasicBlock *body) {
    --callable_depth_;
    if (!body) return;
    auto *continuation = label("callable.continue");
    if (reachable()) jump(continuation);
    std::vector<llvm::BasicBlock *> blocks;
    for (auto it = body->getIterator(); &*it != continuation; ++it) {
      // An enclosing catch may acquire an empty landing block during this
      // call. Its body is emitted later and belongs to the caller.
      if (!it->empty()) blocks.push_back(&*it);
    }
    callable_bodies_.push_back(std::move(blocks));
    enter(continuation);
  }
  struct NativeDefinition {
    llvm::Function *function = nullptr;
    Type result;
    bool returns = true;
    bool complete = false;
    bool checked_null = false;
    std::set<size_t> failures;
    unsigned context_argument = 0;
    llvm::FunctionType *signature = nullptr;
    bool borrowed_result = false;
    bool scalar_result = false;
  };
  struct NativeContext {
    unsigned argument = 0;
    std::vector<llvm::Value *> inputs;
    llvm::StructType *layout = nullptr;
    bool closed = false;
  };
  std::map<llvm::Function *, NativeContext> native_contexts_;
  std::map<llvm::Instruction *, llvm::Function *> context_uses_;
  std::set<llvm::Function *> context_free_entries_;
  static bool external_value(llvm::Value *value, llvm::Function *function) {
    if (auto *instruction = llvm::dyn_cast<llvm::Instruction>(value)) return instruction->getFunction() != function;
    if (auto *argument = llvm::dyn_cast<llvm::Argument>(value)) return argument->getParent() != function;
    return false;
  }
  static bool add_context_input(NativeContext &context, llvm::Value *value, llvm::Function *function) {
    if (!external_value(value, function) ||
        std::find(context.inputs.begin(), context.inputs.end(), value) != context.inputs.end()) return false;
    context.inputs.push_back(value);
    return true;
  }
  bool context_free(const NativeDefinition &definition) const {
    const auto &context = native_contexts_.at(definition.function);
    return context.closed && context.inputs.empty();
  }
  void require_context_free(const NativeDefinition &definition) {
    context_free_entries_.insert(definition.function);
  }
  struct NativeBody {
    llvm::Function *parent;
    llvm::IRBuilderBase::InsertPoint insertion;
    std::vector<Cleanup> cleanups;
    std::vector<llvm::Value *> modules;
    std::vector<llvm::Value *> callables;
    size_t callable_depth;
    std::vector<size_t> protections;
  };
  llvm::FunctionType *native_signature(const std::vector<Type> &parameters, Type result, bool separate_context = false,
                                       const std::vector<bool> &borrowed = {}, bool borrowed_result = false,
                                       bool scalar_result = false) {
    std::vector<llvm::Type *> arguments{pointer_type()};
    if (separate_context) arguments.push_back(pointer_type());
    if (result.kind == Type::procedure && !borrowed_result) arguments.push_back(pointer_type());
    for (size_t i = 0; i < parameters.size(); ++i)
      arguments.push_back(!borrowed.empty() && borrowed[i] ? pointer_type() : storage_type(parameters[i]));
    std::vector<llvm::Type *> returned{i64(), exception_payload_type()};
    if (result != Type::void_type && (result.kind != Type::procedure || borrowed_result))
      returned.push_back(borrowed_result ? pointer_type() : scalar_result ?
          llvm::StructType::get(context(), {storage_type(result), pointer_type()}) : storage_type(result));
    return llvm::FunctionType::get(llvm::StructType::get(context(), returned), arguments, false);
  }
  NativeDefinition indirect_signature(const std::vector<Type> &parameters, Type result, bool checked_null) {
    return {nullptr, result, true, false, checked_null, {}, 0, native_signature(parameters, result)};
  }
  NativeDefinition native_declaration(const std::string &name, const std::vector<Type> &parameters, Type result,
                                      bool separate_context = false, const std::vector<bool> &borrowed = {},
                                      bool borrowed_result = false, bool scalar_result = false) {
    auto *signature = native_signature(parameters, result, separate_context, borrowed, borrowed_result, scalar_result);
    auto *function = llvm::Function::Create(signature, llvm::GlobalValue::InternalLinkage,
                                           "cod.call." + name, *module_);
    const auto argument = separate_context ? 1u : 0u;
    native_contexts_.emplace(function, NativeContext{argument, {}});
    return {function, result, true, false, false, {}, argument, nullptr, borrowed_result, scalar_result};
  }
  llvm::Value *scalar_result(llvm::Value *payload, llvm::Value *address) {
    auto *type = llvm::StructType::get(context(), {payload->getType(), pointer_type()});
    auto *result = b().CreateInsertValue(llvm::PoisonValue::get(type), payload, {0});
    return b().CreateInsertValue(result, address, {1});
  }
  std::pair<llvm::Value *, llvm::Value *> scalar_result_parts(llvm::Value *result) {
    return {b().CreateExtractValue(result, {0}), b().CreateExtractValue(result, {1})};
  }
  std::pair<llvm::Value *, llvm::Instruction *> pending_scalar_read(Type type, llvm::Value *address,
                                                                   llvm::Value *payload) {
    auto *pending = b().Insert(new llvm::FreezeInst(llvm::ConstantInt::getFalse(context())), "scalar.result.view");
    auto *view = label("scalar.result.load"), *value = label("scalar.result.value"), *merge = label("scalar.result.read");
    choose(pending, view, value);
    enter(view);
    auto *loaded = b().CreateLoad(storage_type(type), address);
    auto *loaded_from = here(); jump(merge);
    enter(value);
    auto *value_from = here(); jump(merge);
    enter(merge);
    return {phi(storage_type(type), {{loaded, loaded_from}, {payload, value_from}}), pending};
  }
  void resolve_scalar_read(llvm::Instruction *pending, bool view) {
    pending->replaceAllUsesWith(llvm::ConstantInt::get(llvm::Type::getInt1Ty(context()), view));
    pending->eraseFromParent();
  }
  NativeBody begin_native(NativeDefinition &definition, llvm::BasicBlock *resume = nullptr) {
    NativeBody previous{entry_, b().saveIP(), std::move(cleanups_), std::move(module_regions_),
                        std::move(module_callables_), callable_depth_, std::move(active_protections_)};
    cleanups_.clear(); module_regions_.clear(); module_callables_.clear(); callable_depth_ = 0;
    active_protections_.clear();
    entry_ = definition.function;
    if (resume) { resume->getTerminator()->eraseFromParent(); enter(resume); }
    else enter(llvm::BasicBlock::Create(context(), "entry", entry_));
    return previous;
  }
  void native_return(llvm::Value *value = nullptr, size_t category = 0, llvm::Value *error = nullptr) {
    llvm::Value *returned = llvm::PoisonValue::get(entry_->getReturnType());
    returned = b().CreateInsertValue(returned, integer(i64(), category), {0});
    if (error) returned = b().CreateInsertValue(returned, error, {1});
    if (value) returned = b().CreateInsertValue(returned, value, {2});
    b().CreateRet(returned);
  }
  void end_native(NativeDefinition &definition, NativeBody previous) {
    if (!cleanups_.empty()) throw std::logic_error("unclosed native callable resource scope");
    // Record known inputs for reuse checks. Their layout and loads wait until
    // every demanded body and context edge is available.
    auto &context = native_contexts_.at(definition.function);
    context.inputs.clear();
    context.closed = true;
    for (auto &block : *definition.function)
      for (auto &instruction : block)
        for (const auto &operand : instruction.operands())
          add_context_input(context, operand.get(), definition.function);
    for (const auto &[use, target] : context_uses_) {
      if (use->getFunction() != definition.function) continue;
      if (!target || !native_contexts_.at(target).closed) { context.closed = false; continue; }
      for (auto *input : native_contexts_.at(target).inputs)
        add_context_input(context, input, definition.function);
    }
    entry_ = previous.parent;
    cleanups_ = std::move(previous.cleanups);
    module_regions_ = std::move(previous.modules);
    module_callables_ = std::move(previous.callables);
    active_protections_ = std::move(previous.protections);
    callable_depth_ = previous.callable_depth;
    b().restoreIP(previous.insertion);
    definition.complete = true;
  }
  llvm::Value *native_environment(const NativeDefinition &definition) {
    if (context_free(definition)) return null_pointer();
    auto *pending = pending_context();
    context_uses_.at(pending) = definition.function;
    return pending;
  }
  llvm::Value *native_call(const NativeDefinition &definition, llvm::Value *environment,
                           const std::vector<llvm::Value *> &parameters, llvm::Value *callee = nullptr,
                           llvm::Value *destination = nullptr, llvm::Value *context = nullptr) {
    std::vector<llvm::Value *> arguments{environment};
    if (definition.context_argument) {
      if (!context) throw std::logic_error("procedure adapter requires caller context");
      arguments.push_back(context);
    }
    if (definition.result.kind == Type::procedure && !definition.borrowed_result) {
      if (!destination) throw std::logic_error("native callable result requires caller storage");
      arguments.push_back(destination);
    }
    arguments.insert(arguments.end(), parameters.begin(), parameters.end());
    auto *returned = b().CreateCall(definition.signature ? definition.signature : definition.function->getFunctionType(),
                                    callee ? callee : definition.function, arguments);
    for (size_t protection : active_protections_) callable_protections_[protection].calls.push_back(returned);
    const auto result = [&]() -> llvm::Value * {
      if (definition.result == Type::void_type) return nullptr;
      if (destination) return b().CreateLoad(pointer_type(), destination);
      return b().CreateExtractValue(returned, {2});
    };
    if (definition.complete && definition.failures.empty()) {
      if (!definition.returns) { unreachable(); return nullptr; }
      return result();
    }
    auto *category = b().CreateExtractValue(returned, {0});
    auto *failure = label("call.failure");
    auto *success = label("call.success");
    b().CreateCondBr(is_null(category), success, failure);
    enter(failure);
    auto *error = b().CreateExtractValue(returned, {1});
    dispatch_exception(category, [&](size_t kind) {
      const bool possible = (!exception_categories.at(kind).checked || definition.checked_null) &&
                            (!definition.complete || definition.failures.contains(kind));
      if (possible) exception_handler_(kind, error);
      else unreachable();
    });
    enter(success);
    if (!definition.returns) { unreachable(); return nullptr; }
    return result();
  }
  llvm::Instruction *pending_context() {
    auto *pending = b().Insert(new llvm::FreezeInst(null_pointer()), "procedure.context.pending");
    context_uses_.emplace(pending, nullptr);
    return pending;
  }
  llvm::Instruction *pending_region() {
    return b().Insert(new llvm::FreezeInst(null_pointer()), "region.result.pending");
  }
  llvm::Instruction *pending_callable() {
    return b().Insert(new llvm::FreezeInst(null_pointer()), "callable.selection.pending");
  }
  void resolve_context(llvm::Instruction *pending, const NativeDefinition &definition) {
    context_uses_.at(pending) = definition.function;
  }
  void finish_native_contexts() {
    // A context's external inputs include those needed to construct its
    // callees' contexts. Close that dependency graph before choosing layouts.
    std::map<llvm::Function *, std::set<llvm::Function *>> callers;
    for (const auto &[use, target] : context_uses_) {
      if (!target) throw std::logic_error("unresolved native context");
      if (native_contexts_.contains(use->getFunction())) callers[target].insert(use->getFunction());
    }
    std::vector<llvm::Function *> pending;
    std::set<llvm::Function *> queued;
    for (auto &[function, context] : native_contexts_) {
      context.inputs.clear();
      for (auto &block : *function)
        for (auto &instruction : block)
          for (const auto &operand : instruction.operands())
            add_context_input(context, operand.get(), function);
      pending.push_back(function);
      queued.insert(function);
    }
    while (!pending.empty()) {
      auto *function = pending.back(); pending.pop_back(); queued.erase(function);
      for (auto *caller : callers[function]) {
        auto &context = native_contexts_.at(caller);
        bool changed = false;
        for (auto *input : native_contexts_.at(function).inputs)
          changed |= add_context_input(context, input, caller);
        if (changed && queued.insert(caller).second) pending.push_back(caller);
      }
    }
    for (auto &[function, context] : native_contexts_) {
      if (context_free_entries_.contains(function) && !context.inputs.empty())
        throw std::logic_error("callable adapter retained an uncaptured native dependency");
      std::vector<llvm::Type *> fields;
      for (auto *input : context.inputs) fields.push_back(input->getType());
      context.layout = llvm::StructType::create(this->context(), fields, "cod.context");
    }
    for (const auto &[use, target] : context_uses_) {
      const auto &context = native_contexts_.at(target);
      llvm::Value *storage = null_pointer();
      if (!context.inputs.empty()) {
        auto &entry = use->getFunction()->getEntryBlock();
        llvm::IRBuilder<> allocate(&entry, entry.begin()), write(use);
        storage = allocate.CreateAlloca(context.layout, nullptr, "call.context");
        for (size_t i = 0; i < context.inputs.size(); ++i)
          write.CreateStore(context.inputs[i], write.CreateStructGEP(context.layout, storage, unsigned(i)));
      }
      use->replaceAllUsesWith(storage);
      use->eraseFromParent();
    }
    context_uses_.clear();
    for (const auto &[function, context] : native_contexts_) {
      if (function->empty()) continue;
      llvm::IRBuilder<> loads(&function->getEntryBlock(), function->getEntryBlock().begin());
      for (size_t index = 0; index < context.inputs.size(); ++index) {
        auto *input = context.inputs[index];
        auto *address = loads.CreateStructGEP(context.layout, function->getArg(context.argument), unsigned(index));
        auto *loaded = loads.CreateLoad(input->getType(), address, "context.value");
        for (auto use = input->use_begin(); use != input->use_end();) {
          auto &current = *use++;
          if (const auto *instruction = llvm::dyn_cast<llvm::Instruction>(current.getUser());
              instruction && instruction->getFunction() == function) current.set(loaded);
        }
      }
    }
  }
  void reuse_native_context(NativeDefinition &target, const NativeDefinition &source,
                            llvm::Instruction *pending, bool recursive = true) {
    if (target.function->getFunctionType() != source.function->getFunctionType())
      throw std::logic_error("recursive adapter signature mismatch");
    target.function->replaceAllUsesWith(source.function);
    native_contexts_.erase(target.function);
    target.function->eraseFromParent();
    if (recursive) {
      context_uses_.erase(pending);
      pending->replaceAllUsesWith(source.function->getArg(source.context_argument));
      pending->eraseFromParent();
    } else context_uses_.at(pending) = source.function;
    target = source;
  }
  llvm::Constant *callable_key() {
    return new llvm::GlobalVariable(*module_, llvm::Type::getInt8Ty(context()), true,
        llvm::GlobalValue::PrivateLinkage, llvm::ConstantInt::get(llvm::Type::getInt8Ty(context()), 0), "callable.body");
  }
  struct CallableMatch {
    llvm::Value *entry;
    llvm::CondBrInst *branch;
    llvm::BasicBlock *next;
  };
  CallableMatch match_callable(llvm::Value *image, llvm::Constant *entry) {
    auto *match = label("procedure.selected"), *next = label("procedure.other");
    auto *selected = callable_entry(image);
    auto *branch = b().CreateCondBr(b().CreateICmpEQ(selected, entry), match, next);
    enter(match);
    return {selected, branch, next};
  }
  void extend_callable_match(const CallableMatch &match, llvm::Constant *entry) {
    llvm::IRBuilder<> builder(match.branch);
    match.branch->setCondition(builder.CreateOr(match.branch->getCondition(),
        builder.CreateICmpEQ(match.entry, entry)));
  }
  llvm::StructType *callable_layout(const std::vector<llvm::Value *> &values,
                                   const std::set<size_t> &nested) {
    std::vector<llvm::Type *> fields{i64(), pointer_type()};
    for (size_t i = 0; i < values.size(); ++i)
      fields.push_back(nested.contains(i) ? i64() : values[i]->getType());
    return llvm::StructType::create(context(), fields, "cod.callable");
  }
  llvm::Constant *static_callable(llvm::StructType *layout, llvm::Constant *key) {
    // Direct named calls carry no environment image and allocate no user value.
    auto *descriptor = llvm::ConstantStruct::get(layout,
        {llvm::ConstantInt::get(i64(), module_->getDataLayout().getTypeAllocSize(layout)), key});
    return new llvm::GlobalVariable(*module_, layout, true, llvm::GlobalValue::PrivateLinkage,
                                    descriptor, "callable.direct");
  }
  llvm::Value *callable_field(llvm::Value *image, llvm::StructType *layout, size_t field, bool nested) {
    auto *address = b().CreateStructGEP(layout, image, unsigned(field + 2));
    auto *value = b().CreateLoad(layout->getElementType(unsigned(field + 2)), address);
    return nested ? b().CreateGEP(llvm::Type::getInt8Ty(context()), image, value) : value;
  }
  struct CallableConstruction {
    struct Child {
      size_t field;
      llvm::StoreInst *value, *offset;
    };
    llvm::Instruction *count = nullptr;
    llvm::Value *values = nullptr, *offsets = nullptr;
    std::vector<Child> children;
  };
  llvm::Value *create_callable(llvm::Function *entry, llvm::StructType *layout,
                               const std::vector<llvm::Value *> &values, const std::set<size_t> &nested,
                               llvm::Constant *key = nullptr, CallableConstruction *construction = nullptr) {
    if (entry) callable_entries_.insert(entry);
    const auto &data = module_->getDataLayout();
    auto *children = entry_slot(llvm::ArrayType::get(pointer_type(), nested.size()), "callable.children");
    auto *offsets = entry_slot(llvm::ArrayType::get(i64(), nested.size()), "callable.offsets");
    if (construction) { construction->values = children; construction->offsets = offsets; }
    size_t child = 0;
    for (size_t i : nested) {
      auto *value = b().CreateStore(values[i], b().CreateConstGEP1_64(pointer_type(), children, child));
      auto *offset = b().CreateStore(integer(i64(), data.getStructLayout(layout)->getElementOffset(unsigned(i + 2))),
                                    b().CreateConstGEP1_64(i64(), offsets, child++));
      if (construction) construction->children.push_back({i, value, offset});
    }
    auto *bytes = integer(i64(), data.getTypeAllocSize(layout));
    llvm::Value *count = integer(i64(), nested.size());
    if (construction) count = construction->count = b().Insert(new llvm::FreezeInst(count), "capture.count");
    auto *image = checked_service("cod_lang_callable_create", pointer_type(),
        {bytes, count, children, offsets}, bytes, {ExceptionCategory::out_of_memory});
    b().CreateStore(key ? key : entry, b().CreateStructGEP(layout, image, 1));
    for (size_t i = 0; i < values.size(); ++i)
      if (!nested.contains(i)) b().CreateStore(values[i], b().CreateStructGEP(layout, image, unsigned(i + 2)));
    return image;
  }
  void finish_callable(CallableConstruction &construction, const std::set<size_t> &required) {
    if (!construction.count) return;
    // Body checking decides which dynamic values are captured. Compact the
    // constructor's child list before LLVM sees it; the copy still executes
    // at the original definition point, with its original exception cleanup.
    size_t count = 0;
    for (auto &child : construction.children) {
      if (required.contains(child.field)) {
        for (auto *store : {child.value, child.offset})
          llvm::cast<llvm::GetElementPtrInst>(store->getPointerOperand())->setOperand(1, integer(i64(), count));
        ++count;
      } else {
        child.value->eraseFromParent();
        child.offset->eraseFromParent();
      }
    }
    construction.count->replaceAllUsesWith(integer(i64(), count));
    construction.count->eraseFromParent();
    construction.count = nullptr;
    for (auto *storage : {construction.values, construction.offsets}) {
      auto *allocation = llvm::cast<llvm::AllocaInst>(storage);
      llvm::Value *replacement = null_pointer();
      if (count) {
        llvm::IRBuilder<> builder(allocation);
        auto *element = llvm::cast<llvm::ArrayType>(allocation->getAllocatedType())->getElementType();
        replacement = builder.CreateAlloca(llvm::ArrayType::get(element, count));
      }
      allocation->replaceAllUsesWith(replacement);
      allocation->eraseFromParent();
    }
  }
  llvm::Value *copy_callable(llvm::Value *image) {
    auto *bytes = b().CreateLoad(i64(), image);
    return checked_service("cod_lang_callable_copy", pointer_type(), {image}, bytes, {ExceptionCategory::out_of_memory});
  }
  size_t protect_callable(llvm::Value *image) {
    auto *allocation = here();
    auto *copy = copy_callable(image);
    auto *slot = entry_slot(pointer_type(), "invocation.image", true);
    auto *installed = b().CreateStore(copy, slot);
    auto cleanup = std::make_shared<std::vector<llvm::Instruction *>>();
    cleanups_.push_back({slot, nullptr, true, {}, cleanup, nullptr, cleanup_owner_});
    const auto id = callable_protections_.size();
    callable_protections_.push_back({image, copy, slot, installed,
        llvm::cast<llvm::CondBrInst>(allocation->getTerminator()), std::move(cleanup), {}});
    active_protections_.push_back(id);
    return id;
  }
  llvm::Value *protected_callable(size_t id) { return callable_protections_.at(id).image; }
  void leave_callable_protection(size_t id) {
    if (active_protections_.empty() || active_protections_.back() != id) {
      // An error can leave checking inside an unfinished native body. Keep
      // its original diagnostic instead of throwing from the caller's guard.
      if (std::uncaught_exceptions()) return;
      throw std::logic_error("unbalanced callable invocation protection");
    }
    active_protections_.pop_back();
  }
  void release_callable_protection(size_t id) {
    auto &protection = callable_protections_.at(id);
    release_slot(protection.slot, true, protection.cleanup);
  }
  void callable_write() {
    callable_writers_.insert(entry_);
    for (size_t protection : active_protections_) callable_protections_[protection].required = true;
  }
  void resolve_callable_protections() {
    // Body checking has finished: propagate actual replacement operations
    // through native calls, including recursive cycles. Invocation snapshots
    // are retained only where arguments or a selected body can reach a write.
    std::map<llvm::Function *, std::set<llvm::Function *>> callers;
    for (auto &function : *module_)
      for (auto &block : function)
        for (auto &instruction : block)
          if (auto *call = llvm::dyn_cast<llvm::CallBase>(&instruction))
            if (auto *target = call->getCalledFunction()) callers[target].insert(&function);
    std::vector<llvm::Function *> pending(callable_writers_.begin(), callable_writers_.end());
    while (!pending.empty()) {
      auto *function = pending.back(); pending.pop_back();
      for (auto *caller : callers[function])
        if (callable_writers_.insert(caller).second) pending.push_back(caller);
    }
    for (auto &protection : callable_protections_) {
      for (auto *call : protection.calls)
        protection.required |= callable_writers_.contains(call->getCalledFunction());
      if (protection.required) continue;
      protection.failure->setCondition(llvm::ConstantInt::getFalse(context()));
      for (auto *instruction : *protection.cleanup) instruction->eraseFromParent();
      protection.installed->eraseFromParent();
      protection.image->replaceAllUsesWith(protection.source);
      llvm::cast<llvm::Instruction>(protection.image)->eraseFromParent();
    }
  }
  llvm::Value *hold_callable(llvm::Value *image, bool module_lifetime) {
    auto *slot = entry_slot(pointer_type(), "callable.owner", true);
    b().CreateStore(image, slot);
    if (module_lifetime) module_callables_.push_back(slot);
    else cleanups_.push_back({slot, nullptr, true, {}, {}, nullptr, cleanup_owner_});
    return slot;
  }
  llvm::Value *take_callable(llvm::Value *slot) {
    auto *image = b().CreateLoad(pointer_type(), slot);
    b().CreateStore(null_pointer(), slot);
    return image;
  }
  void replace_callable(llvm::Value *slot, llvm::Value *image) {
    call_runtime("cod_lang_callable_free", void_type(), {b().CreateLoad(pointer_type(), slot)});
    b().CreateStore(image, slot);
  }
  llvm::Value *callable_entry(llvm::Value *image) {
    auto *header = llvm::StructType::get(context(), {i64(), pointer_type()});
    return b().CreateLoad(pointer_type(), b().CreateStructGEP(header, image, 1));
  }
  void set_callable_entry(llvm::Value *image, llvm::Constant *entry) {
    auto *header = llvm::StructType::get(context(), {i64(), pointer_type()});
    b().CreateStore(entry, b().CreateStructGEP(header, image, 1));
  }
  llvm::Value *global_bits(Type type, llvm::Value *value) {
    auto *slot = new llvm::GlobalVariable(*module_, storage_type(type), false, llvm::GlobalValue::InternalLinkage,
        llvm::Constant::getNullValue(storage_type(type)), "global.bits");
    b().CreateStore(value, slot);
    return slot;
  }
  static std::set<size_t> possible_exceptions(const llvm::Value *category) {
    std::set<size_t> result;
    std::vector<const llvm::Value *> pending{category};
    std::set<const llvm::Value *> visited;
    while (!pending.empty()) {
      auto *value = pending.back(); pending.pop_back();
      if (!visited.insert(value).second || llvm::isa<llvm::PoisonValue>(value)) continue;
      if (const auto *constant = llvm::dyn_cast<llvm::ConstantInt>(value)) {
        result.insert(constant->getZExtValue());
      } else if (const auto *phi = llvm::dyn_cast<llvm::PHINode>(value)) {
        for (auto &incoming : phi->incoming_values()) pending.push_back(incoming.get());
      } else if (const auto *select = llvm::dyn_cast<llvm::SelectInst>(value)) {
        pending.push_back(select->getTrueValue()); pending.push_back(select->getFalseValue());
      } else {
        for (size_t kind = 1; kind < exception_categories.size(); ++kind) result.insert(kind);
        return result;
      }
    }
    result.erase(0);
    return result;
  }
  void dispatch_exception(llvm::Value *category, const std::function<void(size_t)> &emit,
                          std::optional<std::set<size_t>> candidates = {}) {
    const auto possible = candidates ? std::move(*candidates) : possible_exceptions(category);
    for (const auto kind : possible) {
      if (kind == *possible.rbegin()) { emit(kind); return; }
      auto *match = label("exception.type");
      auto *next = label("exception.next");
      b().CreateCondBr(b().CreateICmpEQ(category, integer(i64(), kind)), match, next);
      enter(match);
      emit(kind);
      enter(next);
    }
    b().CreateUnreachable();
  }
  llvm::Value *is_null(llvm::Value *value) {
    return b().CreateICmpEQ(value, llvm::Constant::getNullValue(value->getType()));
  }
  void null_check(llvm::Value *value) {
    auto *bad = label("null.raise");
    auto *next = label("nonnull.continue");
    b().CreateCondBr(is_null(value), bad, next);
    enter(bad);
    const auto site = exception_sites_.size();
    exception_sites_.push_back(source_context());
    exception_handler_(exception_id(ExceptionCategory::null_pointer), exception_payload(integer(i64(), site), integer(i64(), 0), integer(i64(), 0)));
    enter(next);
  }
  void missing_return() {
    const auto site = exception_sites_.size();
    exception_sites_.push_back(source_context());
    exception_handler_(exception_id(ExceptionCategory::missing_return),
                       exception_payload(integer(i64(), site), integer(i64(), 0), integer(i64(), 0)));
  }
  llvm::Value *nullable_address(llvm::Value *owner, llvm::Value *offset) {
    return b().CreateSelect(is_null(offset), llvm::ConstantPointerNull::get(pointer_type()), address(owner, offset));
  }
  llvm::Value *nullable_offset(llvm::Value *owner, llvm::Value *target) {
    return b().CreateSelect(is_null(target), integer(i64(), 0), offset_of(owner, target));
  }
  void require_representable_reference(const Place &field, llvm::Value *offset) {
    require(b().CreateICmpNE(address(field.owner, offset), field.address),
            "non-null ref cannot encode a zero-displacement self reference");
  }
  llvm::Value *null_pointer() { return llvm::ConstantPointerNull::get(pointer_type()); }
  llvm::Value *exception_payload(llvm::Value *site, llvm::Value *requested, llvm::Value *available) {
    llvm::Value *payload = llvm::PoisonValue::get(exception_payload_type());
    payload = b().CreateInsertValue(payload, site, {0});
    payload = b().CreateInsertValue(payload, requested, {1});
    payload = b().CreateInsertValue(payload, available, {2});
    return b().CreateInsertValue(payload, integer(i64(), 0), {3});
  }
  llvm::Value *exception_field(llvm::Value *payload, unsigned index) {
    return b().CreateExtractValue(payload, {index});
  }
  llvm::Value *exception_chain(llvm::Value *primary, llvm::Value *cause, llvm::Value *category) {
    auto *handle = checked_service("cod_lang_exception_chain", i64(),
        {category, exception_field(cause, 0), exception_field(cause, 1), exception_field(cause, 2), exception_field(cause, 3)},
        integer(i64(), 5 * sizeof(uint64_t)), {ExceptionCategory::out_of_memory});
    return b().CreateInsertValue(primary, handle, {3});
  }
  llvm::Value *cause_count(llvm::Value *payload) {
    return call_runtime("cod_lang_exception_count", i64(), {exception_field(payload, 3)});
  }
  std::pair<llvm::Value *, llvm::Value *> cause_at(llvm::Value *payload, llvm::Value *index) {
    auto *storage = entry_slot(llvm::ArrayType::get(i64(), 5), "exception.cause");
    call_runtime("cod_lang_exception_cause", void_type(), {exception_field(payload, 3), index, storage});
    return {b().CreateLoad(exception_payload_type(), storage),
            b().CreateLoad(i64(), b().CreateConstGEP1_64(i64(), storage, 4))};
  }
  void uncaught(size_t category, llvm::Value *payload) {
    auto *pending = label("uncaught.pending");
    uncaught_sites_.push_back({pending, category, payload, cleanups_, module_regions_, module_callables_});
    jump(pending);
  }
  void resolve_uncaught() {
    // Each acquisition adds a cleanup prefix. Share those prefixes instead of
    // cloning the entire module's cleanup at every potentially failing call.
    // The three phases preserve local restoration, callable release, then
    // regional release. Only failure paths carry the selectors and payload.
    const auto insertion = b().saveIP();
    auto *parent = entry_;
    std::map<llvm::Function *, std::vector<const UncaughtSite *>> functions;
    for (const auto &site : uncaught_sites_) functions[site.block->getParent()].push_back(&site);
    for (const auto &[function, sites] : functions) {
      entry_ = function;
      auto *category = entry_slot(i64(), "uncaught.category");
      auto *payload = entry_slot(exception_payload_type(), "uncaught.payload");
      auto *callable_selection = entry_slot(i64(), "uncaught.callables");
      auto *region_selection = entry_slot(i64(), "uncaught.regions");
      auto *terminal = label("uncaught.report");
      enter(terminal);
      auto *error = b().CreateLoad(exception_payload_type(), payload);
      call_runtime("cod_lang_uncaught", void_type(), {b().CreateLoad(i64(), category),
          exception_field(error, 0), exception_field(error, 1), exception_field(error, 2), exception_field(error, 3)});
      b().CreateUnreachable();
      auto *regions = label("uncaught.regions");
      enter(regions);
      auto *region_switch = b().CreateSwitch(b().CreateLoad(i64(), region_selection), terminal);
      auto *callables = label("uncaught.callables");
      enter(callables);
      auto *callable_switch = b().CreateSwitch(b().CreateLoad(i64(), callable_selection), regions);
      using ResourceKey = std::pair<llvm::Value *, llvm::BasicBlock *>;
      std::map<ResourceKey, std::pair<llvm::BasicBlock *, size_t>> region_nodes, callable_nodes;
      const auto resources = [&](const auto &slots, auto &nodes, llvm::BasicBlock *end,
                                 llvm::SwitchInst *selection, bool callable) {
        auto *next = end;
        size_t id = 0;
        for (auto *slot : slots) {
          const auto key = ResourceKey{slot, next};
          auto found = nodes.find(key);
          if (found == nodes.end()) {
            auto *block = label("uncaught.release");
            const auto index = nodes.size() + 1;
            found = nodes.emplace(key, std::pair{block, index}).first;
            selection->addCase(integer(i64(), index), block);
            enter(block);
            release_slot(slot, callable);
            jump(next);
          }
          next = found->second.first;
          id = found->second.second;
        }
        return id;
      };
      using CleanupKey = std::tuple<llvm::Value *, llvm::Value *, bool, const EffectWrites *, llvm::BasicBlock *>;
      std::map<CleanupKey, llvm::BasicBlock *> cleanup_nodes;
      for (const auto *site : sites) {
        const auto region_id = resources(site->regions, region_nodes, terminal, region_switch, false);
        const auto callable_id = resources(site->callables, callable_nodes, regions, callable_switch, true);
        auto *next = callables;
        for (const auto &cleanup : site->cleanups) {
          const CleanupKey key{cleanup.address, cleanup.saved, cleanup.callable, cleanup.effect_writes.get(), next};
          auto found = cleanup_nodes.find(key);
          if (found == cleanup_nodes.end()) {
            auto *block = label("uncaught.restore");
            found = cleanup_nodes.emplace(key, block).first;
            enter(block);
            emit_cleanup(cleanup);
            jump(next);
          }
          next = found->second;
        }
        enter(site->block);
        b().CreateStore(integer(i64(), site->category), category);
        b().CreateStore(site->payload, payload);
        b().CreateStore(integer(i64(), region_id), region_selection);
        b().CreateStore(integer(i64(), callable_id), callable_selection);
        jump(next);
      }
    }
    uncaught_sites_.clear();
    entry_ = parent;
    b().restoreIP(insertion);
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
    std::vector<Cleanup> cleanups;
    std::vector<llvm::Value *> modules;
    std::vector<llvm::Value *> callables;
  };
  struct ConversionResume {
    llvm::Function *function;
    llvm::IRBuilderBase::InsertPoint insertion;
    std::vector<Cleanup> cleanups;
    std::vector<llvm::Value *> modules;
    std::vector<llvm::Value *> callables;
  };
  ConversionPoint conversion_point() {
    auto *before = b().GetInsertBlock();
    auto *after = llvm::BasicBlock::Create(context(), "argument.next", entry_);
    b().CreateBr(after);
    b().SetInsertPoint(after);
    return {before, after, cleanups_, module_regions_, module_callables_};
  }
  ConversionResume begin_conversion(const ConversionPoint &point) {
    ConversionResume resume{entry_, b().saveIP(), cleanups_, module_regions_, module_callables_};
    entry_ = point.before->getParent();
    cleanups_ = point.cleanups;
    module_regions_ = point.modules;
    module_callables_ = point.callables;
    point.before->getTerminator()->eraseFromParent();
    b().SetInsertPoint(point.before);
    return resume;
  }
  void end_conversion(const ConversionPoint &point, const ConversionResume &resume) {
    if (reachable()) b().CreateBr(point.after);
    cleanups_ = resume.cleanups;
    module_regions_ = resume.modules;
    module_callables_ = resume.callables;
    entry_ = resume.function;
    b().restoreIP(resume.insertion);
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
    branch.first_end = reachable() ? b().GetInsertBlock() : nullptr;
    branch.first_value = first;
    if (reachable()) b().CreateBr(branch.merge);
    b().SetInsertPoint(branch.otherwise);
  }
  llvm::Value *merge(Branch &branch, Type type, llvm::Value *second = nullptr) {
    auto *second_end = reachable() ? b().GetInsertBlock() : nullptr;
    if (reachable()) b().CreateBr(branch.merge);
    b().SetInsertPoint(branch.merge);
    if (!branch.first_end && !second_end) { b().CreateUnreachable(); return nullptr; }
    if (type == Type::void_type) return nullptr;
    auto *result = b().CreatePHI(type_of(type), 2);
    if (branch.first_end) result->addIncoming(branch.first_value, branch.first_end);
    if (second_end) result->addIncoming(second, second_end);
    return result;
  }

  struct Loop {
    llvm::BasicBlock *header = nullptr, *exit = nullptr;
    llvm::PHINode *index = nullptr, *accumulator = nullptr;
    llvm::PHINode *exception_category = nullptr;
  };
  Loop loop(Type index_type, llvm::Value *first, llvm::Value *last, llvm::Value *initial = nullptr,
            llvm::Value *category = nullptr) {
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
    llvm::PHINode *exception_category = nullptr;
    if (category) {
      exception_category = b().CreatePHI(i64(), 2, "exception.category");
      exception_category->addIncoming(category, before);
    }
    auto *condition = signed_integer(index_type) ? b().CreateICmpSLT(index, last) :
                                                  b().CreateICmpULT(index, last);
    b().CreateCondBr(condition, body, exit);
    b().SetInsertPoint(body);
    return {header, exit, index, accumulator, exception_category};
  }
  void end_loop(Loop &loop, llvm::Value *next = nullptr, llvm::Value *category = nullptr) {
    if (reachable()) {
      auto *end = b().GetInsertBlock();
      auto *index = b().CreateAdd(loop.index, integer(llvm::cast<llvm::IntegerType>(loop.index->getType()), 1));
      loop.index->addIncoming(index, end);
      if (loop.accumulator) loop.accumulator->addIncoming(next, end);
      if (loop.exception_category) loop.exception_category->addIncoming(category, end);
      b().CreateBr(loop.header);
    }
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
    return checked_service("cod_lang_region_alloc", i64(), {owner, bytes, integer(i64(), alignment)},
                           bytes, {ExceptionCategory::region_capacity_exceeded});
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
    auto *bytes = b().CreateZExtOrTrunc(capacity, i64());
    return checked_service("cod_lang_region_new", pointer_type(), {bytes}, bytes, {ExceptionCategory::out_of_memory, ExceptionCategory::invalid_capacity});
  }
  llvm::Value *hold_region(llvm::Value *region, bool module_lifetime) {
    auto *slot = entry_slot(pointer_type(), "region.owner", true);
    b().CreateStore(region, slot);
    if (module_lifetime) module_regions_.push_back(slot);
    else cleanups_.push_back({slot, nullptr, false, {}, {}, nullptr, cleanup_owner_});
    return slot;
  }
  void relinquish_region(llvm::Value *slot) {
    b().CreateStore(llvm::ConstantPointerNull::get(pointer_type()), slot);
  }
  void replace_region(llvm::Value *slot, llvm::Value *owner) {
    call_runtime("cod_lang_region_free", void_type(), {b().CreateLoad(pointer_type(), slot)});
    b().CreateStore(owner, slot);
  }
  llvm::Value *region_root(llvm::Value *owner) {
    return call_runtime("cod_lang_region_root", i64(), {owner});
  }
  llvm::Value *reserve_root(llvm::Value *owner, Type type) {
    const auto &layout = module_->getDataLayout();
    auto *record = record_type(type);
    auto *bytes = integer(i64(), std::max<uint64_t>(1, layout.getTypeAllocSize(record).getFixedValue()));
    return checked_service("cod_lang_region_reserve_root", i64(),
        {owner, bytes, integer(i64(), layout.getABITypeAlign(record).value())}, bytes, {ExceptionCategory::region_capacity_exceeded});
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
      write({fields[i], owner, field}, args[i]);
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
    write({element, owner, b().CreateGEP(element_type, data, fill.index)}, initial);
    end_loop(fill);
    b().CreateBr(done);
    b().SetInsertPoint(done);
    return offset;
  }
  // A place denotes storage; reading it produces an SSA value at that point.
  // Neither a bits read nor its users promise that aliased storage is immutable.
  llvm::Value *local_storage(Type type) { return entry_slot(storage_type(type), "local.bits"); }
  llvm::Value *pointer_storage(llvm::Value *address) {
    auto *slot = entry_slot(address->getType(), "local.ptr");
    b().CreateStore(address, slot);
    return slot;
  }
  llvm::Value *read_pointer(llvm::Value *slot, llvm::Value *shape) { return b().CreateLoad(shape->getType(), slot); }
  llvm::Value *retained_pointer(const std::vector<llvm::Value *> &slots, llvm::Value *shape) {
    llvm::Value *result = nullptr;
    for (auto *slot : slots) {
      auto *candidate = read_pointer(slot, shape);
      result = result ? b().CreateSelect(is_null(candidate), result, candidate) : candidate;
    }
    return result;
  }
  void write_pointer(llvm::Value *slot, llvm::Value *address) { b().CreateStore(address, slot); }
  Place referent(llvm::Value *owner, llvm::Value *source, Type type) {
    return {type, owner, checked_address(owner, source)};
  }
  Place field(llvm::Value *owner, llvm::Value *source, Type record, size_t index, Type type) {
    return {type, owner, b().CreateStructGEP(record_type(record), checked_address(owner, source), unsigned(index))};
  }
  std::pair<Place, llvm::Instruction *> pending_field(llvm::Value *owner, llvm::Value *source, Type type) {
    auto *offset = b().Insert(new llvm::FreezeInst(integer(i64(), 0)), "root.field.offset");
    auto *address = b().CreateGEP(llvm::Type::getInt8Ty(context()), checked_address(owner, source), offset);
    return {{type, owner, address}, offset};
  }
  void resolve_field(llvm::Instruction *offset, Type record, size_t index) {
    const auto *layout = module_->getDataLayout().getStructLayout(record_type(record));
    // Keep the instruction stable for native context descriptors. LLVM folds
    // the now-constant offset when optimizing the finished module.
    offset->setOperand(0, integer(i64(), layout->getElementOffset(unsigned(index))));
  }
  Place element(llvm::Value *owner, llvm::Value *source, Type index_type, llvm::Value *index, Type type) {
    return {type, owner, array_address(owner, source, index_type, index, type)};
  }
  llvm::Value *reference(const Place &place) { return offset_of(place.owner, place.address); }
  llvm::Value *read(const Place &place) {
    if (place.type.kind == Type::array || place.type.kind == Type::regional) return reference(place);
    if (place.type.kind == Type::reference) return read_reference(place.owner, place.address);
    return b().CreateLoad(storage_type(place.type), place.address);
  }
  void write(const Place &place, llvm::Value *value) {
    if (place.type.kind == Type::array || place.type.kind == Type::regional)
      throw std::logic_error("regional aggregates cannot be copied");
    if (place.type.kind == Type::reference) store_reference(place.owner, place.address, value);
    else b().CreateStore(value, place.address);
  }
  llvm::Value *array_length(llvm::Value *owner, llvm::Value *source) {
    return b().CreateLoad(i64(), b().CreateStructGEP(array_header_, checked_address(owner, source), 1));
  }
  llvm::Value *argument_element(Type type, llvm::Value *index, const std::vector<llvm::Value *> &values) {
    require(b().CreateICmpULT(index, integer(i64(), values.size())), "variadic argument index out of bounds");
    if (values.empty()) return llvm::PoisonValue::get(type_of(type));
    auto *result = values.front();
    for (size_t i = 1; i < values.size(); ++i)
      result = b().CreateSelect(b().CreateICmpEQ(index, integer(i64(), i)), values[i], result);
    return result;
  }

  std::pair<std::string, std::string> finish(bool emit_text, bool emit_bitcode) {
    if (!cleanups_.empty()) throw std::logic_error("unclosed LLVM resource scope");
    if (reachable()) {
      for (auto slot = module_callables_.rbegin(); slot != module_callables_.rend(); ++slot)
        release_slot(*slot, true);
      for (auto region = module_regions_.rbegin(); region != module_regions_.rend(); ++region)
        release_slot(*region, false);
      call_runtime("cod_lang_exception_finish", void_type(), {});
      b().CreateRet(integer(llvm::Type::getInt32Ty(context()), 0));
    }
    finish_native_contexts();
    resolve_callable_protections();
    finish_continuations();
    // Demand pruning leaves provisional loads/GEPs for omitted captures. Drop
    // them before serialization and native optimization, not in a later host pass.
    llvm::SmallVector<llvm::WeakTrackingVH> unused;
    for (auto &function : *module_)
      for (auto &block : function)
        for (auto &instruction : block)
          if (llvm::isInstructionTriviallyDead(&instruction)) unused.emplace_back(&instruction);
    llvm::RecursivelyDeleteTriviallyDeadInstructions(unused);
    // Resolve sites against the complete program, including sites emitted after
    // an earlier throw whose cause may originate on a previous loop iteration.
    auto *lookup = llvm::Function::Create(llvm::FunctionType::get(pointer_type(), {i64()}, false),
        llvm::GlobalValue::ExternalLinkage, "cod_lang_exception_site", *module_);
    llvm::IRBuilder<> sites(llvm::BasicBlock::Create(context(), "entry", lookup));
    auto *unknown = llvm::BasicBlock::Create(context(), "unknown", lookup);
    auto *dispatch = sites.CreateSwitch(lookup->getArg(0), unknown, unsigned(exception_sites_.size()));
    for (size_t i = 0; i < exception_sites_.size(); ++i) {
      auto *known = llvm::BasicBlock::Create(context(), "site", lookup);
      dispatch->addCase(integer(i64(), i), known);
      sites.SetInsertPoint(known);
      sites.CreateRet(exception_sites_[i]);
    }
    sites.SetInsertPoint(unknown);
    sites.CreateRet(llvm::ConstantPointerNull::get(pointer_type()));
    // The checker specializes callable bodies with their actual module/effect
    // context. Outline those checked regions after all escaping SSA uses are
    // known, including captured values returned by nested factories. LLVM
    // derives typed inputs/outputs and represents exceptional exits explicitly.
    std::map<llvm::Function *, std::unique_ptr<llvm::CodeExtractorAnalysisCache>> extraction;
    for (const auto &body : callable_bodies_) {
      auto &cache = extraction[body.front()->getParent()];
      if (!cache) cache = std::make_unique<llvm::CodeExtractorAnalysisCache>(*body.front()->getParent());
      llvm::CodeExtractor extractor(body, nullptr, false, nullptr, nullptr, nullptr,
                                    false, false, nullptr, {}, "callable");
      if (!extractor.extractCodeRegion(*cache))
        throw std::logic_error("cannot outline checked callable body");
    }
    std::string errors;
    // A constructed but never invoked callable does not demand its body.
    for (auto *function : callable_entries_)
      if (function->empty()) {
        llvm::IRBuilder<> unused(llvm::BasicBlock::Create(context(), "undemanded", function));
        unused.CreateUnreachable();
      }
    llvm::raw_string_ostream diagnostics(errors);
    if (llvm::verifyModule(*module_, &diagnostics))
      throw std::runtime_error("invalid emitted LLVM IR: " + errors);
    std::string text, bitcode;
    llvm::raw_string_ostream text_stream(text), bitcode_stream(bitcode);
    if (emit_text) module_->print(text_stream, nullptr);
    if (emit_bitcode) llvm::WriteBitcodeToFile(*module_, bitcode_stream);
    return {std::move(text), std::move(bitcode)};
  }
};

} // namespace cod::language
