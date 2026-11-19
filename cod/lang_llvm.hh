//===--- lang_llvm.hh - Checked scalar native emission --------------===//
#pragma once

#include "lang_checked_program.hh"
#include "lang_exceptions.hh"

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

#include <array>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <utility>

namespace cod::language {

class LLVMEmission {
  llvm::LLVMContext context_;
  llvm::Module module_{"cod.program", context_};
  llvm::IRBuilder<> b_{context_};
  // Private execution-local value transport. Callable environments are complete
  // scope-owned images; they are not regional-storable bits.
  llvm::StructType *exception_ = llvm::StructType::get(context_, std::vector<llvm::Type *>(5, b_.getInt64Ty()));
  llvm::StructType *scalar_ = llvm::StructType::create(context_, {b_.getInt64Ty(), b_.getInt32Ty()}, "cod.scalar");
  llvm::StructType *outcome_ = llvm::StructType::create(context_,
      {scalar_, b_.getInt64Ty(), b_.getInt64Ty(), b_.getPtrTy(), b_.getInt64Ty(), b_.getInt64Ty(), b_.getInt64Ty()}, "cod.outcome");
  llvm::StructType *here_ = llvm::StructType::create(context_, {b_.getPtrTy(), b_.getPtrTy()}, "cod.here");
  llvm::StructType *here_transfer_ = llvm::StructType::create(context_, "cod.here.transfer");
  bool protected_outcomes_ = false;
  const CheckedProgram *program_ = nullptr;
  // Transient native parameter roles shared by declarations, calls and entry.
  struct CallLayout {
    size_t operands = 0, owners = 0, captures = 0, retained = 0;
    bool declaration_context = false, destination = false, accepts_here = false, here_group = false;
    size_t captures_index() const { return operands + owners; }
    size_t retained_begin() const { return captures_index() + (captures != 0); }
    size_t query_index() const { return retained_begin() + retained; }
    size_t result_index() const { return query_index() + declaration_context; }
    size_t here_index() const { return result_index() + destination; }
    size_t parameter_count() const { return here_index() + accepts_here + here_group; }
  };
  std::vector<CallLayout> call_layouts_;
  bool specialize_ = true;
  bool bounds_checks_ = true;
  bool environments_ = false;
  unsigned environment_field_ = 0;
  std::vector<Type> callable_types_;
  std::vector<llvm::Function *> general_, specialized_;
  struct HereEntry { llvm::Function *entry; llvm::StructType *activation; bool group; };
  std::map<size_t, HereEntry> here_entries_;
  std::vector<llvm::GlobalVariable *> symbols_;
  std::map<size_t, llvm::AllocaInst *> effect_values_;
  // Operation facts retain the checked interpretation that justifies them.
  struct EmissionContext {
    const std::map<const Expression *, CheckedOperation> *operations;
    const CheckedInstance *instance = nullptr;
    const CheckedOperation &at(const Expression *expression) const { return operations->at(expression); }
  };
  std::vector<EmissionContext> contexts_;
  class ContextSelection {
    std::vector<EmissionContext> &active_;
    std::vector<EmissionContext> saved_;
  public:
    explicit ContextSelection(std::vector<EmissionContext> &active)
        : active_(active), saved_(std::exchange(active, {})) {}
    ContextSelection(const ContextSelection &) = delete;
    ContextSelection &operator=(const ContextSelection &) = delete;
    ~ContextSelection() { active_ = std::move(saved_); }
    const std::vector<EmissionContext> &all() const { return saved_; }
  };
  std::vector<llvm::Value *> arguments_, retained_types_;
  std::vector<llvm::AllocaInst *> storage_;
  // A checked place role and its native slots describe one lexical binding.
  // Context-dependent type/access facts stay in EmissionContext; physical
  // payload storage has its own indices, independently of bindings.
  struct EmissionBinding {
    bool value_binding;
    llvm::Value *reference_slot = nullptr;
    llvm::AllocaInst *selected_place = nullptr;
    llvm::Value *direct_place = nullptr;
  };
  std::vector<EmissionBinding> bindings_;
  std::map<size_t, llvm::Value *> owner_identities_;
  std::vector<llvm::AllocaInst *> parameter_values_;
  llvm::Value *query_environment_ = nullptr;
  llvm::Value *here_argument_ = nullptr;
  llvm::Value *here_destination_ = nullptr;
  struct SeenReturn {
    llvm::Value *present{}, *value{};
  } suspended_return_;
  llvm::Function *function_ = nullptr;
  llvm::BasicBlock *finish_ = nullptr;
  // Live slots and suspended values share the complete pending state. Native
  // call outcomes carry a protected destination independently of local return and failure.
  template <typename T> struct CompletionState {
    T value{}, category{}, site{}, fatal{}, requested{}, available{}, cause{}, kind{}, has_return{}, transfer{};
    auto fields() const { return std::array{value, category, site, fatal, requested, available, cause, kind, has_return, transfer}; }
    template <typename Transform> auto transform(const Transform &apply) const {
      return CompletionState<decltype(apply(value))>{apply(value), apply(category), apply(site), apply(fatal),
          apply(requested), apply(available), apply(cause), apply(kind), apply(has_return), transfer ? apply(transfer) : nullptr};
    }
  };
  CompletionState<llvm::AllocaInst *> completion_;
  std::vector<Location> sites_;
  enum class Projection { value, target, slot, receiver };
  static bool carries_value(Projection projection) { return projection == Projection::value || projection == Projection::receiver; }
  struct Deferred {
    const Expression *action;
    llvm::StructType *layout;
    std::set<size_t> captures;
  };
  struct Control {
    Control *parent;
    llvm::BasicBlock *exit;
  };
  Control *control_ = nullptr;
  struct Lifetime { std::vector<llvm::AllocaInst *> values; };
  Lifetime *temporaries_ = nullptr;
  std::map<llvm::Value *, llvm::AllocaInst *> value_owners_;
  struct Scope {
    Scope *parent;
    llvm::BasicBlock *cleanup;
    llvm::AllocaInst *head;
    std::vector<Deferred> deferred;
    Lifetime values;
  };
  llvm::StructType *deferred_header_ = llvm::StructType::get(context_, {b_.getPtrTy(), b_.getInt64Ty()});
  Scope *scope_ = nullptr;
  struct Choice {
    std::vector<Type> inputs;
    const CheckedOperation *operation;
    std::vector<const CheckedInstance *> contexts = {};
  };

  llvm::ConstantInt *integer(std::uint64_t value) { return b_.getInt64(value); }
  llvm::Constant *null_pointer() { return llvm::ConstantPointerNull::get(b_.getPtrTy()); }
  llvm::Value *empty_scalar() { return llvm::Constant::getNullValue(scalar_); }
  llvm::BasicBlock *block(const std::string &name) { return llvm::BasicBlock::Create(context_, name, function_); }
  bool reachable() const { return !b_.GetInsertBlock()->hasTerminator(); }
  llvm::AllocaInst *slot(llvm::Type *type, const std::string &name) {
    llvm::IRBuilder<> entry(&function_->getEntryBlock(), function_->getEntryBlock().begin());
    return entry.CreateAlloca(type, nullptr, name);
  }
  llvm::Value *load(llvm::AllocaInst *place) { return b_.CreateLoad(place->getAllocatedType(), place); }
  llvm::Value *binding_slot(size_t id) {
    const auto &binding = bindings_.at(id);
    return binding.selected_place ? load(binding.selected_place) : binding.reference_slot;
  }
  llvm::Value *binding_address(size_t id, std::optional<size_t> place) {
    const auto &binding = bindings_.at(id);
    if (binding.selected_place) {
      auto *selected = load(binding.selected_place);
      return binding.value_binding ? selected : b_.CreateLoad(b_.getPtrTy(), selected);
    }
    if (binding.reference_slot) return b_.CreateLoad(b_.getPtrTy(), binding.reference_slot);
    if (binding.direct_place) return binding.direct_place;
    return storage_.at(*place);
  }
  llvm::Value *environment(llvm::Value *value) {
    return environments_ ? b_.CreateExtractValue(value, environment_field_) : null_pointer();
  }
  void release_value(llvm::Value *value) {
    if (!environments_) return;
    auto *image = environment(value);
    auto *release = block("image.release"), *done = block("image.released");
    b_.CreateCondBr(b_.CreateICmpEQ(image, null_pointer()), done, release);
    b_.SetInsertPoint(release);
    b_.CreateCall(runtime("cod_lang_callable_free", b_.getVoidTy(), {b_.getPtrTy()}), {image});
    b_.CreateBr(done); b_.SetInsertPoint(done);
  }
  void clear_value(llvm::AllocaInst *place) { if (environments_) release_value(load(place)); b_.CreateStore(empty_scalar(), place); }
  void release_values(const Lifetime &lifetime) {
    for (auto i = lifetime.values.rbegin(); i != lifetime.values.rend(); ++i) clear_value(*i);
  }
  void initialize_value(llvm::AllocaInst *place) {
    llvm::IRBuilder<> entry(place->getParent(), std::next(place->getIterator()));
    entry.CreateStore(empty_scalar(), place);
  }
  llvm::AllocaInst *owned_slot(Lifetime &lifetime, const std::string &name) {
    auto *place = slot(scalar_, name); initialize_value(place); lifetime.values.push_back(place); return place;
  }
  llvm::Value *copy_value(llvm::Value *value, const Location &at) {
    if (!environments_) return value;
    auto *image = environment(value);
    auto *start = b_.GetInsertBlock(), *copy = block("image.copy"), *done = block("image.copied");
    b_.CreateCondBr(b_.CreateICmpEQ(image, null_pointer()), done, copy);
    b_.SetInsertPoint(copy);
    auto *status = slot(b_.getInt64Ty(), "image.status"); b_.CreateStore(integer(0), status);
    auto *copied = b_.CreateCall(runtime("cod_lang_callable_copy", b_.getPtrTy(), {b_.getPtrTy(), b_.getPtrTy()}), {image, status});
    auto *good = block("image.copy.ready"), *bad = block("image.copy.failed");
    b_.CreateCondBr(b_.CreateICmpEQ(copied, null_pointer()), bad, good);
    b_.SetInsertPoint(bad); fail_path(ExceptionCategory::out_of_memory, at);
    b_.SetInsertPoint(good);
    auto *result = b_.CreateInsertValue(value, copied, environment_field_);
    b_.CreateBr(done); b_.SetInsertPoint(done);
    auto *joined = b_.CreatePHI(scalar_, 2); joined->addIncoming(value, start); joined->addIncoming(result, good); return joined;
  }
  llvm::Value *own_value(llvm::Value *value) {
    if (!environments_) return value;
    auto *place = owned_slot(*temporaries_, "temporary.value");
    clear_value(place); b_.CreateStore(value, place);
    auto *owned = load(place); value_owners_[owned] = place; return owned;
  }
  void take_value(llvm::Value *value) {
    if (const auto found = value_owners_.find(value); found != value_owners_.end()) b_.CreateStore(empty_scalar(), found->second);
  }
  llvm::Value *copy_temporary(llvm::Value *value, const Location &at) {
    if (value_owners_.contains(value)) take_value(value); else value = copy_value(value, at);
    return own_value(value);
  }
  void assign_value(llvm::Value *address, llvm::Value *value, const Location &at) {
    if (!environments_) { b_.CreateStore(value, address); return; }
    if (value_owners_.contains(value)) take_value(value); else value = copy_value(value, at);
    release_value(b_.CreateLoad(scalar_, address)); b_.CreateStore(value, address);
  }
  llvm::Value *convert_value(llvm::Value *value, const Type &from, const Type &to) {
    return to.kind == Type::procedure ? value : pack(convert(value, from, to), to);
  }
  llvm::Value *temporary_scope(const Location &at, llvm::Type *result_type, const std::function<llvm::Value *()> &emit) {
    if (!environments_) return emit();
    auto *outer = temporaries_;
    auto *saved = result_type == scalar_ ? owned_slot(*outer, "expression.value") : slot(result_type, "expression.address");
    Lifetime lifetime;
    Control control{control_, block("expression.cleanup")};
    control_ = &control; temporaries_ = &lifetime;
    auto *value = emit();
    if (reachable()) {
      if (result_type == scalar_) {
        if (value_owners_.contains(value)) take_value(value); else value = copy_value(value, at);
        clear_value(saved);
      }
      b_.CreateStore(value, saved); b_.CreateBr(control.exit);
    }
    b_.SetInsertPoint(control.exit); release_values(lifetime);
    control_ = control.parent; temporaries_ = outer;
    auto *done = block("expression.completed");
    b_.CreateCondBr(b_.CreateICmpEQ(load(completion_.kind), b_.getInt8(0)), done, control_->exit);
    b_.SetInsertPoint(done);
    auto *result = load(saved); if (result_type == scalar_) value_owners_[result] = saved; return result;
  }
  unsigned tag(const Type &type) {
    if (type.kind == Type::procedure) {
      const auto found = std::find(callable_types_.begin(), callable_types_.end(), type);
      const auto id = std::distance(callable_types_.begin(), found);
      if (found == callable_types_.end()) callable_types_.push_back(type);
      return 8 + exception_categories.size() + id;
    }
    switch (type.kind) {
    case Type::integer: return 1; case Type::uint8: return 2; case Type::uint32: return 3;
    case Type::uint64: return 4; case Type::boolean: return 5; case Type::float32: return 6;
    case Type::float64: return 7; case Type::void_type: return 0;
    case Type::exception: return 8 + type.identity;
    default: throw std::logic_error("non-scalar checked native type");
    }
  }
  llvm::Type *native_type(const Type &type) {
    switch (type.kind) {
    case Type::integer: case Type::uint64: case Type::procedure: return b_.getInt64Ty();
    case Type::uint8: return b_.getInt8Ty(); case Type::uint32: return b_.getInt32Ty();
    case Type::boolean: return b_.getInt1Ty(); case Type::float32: return b_.getFloatTy();
    case Type::float64: return b_.getDoubleTy();
    case Type::exception: return exception_;
    default: throw std::logic_error("non-scalar native payload");
    }
  }
  llvm::Value *pack(llvm::Value *value, const Type &type) {
    if (type.kind == Type::exception) {
      auto *result = b_.CreateInsertValue(empty_scalar(), value, 2);
      return b_.CreateInsertValue(result, b_.getInt32(tag(type)), 1);
    }
    if (floating(type)) value = b_.CreateBitCast(value, b_.getIntNTy(type == Type::float32 ? 32 : 64));
    value = b_.CreateZExtOrTrunc(value, b_.getInt64Ty());
    auto *result = b_.CreateInsertValue(empty_scalar(), value, 0);
    return b_.CreateInsertValue(result, b_.getInt32(tag(type)), 1);
  }
  llvm::Value *unpack(llvm::Value *value, const Type &type) {
    if (type.kind == Type::exception) return b_.CreateExtractValue(value, 2);
    value = b_.CreateExtractValue(value, 0);
    if (floating(type)) {
      value = b_.CreateTruncOrBitCast(value, b_.getIntNTy(type == Type::float32 ? 32 : 64));
      return b_.CreateBitCast(value, native_type(type));
    }
    return b_.CreateTruncOrBitCast(value, native_type(type));
  }
  llvm::Value *view_address(llvm::Value *value) { return b_.CreateIntToPtr(b_.CreateExtractValue(value, 0), b_.getPtrTy()); }
  llvm::Value *pack_view(llvm::Value *address, const Expression &expr, bool returned = false) {
    const auto type = [&](size_t i) { const auto &fact = contexts_[i].at(&expr); return returned ? fact.operand_type : fact.type; };
    // An untyped null has no target identity.
    const auto target_tag = [&](const Type &type) { return type.kind == Type::null_ref ? 0u : tag(type); };
    llvm::Value *identity = b_.getInt32(target_tag(type(0)));
    if (std::any_of(contexts_.begin(), contexts_.end(), [&](const auto &context) {
          const auto &fact = context.at(&expr); return (returned ? fact.operand_type : fact.type) != type(0);
        })) {
      std::optional<size_t> parameter;
      for (size_t p = 0; p < arguments_.size(); ++p) {
        bool same = true;
        for (size_t i = 0; i < contexts_.size(); ++i) same = same && contexts_[i].instance->context.at(p) == type(i);
        if (same) { parameter = p; break; }
      }
      if (parameter) identity = b_.CreateExtractValue(arguments_[*parameter], 1);
      else for (size_t i = 1; i < contexts_.size(); ++i)
        identity = b_.CreateSelect(matches_context(*contexts_[i].instance), b_.getInt32(target_tag(type(i))), identity);
    }
    auto *value = b_.CreateInsertValue(empty_scalar(), b_.CreatePtrToInt(address, b_.getInt64Ty()), 0);
    return b_.CreateInsertValue(value, identity, 1);
  }
  llvm::Value *convert(llvm::Value *value, const Type &from, const Type &to) {
    value = unpack(value, from);
    if (from == to || from.kind == Type::exception) return value;
    if (floating(from)) return b_.CreateFPExt(value, native_type(to));
    if (floating(to)) return from == Type::integer ? b_.CreateSIToFP(value, native_type(to)) : b_.CreateUIToFP(value, native_type(to));
    return b_.CreateZExt(value, native_type(to));
  }
  llvm::FunctionCallee runtime(const std::string &name, llvm::Type *result, std::vector<llvm::Type *> arguments) {
    return module_.getOrInsertFunction(name, llvm::FunctionType::get(result, arguments, false));
  }
  std::uint64_t site(const Location &at) { sites_.push_back(at); return sites_.size(); }
  llvm::Value *outcome(llvm::Value *value, llvm::Value *category, llvm::Value *where, llvm::Value *fatal,
      llvm::Value *requested = nullptr, llvm::Value *available = nullptr, llvm::Value *cause = nullptr,
      llvm::Value *transfer = nullptr) {
    llvm::Value *result = llvm::UndefValue::get(outcome_);
    result = b_.CreateInsertValue(result, value, 0);
    result = b_.CreateInsertValue(result, category, 1);
    result = b_.CreateInsertValue(result, where, 2);
    result = b_.CreateInsertValue(result, fatal, 3);
    result = b_.CreateInsertValue(result, requested ? requested : integer(0), 4);
    result = b_.CreateInsertValue(result, available ? available : integer(0), 5);
    result = b_.CreateInsertValue(result, cause ? cause : integer(0), 6);
    if (protected_outcomes_) result = b_.CreateInsertValue(result, transfer ? transfer : null_pointer(), 7);
    return result;
  }
  llvm::Value *failed(llvm::Value *result) {
    return b_.CreateOr(b_.CreateICmpNE(b_.CreateExtractValue(result, 1), integer(0)),
        b_.CreateICmpNE(b_.CreateExtractValue(result, 3), null_pointer()));
  }
  llvm::Value *abrupt_outcome(llvm::Value *result) {
    auto *failure = failed(result);
    return protected_outcomes_ ? b_.CreateOr(failure, b_.CreateIsNotNull(b_.CreateExtractValue(result, 7))) : failure;
  }
  void preserve_transfer(llvm::Value *outer) {
    if (!protected_outcomes_) return;
    auto *kind = load(completion_.kind);
    if (here_destination_) {
      // An explicit continue in the group's own cleanup completes that
      // invocation before its earlier transfer has left the group.
      auto *local_feedback = b_.CreateAnd(b_.CreateICmpEQ(kind, b_.getInt8(1)), b_.CreateICmpEQ(outer, here_destination_));
      outer = b_.CreateSelect(local_feedback, null_pointer(), outer);
    }
    auto *inner = load(completion_.transfer);
    auto *pending = b_.CreateSelect(b_.CreateIsNull(inner), outer, inner);
    b_.CreateStore(pending, completion_.transfer);
    auto *leaves = b_.CreateAnd(b_.CreateIsNotNull(pending), b_.CreateICmpNE(kind, b_.getInt8(2)));
    b_.CreateStore(b_.CreateSelect(leaves, b_.getInt8(3), kind), completion_.kind);
  }
  void save_abrupt(llvm::Value *result) {
    b_.CreateStore(b_.CreateExtractValue(result, 1), completion_.category);
    b_.CreateStore(b_.CreateExtractValue(result, 2), completion_.site);
    b_.CreateStore(b_.CreateExtractValue(result, 3), completion_.fatal);
    b_.CreateStore(b_.CreateExtractValue(result, 4), completion_.requested);
    b_.CreateStore(b_.CreateExtractValue(result, 5), completion_.available);
    b_.CreateStore(b_.CreateExtractValue(result, 6), completion_.cause);
    if (protected_outcomes_) {
      auto *incoming = b_.CreateExtractValue(result, 7);
      b_.CreateStore(b_.CreateSelect(b_.CreateIsNotNull(incoming), incoming, load(completion_.transfer)), completion_.transfer);
      b_.CreateStore(b_.CreateSelect(failed(result), b_.getInt8(2), b_.getInt8(3)), completion_.kind);
    } else b_.CreateStore(b_.getInt8(2), completion_.kind);
  }
  void save_return(llvm::Value *value, const Location &at) {
    assign_value(completion_.value, value, at);
    b_.CreateStore(integer(0), completion_.category);
    b_.CreateStore(null_pointer(), completion_.fatal);
    b_.CreateStore(b_.getInt1(!here_destination_), completion_.has_return);
    b_.CreateStore(b_.getInt8(1), completion_.kind);
  }
  llvm::Value *observed_return() {
    if (!suspended_return_.present) throw std::logic_error("return inspection has no suspended completion");
    return b_.CreateOr(suspended_return_.present, load(completion_.has_return));
  }
  llvm::Value *observed_return_value() {
    if (!suspended_return_.value) throw std::logic_error("return binding has no suspended result");
    return b_.CreateSelect(load(completion_.has_return), load(completion_.value), suspended_return_.value);
  }
  void fail_path(ExceptionCategory category, const Location &at, llvm::Value *fatal = nullptr) {
    save_abrupt(outcome(empty_scalar(), integer(exception_id(category)), integer(site(at)), fatal ? fatal : null_pointer()));
    b_.CreateBr(control_->exit);
  }
  llvm::Value *active_exception() {
    llvm::Value *value = llvm::UndefValue::get(exception_);
    unsigned index = 0;
    for (auto *place : {completion_.category, completion_.site, completion_.requested, completion_.available, completion_.cause})
      value = b_.CreateInsertValue(value, load(place), index++);
    return value;
  }
  void raise_exception(llvm::Value *value) {
    save_abrupt(outcome(empty_scalar(), b_.CreateExtractValue(value, 0), b_.CreateExtractValue(value, 1), null_pointer(),
        b_.CreateExtractValue(value, 2), b_.CreateExtractValue(value, 3), b_.CreateExtractValue(value, 4)));
    b_.CreateBr(control_->exit);
  }
  void require(llvm::Value *condition, const Location &at, const std::string &message) {
    auto *good = block("numeric.valid"), *bad = block("numeric.failure");
    b_.CreateCondBr(condition, good, bad);
    b_.SetInsertPoint(bad);
    fail_path(ExceptionCategory::system, at, b_.CreateGlobalString(location_text(at) + ": " + message));
    b_.SetInsertPoint(good);
  }
  std::vector<Choice> choices(const Expression &expr, const std::vector<const Expression *> &inputs, bool prepared = false) {
    std::vector<Choice> result;
    for (const auto &context : contexts_) {
      if (std::any_of(inputs.begin(), inputs.end(), [&](const auto *input) { return context.at(input).type == Type::never; })) continue;
      Choice choice{{}, &context.at(&expr)};
      for (size_t p = 0; p < inputs.size(); ++p)
        choice.inputs.push_back(prepared ? choice.operation->arguments.at(p).type : context.at(inputs[p]).type);
      const auto same = [&](const Choice &other) {
        return other.inputs == choice.inputs && other.operation->callee == choice.operation->callee &&
            other.operation->symbol == choice.operation->symbol &&
            other.operation->type == choice.operation->type && other.operation->operand_type == choice.operation->operand_type &&
            other.operation->arguments == choice.operation->arguments && other.operation->builtin == choice.operation->builtin &&
            other.operation->builtin_name == choice.operation->builtin_name;
      };
      auto found = std::find_if(result.begin(), result.end(), same);
      if (found == result.end()) { result.push_back(std::move(choice)); found = std::prev(result.end()); }
      if (context.instance) found->contexts.push_back(context.instance);
    }
    for (auto &choice : result)
      if (std::none_of(result.begin(), result.end(), [&](const auto &other) { return &choice != &other && choice.inputs == other.inputs; }))
        choice.contexts.clear();
    return result;
  }
  llvm::Value *matches(const std::vector<llvm::Value *> &values, const std::vector<Type> &types) {
    llvm::Value *condition = b_.getTrue();
    for (size_t i = 0; i < values.size(); ++i)
      condition = b_.CreateAnd(condition, b_.CreateICmpEQ(b_.CreateExtractValue(values[i], 1), b_.getInt32(tag(types[i]))));
    return condition;
  }
  llvm::Value *matches_context(const CheckedInstance &instance) {
    auto *condition = matches(arguments_, instance.context);
    if (query_environment_)
      condition = b_.CreateAnd(condition, b_.CreateICmpEQ(query_environment_, b_.getInt32(instance.query_environment)));
    return condition;
  }
  llvm::Value *choose(const Expression &expr, const std::vector<llvm::Value *> &inputs, const std::vector<Choice> &cases,
                      llvm::Type *result_type, const std::function<llvm::Value *(const Choice &)> &emit_choice) {
    if (cases.size() == 1) return emit_choice(cases.front());
    auto *join = block("operation.join");
    std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> incoming;
    for (const auto &choice : cases) {
      auto *selected = block("operation.selected"), *next = block("operation.next");
      auto *condition = matches(inputs, choice.inputs);
      if (!choice.contexts.empty()) {
        llvm::Value *context = b_.getFalse();
        for (const auto *checked : choice.contexts) context = b_.CreateOr(context, matches_context(*checked));
        condition = b_.CreateAnd(condition, context);
      }
      b_.CreateCondBr(condition, selected, next);
      b_.SetInsertPoint(selected);
      auto *value = emit_choice(choice);
      if (reachable()) { incoming.emplace_back(value, b_.GetInsertBlock()); b_.CreateBr(join); }
      b_.SetInsertPoint(next);
    }
    if (result_type == outcome_) {
      auto *error = outcome(empty_scalar(), integer(exception_id(ExceptionCategory::unsound_runtime_typing)), integer(site(expr.at)), null_pointer());
      incoming.emplace_back(error, b_.GetInsertBlock());
      b_.CreateBr(join);
    } else fail_path(ExceptionCategory::unsound_runtime_typing, expr.at);
    b_.SetInsertPoint(join);
    auto *result = b_.CreatePHI(result_type, incoming.size(), "operation.result");
    for (const auto &[value, predecessor] : incoming) result->addIncoming(value, predecessor);
    return result;
  }
  llvm::Value *convert_result(const Expression &expr, const Expression &input, llvm::Value *value,
      Type CheckedOperation::*destination = &CheckedOperation::operand_type) {
    auto cases = choices(expr, {&input});
    std::erase_if(cases, [&](const Choice &choice) { return choice.operation->*destination == Type::never; });
    if (std::all_of(cases.begin(), cases.end(), [&](const Choice &choice) {
          return choice.inputs.front() == choice.operation->*destination;
        })) return value;
    return choose(expr, {value}, cases, scalar_, [&](const Choice &choice) {
      if (choice.inputs.front() == Type::void_type) return empty_scalar();
      return convert_value(value, choice.inputs.front(), choice.operation->*destination);
    });
  }
  llvm::Value *binary(const Expression &expr, llvm::Value *left, llvm::Value *right, const Choice &choice) {
    const auto type = choice.operation->operand_type;
    left = convert(left, choice.inputs[0], type); right = convert(right, choice.inputs[1], type);
    const auto &op = choice.operation->builtin_name;
    const bool signed_value = type == Type::integer;
    llvm::Value *result = nullptr;
    if (floating(type)) {
      if (op == "+") result = b_.CreateFAdd(left, right);
      else if (op == "-") result = b_.CreateFSub(left, right);
      else if (op == "*") result = b_.CreateFMul(left, right);
      else if (op == "/") result = b_.CreateFDiv(left, right);
      else if (op == "==") result = b_.CreateFCmpOEQ(left, right);
      else if (op == "!=") result = b_.CreateFCmpUNE(left, right);
      else if (op == "<") result = b_.CreateFCmpOLT(left, right);
      else if (op == "<=") result = b_.CreateFCmpOLE(left, right);
      else if (op == ">") result = b_.CreateFCmpOGT(left, right);
      else if (op == ">=") result = b_.CreateFCmpOGE(left, right);
    } else if (op == "==") result = b_.CreateICmpEQ(left, right);
    else if (op == "!=") result = b_.CreateICmpNE(left, right);
    else if (op == "<") result = signed_value ? b_.CreateICmpSLT(left, right) : b_.CreateICmpULT(left, right);
    else if (op == "<=") result = signed_value ? b_.CreateICmpSLE(left, right) : b_.CreateICmpULE(left, right);
    else if (op == ">") result = signed_value ? b_.CreateICmpSGT(left, right) : b_.CreateICmpUGT(left, right);
    else if (op == ">=") result = signed_value ? b_.CreateICmpSGE(left, right) : b_.CreateICmpUGE(left, right);
    else if (op == "+" || op == "-" || op == "*") {
      const auto intrinsic = op == "+" ? (signed_value ? llvm::Intrinsic::sadd_with_overflow : llvm::Intrinsic::uadd_with_overflow) :
          op == "-" ? (signed_value ? llvm::Intrinsic::ssub_with_overflow : llvm::Intrinsic::usub_with_overflow) :
                      (signed_value ? llvm::Intrinsic::smul_with_overflow : llvm::Intrinsic::umul_with_overflow);
      auto *checked = b_.CreateIntrinsic(intrinsic, {left->getType()}, {left, right});
      const auto operation = op == "+" ? "addition" : op == "-" ? "subtraction" : "multiplication";
      require(b_.CreateNot(b_.CreateExtractValue(checked, 1)), expr.at, type_name(type) + " " + operation + " overflow");
      result = b_.CreateExtractValue(checked, 0);
    } else if (op == "/" || op == "%") {
      auto *native = llvm::cast<llvm::IntegerType>(left->getType());
      require(b_.CreateICmpNE(right, llvm::ConstantInt::get(native, 0)), expr.at, "division by zero");
      if (signed_value) {
        auto *overflow = b_.CreateAnd(b_.CreateICmpEQ(left, llvm::ConstantInt::get(context_, llvm::APInt::getSignedMinValue(native->getBitWidth()))),
            b_.CreateICmpEQ(right, llvm::ConstantInt::getSigned(native, -1)));
        if (op == "/") {
          require(b_.CreateNot(overflow), expr.at, "i64 division overflow"); result = b_.CreateSDiv(left, right);
        } else result = b_.CreateSRem(left, b_.CreateSelect(overflow, llvm::ConstantInt::get(native, 1), right));
      } else result = op == "/" ? b_.CreateUDiv(left, right) : b_.CreateURem(left, right);
    }
    if (!result) throw std::logic_error("unchecked scalar operator");
    return pack(result, choice.operation->type);
  }
  const CheckedFrame &input_frame(size_t method) const {
    return program_->instances.at(program_->methods.at(method).instances.front()).frame;
  }
  llvm::Value *owner_identity(size_t owner, const Location &at) {
    if (const auto found = owner_identities_.find(owner); found != owner_identities_.end()) return found->second;
    if (owner > null_target / 2) implementation_gap(at, "runtime identity for an owner without an input-bound actual context");
    // Admitted lexical cells have stable native storage. One activation-local
    // address names their owner; it is shared across all views and forwarded calls.
    auto *identity = slot(b_.getInt8Ty(), "owner.identity");
    owner_identities_.emplace(owner, identity);
    return identity;
  }
  llvm::Value *selected_owner(const Expression &expr, const Expression *call = nullptr,
                              std::optional<size_t> method = {}, const Type *callable_view = nullptr) {
    std::map<size_t, std::vector<size_t>> alternatives;
    for (size_t i = 0; i < contexts_.size(); ++i) {
      if (call) {
        const auto &invocation = contexts_[i].at(call);
        if (callable_view) {
          if (!invocation.callable_call || invocation.operand_type != *callable_view) continue;
        } else if (!invocation.callee || program_->instances[*invocation.callee].method != *method) continue;
      }
      const auto &fact = contexts_[i].at(&expr);
      if (fact.type == Type::never) continue;
      if (fact.owners.size() != 1) throw std::logic_error("unchecked reference owner reached native transport");
      alternatives[*fact.owners.begin()].push_back(i);
    }
    return contextual_value(expr, alternatives, "owner", [&](size_t owner) { return owner_identity(owner, expr.at); }, b_.getPtrTy());
  }
  llvm::Value *prepare_here(const CheckedOperation::SuppliedHere &body, const Location &at,
                            llvm::Value *returning = nullptr) {
    const auto &entry = here_entries_.at(body.instance);
    llvm::Value *invoke = entry.entry;
    if (body.returning) {
      const auto &alternate = here_entries_.at(body.returning->body);
      if (!returning || alternate.activation != entry.activation || alternate.entry->getFunctionType() != entry.entry->getFunctionType())
        throw std::logic_error("supplied here return state changed the activation layout");
      invoke = b_.CreateSelect(returning, alternate.entry, entry.entry);
    }
    auto *value = b_.CreateInsertValue(llvm::UndefValue::get(here_), invoke, 0);
    if (!entry.group && body.places.empty() && body.owners.empty()) return b_.CreateInsertValue(value, null_pointer(), 1);
    auto *activation = slot(entry.activation, "here.activation");
    for (size_t i = 0; i < body.places.size(); ++i) {
      const auto &place = *body.places[i];
      auto *address = emit_expression(place, Projection::target);
      b_.CreateStore(pack_view(address, place), b_.CreateStructGEP(entry.activation, activation, entry.group + i));
    }
    for (size_t i = 0; i < body.owners.size(); ++i)
      b_.CreateStore(owner_identity(body.owners[i], at), b_.CreateStructGEP(entry.activation, activation, entry.group + body.places.size() + i));
    return b_.CreateInsertValue(value, activation, 1);
  }
  llvm::Value *native_call(const Expression &expr, llvm::Function *target, size_t method, std::vector<llvm::Value *> prepared,
                           const std::vector<llvm::Value *> &captured = {}, std::optional<size_t> instance = {},
                           const Type *callable_view = nullptr, llvm::AllocaInst *here_site = nullptr) {
    const auto &layout = call_layouts_.at(method);
    const auto &inputs = input_frame(method).inputs;
    if (prepared.size() != layout.operands) throw std::logic_error("unchecked native call arity");
    for (size_t i = 0; i < prepared.size(); ++i) {
      if (inputs[i].mode == ArgumentMode::value) take_value(prepared[i]);
    }
    llvm::AllocaInst *destination = nullptr;
    const size_t offset = expr.kind == Expression::Kind::value_call ? 1 : 0;
    const auto &invocation = contexts_.front().at(&expr);
    for (const auto &[owner, input] : input_frame(method).owners) {
      if (program_->methods[method].scope_inputs) {
        std::map<size_t, std::vector<size_t>> alternatives;
        for (size_t i = 0; i < contexts_.size(); ++i) {
          const auto &call = contexts_[i].at(&expr);
          if (call.callee && program_->instances[*call.callee].method == method)
            alternatives[call.scope_owners.at(null_target - 1 - owner)].push_back(i);
        }
        prepared.push_back(contextual_value(expr, alternatives, "owner", [&](size_t actual) {
          return owner_identity(actual, expr.at);
        }, b_.getPtrTy()));
      } else {
        prepared.push_back(selected_owner(invocation.argument(expr, input + offset), &expr, method, callable_view));
      }
    }
    if (captured.size() != layout.captures) throw std::logic_error("native call lacks its lexical captures");
    if (!captured.empty()) {
      auto *values = slot(llvm::ArrayType::get(scalar_, captured.size()), "call.captures");
      for (size_t i = 0; i < captured.size(); ++i)
        b_.CreateStore(captured[i], b_.CreateConstGEP2_64(values->getAllocatedType(), values, 0, i));
      prepared.push_back(values);
    }
    if (instance) {
      for (const auto &type : program_->instances[*instance].retained_types) prepared.push_back(b_.getInt32(tag(type)));
    } else if (layout.retained) {
      if (layout.retained != retained_types_.size())
        throw std::logic_error("native call lacks its lexical type context");
      prepared.insert(prepared.end(), retained_types_.begin(), retained_types_.end());
    }
    if (layout.declaration_context) {
      if (!instance && !query_environment_) throw std::logic_error("native call lacks its declaration evidence context");
      prepared.push_back(instance ? b_.getInt32(program_->instances[*instance].query_environment) : query_environment_);
    }
    if (layout.destination) { destination = slot(scalar_, "callable.result"); prepared.push_back(destination); }
    llvm::Value *callee = target;
    if (layout.accepts_here) {
      std::map<std::optional<CheckedOperation::SuppliedHere>, std::vector<size_t>> entries;
      for (size_t i = 0; i < contexts_.size(); ++i) {
        const auto &call = contexts_[i].at(&expr);
        if (!call.callee || program_->instances[*call.callee].method != method) continue;
        if (!call.supplied_here && !call.forwards_here) throw std::logic_error("unchecked here argument reached native call");
        entries[call.supplied_here].push_back(i);
      }
      const auto argument_value = [&](const auto &supplied, llvm::Value *returning) -> llvm::Value * {
        if (supplied) return prepare_here(*supplied, expr.at, returning);
        if (!here_argument_) throw std::logic_error("no here argument to forward");
        return here_argument_;
      };
      llvm::Value *argument;
      if (std::ranges::any_of(entries, [](const auto &entry) { return entry.first && entry.first->returning; })) {
        auto *returning = observed_return();
        auto *dispatch_type = llvm::StructType::get(context_, {here_, b_.getPtrTy()});
        auto *dispatch = contextual_value(expr, entries, "here.dispatch", [&](const auto &supplied) {
          llvm::Value *receiver = target;
          if (supplied && supplied->returning) {
            const auto id = supplied->returning->receiver;
            const bool specialized = specialize_ && instance && target == specialized_.at(*instance);
            auto *alternate = specialized ? specialized_.at(id) : general_.at(program_->instances[id].method);
            if (!alternate || target->getFunctionType() != alternate->getFunctionType())
              throw std::logic_error("supplied here return state changed the receiver layout");
            receiver = b_.CreateSelect(returning, alternate, target);
          }
          auto *value = b_.CreateInsertValue(llvm::UndefValue::get(dispatch_type), argument_value(supplied, returning), 0);
          return b_.CreateInsertValue(value, receiver, 1);
        }, dispatch_type);
        argument = b_.CreateExtractValue(dispatch, 0);
        callee = b_.CreateExtractValue(dispatch, 1);
      } else argument = contextual_value(expr, entries, "here.argument", [&](const auto &supplied) {
        return argument_value(supplied, nullptr);
      }, here_);
      if (here_site) b_.CreateStore(b_.CreateExtractValue(argument, 1), here_site);
      prepared.push_back(argument);
    }
    if (!target || prepared.size() != layout.parameter_count() || target->arg_size() != layout.parameter_count())
      throw std::logic_error("native call disagrees with its parameter roles");
    llvm::Value *result = b_.CreateCall(target->getFunctionType(), callee, prepared);
    if (destination) result = b_.CreateInsertValue(result, load(destination), 0);
    return result;
  }
  llvm::Value *invoke_callable(const Expression &expr, const CheckedCallable &value,
      const std::vector<Type> &actual, std::vector<llvm::Value *> arguments, llvm::Value *callee) {
    const auto instance_id = value.target(actual);
    const auto &instance = program_->instances[instance_id];
    const auto method = instance.method;
    auto *target = specialize_ ? specialized_[instance_id] : general_[method];
    for (size_t i = 0; i < arguments.size(); ++i) if (actual[i] != instance.arguments[i]) {
      auto *original = arguments[i];
      arguments[i] = convert_value(original, actual[i], instance.arguments[i]);
      if (const auto owned = value_owners_.find(original); owned != value_owners_.end()) value_owners_[arguments[i]] = owned->second;
    }
    std::vector<std::pair<size_t, llvm::Value *>> saved;
    auto *image = environment(callee);
    const auto symbols = program_->captured_symbols(value);
    if (expr.kind == Expression::Kind::perform && !symbols.empty()) {
      size_t index = program_->methods[method].captures.size();
      auto *layout = image_layout(index + symbols.size());
      for (const auto symbol : symbols) {
        saved.emplace_back(symbol, b_.CreateLoad(scalar_, symbols_.at(symbol), "replay.previous"));
        b_.CreateStore(image_capture(layout, image, index++), symbols_.at(symbol));
      }
    }
    std::vector<llvm::Value *> captured;
    const auto count = program_->methods[method].captures.size();
    for (size_t i = 0; i < count; ++i) captured.push_back(image_capture(image_layout(count), image, i));
    auto *result = native_call(expr, target, method, std::move(arguments), captured, instance_id, &value.type);
    for (const auto &[symbol, previous] : saved) b_.CreateStore(previous, symbols_.at(symbol));
    if (!instance.result.borrowed() && *instance.result.type != *value.type.target) {
      auto *adapt = block("callable.result.prepare"), *join = block("callable.result.prepared");
      auto *previous = b_.GetInsertBlock();
      b_.CreateCondBr(abrupt_outcome(result), join, adapt);
      b_.SetInsertPoint(adapt);
      auto *converted = convert_value(b_.CreateExtractValue(result, 0), *instance.result.type, *value.type.target);
      auto *completed = b_.CreateInsertValue(result, converted, 0);
      b_.CreateBr(join); b_.SetInsertPoint(join);
      auto *joined = b_.CreatePHI(outcome_, 2);
      joined->addIncoming(result, previous); joined->addIncoming(completed, adapt);
      result = joined;
    }
    return result;
  }
  llvm::Value *invoke_value(const Expression &expr, const std::vector<llvm::Value *> &prepared) {
    std::vector<Choice> cases;
    for (const auto &context : contexts_) {
      const auto &fact = context.at(&expr);
      if (fact.type == Type::never) continue;
      Choice choice{{fact.operand_type}, &fact};
      for (size_t i = expr.kind == Expression::Kind::value_call ? 1 : 0; i < expr.children.size(); ++i)
        choice.inputs.push_back(fact.arguments.at(i).type);
      if (std::none_of(cases.begin(), cases.end(), [&](const auto &other) { return other.inputs == choice.inputs; }))
        cases.push_back(std::move(choice));
    }
    return choose(expr, prepared, cases, outcome_, [&](const Choice &choice) -> llvm::Value * {
      auto *join = block("callable.completed"), *invalid = block("callable.invalid");
      auto *select = b_.CreateSwitch(b_.CreateExtractValue(prepared.front(), 0), invalid);
      std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> incoming;
      for (size_t id = 0; id < program_->callables.size(); ++id) {
        const auto &value = program_->callables[id];
        if (value.type != choice.inputs.front()) continue;
        if (std::none_of(contexts_.begin(), contexts_.end(), [&](const auto &context) {
              const auto &fact = context.at(&expr);
              return fact.type != Type::never && fact.operand_type == value.type && (!fact.callable_choices || fact.callable_choices->contains(id));
            })) continue;
        auto *selected = block("callable.selected"); select->addCase(integer(id + 1), selected);
        b_.SetInsertPoint(selected);
        const std::vector<Type> admitted{choice.inputs.begin() + 1, choice.inputs.end()};
        std::vector<Choice> selections;
        for (const auto &[inputs, target] : value.alternatives(admitted)) selections.push_back({inputs, choice.operation});
        std::vector<llvm::Value *> arguments{prepared.begin() + 1, prepared.end()};
        for (size_t i = 0; i < admitted.size(); ++i)
          if ((value.type.argument_modes.empty() || value.type.argument_modes[i] == ArgumentMode::value) &&
              admitted[i].kind == Type::exception && !admitted[i].identity) {
            auto *category = b_.CreateExtractValue(unpack(arguments[i], admitted[i]), 0);
            auto *valid = b_.CreateAnd(b_.CreateICmpUGT(category, integer(0)),
                                      b_.CreateICmpULT(category, integer(exception_categories.size())));
            auto *identity = b_.CreateAdd(b_.CreateTrunc(category, b_.getInt32Ty()), b_.getInt32(tag(exception_type(0))));
            arguments[i] = b_.CreateInsertValue(arguments[i], b_.CreateSelect(valid, identity, b_.getInt32(0)), 1);
          }
        auto *result = choose(expr, arguments, selections, outcome_, [&](const Choice &selected) {
          return invoke_callable(expr, value, selected.inputs, arguments, prepared.front());
        });
        incoming.emplace_back(result, b_.GetInsertBlock()); b_.CreateBr(join);
      }
      b_.SetInsertPoint(invalid);
      incoming.emplace_back(outcome(empty_scalar(), integer(exception_id(ExceptionCategory::unsound_runtime_typing)), integer(site(expr.at)), null_pointer()), invalid);
      b_.CreateBr(join); b_.SetInsertPoint(join);
      auto *result = b_.CreatePHI(outcome_, incoming.size());
      for (const auto &[value, from] : incoming) result->addIncoming(value, from);
      return result;
    });
  }
  llvm::Value *eager_builtin(const Expression &expr, const std::vector<llvm::Value *> &arguments, const Choice &choice) {
    if (choice.operation->builtin == BuiltinOperation::binary) return binary(expr, arguments[0], arguments[1], choice);
    const auto type = choice.operation->operand_type;
    auto *argument = unpack(arguments.front(), type);
    if (choice.operation->builtin_name == "!") return pack(b_.CreateNot(argument), type);
    if (choice.operation->builtin_name == "+") return arguments.front();
    if (floating(type)) return pack(b_.CreateFNeg(argument), type);
    auto *zero = pack(llvm::ConstantInt::get(llvm::cast<llvm::IntegerType>(native_type(type)), 0), type);
    return binary(expr, zero, arguments.front(), Choice{{type, type}, choice.operation});
  }
  llvm::Value *swap_builtin(const std::vector<llvm::Value *> &arguments, BuiltinOperation operation) {
    auto *left = view_address(arguments[0]), *right = view_address(arguments[1]);
    llvm::Type *type = scalar_;
    if (operation == BuiltinOperation::reference_swap) type = b_.getPtrTy();
    auto *left_value = b_.CreateLoad(type, left), *right_value = b_.CreateLoad(type, right);
    b_.CreateStore(right_value, left); b_.CreateStore(left_value, right);
    return outcome(empty_scalar(), integer(0), integer(0), null_pointer());
  }
  llvm::Value *invoke(const Expression &expr, const std::vector<llvm::Value *> &arguments, llvm::AllocaInst *here_site = nullptr) {
    const auto &fact = contexts_.front().at(&expr);
    if (fact.invokes_here) {
      if (!here_argument_) throw std::logic_error("no here argument to invoke");
      auto prepared = arguments;
      std::vector<llvm::Type *> parameters(arguments.size(), scalar_);
      for (const auto input : here_owner_inputs(fact.operand_type)) {
        prepared.push_back(selected_owner(fact.argument(expr, input)));
        parameters.push_back(b_.getPtrTy());
      }
      prepared.push_back(b_.CreateExtractValue(here_argument_, 1));
      auto *entry = b_.CreateExtractValue(here_argument_, 0);
      parameters.push_back(b_.getPtrTy());
      return b_.CreateCall(llvm::FunctionType::get(outcome_, parameters, false), entry, prepared);
    }
    if (fact.callable_call) return invoke_value(expr, arguments);
    if (std::all_of(contexts_.begin(), contexts_.end(), [&](const auto &context) {
          return context.at(&expr).builtin == BuiltinOperation::reference_swap;
        })) return swap_builtin(arguments, BuiltinOperation::reference_swap);
    if (fact.builtin == BuiltinOperation::identity) {
      auto *left_owner = selected_owner(*expr.children[0]);
      auto *right_owner = selected_owner(*expr.children[1]);
      auto *equal = b_.CreateAnd(b_.CreateICmpEQ(left_owner, right_owner),
          b_.CreateICmpEQ(view_address(arguments[0]), view_address(arguments[1])));
      if (expr.text == "is not") equal = b_.CreateNot(equal);
      return outcome(pack(equal, Type::boolean), integer(0), integer(0), null_pointer());
    }
    std::vector<const Expression *> source_inputs;
    for (size_t i = 0; i < fact.argument_count(expr); ++i) source_inputs.push_back(&fact.argument(expr, i));
    const auto cases = choices(expr, source_inputs, true);
    const std::vector<llvm::Value *> inputs(arguments.begin(), arguments.begin() + source_inputs.size());
    const std::vector<llvm::Value *> captured(arguments.begin() + source_inputs.size(), arguments.end());
    if (cases.front().operation->callee && (cases.size() > 1 || !specialize_)) {
      const auto id = *cases.front().operation->callee;
      const auto &target = program_->instances[id];
      // Share the call only when its lexical and declaration context also agrees.
      if (std::all_of(cases.begin(), cases.end(), [&](const Choice &choice) {
            if (!choice.operation->callee) return false;
            const auto &other = program_->instances[*choice.operation->callee];
            return other.method == target.method && other.retained_types == target.retained_types &&
                other.query_environment == target.query_environment;
          })) return native_call(expr, general_[target.method], target.method, inputs, captured, id, nullptr, here_site);
    }
    return choose(expr, inputs, cases, outcome_, [&](const Choice &choice) -> llvm::Value * {
      if (choice.operation->callee) {
        const auto id = *choice.operation->callee;
        auto *target = specialize_ && cases.size() == 1 ? specialized_[id] : general_[program_->instances[id].method];
        return native_call(expr, target, program_->instances[id].method, inputs, captured, id, nullptr, here_site);
      }
      if (choice.operation->builtin == BuiltinOperation::unary || choice.operation->builtin == BuiltinOperation::binary)
        return outcome(eager_builtin(expr, arguments, choice), integer(0), integer(0), null_pointer());
      if (choice.operation->builtin == BuiltinOperation::bits_swap || choice.operation->builtin == BuiltinOperation::reference_swap)
        return swap_builtin(arguments, choice.operation->builtin);
      if (choice.operation->builtin == BuiltinOperation::cause_at) {
        auto *payload = unpack(arguments[0], choice.inputs[0]);
        auto *buffer = slot(exception_, "exception.cause");
        auto *index = unpack(arguments[1], Type::uint64);
        auto *found = b_.CreateCall(runtime("cod_lang_exception_cause", b_.getInt64Ty(),
            {b_.getInt64Ty(), b_.getInt64Ty(), b_.getPtrTy(), b_.getInt64Ty()}),
            {b_.CreateExtractValue(payload, 4), index, buffer, integer(bounds_checks_)});
        const auto selected = [&] {
          auto *loaded = load(buffer);
          llvm::Value *result = llvm::UndefValue::get(exception_);
          result = b_.CreateInsertValue(result, b_.CreateExtractValue(loaded, 4), 0);
          for (unsigned i = 0; i < 4; ++i) result = b_.CreateInsertValue(result, b_.CreateExtractValue(loaded, i), i + 1);
          return outcome(pack(result, exception_type(0)), integer(0), integer(0), null_pointer());
        };
        if (!bounds_checks_) return selected();
        auto *valid = block("bounds.valid"), *invalid = block("bounds.failure"), *done = block("bounds.done");
        b_.CreateCondBr(b_.CreateICmpNE(found, integer(0)), valid, invalid);
        b_.SetInsertPoint(invalid);
        auto *available = b_.CreateLoad(b_.getInt64Ty(), b_.CreateStructGEP(exception_, buffer, 0));
        auto *error = outcome(empty_scalar(), integer(exception_id(ExceptionCategory::index_out_of_bounds)),
            integer(site(expr.at)), null_pointer(), index, available);
        b_.CreateBr(done);
        b_.SetInsertPoint(valid);
        auto *value = selected();
        b_.CreateBr(done);
        b_.SetInsertPoint(done);
        auto *result = b_.CreatePHI(outcome_, 2);
        result->addIncoming(error, invalid); result->addIncoming(value, valid);
        return result;
      }
      if (choice.operation->builtin != BuiltinOperation::print) throw std::logic_error("unchecked call reached native invocation");
      const auto type = choice.inputs.front();
      auto *argument = unpack(arguments.front(), type);
      std::string suffix;
      if (type == Type::boolean) { suffix = "bool"; argument = b_.CreateZExt(argument, b_.getInt32Ty()); }
      else if (type == Type::integer) suffix = "i64";
      else if (type == Type::float32) suffix = "f32";
      else if (type == Type::float64) suffix = "f64";
      else { suffix = "u64"; argument = b_.CreateZExtOrTrunc(argument, b_.getInt64Ty()); }
      b_.CreateCall(runtime("cod_lang_print_" + suffix, b_.getVoidTy(), {argument->getType()}), {argument});
      return outcome(empty_scalar(), integer(0), integer(0), null_pointer());
    });
  }
  llvm::Value *consume_here_transfer(llvm::Value *result, llvm::Value *here_site, const Location &at) {
    auto *target = b_.CreateExtractValue(result, 7);
    auto *matched = block("here.leaves"), *ordinary = block("here.call.outcome"), *join = block("here.outcome");
    b_.CreateCondBr(b_.CreateAnd(b_.CreateIsNotNull(target), b_.CreateICmpEQ(target, here_site)), matched, ordinary);
    b_.SetInsertPoint(ordinary);
    b_.CreateBr(join);
    b_.SetInsertPoint(matched);
    auto *cancel = block("here.cleanup.failed"), *deliver = block("here.deliver");
    b_.CreateCondBr(failed(result), cancel, deliver);
    b_.SetInsertPoint(cancel);
    auto *cleanup_failure = b_.CreateInsertValue(result, null_pointer(), 7);
    b_.CreateBr(join);
    b_.SetInsertPoint(deliver);
    auto *kind = b_.CreateLoad(b_.getInt8Ty(), b_.CreateStructGEP(here_transfer_, target, 0));
    auto *payload = b_.CreateLoad(outcome_, b_.CreateStructGEP(here_transfer_, target, 1));
    auto *returning = block("here.supplier.return");
    b_.CreateCondBr(b_.CreateICmpEQ(kind, b_.getInt8(static_cast<unsigned>(HereExit::return_supplier))), returning, join);
    b_.SetInsertPoint(returning);
    save_return(b_.CreateExtractValue(payload, 0), at);
    b_.CreateBr(control_->exit);
    b_.SetInsertPoint(join);
    auto *outcome = b_.CreatePHI(outcome_, 3);
    outcome->addIncoming(result, ordinary);
    outcome->addIncoming(cleanup_failure, cancel);
    outcome->addIncoming(payload, deliver);
    return outcome;
  }
  llvm::Value *propagate(llvm::Value *result) {
    auto *bad = block("call.failed"), *good = block("call.completed");
    b_.CreateCondBr(abrupt_outcome(result), bad, good);
    b_.SetInsertPoint(bad); save_abrupt(result); b_.CreateBr(control_->exit);
    b_.SetInsertPoint(good);
    return b_.CreateExtractValue(result, 0);
  }
  void collect_deferred(const Expression &expr, Scope &scope) {
    for (const auto &child : expr.children) collect_deferred(*child, scope);
    if (expr.kind != Expression::Kind::deferred) return;
    const auto &body = *expr.children.front();
    if (body.kind != Expression::Kind::block && std::all_of(contexts_.begin(), contexts_.end(),
        [&](const auto &context) { return context.at(&body).type == Type::never; })) return;
    std::set<std::set<size_t>> layouts;
    for (const auto &context : contexts_) layouts.insert(context.at(&expr).captures);
    for (const auto &captures : layouts) {
      std::vector<llvm::Type *> fields{deferred_header_};
      fields.insert(fields.end(), body.kind == Expression::Kind::block ? captures.size() : prepared_count(body), scalar_);
      scope.deferred.push_back({&expr, llvm::StructType::get(context_, fields), captures});
    }
  }
  void complete_cleanup(const std::function<void()> &emit, CleanupBoundary boundary, bool handles_failure = false) {
    // Suspend the pending completion while cleanup executes its own control flow.
    const auto pending = completion_.transform([&](auto *place) { return load(place); });
    if (environments_) b_.CreateStore(empty_scalar(), completion_.value);
    auto *pending_failure = b_.CreateICmpEQ(pending.kind, b_.getInt8(2));
    auto *entered_failure = handles_failure ? pending_failure : b_.getFalse();
    b_.CreateStore(b_.CreateSelect(entered_failure, b_.getInt8(2), b_.getInt8(0)), completion_.kind);
    b_.CreateStore(b_.CreateSelect(entered_failure, pending.category, integer(0)), completion_.category);
    b_.CreateStore(b_.CreateSelect(entered_failure, pending.fatal, null_pointer()), completion_.fatal);
    b_.CreateStore(b_.getFalse(), completion_.has_return);
    if (protected_outcomes_) b_.CreateStore(null_pointer(), completion_.transfer);
    Control action{control_, block("cleanup.completed")};
    control_ = &action;
    const auto outer_return = suspended_return_;
    suspended_return_ = {
        outer_return.present ? b_.CreateOr(outer_return.present, pending.has_return) : pending.has_return,
        outer_return.value ? b_.CreateSelect(pending.has_return, pending.value, outer_return.value) : pending.value};
    emit();
    suspended_return_ = outer_return;
    if (reachable()) b_.CreateBr(action.exit);
    b_.SetInsertPoint(action.exit);
    control_ = action.parent;
    auto *restore = block("cleanup.preserve"), *replace = block("cleanup.replace"), *done = block("cleanup.done");
    b_.CreateCondBr(b_.CreateICmpEQ(load(completion_.kind), b_.getInt8(0)), restore, replace);
    b_.SetInsertPoint(restore);
    if (environments_) clear_value(completion_.value);
    const auto places = completion_.fields();
    const auto values = pending.fields();
    for (size_t i = 0; i < places.size(); ++i) if (places[i]) b_.CreateStore(values[i], places[i]);
    if (handles_failure) {
      auto *resumed = b_.CreateSelect(pending.has_return, b_.getInt8(1), b_.getInt8(0));
      b_.CreateStore(b_.CreateSelect(pending_failure, resumed, pending.kind), completion_.kind);
      b_.CreateStore(b_.CreateSelect(pending_failure, integer(0), pending.category), completion_.category);
      b_.CreateStore(b_.CreateSelect(pending_failure, null_pointer(), pending.fatal), completion_.fatal);
    }
    b_.CreateBr(done);
    b_.SetInsertPoint(replace);
    if (boundary == CleanupBoundary::deferred) {
      auto *retain = block("cleanup.retain.return"), *replaced = block("cleanup.replaced.return");
      b_.CreateCondBr(b_.CreateAnd(b_.CreateICmpEQ(load(completion_.kind), b_.getInt8(2)), b_.CreateNot(load(completion_.has_return))), retain, replaced);
      b_.SetInsertPoint(retain);
      if (environments_) clear_value(completion_.value);
      b_.CreateStore(pending.value, completion_.value);
      b_.CreateStore(pending.has_return, completion_.has_return);
      b_.CreateBr(done);
      b_.SetInsertPoint(replaced);
    }
    release_value(pending.value);
    b_.CreateBr(done);
    b_.SetInsertPoint(done);
    auto *transfer = pending.transfer;
    if (boundary == CleanupBoundary::ordinary_finally && here_destination_) {
      // A failure in the supplying group's own finally cancels its pending
      // outward transfer. Receiver cleanup keeps the destination for delivery.
      auto *replaced = b_.CreateAnd(b_.CreateICmpEQ(pending.kind, b_.getInt8(3)),
                                   b_.CreateICmpEQ(load(completion_.kind), b_.getInt8(2)));
      auto *local = b_.CreateICmpEQ(transfer, here_destination_);
      transfer = b_.CreateSelect(b_.CreateAnd(replaced, local), null_pointer(), transfer);
    }
    preserve_transfer(transfer);
  }
  void drain(Scope &scope) {
    b_.SetInsertPoint(scope.cleanup);
    if (scope.deferred.empty()) return;
    auto *pop = block("defer.pop"), *done = block("defer.done"), *invalid = block("defer.invalid");
    auto *record = load(scope.head);
    b_.CreateCondBr(b_.CreateICmpEQ(record, null_pointer()), done, pop);
    b_.SetInsertPoint(pop);
    auto *previous = b_.CreateLoad(b_.getPtrTy(), b_.CreateStructGEP(deferred_header_, record, 0));
    auto *action = b_.CreateLoad(b_.getInt64Ty(), b_.CreateStructGEP(deferred_header_, record, 1));
    b_.CreateStore(previous, scope.head);
    auto *dispatch = b_.CreateSwitch(action, invalid, scope.deferred.size());
    const auto release = runtime("cod_lang_defer_free", b_.getVoidTy(), {b_.getPtrTy()});
    for (size_t i = 0; i < scope.deferred.size(); ++i) {
      const auto &descriptor = scope.deferred[i];
      const auto &body = *descriptor.action->children.front();
      auto *run = block("defer.invoke");
      dispatch->addCase(integer(i), run);
      b_.SetInsertPoint(run);
      const ContextSelection selection(contexts_);
      for (const auto &context : selection.all())
        if (context.at(descriptor.action).captures == descriptor.captures &&
            (body.kind == Expression::Kind::block || context.at(&body).type != Type::never))
          contexts_.push_back(context);
      if (body.kind == Expression::Kind::block) {
        std::vector<std::pair<size_t, llvm::AllocaInst *>> bindings;
        size_t field = 1;
        for (const auto id : descriptor.captures) {
          auto *captured = slot(scalar_, "defer.caught");
          b_.CreateStore(b_.CreateLoad(scalar_, b_.CreateStructGEP(descriptor.layout, record, field++)), captured);
          bindings.emplace_back(id, storage_.at(id));
          storage_[id] = captured;
        }
        b_.CreateCall(release, {record});
        auto *pending_failure = b_.CreateICmpEQ(load(completion_.kind), b_.getInt8(2));
        complete_cleanup([&] {
          if (descriptor.action->children.size() == 1) emit_expression(body, Projection::value, true);
          else emit_finally(*descriptor.action, Projection::value, true, pending_failure);
        }, CleanupBoundary::deferred, descriptor.action->children.size() > 1);
        for (const auto &[id, place] : bindings) storage_[id] = place;
        b_.CreateBr(scope.cleanup);
        continue;
      }
      std::vector<llvm::Value *> arguments;
      for (size_t j = 0; j < prepared_count(body); ++j)
        arguments.push_back(b_.CreateLoad(scalar_, b_.CreateStructGEP(descriptor.layout, record, j + 1)));
      // Prepared values and checked borrowed addresses do not refer into their record.
      b_.CreateCall(release, {record});
      if (body.kind == Expression::Kind::transfer || body.kind == Expression::Kind::rebind) {
        if (body.kind == Expression::Kind::transfer) arguments[1] = own_value(arguments[1]);
        assign_prepared(body, arguments);
        b_.CreateBr(scope.cleanup);
        continue;
      }
      if (std::any_of(contexts_.begin(), contexts_.end(), [&](const auto &context) {
            const auto &call = context.at(&body);
            return call.destination_arguments && std::any_of(call.arguments.begin(), call.arguments.end(),
                [](const auto &argument) { return argument.binding_slot && argument.mode == ArgumentMode::borrowed_read; });
          })) {
        complete_cleanup([&] {
          check_destination_arguments(body, arguments, true);
          auto *result = invoke(body, arguments);
          release_value(b_.CreateExtractValue(result, 0));
          auto *replace = block("defer.failed"), *done = block("defer.completed");
          b_.CreateCondBr(abrupt_outcome(result), replace, done);
          b_.SetInsertPoint(replace); save_abrupt(result); b_.CreateBr(done);
          b_.SetInsertPoint(done);
        }, CleanupBoundary::deferred);
        b_.CreateBr(scope.cleanup);
        continue;
      }
      auto *result = invoke(body, arguments);
      release_value(b_.CreateExtractValue(result, 0));
      if (contexts_.front().at(&body).callable_call) release_value(arguments.front());
      if (contexts_.front().at(&body).callee)
        for (size_t i = body.children.size(); i < arguments.size(); ++i) release_value(arguments[i]);
      auto *replace = block("defer.failed");
      b_.CreateCondBr(abrupt_outcome(result), replace, scope.cleanup);
      b_.SetInsertPoint(replace); save_abrupt(result); b_.CreateBr(scope.cleanup);
    }
    b_.SetInsertPoint(invalid);
    b_.CreateCall(release, {record});
    save_abrupt(outcome(empty_scalar(), integer(exception_id(ExceptionCategory::unsound_runtime_typing)), integer(0), null_pointer()));
    b_.CreateBr(scope.cleanup);
    b_.SetInsertPoint(done);
  }
  llvm::Value *sequence(const std::vector<const Expression *> &expressions, Projection projection = Projection::value, bool discard_last = false) {
    llvm::Value *value = empty_scalar();
    for (size_t i = 0; i < expressions.size(); ++i) {
      if (!reachable()) break;
      value = emit_expression(*expressions[i], i + 1 == expressions.size() ? projection : Projection::value, i + 1 == expressions.size() ? discard_last : true);
    }
    return value;
  }
  void collect_effects(const Expression &expr, std::set<size_t> &symbols) const {
    using K = Expression::Kind;
    if (expr.kind == K::callable) return;
    if (expr.kind == K::effect || expr.kind == K::procedure_effect) {
      const auto &effect = contexts_.front().at(&expr);
      if (effect.publishes_effect) symbols.insert(*effect.symbol);
    }
    const size_t children = expr.kind == K::range_for ? 2 : expr.kind == K::fold ? 3 : expr.children.size();
    for (size_t i = 0; i < children; ++i) collect_effects(*expr.children[i], symbols);
  }
  struct SavedEffect {
    size_t symbol;
    llvm::Value *value;
    llvm::AllocaInst *owned = nullptr, *parent = nullptr;
  };
  using SavedEffects = std::vector<SavedEffect>;
  SavedEffects save_effects(const std::vector<const Expression *> &expressions) {
    std::set<size_t> symbols;
    for (const auto *expr : expressions) collect_effects(*expr, symbols);
    SavedEffects saved;
    for (const auto id : symbols) {
      SavedEffect effect{id, b_.CreateLoad(scalar_, symbols_.at(id), "effect.previous")};
      if (program_->symbols[id].type->kind == Type::procedure) {
        effect.owned = slot(scalar_, "effect.owned"); b_.CreateStore(empty_scalar(), effect.owned);
        if (const auto outer = effect_values_.find(id); outer != effect_values_.end()) effect.parent = outer->second;
        effect_values_[id] = effect.owned;
      }
      saved.push_back(effect);
    }
    return saved;
  }
  void restore_effects(const SavedEffects &saved) {
    for (const auto &effect : saved) {
      if (effect.owned) {
        clear_value(effect.owned);
        if (effect.parent) effect_values_[effect.symbol] = effect.parent;
        else effect_values_.erase(effect.symbol);
      }
      b_.CreateStore(effect.value, symbols_.at(effect.symbol));
    }
  }
  llvm::Value *finish_effect(const CheckedOperation &effect, llvm::Value *value, const Location &at) {
    if (!effect.publishes_effect) {
      // Complete the candidate image, including any required copy, but
      // release it as a temporary without publishing or saving a binding.
      if (effect.operand_type.kind == Type::procedure) {
        if (value_owners_.contains(value)) take_value(value);
        else value = copy_value(value, at);
        release_value(value);
      }
      return empty_scalar();
    }
    if (effect.operand_type.kind == Type::procedure) {
      auto *owned = effect_values_.at(*effect.symbol);
      assign_value(owned, value, at);
      value = load(owned);
    }
    b_.CreateStore(value, symbols_.at(*effect.symbol));
    return empty_scalar();
  }
  llvm::Value *iteration(const Expression &expr, bool discard) {
    const auto saved = save_effects({&expr});
    if (saved.empty()) return emit_expression(expr, Projection::value, discard);
    Control control{control_, block("iteration.restore")};
    control_ = &control;
    auto *result = slot(scalar_, "iteration.value");
    auto *value = emit_expression(expr, Projection::value, discard);
    if (reachable()) { b_.CreateStore(value, result); b_.CreateBr(control.exit); }
    b_.SetInsertPoint(control.exit);
    restore_effects(saved);
    control_ = control.parent;
    auto *continuation = block("iteration.continue");
    b_.CreateCondBr(b_.CreateICmpEQ(load(completion_.kind), b_.getInt8(0)), continuation, control_->exit);
    b_.SetInsertPoint(continuation);
    if (std::all_of(contexts_.begin(), contexts_.end(), [&](const auto &context) { return context.at(&expr).type == Type::never; })) {
      b_.CreateUnreachable(); return nullptr;
    }
    return load(result);
  }
  void call_scope(const std::vector<const Expression *> &expressions, const Location &at,
                  llvm::Value *requires_return = nullptr, const Type *feedback = nullptr) {
    auto *outer_temporaries = temporaries_;
    Scope scope{scope_, block("scope.cleanup"), slot(b_.getPtrTy(), "defer.head"), {}, {}};
    if (environments_) scope.values.values = parameter_values_;
    Control control{control_, scope.cleanup};
    control_ = &control;
    scope_ = &scope;
    temporaries_ = &scope.values;
    b_.CreateStore(null_pointer(), scope.head);
    for (const auto *expr : expressions) collect_deferred(*expr, scope);
    const auto effects = save_effects(expressions);
    const bool supplies_value = feedback && *feedback != Type::void_type;
    auto *value = sequence(expressions, Projection::value, !supplies_value);
    if (supplies_value && reachable()) {
      const auto &body = *expressions.back();
      value = choose(body, {value}, choices(body, {&body}), scalar_, [&](const Choice &choice) {
        return convert_value(value, choice.inputs.front(), *feedback);
      });
      assign_value(completion_.value, value, body.at);
    }
    if (reachable()) b_.CreateBr(scope.cleanup);
    drain(scope); release_values(scope.values); restore_effects(effects);
    scope_ = scope.parent; control_ = control.parent; temporaries_ = outer_temporaries;
    if (requires_return) {
      auto *missing = block("return.missing");
      b_.CreateCondBr(b_.CreateAnd(requires_return, b_.CreateICmpEQ(load(completion_.kind), b_.getInt8(0))), missing, finish_);
      b_.SetInsertPoint(missing);
      save_abrupt(outcome(empty_scalar(), integer(exception_id(ExceptionCategory::missing_return)), integer(site(at)), null_pointer()));
    }
    b_.CreateBr(finish_);
  }
  llvm::Value *emit_finally(const Expression &expr, Projection projection, bool discard = false, llvm::Value *skip_body = nullptr) {
    const bool read = carries_value(projection);
    Control guarded{control_, block("try.completed")};
    control_ = &guarded;
    auto *saved = read && environments_ ? owned_slot(*temporaries_, "try.value")
        : slot(read ? scalar_ : static_cast<llvm::Type *>(b_.getPtrTy()), "try.value");
    if (skip_body) {
      auto *body = block("defer.body");
      b_.CreateCondBr(skip_body, guarded.exit, body);
      b_.SetInsertPoint(body);
    }
    auto *value = emit_expression(*expr.children[0], projection, discard);
    if (reachable()) { if (read) assign_value(saved, discard ? empty_scalar() : value, expr.at); else b_.CreateStore(value, saved); b_.CreateBr(guarded.exit); }
    b_.SetInsertPoint(guarded.exit);
    control_ = guarded.parent;
    auto *handled = block("try.handled"), *dispatch = block("catch.dispatch");
    auto *catchable = b_.CreateAnd(b_.CreateICmpNE(load(completion_.category), integer(0)), b_.CreateICmpEQ(load(completion_.fatal), null_pointer()));
    b_.CreateCondBr(catchable, dispatch, handled);
    b_.SetInsertPoint(dispatch);
    const Expression *finalizer = nullptr;
    for (size_t i = 1; i < expr.children.size(); ++i) {
      const auto &handler = *expr.children[i];
      if (handler.kind == Expression::Kind::block) { finalizer = &handler; break; }
      const auto &fact = contexts_.front().at(&handler);
      auto *selected = block("catch.selected"), *next = block("catch.next");
      auto *caught = choose(handler, {}, choices(handler, {}), scalar_, [&](const Choice &choice) -> llvm::Value * {
        const auto &type = choice.operation->type;
        auto *value = pack(active_exception(), type);
        return type.identity ? b_.CreateSelect(b_.CreateICmpEQ(load(completion_.category), integer(type.identity)), value, empty_scalar()) : value;
      });
      b_.CreateCondBr(b_.CreateICmpNE(b_.CreateExtractValue(caught, 1), b_.getInt32(0)), selected, next);
      b_.SetInsertPoint(selected);
      b_.CreateStore(caught, storage_.at(*fact.place));
      auto *pending_transfer = protected_outcomes_ ? load(completion_.transfer) : nullptr;
      if (protected_outcomes_) b_.CreateStore(null_pointer(), completion_.transfer);
      b_.CreateStore(b_.getInt8(0), completion_.kind); b_.CreateStore(integer(0), completion_.category); b_.CreateStore(null_pointer(), completion_.fatal);
      b_.CreateStore(b_.getFalse(), completion_.has_return);
      clear_value(completion_.value);
      Control catching{control_, protected_outcomes_ ? block("catch.completed") : handled};
      control_ = &catching;
      auto *result = emit_expression(*handler.children.front(), projection, discard);
      if (reachable()) { if (read) assign_value(saved, discard ? empty_scalar() : result, handler.at); else b_.CreateStore(result, saved); b_.CreateBr(catching.exit); }
      if (protected_outcomes_) {
        b_.SetInsertPoint(catching.exit);
        preserve_transfer(pending_transfer);
        b_.CreateBr(handled);
      }
      control_ = catching.parent;
      b_.SetInsertPoint(next);
    }
    b_.CreateBr(handled);
    b_.SetInsertPoint(handled);
    if (finalizer) complete_cleanup([&] { emit_expression(*finalizer); },
        expr.kind == Expression::Kind::deferred ? CleanupBoundary::deferred : CleanupBoundary::ordinary_finally);
    auto *done = block("try.continue");
    b_.CreateCondBr(b_.CreateICmpEQ(load(completion_.kind), b_.getInt8(0)), done, control_->exit);
    b_.SetInsertPoint(done);
    if (read && discard) return empty_scalar();
    if (contexts_.front().at(&expr).type == Type::never) { b_.CreateUnreachable(); return nullptr; }
    auto *result = load(saved);
    if (environments_ && read) value_owners_[result] = saved;
    return result;
  }
  llvm::Value *checked_address(llvm::Value *address, bool nullable, const Location &at) {
    if (!nullable) return address;
    auto *present = block("reference.present"), *absent = block("reference.absent");
    b_.CreateCondBr(b_.CreateICmpEQ(address, null_pointer()), absent, present);
    b_.SetInsertPoint(absent); fail_path(ExceptionCategory::null_pointer, at);
    b_.SetInsertPoint(present);
    return address;
  }
  llvm::Value *checked_address(llvm::Value *address, const Expression &expr, const Location &at, bool callable = false) {
    return checked_address(address, std::any_of(contexts_.begin(), contexts_.end(), [&](const auto &context) {
      return callable ? context.at(&expr).callable_nullable : context.at(&expr).targets.contains(null_target);
    }), at);
  }
  size_t prepared_count(const Expression &expr) const {
    const auto &fact = contexts_.front().at(&expr);
    return fact.argument_count(expr) + (expr.kind != Expression::Kind::value_call && fact.callable_call ? 1 : 0)
        + (fact.callee ? fact.callable_captures.size() : 0);
  }
  static bool inert_argument(const Expression &expr) {
    using K = Expression::Kind;
    return expr.kind == K::name || expr.kind == K::integer || expr.kind == K::real || expr.kind == K::boolean || expr.kind == K::null_ref;
  }
  llvm::Value *prepare_value(const Expression &expr, size_t index, llvm::Value *value) {
    const auto &child = contexts_.front().at(&expr).argument(expr, index);
    const auto cases = choices(expr, {&child});
    if (std::all_of(cases.begin(), cases.end(), [&](const Choice &choice) {
          return choice.inputs.front() == choice.operation->arguments.at(index).type;
        })) return value;
    auto *prepared = choose(child, {value}, cases, scalar_, [&](const Choice &choice) {
      return convert_value(value, choice.inputs.front(), choice.operation->arguments.at(index).type);
    });
    // Type adaptation preserves the ownership of an independent prepared value.
    if (const auto found = value_owners_.find(value); found != value_owners_.end()) value_owners_[prepared] = found->second;
    return prepared;
  }
  template <typename Key, typename Emit>
  llvm::Value *contextual_value(const Expression &expr, const std::map<Key, std::vector<size_t>> &alternatives,
                                 const std::string &label, const Emit &emit, llvm::Type *result_type = nullptr) {
    if (alternatives.size() == 1) return emit(alternatives.begin()->first);
    if (!result_type) result_type = scalar_;
    auto *join = block(label + ".prepared");
    auto *saved = environments_ && result_type == scalar_ ? owned_slot(*temporaries_, label + ".value") : nullptr;
    std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> incoming;
    for (const auto &[key, indices] : alternatives) {
      auto *selected = block(label + ".prepare"), *next = block(label + ".next");
      llvm::Value *condition = b_.getFalse();
      const ContextSelection selection(contexts_);
      for (const auto i : indices) {
        const auto &context = selection.all().at(i);
        condition = b_.CreateOr(condition, matches_context(*context.instance));
        contexts_.push_back(context);
      }
      b_.CreateCondBr(condition, selected, next);
      b_.SetInsertPoint(selected);
      auto *value = emit(key);
      if (reachable()) {
        if (saved) assign_value(saved, value, expr.at);
        incoming.emplace_back(value, b_.GetInsertBlock()); b_.CreateBr(join);
      }
      b_.SetInsertPoint(next);
    }
    fail_path(ExceptionCategory::unsound_runtime_typing, expr.at);
    b_.SetInsertPoint(join);
    if (incoming.empty()) { b_.CreateUnreachable(); return nullptr; }
    if (saved) { auto *value = load(saved); value_owners_[value] = saved; return value; }
    auto *value = b_.CreatePHI(result_type, incoming.size(), label + ".value");
    for (const auto &[prepared, from] : incoming) value->addIncoming(prepared, from);
    return value;
  }
  llvm::Value *prepare_argument(const Expression &expr, size_t index, bool copy) {
    const auto &child = contexts_.front().at(&expr).argument(expr, index);
    const auto prepare = [&](const std::pair<ArgumentMode, bool> &plan) -> llvm::Value * {
      const auto [mode, binding_slot] = plan;
      auto *value = emit_expression(child, binding_slot ? Projection::slot : mode == ArgumentMode::value ? Projection::value : Projection::target);
      if (!reachable()) return nullptr;
      if (mode != ArgumentMode::value) {
        if ((mode == ArgumentMode::borrowed_read && !contexts_.front().at(&expr).destination_arguments) ||
            (expr.kind == Expression::Kind::transfer && index == 0))
          value = checked_address(value, child, child.at);
        return pack_view(value, child);
      }
      value = prepare_value(expr, index, value);
      return copy ? copy_temporary(value, child.at) : value;
    };
    std::map<std::pair<ArgumentMode, bool>, std::vector<size_t>> alternatives;
    for (size_t i = 0; i < contexts_.size(); ++i) {
      const auto &argument = contexts_[i].at(&expr).arguments.at(index);
      alternatives[{argument.mode, argument.binding_slot}].push_back(i);
    }
    return contextual_value(child, alternatives, "argument", prepare);
  }
  std::vector<llvm::Value *> prepare_arguments(const Expression &expr, bool deferred = false) {
    std::vector<llvm::Value *> arguments;
    const auto &fact = contexts_.front().at(&expr);
    const size_t offset = expr.kind == Expression::Kind::value_call && fact.callable_call ? 1 : 0;
    bool protect = deferred;
    for (size_t i = offset; !protect && i < fact.argument_count(expr); ++i)
      protect = !inert_argument(fact.argument(expr, i));
    std::vector<llvm::Value *> captured;
    if (fact.callee) for (size_t i = 0; i < fact.callable_captures.size(); ++i) {
      std::vector<Choice> cases;
      for (const auto &context : contexts_)
        cases.push_back({{}, &context.at(&expr).callable_captures.at(i), context.instance
            ? std::vector<const CheckedInstance *>{context.instance} : std::vector<const CheckedInstance *>{}});
      auto *value = choose(expr, {}, cases, scalar_, [&](const Choice &choice) { return captured_value(*choice.operation, expr.at); });
      captured.push_back(protect ? copy_temporary(value, expr.at) : value);
    }
    if (expr.kind != Expression::Kind::value_call && fact.callable_call) {
      auto *address = fact.symbol ? static_cast<llvm::Value *>(symbols_.at(*fact.symbol))
          : checked_address(binding_address(*fact.callable_binding, fact.callable_place), expr, expr.at, true);
      auto *value = b_.CreateLoad(scalar_, address);
      arguments.push_back(protect ? copy_temporary(value, expr.at) : value);
    }

    const auto first = arguments.size();
    arguments.resize(first + fact.argument_count(expr));
    for (size_t written = 0; written < fact.argument_count(expr); ++written) {
      const auto i = fact.argument_evaluation_order.empty() ? written : fact.argument_evaluation_order.at(written);
      auto *value = prepare_argument(expr, i, !offset || i || protect);
      if (!reachable()) return {};
      arguments[first + i] = value;
    }
    check_destination_arguments(expr, arguments);
    arguments.insert(arguments.end(), captured.begin(), captured.end());
    return arguments;
  }
  void check_destination_arguments(const Expression &expr, const std::vector<llvm::Value *> &arguments,
                                   bool deferred = false) {
    if (contexts_.front().at(&expr).destination_arguments) for (size_t i = 0; i < expr.children.size(); ++i) {
      std::map<bool, std::vector<size_t>> alternatives;
      for (size_t j = 0; j < contexts_.size(); ++j)
        alternatives[contexts_[j].at(&expr).arguments.at(i).mode == ArgumentMode::borrowed_read].push_back(j);
      contextual_value(*expr.children[i], alternatives, "destination.check", [&](bool read) {
        if (read) {
          auto *address = view_address(arguments[i]);
          if (contexts_.front().at(&expr).arguments[i].binding_slot) {
            address = b_.CreateLoad(b_.getPtrTy(), address);
            checked_address(address, deferred || std::any_of(contexts_.begin(), contexts_.end(), [&](const auto &context) {
              return context.at(&expr).nullable_arguments.contains(i);
            }), expr.at);
          } else checked_address(address, *expr.children[i], expr.at);
        }
        return b_.getTrue();
      }, b_.getInt1Ty());
    }
  }
  llvm::Value *assign_prepared(const Expression &expr, const std::vector<llvm::Value *> &arguments) {
    auto *destination = view_address(arguments[0]);
    if (expr.kind == Expression::Kind::transfer) {
      assign_value(destination, arguments[1], expr.at);
      return destination;
    }
    auto *target = view_address(arguments[1]);
    b_.CreateStore(target, destination);
    return target;
  }
  llvm::Value *captured_value(const CheckedOperation &fact, const Location &at) {
    if (fact.callable) return own_value(materialize(fact, fact.type, at));
    auto *address = binding_address(*fact.binding, fact.place);
    return b_.CreateLoad(scalar_, address);
  }
  llvm::StructType *image_layout(size_t captures) {
    auto fields = std::vector<llvm::Type *>{b_.getInt64Ty()}; fields.insert(fields.end(), captures, scalar_);
    return llvm::StructType::get(context_, fields);
  }
  llvm::Value *image_capture(llvm::StructType *layout, llvm::Value *image, size_t index) {
    auto *value = b_.CreateLoad(scalar_, b_.CreateStructGEP(layout, image, index + 1));
    auto *offset = b_.CreatePtrToInt(environment(value), b_.getInt64Ty());
    auto *nested = b_.CreateGEP(b_.getInt8Ty(), image, offset);
    auto *absolute = b_.CreateSelect(b_.CreateICmpEQ(offset, integer(0)), null_pointer(), nested);
    return b_.CreateInsertValue(value, absolute, environment_field_);
  }
  llvm::Value *materialize(const CheckedOperation &fact, const Type &type, const Location &at) {
    auto *value = temporary_scope(at, scalar_, [&] { return own_value(create_callable_image(fact, type, at)); });
    // Return a fresh image after releasing every temporary child image.
    take_value(value);
    return value;
  }
  llvm::Value *create_callable_image(const CheckedOperation &fact, const Type &type, const Location &at) {
    auto *value = pack(integer(*fact.callable + 1), type);
    const auto symbols = program_->captured_symbols(program_->callables[*fact.callable]);
    const auto count = fact.callable_captures.size() + symbols.size();
    if (!count) return value;
    auto *layout = image_layout(count);
    auto *children = slot(llvm::ArrayType::get(b_.getPtrTy(), count), "image.children");
    auto *offsets = slot(llvm::ArrayType::get(b_.getInt64Ty(), count), "image.offsets");
    std::vector<llvm::Value *> captures;
    for (const auto &capture : fact.callable_captures) captures.push_back(captured_value(capture, at));
    for (const auto symbol : symbols) captures.push_back(b_.CreateLoad(scalar_, symbols_.at(symbol), "effect.capture"));
    const auto &data = module_.getDataLayout();
    for (size_t i = 0; i < count; ++i) {
      auto *captured = captures[i];
      b_.CreateStore(environment(captured), b_.CreateConstGEP2_64(children->getAllocatedType(), children, 0, i));
      const auto offset = data.getStructLayout(layout)->getElementOffset(i + 1) + data.getStructLayout(scalar_)->getElementOffset(environment_field_);
      b_.CreateStore(integer(offset), b_.CreateConstGEP2_64(offsets->getAllocatedType(), offsets, 0, i));
    }
    auto *status = slot(b_.getInt64Ty(), "image.status"); b_.CreateStore(integer(0), status);
    auto *image = b_.CreateCall(runtime("cod_lang_callable_create", b_.getPtrTy(),
        {b_.getInt64Ty(), b_.getInt64Ty(), b_.getPtrTy(), b_.getPtrTy(), b_.getPtrTy()}),
        {integer(data.getTypeAllocSize(layout).getFixedValue()), integer(count), children, offsets, status});
    auto *ready = block("image.created"), *failed = block("image.creation.failed");
    b_.CreateCondBr(b_.CreateICmpEQ(image, null_pointer()), failed, ready);
    b_.SetInsertPoint(failed); fail_path(ExceptionCategory::out_of_memory, at);
    b_.SetInsertPoint(ready);
    for (size_t i = 0; i < count; ++i) {
      auto *address = b_.CreateStructGEP(layout, image, i + 1);
      auto *relative = b_.CreateLoad(b_.getPtrTy(), b_.CreateStructGEP(scalar_, address, environment_field_));
      b_.CreateStore(b_.CreateInsertValue(captures[i], relative, environment_field_), address);
    }
    return b_.CreateInsertValue(value, image, environment_field_);
  }
  llvm::Value *declare_local(const Expression &declaration) {
    std::map<std::pair<bool, bool>, std::vector<size_t>> alternatives;
    for (size_t i = 0; i < contexts_.size(); ++i) {
      const auto &binding = contexts_[i].at(&declaration);
      alternatives[{binding.view, binding.mixed_view}].push_back(i);
    }
    return contextual_value(declaration, alternatives, "storage", [&](auto mode) {
      const auto [borrowed, mixed] = mode;
      const auto &binding = contexts_.front().at(&declaration);
      llvm::Value *address = binding.place ? static_cast<llvm::Value *>(storage_.at(*binding.place)) : null_pointer();
      if (environments_ && binding.place) scope_->values.values.push_back(storage_.at(*binding.place));
      if (mixed) {
        auto *value = emit_expression(*declaration.children.front(), Projection::receiver);
        if (!reachable()) return static_cast<llvm::Value *>(nullptr);
        auto *existing = b_.GetInsertBlock(), *construct = block("storage.construct"), *ready = block("storage.ready");
        auto *target = view_address(value);
        b_.CreateCondBr(b_.CreateICmpEQ(b_.CreateExtractValue(value, 1), b_.getInt32(0)), ready, construct);
        b_.SetInsertPoint(construct);
        assign_value(address, value, declaration.at);
        auto *constructed = b_.GetInsertBlock(); b_.CreateBr(ready);
        b_.SetInsertPoint(ready);
        auto *selected = b_.CreatePHI(b_.getPtrTy(), 2, "storage.target");
        selected->addIncoming(target, existing); selected->addIncoming(address, constructed);
        address = selected;
      } else if (borrowed) {
        address = emit_expression(*declaration.children.front(), Projection::target);
        if (!reachable()) return static_cast<llvm::Value *>(nullptr);
      } else if (binding.initializes) {
        const auto *initializer = declaration.children.front().get();
        if (initializer->kind == Expression::Kind::make_local)
          initializer = initializer->children[initializer->annotation ? 0 : 1].get();
        auto *value = emit_expression(*initializer);
        if (!reachable()) return static_cast<llvm::Value *>(nullptr);
        value = convert_result(declaration, *initializer, value);
        assign_value(address, value, declaration.at);
      }
      if (auto *slot = bindings_.at(*binding.binding).reference_slot) b_.CreateStore(address, slot);
      return empty_scalar();
    });
  }
  llvm::Value *emit_expression(const Expression &expr, Projection projection = Projection::value, bool discard = false) {
    const bool abrupt = std::all_of(contexts_.begin(), contexts_.end(), [&](const auto &context) { return context.at(&expr).type == Type::never; });
    // An abrupt mixed branch has no target/slot result. Retain its intermediate
    // value or view through cleanup without forcing the parent's result shape.
    if (!carries_value(projection) && abrupt &&
        std::any_of(contexts_.begin(), contexts_.end(), [&](const auto &context) { return context.at(&expr).mixed_view; }))
      projection = Projection::receiver;
    llvm::Value *value;
    if (projection == Projection::receiver) {
      std::map<unsigned, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < contexts_.size(); ++i) {
        const auto &fact = contexts_[i].at(&expr);
        alternatives[fact.mixed_view ? 2u : fact.view ? 1u : 0u].push_back(i);
      }
      value = contextual_value(expr, alternatives, "receiver", [&](unsigned kind) -> llvm::Value * {
        if (!kind) return emit_expression(expr, Projection::value, discard);
        if (kind == 2) return emit_checked_expression(expr, projection, discard);
        auto *target = emit_expression(expr, Projection::target, discard);
        if (!reachable()) return nullptr;
        // Receiver-only target encoding: no value type or owned payload. Valid
        // materialized values have nonzero tags and keep normal payload cleanup.
        return b_.CreateInsertValue(empty_scalar(), b_.CreatePtrToInt(target, b_.getInt64Ty()), 0);
      });
    } else value = emit_checked_expression(expr, projection, discard);
    if (abrupt) {
      if (reachable()) b_.CreateUnreachable();
      return nullptr;
    }
    return value;
  }
  static Projection result_source(const CheckedOperation &fact) {
    if (!fact.view || fact.type == Type::never) return Projection::value;
    return fact.mixed_view ? Projection::receiver : Projection::target;
  }
  llvm::Value *emit_projected_value(const Expression &expr, Projection source, bool discard) {
    auto *result = emit_expression(expr, source, discard);
    if (source == Projection::value || !reachable()) return result;
    if (discard) return empty_scalar();
    if (source == Projection::target) return b_.CreateLoad(scalar_, checked_address(result, expr, expr.at));
    // Carry bits or the selected target through the operand's own cleanup.
    // The receiver encoding reserves tag zero for a target address.
    auto *saved = environments_ ? owned_slot(*temporaries_, "projection.value") : nullptr;
    auto *direct = block("projection.direct"), *borrowed = block("projection.target"), *done = block("projection.ready");
    b_.CreateCondBr(b_.CreateICmpEQ(b_.CreateExtractValue(result, 1), b_.getInt32(0)), borrowed, direct);
    b_.SetInsertPoint(direct);
    if (saved) assign_value(saved, result, expr.at);
    auto *value_path = b_.GetInsertBlock(); b_.CreateBr(done);
    b_.SetInsertPoint(borrowed);
    auto *value = b_.CreateLoad(scalar_, checked_address(view_address(result), expr, expr.at));
    if (saved) assign_value(saved, value, expr.at);
    auto *target_path = b_.GetInsertBlock(); b_.CreateBr(done);
    b_.SetInsertPoint(done);
    if (saved) {
      auto *owned = load(saved); value_owners_[owned] = saved; return owned;
    }
    auto *selected = b_.CreatePHI(scalar_, 2, "projection.value");
    selected->addIncoming(result, value_path); selected->addIncoming(value, target_path);
    return selected;
  }
  llvm::Value *emit_conditional_bits(const Expression &conditional, const Expression &arm, bool discard) {
    std::map<Projection, std::vector<size_t>> alternatives;
    for (size_t i = 0; i < contexts_.size(); ++i) {
      const auto &fact = contexts_[i].at(&arm);
      const auto source = contexts_[i].at(&conditional).scalar_value_join ? result_source(fact) : Projection::value;
      alternatives[source].push_back(i);
    }
    return contextual_value(arm, alternatives, "if.arm", [&](Projection source) {
      return emit_projected_value(arm, source, discard);
    });
  }
  llvm::Value *emit_checked_expression(const Expression &expr, Projection projection, bool discard) {
    const bool read = carries_value(projection);
    using K = Expression::Kind;
    const auto &fact = contexts_.front().at(&expr);
    if (fact.argument_sources) {
      std::map<std::vector<const Expression *>, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < contexts_.size(); ++i)
        alternatives[contexts_[i].at(&expr).argument_sources.value()].push_back(i);
      if (alternatives.size() > 1)
        return contextual_value(expr, alternatives, "scope.inputs", [&](const auto &) {
          return emit_checked_expression(expr, projection, discard);
        });
    }
    if (std::any_of(contexts_.begin(), contexts_.end(), [&](const auto &context) {
          return context.at(&expr).borrowed_result != fact.borrowed_result;
        })) {
      if (!read) implementation_gap(expr.at, "context-dependent value/view results without a common value-consuming boundary");
      std::map<bool, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < contexts_.size(); ++i) alternatives[contexts_[i].at(&expr).borrowed_result].push_back(i);
      return contextual_value(expr, alternatives, "result", [&](bool) { return emit_expression(expr, projection, discard); });
    }
    if (std::all_of(contexts_.begin(), contexts_.end(), [&](const auto &context) {
          const auto builtin = context.at(&expr).builtin;
          return builtin == BuiltinOperation::unary || builtin == BuiltinOperation::binary;
        })) {
      std::vector<llvm::Value *> arguments;
      std::vector<const Expression *> inputs;
      for (const auto &child : expr.children) {
        arguments.push_back(emit_expression(*child)); inputs.push_back(child.get());
        if (!reachable()) return nullptr;
      }
      return choose(expr, arguments, choices(expr, inputs), scalar_, [&](const Choice &choice) { return eager_builtin(expr, arguments, choice); });
    }
    if (fact.callable) {
      const bool installation = expr.kind == K::effect || expr.kind == K::procedure_effect;
      const auto materialize = [&](const CheckedOperation &value) {
        return this->materialize(value, installation ? value.operand_type : value.type, expr.at);
      };
      llvm::Value *value;
      if (std::any_of(contexts_.begin(), contexts_.end(), [&](const auto &context) { return context.at(&expr).callable != fact.callable; }) || (!fact.callable_captures.empty() && contexts_.size() > 1)) {
        std::vector<Choice> cases;
        for (const auto &context : contexts_) cases.push_back({{}, &context.at(&expr), {context.instance}});
        value = choose(expr, {}, cases, scalar_, [&](const Choice &choice) { return materialize(*choice.operation); });
      } else value = materialize(fact);
      value = own_value(value);
      return installation ? finish_effect(fact, value, expr.at) : value;
    }
    switch (expr.kind) {
    case K::grouped: {
      const auto &operand = *expr.children.front();
      if (projection != Projection::value) return emit_expression(operand, projection, discard);
      std::map<std::pair<Projection, bool>, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < contexts_.size(); ++i) {
        const auto &result = contexts_[i].at(&expr);
        const auto source = result_source(contexts_[i].at(&operand));
        alternatives[{source, source == Projection::target && !result.view &&
                              result.type.kind == Type::procedure}].push_back(i);
      }
      return contextual_value(expr, alternatives, "group", [&](const auto &mode) -> llvm::Value * {
        const auto [source, snapshot_callable] = mode;
        auto *value = emit_projected_value(operand, source, discard);
        return reachable() && snapshot_callable && !discard ? copy_temporary(value, expr.at) : value;
      });
    }
    case K::typed_expression: {
      const auto &operand = *expr.children.front();
      std::map<Projection, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < contexts_.size(); ++i)
        alternatives[result_source(contexts_[i].at(&operand))].push_back(i);
      return contextual_value(expr, alternatives, "discard", [&](Projection source) -> llvm::Value * {
        emit_expression(operand, source);
        return reachable() ? empty_scalar() : nullptr;
      });
    }
    case K::callable: return empty_scalar();
    case K::null_ref: return null_pointer();
    case K::symbol: return b_.CreateLoad(scalar_, symbols_.at(*fact.symbol), "effect.current");
    case K::touch: return empty_scalar();
    case K::effect: {
      auto *value = emit_expression(*expr.children.front());
      if (!reachable()) return nullptr;
      return finish_effect(fact, convert_result(expr, *expr.children.front(), value), expr.at);
    }
    case K::integer: case K::boolean:
      return pack(llvm::ConstantInt::get(llvm::cast<llvm::IntegerType>(native_type(fact.type)), fact.integer), fact.type);
    case K::return_test: {
      std::map<bool, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < contexts_.size(); ++i) alternatives[contexts_[i].at(&expr).return_pattern_matches].push_back(i);
      return contextual_value(expr, alternatives, "return.matches", [&](bool matches) {
        return pack(matches ? observed_return() : b_.getFalse(), Type::boolean);
      });
    }
    case K::real: {
      auto text = expr.text;
      if (text.ends_with("f32") || text.ends_with("f64")) text.resize(text.size() - 3);
      if (text.find_first_of(".eE") == std::string::npos) text += ".0";
      return pack(llvm::ConstantFP::get(native_type(fact.type), text), fact.type);
    }
    case K::name: {
      if (projection == Projection::slot) return binding_slot(*fact.binding);
      std::map<std::optional<size_t>, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < contexts_.size(); ++i) alternatives[contexts_[i].at(&expr).captured_place].push_back(i);
      auto *address = contextual_value(expr, alternatives, "storage.address", [&](std::optional<size_t> captured) {
        const auto &selected = contexts_.front().at(&expr);
        auto *address = binding_address(*selected.binding, selected.place);
        if (captured && !bindings_.at(*selected.binding).value_binding)
          address = b_.CreateSelect(b_.CreateICmpEQ(address, null_pointer()), null_pointer(), storage_.at(*captured));
        return address;
      }, b_.getPtrTy());
      return read ? b_.CreateLoad(scalar_, checked_address(address, expr, expr.at)) : address;
    }
    case K::unary:
      if (fact.minimum_literal) return pack(integer(std::uint64_t{1} << 63), Type::integer);
      [[fallthrough]];
    case K::binary: {
      if (expr.text == "&&" || expr.text == "||") {
        auto *left = emit_expression(*expr.children[0]);
        if (!reachable()) return nullptr;
        auto *start = b_.GetInsertBlock(), *right_block = block("logical.right"), *join = block("logical.join");
        b_.CreateCondBr(unpack(left, Type::boolean), expr.text == "&&" ? right_block : join, expr.text == "&&" ? join : right_block);
        b_.SetInsertPoint(right_block);
        auto *right = emit_expression(*expr.children[1]);
        auto *right_end = b_.GetInsertBlock();
        const bool right_reaches = reachable();
        if (right_reaches) b_.CreateBr(join);
        b_.SetInsertPoint(join);
        auto *value = b_.CreatePHI(scalar_, right_reaches ? 2 : 1);
        value->addIncoming(left, start);
        if (right_reaches) value->addIncoming(right, right_end);
        return value;
      }
      [[fallthrough]];
    }
    case K::value_call: case K::call: case K::behave: case K::perform: {
      return temporary_scope(expr.at, !fact.borrowed_result || read ? scalar_ : static_cast<llvm::Type *>(b_.getPtrTy()), [&]() -> llvm::Value * {
        const auto arguments = prepare_arguments(expr);
        if (!reachable()) return nullptr;
        llvm::AllocaInst *here_site = nullptr;
        if (fact.supplied_here && program_->methods[program_->instances[fact.supplied_here->instance].method].here_group) {
          here_site = slot(b_.getPtrTy(), "here.call.destination");
          b_.CreateStore(null_pointer(), here_site);
        }
        auto *result = invoke(expr, arguments, here_site);
        if (here_site) result = consume_here_transfer(result, load(here_site), expr.at);
        auto *value = propagate(result);
        if (!fact.borrowed_result) return own_value(value);
        auto *address = view_address(value);
        return !read ? address : discard ? empty_scalar() : b_.CreateLoad(scalar_, checked_address(address, expr, expr.at));
      });
    }
    case K::conditional: {
      const auto &test = *expr.children[0];
      auto *condition = emit_expression(test);
      if (!reachable()) return nullptr;
      auto *yes = block("if.then"), *no = block("if.else"), *join = block("if.join");
      b_.CreateCondBr(unpack(condition, Type::boolean), yes, no);
      std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> incoming;
      for (size_t i = 1; i < 3; ++i) {
        b_.SetInsertPoint(i == 1 ? yes : no);
        if (i == 1 && test.kind == K::return_test && !test.binding.empty()) {
          std::map<std::optional<size_t>, std::vector<size_t>> destinations;
          for (size_t context = 0; context < contexts_.size(); ++context) {
            const auto &pattern = contexts_[context].at(&test);
            destinations[pattern.return_pattern_matches ? pattern.place : std::nullopt].push_back(context);
          }
          contextual_value(test, destinations, "return.binding", [&](std::optional<size_t> place) {
            if (place) b_.CreateStore(observed_return_value(), storage_.at(*place));
            return empty_scalar();
          });
        }
        const auto &arm = *expr.children[i];
        auto *value = projection == Projection::value ? emit_conditional_bits(expr, arm, discard)
                                                      : emit_expression(arm, projection, discard);
        if (reachable()) { incoming.emplace_back(value, b_.GetInsertBlock()); b_.CreateBr(join); }
      }
      b_.SetInsertPoint(join);
      if (incoming.empty()) { b_.CreateUnreachable(); return nullptr; }
      auto *result = b_.CreatePHI(read ? scalar_ : static_cast<llvm::Type *>(b_.getPtrTy()), incoming.size(), "if.value");
      for (const auto &[value, predecessor] : incoming) result->addIncoming(value, predecessor);
      return result;
    }
    case K::block: {
      std::vector<const Expression *> expressions;
      for (const auto &child : expr.children) expressions.push_back(child.get());
      return sequence(expressions, projection, discard);
    }
    case K::range_for: case K::fold: {
      const bool fold = expr.kind == K::fold;
      auto *first = emit_expression(*expr.children[0]);
      if (!reachable()) return nullptr;
      auto *last = emit_expression(*expr.children[1]);
      if (!reachable()) return nullptr;
      auto *index = storage_.at(*fact.place);
      b_.CreateStore(first, index);
      llvm::AllocaInst *accumulator = fold ? storage_.at(*fact.accumulator) : nullptr;
      if (fold) {
        auto *initial = emit_expression(*expr.children[2]);
        if (!reachable()) return nullptr;
        if (environments_) scope_->values.values.push_back(accumulator);
        assign_value(accumulator, initial, expr.at);
      }
      const auto cases = choices(expr, {expr.children[0].get(), expr.children[1].get()});
      const auto test = [&] {
        auto *current = load(index);
        return choose(expr, {current, last}, cases, b_.getInt1Ty(), [&](const Choice &choice) {
          const auto type = choice.operation->operand_type;
          auto *left = unpack(current, type), *right = unpack(last, type);
          return type == Type::integer ? b_.CreateICmpSLT(left, right) : b_.CreateICmpULT(left, right);
        });
      };
      auto *body = block("range.body"), *empty = block("range.empty"), *done = block("range.done");
      b_.CreateCondBr(test(), body, empty);
      b_.SetInsertPoint(body);
      auto *next = iteration(*expr.children[fold ? 3 : 2], !fold);
      if (reachable()) {
        if (fold) {
          next = convert_result(expr, *expr.children[3], next, &CheckedOperation::type);
          assign_value(accumulator, next, expr.at);
        }
        auto *current = load(index);
        auto *advanced = choose(expr, {current, last}, cases, scalar_, [&](const Choice &choice) {
          const auto type = choice.operation->operand_type;
          auto *value = unpack(current, type);
          // The half-open test proves current < last, so increment cannot wrap.
          return pack(b_.CreateAdd(value, llvm::ConstantInt::get(value->getType(), 1)), type);
        });
        b_.CreateStore(advanced, index);
        b_.CreateCondBr(test(), body, done);
      }
      b_.SetInsertPoint(empty);
      if (!fold && expr.children.size() == 4) iteration(*expr.children[3], true);
      if (reachable()) b_.CreateBr(done);
      b_.SetInsertPoint(done);
      if (fact.type == Type::never) { b_.CreateUnreachable(); return nullptr; }
      return fold ? load(accumulator) : empty_scalar();
    }
    case K::alias: {
      const bool selected = fact.binding.has_value();
      const bool value = !selected || bindings_.at(*fact.binding).value_binding;
      auto *address = emit_expression(*expr.children.front(), value ? Projection::target : Projection::slot, true);
      if (reachable() && selected) b_.CreateStore(address, bindings_.at(*fact.binding).selected_place);
      return reachable() ? empty_scalar() : nullptr;
    }
    case K::pointer_group: {
      for (const auto &declaration : expr.children) {
        if (declaration->kind == K::block) {
          emit_expression(*declaration, Projection::value, true);
          if (!reachable()) return nullptr;
          continue;
        }
        declare_local(*declaration);
        if (!reachable()) return nullptr;
      }
      return empty_scalar();
    }
    case K::transfer: case K::rebind: {
      if (projection == Projection::slot) throw std::logic_error("assignment result has no binding slot");
      const auto arguments = prepare_arguments(expr);
      if (!reachable()) return nullptr;
      auto *address = assign_prepared(expr, arguments);
      if (expr.kind == K::transfer) return read ? b_.CreateLoad(scalar_, address) : address;
      return !read ? address : discard ? empty_scalar() : b_.CreateLoad(scalar_, checked_address(address, expr, expr.at));
    }
    case K::return_tail: {
      std::map<bool, std::vector<size_t>> completion;
      for (size_t i = 0; i < contexts_.size(); ++i)
        completion[contexts_[i].at(&expr).type == Type::void_type].push_back(i);
      if (completion.size() > 1)
        return contextual_value(expr, completion, "tail", [&](bool) { return emit_expression(expr, projection, discard); });
      if (fact.type == Type::void_type) return emit_expression(*expr.children.front(), Projection::value, true);
      [[fallthrough]];
    }
    case K::return_value: case K::break_value: case K::continue_value: {
      llvm::Value *value = empty_scalar();
      if (!expr.children.empty()) {
        value = emit_expression(*expr.children.front(), fact.borrowed_result ? Projection::target : Projection::value);
        if (!reachable()) return nullptr;
        if (fact.borrowed_result) value = pack_view(value, expr, true);
        else value = convert_result(expr, *expr.children.front(), value);
      }
      if (fact.here_exit) {
        if (!here_destination_) throw std::logic_error("here group has no protected destination");
        auto *payload = outcome(value, integer(0), integer(0), null_pointer());
        b_.CreateStore(b_.getInt8(static_cast<unsigned>(*fact.here_exit)), b_.CreateStructGEP(here_transfer_, here_destination_, 0));
        b_.CreateStore(payload, b_.CreateStructGEP(here_transfer_, here_destination_, 1));
        b_.CreateStore(here_destination_, completion_.transfer);
        b_.CreateStore(b_.getInt1(*fact.here_exit == HereExit::return_supplier), completion_.has_return);
        b_.CreateStore(b_.getInt8(3), completion_.kind);
      } else save_return(value, expr.at);
      b_.CreateBr(control_->exit);
      return nullptr;
    }
    case K::field: {
      auto *value = emit_expression(*expr.children[0]);
      if (!reachable()) return nullptr;
      const auto &type = contexts_.front().at(expr.children[0].get()).type;
      auto *payload = unpack(value, type);
      const unsigned index = expr.text == "category" ? 0 : expr.text == "site" ? 1 : expr.text == "requested" ? 2 : expr.text == "available" ? 3 : 4;
      auto *field = b_.CreateExtractValue(payload, index);
      if (expr.text == "cause_count") field = b_.CreateCall(runtime("cod_lang_exception_count", b_.getInt64Ty(), {b_.getInt64Ty()}), {field});
      return pack(field, Type::uint64);
    }
    case K::raise: {
      llvm::Value *value = expr.children.empty() ? load(storage_.at(*fact.place)) : emit_expression(*expr.children[0]);
      if (!reachable()) return nullptr;
      value = unpack(value, exception_type(0));
      if (expr.children.size() == 2) {
        auto *cause = emit_expression(*expr.children[1]);
        if (!reachable()) return nullptr;
        cause = unpack(cause, exception_type(0));
        auto *status = slot(b_.getInt64Ty(), "exception.chain.status");
        b_.CreateStore(integer(0), status);
        std::vector<llvm::Value *> operands;
        for (unsigned i = 0; i < 5; ++i) operands.push_back(b_.CreateExtractValue(cause, i));
        operands.push_back(status);
        auto types = std::vector<llvm::Type *>(5, b_.getInt64Ty()); types.push_back(b_.getPtrTy());
        auto *handle = b_.CreateCall(runtime("cod_lang_exception_chain", b_.getInt64Ty(), types), operands);
        auto *ready = block("exception.chained"), *failed = block("exception.chain.failed");
        b_.CreateCondBr(b_.CreateICmpEQ(load(status), integer(0)), ready, failed);
        b_.SetInsertPoint(failed); fail_path(ExceptionCategory::out_of_memory, expr.at);
        b_.SetInsertPoint(ready);
        value = b_.CreateInsertValue(value, handle, 4);
      }
      raise_exception(value);
      return nullptr;
    }
    case K::try_scope: return emit_finally(expr, projection, discard);
    case K::deferred: {
      std::map<std::set<size_t>, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < contexts_.size(); ++i) alternatives[contexts_[i].at(&expr).captures].push_back(i);
      if (alternatives.size() > 1)
        return contextual_value(expr, alternatives, "defer", [&](const auto &) { return emit_expression(expr, projection, discard); });
      return temporary_scope(expr.at, scalar_, [&]() -> llvm::Value * {
      const auto &body = *expr.children.front();
      std::vector<llvm::Value *> arguments;
      if (body.kind == K::block) for (const auto id : fact.captures) arguments.push_back(load(storage_.at(id)));
      else arguments = prepare_arguments(body, true);
      if (!reachable()) return nullptr;
      auto record = std::find_if(scope_->deferred.begin(), scope_->deferred.end(), [&](const Deferred &entry) {
        return entry.action == &expr && entry.captures == fact.captures;
      });
      if (record == scope_->deferred.end()) throw std::logic_error("uncollected deferred action");
      auto *status = slot(b_.getInt64Ty(), "defer.status");
      b_.CreateStore(integer(0), status);
      const auto bytes = module_.getDataLayout().getTypeAllocSize(record->layout).getFixedValue();
      auto *prepared = b_.CreateCall(runtime("cod_lang_defer_allocate", b_.getPtrTy(), {b_.getInt64Ty(), b_.getPtrTy()}), {integer(bytes), status});
      auto *ready = block("defer.prepared"), *failed = block("defer.allocation_failed");
      b_.CreateCondBr(b_.CreateICmpEQ(prepared, null_pointer()), failed, ready);
      b_.SetInsertPoint(failed); fail_path(ExceptionCategory::out_of_memory, expr.at);
      b_.SetInsertPoint(ready);
      b_.CreateStore(load(scope_->head), b_.CreateStructGEP(deferred_header_, prepared, 0));
      b_.CreateStore(integer(std::distance(scope_->deferred.begin(), record)), b_.CreateStructGEP(deferred_header_, prepared, 1));
      for (size_t i = 0; i < arguments.size(); ++i) {
        b_.CreateStore(arguments[i], b_.CreateStructGEP(record->layout, prepared, i + 1));
        take_value(arguments[i]);
      }
      b_.CreateStore(prepared, scope_->head);
      return empty_scalar();
      });
    }
    default: throw std::logic_error("unchecked expression reached LLVM emission");
    }
  }
  llvm::Function *declare(const std::string &name, const CallLayout &layout) {
    std::vector<llvm::Type *> parameters(layout.operands, scalar_);
    parameters.insert(parameters.end(), layout.owners, b_.getPtrTy());
    if (layout.captures) parameters.push_back(b_.getPtrTy());
    parameters.insert(parameters.end(), layout.retained + layout.declaration_context, b_.getInt32Ty());
    if (layout.destination) parameters.push_back(b_.getPtrTy());
    if (layout.accepts_here) parameters.push_back(here_);
    if (layout.here_group) parameters.push_back(b_.getPtrTy());
    return llvm::Function::Create(llvm::FunctionType::get(outcome_, parameters, false),
        llvm::GlobalValue::InternalLinkage, name, module_);
  }
  void declare_here_entry(size_t id) {
    const auto &instance = program_->instances.at(id);
    const auto &method = program_->methods.at(instance.method);
    const auto &layout = call_layouts_.at(instance.method);
    const auto &inputs = *method.scope_inputs;
    const auto written = method.source->parameters.size();
    const auto places = layout.operands - written;
    const auto handed_owners = inputs.here_input_owners;
    if (layout.captures || layout.retained || layout.declaration_context || layout.destination || layout.accepts_here)
      throw std::logic_error("unexpected parameter role in supplied here body");
    // The first field is the transfer record, so its address also identifies
    // the originating call activation across every forwarding receiver.
    std::vector<llvm::Type *> fields;
    if (layout.here_group) fields.push_back(here_transfer_);
    fields.insert(fields.end(), places, scalar_);
    fields.insert(fields.end(), layout.owners - handed_owners, b_.getPtrTy());
    auto *activation = llvm::StructType::get(context_, fields);
    std::vector<llvm::Type *> parameters(written, scalar_);
    parameters.insert(parameters.end(), handed_owners, b_.getPtrTy());
    parameters.push_back(b_.getPtrTy());
    auto *entry = llvm::Function::Create(llvm::FunctionType::get(outcome_, parameters, false),
        llvm::GlobalValue::InternalLinkage, "cod.here.entry." + std::to_string(id), module_);
    here_entries_.emplace(id, HereEntry{entry, activation, layout.here_group});
    llvm::IRBuilder<> body(llvm::BasicBlock::Create(context_, "entry", entry));
    auto *borrowed = entry->getArg(written + handed_owners);
    std::vector<llvm::Value *> prepared;
    for (size_t i = 0; i < written; ++i) {
      llvm::Value *value = entry->getArg(i);
      if (inputs.here_snapshots.contains(i)) {
        auto *address = body.CreateIntToPtr(body.CreateExtractValue(value, 0), body.getPtrTy());
        value = body.CreateLoad(scalar_, address);
      }
      prepared.push_back(value);
    }
    for (size_t i = 0; i < places; ++i)
      prepared.push_back(body.CreateLoad(scalar_, body.CreateStructGEP(activation, borrowed, layout.here_group + i)));
    for (const auto &[owner, input] : instance.frame.owners) {
      const auto role = null_target - 1 - owner;
      if (role < handed_owners) prepared.push_back(entry->getArg(written + role));
      else {
        const auto field = layout.here_group + places + role - handed_owners;
        prepared.push_back(body.CreateLoad(b_.getPtrTy(), body.CreateStructGEP(activation, borrowed, field)));
      }
    }
    if (layout.here_group) prepared.push_back(borrowed);
    auto *result = body.CreateCall(general_.at(instance.method), prepared);
    if (!layout.here_group) { body.CreateRet(result); return; }
    auto *forward = llvm::BasicBlock::Create(context_, "here.failure", entry);
    auto *completed = llvm::BasicBlock::Create(context_, "here.completed", entry);
    auto *failure = body.CreateOr(body.CreateICmpNE(body.CreateExtractValue(result, 1), integer(0)),
                                  body.CreateIsNotNull(body.CreateExtractValue(result, 3)));
    body.CreateCondBr(failure, forward, completed);
    body.SetInsertPoint(completed);
    body.CreateRet(result);
    body.SetInsertPoint(forward);
    body.CreateStore(body.getInt8(static_cast<unsigned>(HereExit::failure)), body.CreateStructGEP(here_transfer_, borrowed, 0));
    auto *plain_failure = body.CreateInsertValue(result, null_pointer(), 7);
    body.CreateStore(plain_failure, body.CreateStructGEP(here_transfer_, borrowed, 1));
    body.CreateRet(body.CreateInsertValue(llvm::Constant::getNullValue(outcome_), borrowed, 7));
  }
  void begin(llvm::Function *function, const std::vector<CheckedStorage> &storage, const std::vector<CheckedBinding> &bindings,
             const CheckedFrame &frame = {}) {
    function_ = function; arguments_.clear(); retained_types_.clear(); parameter_values_.clear(); value_owners_.clear(); query_environment_ = nullptr; here_argument_ = nullptr; here_destination_ = nullptr; temporaries_ = nullptr;
    suspended_return_ = {};
    for (size_t i = 0; i < frame.inputs.size(); ++i) arguments_.push_back(function->getArg(i));
    b_.SetInsertPoint(block("entry")); finish_ = block("completion"); scope_ = nullptr; control_ = nullptr;
    owner_identities_.clear();
    size_t owner_index = frame.inputs.size();
    for (const auto &[owner, input] : frame.owners) owner_identities_[owner] = function->getArg(owner_index++);
    completion_ = {slot(scalar_, "return.value"), slot(b_.getInt64Ty(), "exception.category"),
        slot(b_.getInt64Ty(), "exception.site"), slot(b_.getPtrTy(), "numeric.failure"),
        slot(b_.getInt64Ty(), "exception.requested"), slot(b_.getInt64Ty(), "exception.available"),
        slot(b_.getInt64Ty(), "exception.cause"), slot(b_.getInt8Ty(), "completion.kind"),
        slot(b_.getInt1Ty(), "completion.has.return")};
    if (protected_outcomes_) {
      completion_.transfer = slot(b_.getPtrTy(), "completion.here");
      b_.CreateStore(null_pointer(), completion_.transfer);
    }
    b_.CreateStore(b_.getFalse(), completion_.has_return);
    b_.CreateStore(empty_scalar(), completion_.value); b_.CreateStore(integer(0), completion_.category);
    b_.CreateStore(integer(0), completion_.site); b_.CreateStore(null_pointer(), completion_.fatal); b_.CreateStore(b_.getInt8(0), completion_.kind);
    for (auto *place : {completion_.requested, completion_.available, completion_.cause}) b_.CreateStore(integer(0), place);
    storage_.clear();
    for (const auto &binding : storage) {
      auto *place = binding.borrowed ? nullptr : slot(scalar_, binding.name + ".storage");
      storage_.push_back(place);
      if (place && environments_) initialize_value(place);
    }
    bindings_.clear();
    for (const auto &binding : bindings) bindings_.push_back({binding.value_binding});
    for (size_t i = 0; i < frame.inputs.size(); ++i) if (frame.inputs[i].binding_slot)
      bindings_.at(storage.at(frame.inputs[i].place).binding).reference_slot = view_address(arguments_[i]);
    for (size_t i = 0; i < bindings.size(); ++i) {
      auto &binding = bindings_[i];
      const auto &declaration = bindings[i];
      if (!binding.reference_slot && !declaration.value_binding && !declaration.alias && declaration.selection.empty())
        binding.reference_slot = slot(b_.getPtrTy(), "reference.target");
      binding.selected_place = declaration.selection.empty() ? nullptr : slot(b_.getPtrTy(), "alias.selection");
    }
    for (size_t i = 0; i < arguments_.size(); ++i) {
      const auto &input = frame.inputs[i];
      const auto place = input.place, binding = storage.at(place).binding;
      if (input.binding_slot) continue;
      if (input.mode != ArgumentMode::value) {
        auto *address = view_address(arguments_[i]);
        auto &bound = bindings_.at(binding);
        if (bound.value_binding) bound.direct_place = address;
        else b_.CreateStore(address, bound.reference_slot);
        continue;
      }
      b_.CreateStore(arguments_[i], storage_.at(place));
      if (environments_) parameter_values_.push_back(storage_.at(place));
      if (auto *slot = bindings_.at(binding).reference_slot) b_.CreateStore(storage_.at(place), slot);
    }
  }
  void body(llvm::Function *function, const CheckedMethod &method, std::optional<size_t> instance) {
    const auto &layout = program_->instances[instance ? *instance : method.instances.front()];
    const auto prepared = checked_body(*program_, method, instance);
    const auto &frame = prepared.frame;
    const auto &call = call_layouts_.at(layout.method);
    if (function->arg_size() != call.parameter_count()) throw std::logic_error("native body disagrees with its parameter roles");
    begin(function, prepared.storage, layout.bindings, frame);
    if (call.accepts_here) here_argument_ = function->getArg(call.here_index());
    if (call.here_group) here_destination_ = function->getArg(call.here_index());
    if (method.here_group && method.here_group->supplier_return.has_value())
      suspended_return_.present = b_.getInt1(*method.here_group->supplier_return);
    contexts_.clear();
    for (const auto &context : prepared.contexts) contexts_.push_back({&context.operations(), context.instance});
    if (call.captures) {
      auto *values = function->getArg(call.captures_index());
      for (size_t c = 0; c < call.captures; ++c) {
        auto *captured = b_.CreateLoad(scalar_, b_.CreateGEP(scalar_, values, integer(c)));
        // Writable locals acquire complete values below, after the incoming
        // context is validated. Read-only observation borrows immutable data.
        if (!prepared.writable_captures.contains(c)) b_.CreateStore(captured, storage_.at(frame.captures.at(c)));
        arguments_.push_back(captured);
      }
    }
    for (size_t i = 0; i < call.retained; ++i) {
      auto *type = function->getArg(call.retained_begin() + i);
      retained_types_.push_back(type);
      arguments_.push_back(b_.CreateInsertValue(empty_scalar(), type, 1));
    }
    if (call.declaration_context)
      query_environment_ = function->getArg(call.query_index());
    llvm::Value *requires_return = nullptr;
    const auto needs_return = [&](size_t id) {
      return method.source->completion_requires_return(*program_->instances[id].result.type);
    };
    if (instance) {
      if (needs_return(*instance)) requires_return = b_.getTrue();
    }
    else {
      const bool all_return = std::ranges::all_of(method.instances, needs_return);
      if (all_return) requires_return = b_.getTrue();
      llvm::Value *admitted = b_.getFalse();
      for (const auto id : method.instances) {
        auto *context = matches_context(program_->instances[id]);
        admitted = b_.CreateOr(admitted, context);
        if (!all_return && needs_return(id))
          requires_return = requires_return ? b_.CreateOr(requires_return, context) : context;
      }
      auto *valid = block("arguments.valid"), *invalid = block("arguments.invalid");
      b_.CreateCondBr(admitted, valid, invalid);
      b_.SetInsertPoint(invalid);
      for (auto *place : parameter_values_) clear_value(place);
      b_.CreateRet(outcome(empty_scalar(), integer(exception_id(ExceptionCategory::unsound_runtime_typing)),
          integer(site(method.source->at)), null_pointer()));
      b_.SetInsertPoint(valid);
    }
    std::optional<Control> initialization;
    if (!prepared.writable_captures.empty()) {
      initialization = Control{nullptr, block("inputs.failed")};
      control_ = &*initialization;
    }
    for (const auto c : prepared.writable_captures) {
      auto *place = storage_.at(frame.captures.at(c));
      parameter_values_.push_back(place);
      auto *value = arguments_.at(call.operands + c);
      b_.CreateStore(copy_value(value, method.captures.at(c).at), place);
    }
    call_scope({method.source->body.get()}, method.source->at, requires_return,
        call.here_group ? &*layout.result.type : nullptr);
    if (initialization) {
      b_.SetInsertPoint(initialization->exit);
      for (auto i = parameter_values_.rbegin(); i != parameter_values_.rend(); ++i) clear_value(*i);
      b_.CreateBr(finish_);
    }
    control_ = nullptr;
    b_.SetInsertPoint(finish_);
    if (environments_) {
      auto *failed = block("return.discard"), *ready = block("return.ready");
      auto *kind = load(completion_.kind);
      auto *discard = protected_outcomes_ ? b_.CreateICmpUGE(kind, b_.getInt8(2)) : b_.CreateICmpEQ(kind, b_.getInt8(2));
      b_.CreateCondBr(discard, failed, ready);
      b_.SetInsertPoint(failed); clear_value(completion_.value); b_.CreateBr(ready); b_.SetInsertPoint(ready);
    }
    if (call.destination) b_.CreateStore(load(completion_.value), function->getArg(call.result_index()));
    b_.CreateRet(outcome(load(completion_.value), load(completion_.category), load(completion_.site), load(completion_.fatal),
        load(completion_.requested), load(completion_.available), load(completion_.cause),
        protected_outcomes_ ? load(completion_.transfer) : nullptr));
  }
  void emit_sites() {
    function_ = llvm::Function::Create(llvm::FunctionType::get(b_.getPtrTy(), {b_.getInt64Ty()}, false),
        llvm::GlobalValue::ExternalLinkage, "cod_lang_exception_site", module_);
    b_.SetInsertPoint(block("entry"));
    auto *unknown = block("unknown");
    auto *select = b_.CreateSwitch(function_->getArg(0), unknown, sites_.size());
    for (size_t i = 0; i < sites_.size(); ++i) {
      auto *known = block("site"); select->addCase(integer(i + 1), known); b_.SetInsertPoint(known);
      b_.CreateRet(b_.CreateGlobalString(location_text(sites_[i])));
    }
    b_.SetInsertPoint(unknown); b_.CreateRet(null_pointer());
  }

public:
  std::pair<std::string, std::string> emit(const CheckedProgram &program, bool emit_text, bool emit_bitcode, bool specialize = true, bool bounds_checks = true) {
    program_ = &program; specialize_ = specialize; bounds_checks_ = bounds_checks;
    protected_outcomes_ = std::ranges::any_of(program.methods, [](const auto &method) { return method.here_group && !method.instances.empty(); });
    if (protected_outcomes_) {
      std::vector<llvm::Type *> fields(outcome_->element_begin(), outcome_->element_end());
      fields.push_back(b_.getPtrTy());
      outcome_->setBody(fields);
      here_transfer_->setBody({b_.getInt8Ty(), outcome_});
    }
    environments_ = std::any_of(program.methods.begin(), program.methods.end(), [](const auto &method) { return !method.captures.empty(); }) ||
        std::any_of(program.callables.begin(), program.callables.end(), [&](const auto &value) { return !program.captured_symbols(value).empty(); });
    std::vector<llvm::Type *> fields{b_.getInt64Ty(), b_.getInt32Ty()};
    if (program.exception_values) fields.push_back(exception_);
    if (environments_) { environment_field_ = fields.size(); fields.push_back(b_.getPtrTy()); }
    scalar_->setBody(fields);
    std::vector<bool> emitted_methods(program.methods.size());
    std::vector<size_t> pending_methods;
    const auto retain_method = [&](size_t id) {
      if (emitted_methods[id]) return;
      emitted_methods[id] = true;
      pending_methods.push_back(id);
    };
    // Scoped profiles can be created by provisional or restricted proofs.
    // Emit them when the final operations need their executable body.
    for (size_t i = 0; i < program.methods.size(); ++i)
      if (!program.methods[i].scope_inputs && !program.methods[i].here_argument_instance && !program.methods[i].instances.empty())
        retain_method(i);
    std::set<size_t> used_symbols;
    std::set<size_t> here_instances;
    const auto collect_operations = [&](const auto &operations) {
      for (const auto &[expr, operation] : operations) {
        if (operation.callee) retain_method(program.instances[*operation.callee].method);
        if (operation.supplied_here) {
          here_instances.insert(operation.supplied_here->instance);
          retain_method(program.instances[operation.supplied_here->instance].method);
          if (const auto &returning = operation.supplied_here->returning) {
            here_instances.insert(returning->body);
            retain_method(program.instances[returning->body].method);
            retain_method(program.instances[returning->receiver].method);
          }
        }
        const bool installation = expr->kind == Expression::Kind::effect || expr->kind == Expression::Kind::procedure_effect;
        if (operation.symbol && (expr->kind == Expression::Kind::symbol || (installation && operation.publishes_effect) || operation.callable_call))
          used_symbols.insert(*operation.symbol);
      }
    };
    collect_operations(program.entry);
    for (const auto &value : program.callables) {
      for (const auto instance : value.implementations()) retain_method(program.instances[instance].method);
      const auto symbols = program.captured_symbols(value);
      used_symbols.insert(symbols.begin(), symbols.end());
    }
    for (size_t i = 0; i < pending_methods.size(); ++i) {
      const auto &method = program.methods[pending_methods[i]];
      for (const auto id : method.instances) collect_operations(program.instances[id].operations);
    }
    for (size_t i = 0; i < program.symbols.size(); ++i) {
      const auto &symbol = program.symbols[i];
      llvm::GlobalVariable *slot = nullptr;
      if (used_symbols.contains(i) && symbol.type) {
        slot = new llvm::GlobalVariable(module_, scalar_, false, llvm::GlobalValue::InternalLinkage,
            llvm::Constant::getNullValue(scalar_), "cod.symbol." + std::to_string(i));
        slot->setThreadLocalMode(llvm::GlobalVariable::GeneralDynamicTLSModel);
      }
      symbols_.push_back(slot);
    }
    for (const auto &instance : program.instances)
      if (!instance.complete) throw std::logic_error("incomplete checking obligation reached native emission");
    const llvm::Triple triple(llvm::sys::getDefaultTargetTriple());
    clang::DiagnosticOptions diagnostic_options;
    clang::IgnoringDiagConsumer consumer;
    clang::DiagnosticsEngine diagnostics(llvm::makeIntrusiveRefCnt<clang::DiagnosticIDs>(), diagnostic_options, &consumer, false);
    clang::TargetOptions options; options.Triple = triple.str();
    std::unique_ptr<clang::TargetInfo> target(clang::TargetInfo::CreateTargetInfo(diagnostics, options));
    if (!target) throw std::runtime_error("cannot obtain the native target data layout");
    module_.setTargetTriple(triple); module_.setDataLayout(target->getDataLayoutString());
    call_layouts_.resize(program.methods.size());
    for (size_t i = 0; i < program.methods.size(); ++i) {
      if (!emitted_methods[i]) continue;
      const auto &method = program.methods[i];
      const auto &frame = input_frame(i);
      const bool destination = std::any_of(method.instances.begin(), method.instances.end(), [&](size_t id) {
        const auto &result = program.instances[id].result;
        return !result.borrowed() && result.type->kind == Type::procedure;
      });
      call_layouts_[i] = {frame.inputs.size(), frame.owners.size(), method.captures.size(),
          method.retained_types.size(), method.declaration_context(), destination, method.here.has_value(), method.here_group.has_value()};
    }
    general_.resize(program.methods.size()); specialized_.resize(program.instances.size());
    for (size_t i = 0; i < program.methods.size(); ++i) {
      if (!emitted_methods[i]) continue;
      const auto &method = program.methods[i];
      const auto &layout = call_layouts_[i];
      const auto prefix = "cod.method." + std::to_string(method.definition) + "." + method.source->name +
          (i == method.definition ? std::string{} : ".interpretation." + std::to_string(i));
      if (method.instances.size() == 1 && method.source->type_parameters.empty() && method.captures.empty() &&
          method.retained_types.empty() && !method.declaration_context()) {
        general_[i] = declare(prefix + ".native", layout);
        specialized_[method.instances.front()] = general_[i];
        continue;
      }
      general_[i] = declare(prefix + ".general", layout);
      if (specialize) for (const auto id : method.instances) {
        std::string name = prefix + ".specialized";
        for (const auto &type : program.instances[id].context) name += "." + type_name(type);
        if (method.declaration_context()) name += ".query." + std::to_string(program.instances[id].query_environment);
        specialized_[id] = declare(name, layout);
      }
    }
    for (const auto instance : here_instances) declare_here_entry(instance);
    for (size_t i = 0; i < program.methods.size(); ++i) {
      if (!emitted_methods[i]) continue;
      const auto &method = program.methods[i];
      const auto first = method.instances.front();
      if (specialized_[first] == general_[i]) {
        body(general_[i], method, first);
        continue;
      }
      body(general_[i], method, {});
      if (specialize) for (const auto id : method.instances) body(specialized_[id], method, id);
    }
    auto *main = llvm::Function::Create(llvm::FunctionType::get(b_.getInt32Ty(), false), llvm::GlobalValue::ExternalLinkage, "main", module_);
    std::vector<const Expression *> initializers;
    for (const auto &initializer : program.initializers) initializers.push_back(initializer.expression);
    begin(main, program.entry_storage, program.entry_bindings); contexts_ = {{&program.entry}}; call_scope(initializers, Location{});
    b_.SetInsertPoint(finish_);
    auto *fatal = block("fatal"), *exception = block("exception"), *normal = block("normal");
    b_.CreateCondBr(b_.CreateICmpNE(load(completion_.fatal), null_pointer()), fatal, exception);
    b_.SetInsertPoint(fatal);
    b_.CreateCall(runtime("cod_lang_fail", b_.getVoidTy(), {b_.getPtrTy()}), {load(completion_.fatal)}); b_.CreateUnreachable();
    b_.SetInsertPoint(exception);
    auto *uncaught = block("uncaught");
    b_.CreateCondBr(b_.CreateICmpNE(load(completion_.category), integer(0)), uncaught, normal);
    b_.SetInsertPoint(uncaught);
    b_.CreateCall(runtime("cod_lang_uncaught", b_.getVoidTy(), std::vector<llvm::Type *>(5, b_.getInt64Ty())),
        {load(completion_.category), load(completion_.site), load(completion_.requested), load(completion_.available), load(completion_.cause)}); b_.CreateUnreachable();
    b_.SetInsertPoint(normal);
    b_.CreateCall(runtime("cod_lang_exception_finish", b_.getVoidTy(), {})); b_.CreateRet(b_.getInt32(0));
    emit_sites();
    std::string errors;
    llvm::raw_string_ostream diagnostics_out(errors);
    if (llvm::verifyModule(module_, &diagnostics_out)) throw std::logic_error("invalid emitted CoD module: " + errors);
    std::string text, bitcode;
    if (emit_text) { llvm::raw_string_ostream output(text); module_.print(output, nullptr); }
    if (emit_bitcode) { llvm::raw_string_ostream output(bitcode); llvm::WriteBitcodeToFile(module_, output); }
    return {std::move(text), std::move(bitcode)};
  }
};

} // namespace cod::language
