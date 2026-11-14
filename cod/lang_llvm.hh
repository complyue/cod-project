//===--- lang_llvm.hh - Checked scalar native emission --------------===//
#pragma once

#include "lang_checked.hh"
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
  // lexical images; they are not regional-storable bits.
  llvm::StructType *exception_ = llvm::StructType::get(context_, std::vector<llvm::Type *>(5, b_.getInt64Ty()));
  llvm::StructType *scalar_ = llvm::StructType::create(context_, {b_.getInt64Ty(), b_.getInt32Ty()}, "cod.scalar");
  llvm::StructType *outcome_ = llvm::StructType::create(context_,
      {scalar_, b_.getInt64Ty(), b_.getInt64Ty(), b_.getPtrTy(), b_.getInt64Ty(), b_.getInt64Ty(), b_.getInt64Ty()}, "cod.outcome");
  const CheckedProgram *program_ = nullptr;
  bool specialize_ = true;
  bool environments_ = false;
  unsigned environment_field_ = 0;
  llvm::Value *environment_ = nullptr;
  std::vector<Type> callable_types_;
  std::vector<llvm::Function *> general_, specialized_;
  std::vector<const std::map<const Expression *, CheckedOperation> *> facts_;
  std::vector<llvm::Value *> arguments_, retained_types_;
  std::vector<llvm::AllocaInst *> storage_, bindings_;
  std::vector<llvm::AllocaInst *> selections_;
  const std::vector<CheckedBinding> *binding_layout_ = nullptr;
  std::vector<llvm::AllocaInst *> parameter_values_;
  std::vector<const CheckedInstance *> frame_contexts_;
  llvm::Value *query_environment_ = nullptr;
  llvm::Function *function_ = nullptr;
  llvm::BasicBlock *finish_ = nullptr;
  llvm::AllocaInst *result_ = nullptr, *category_ = nullptr, *site_ = nullptr, *fatal_ = nullptr, *exit_ = nullptr;
  llvm::AllocaInst *requested_ = nullptr, *available_ = nullptr, *cause_ = nullptr;
  llvm::AllocaInst *returning_ = nullptr;
  std::vector<Location> sites_;
  enum class Projection { value, target, slot };
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
    return selections_.at(id) ? load(selections_[id]) : bindings_.at(id);
  }
  llvm::Value *binding_address(size_t id, std::optional<size_t> place) {
    if (selections_.at(id)) {
      auto *selected = load(selections_[id]);
      return binding_layout_->at(id).value_binding ? selected : b_.CreateLoad(b_.getPtrTy(), selected);
    }
    return bindings_.at(id) ? load(bindings_[id]) : storage_.at(*place);
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
    b_.CreateCondBr(b_.CreateICmpEQ(load(exit_), b_.getInt8(0)), done, control_->exit);
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
    const auto type = [&](size_t i) { const auto &fact = facts_[i]->at(&expr); return returned ? fact.operand_type : fact.type; };
    llvm::Value *identity = b_.getInt32(tag(type(0)));
    if (std::any_of(facts_.begin(), facts_.end(), [&](const auto *facts) {
          const auto &fact = facts->at(&expr); return (returned ? fact.operand_type : fact.type) != type(0);
        })) {
      std::optional<size_t> parameter;
      for (size_t p = 0; p < arguments_.size(); ++p) {
        bool same = true;
        for (size_t i = 0; i < facts_.size(); ++i) same = same && frame_contexts_[i]->context.at(p) == type(i);
        if (same) { parameter = p; break; }
      }
      if (parameter) identity = b_.CreateExtractValue(arguments_[*parameter], 1);
      else for (size_t i = 1; i < facts_.size(); ++i)
        identity = b_.CreateSelect(matches_context(*frame_contexts_[i]), b_.getInt32(tag(type(i))), identity);
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
      llvm::Value *requested = nullptr, llvm::Value *available = nullptr, llvm::Value *cause = nullptr) {
    llvm::Value *result = llvm::UndefValue::get(outcome_);
    result = b_.CreateInsertValue(result, value, 0);
    result = b_.CreateInsertValue(result, category, 1);
    result = b_.CreateInsertValue(result, where, 2);
    result = b_.CreateInsertValue(result, fatal, 3);
    result = b_.CreateInsertValue(result, requested ? requested : integer(0), 4);
    result = b_.CreateInsertValue(result, available ? available : integer(0), 5);
    return b_.CreateInsertValue(result, cause ? cause : integer(0), 6);
  }
  llvm::Value *failed(llvm::Value *result) {
    return b_.CreateOr(b_.CreateICmpNE(b_.CreateExtractValue(result, 1), integer(0)),
        b_.CreateICmpNE(b_.CreateExtractValue(result, 3), null_pointer()));
  }
  void save_failure(llvm::Value *result) {
    b_.CreateStore(b_.CreateExtractValue(result, 1), category_);
    b_.CreateStore(b_.CreateExtractValue(result, 2), site_);
    b_.CreateStore(b_.CreateExtractValue(result, 3), fatal_);
    b_.CreateStore(b_.CreateExtractValue(result, 4), requested_);
    b_.CreateStore(b_.CreateExtractValue(result, 5), available_);
    b_.CreateStore(b_.CreateExtractValue(result, 6), cause_);
    b_.CreateStore(b_.getInt8(2), exit_);
  }
  void fail_path(ExceptionCategory category, const Location &at, llvm::Value *fatal = nullptr) {
    save_failure(outcome(empty_scalar(), integer(exception_id(category)), integer(site(at)), fatal ? fatal : null_pointer()));
    b_.CreateBr(control_->exit);
  }
  llvm::Value *active_exception() {
    llvm::Value *value = llvm::UndefValue::get(exception_);
    unsigned index = 0;
    for (auto *place : {category_, site_, requested_, available_, cause_}) value = b_.CreateInsertValue(value, load(place), index++);
    return value;
  }
  void raise_exception(llvm::Value *value) {
    save_failure(outcome(empty_scalar(), b_.CreateExtractValue(value, 0), b_.CreateExtractValue(value, 1), null_pointer(),
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
    for (size_t i = 0; i < facts_.size(); ++i) {
      const auto *facts = facts_[i];
      if (std::any_of(inputs.begin(), inputs.end(), [&](const auto *input) { return facts->at(input).type == Type::never; })) continue;
      Choice choice{{}, &facts->at(&expr)};
      for (size_t p = 0; p < inputs.size(); ++p)
        choice.inputs.push_back(prepared ? choice.operation->arguments.at(p).type : facts->at(inputs[p]).type);
      const auto same = [&](const Choice &other) {
        return other.inputs == choice.inputs && other.operation->callee == choice.operation->callee &&
            other.operation->type == choice.operation->type && other.operation->operand_type == choice.operation->operand_type &&
            other.operation->arguments == choice.operation->arguments && other.operation->builtin == choice.operation->builtin;
      };
      auto found = std::find_if(result.begin(), result.end(), same);
      if (found == result.end()) { result.push_back(std::move(choice)); found = std::prev(result.end()); }
      if (i < frame_contexts_.size()) found->contexts.push_back(frame_contexts_[i]);
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
  llvm::Value *convert_result(const Expression &expr, const Expression &input, llvm::Value *value) {
    const auto cases = choices(expr, {&input});
    if (std::all_of(cases.begin(), cases.end(), [](const Choice &choice) {
          return choice.inputs.front() == choice.operation->operand_type;
        })) return value;
    return choose(expr, {value}, cases, scalar_, [&](const Choice &choice) {
      if (choice.inputs.front() == Type::void_type) return empty_scalar();
      return convert_value(value, choice.inputs.front(), choice.operation->operand_type);
    });
  }
  llvm::Value *binary(const Expression &expr, llvm::Value *left, llvm::Value *right, const Choice &choice) {
    const auto type = choice.operation->operand_type;
    left = convert(left, choice.inputs[0], type); right = convert(right, choice.inputs[1], type);
    const auto &op = expr.text;
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
  bool caller_destination(size_t method) const {
    if (program_->methods[method].result.borrowed()) return false;
    const auto &instances = program_->methods[method].instances;
    return std::any_of(instances.begin(), instances.end(), [&](size_t id) { return program_->instances[id].result.kind == Type::procedure; });
  }
  llvm::Value *native_call(llvm::Function *target, size_t method, std::vector<llvm::Value *> prepared,
                           llvm::Value *captured = nullptr, std::optional<size_t> instance = {}) {
    for (size_t i = 0; i < prepared.size(); ++i) {
      if (program_->methods[method].source->parameters[i].owner.empty()) take_value(prepared[i]);
    }
    llvm::AllocaInst *destination = nullptr;
    if (!program_->methods[method].captures.empty()) prepared.push_back(captured ? captured : environment_);
    if (instance) {
      for (const auto &type : program_->instances[*instance].retained_types) prepared.push_back(b_.getInt32(tag(type)));
    } else if (!program_->methods[method].retained_types.empty()) {
      if (program_->methods[method].retained_types.size() != retained_types_.size())
        throw std::logic_error("native call lacks its lexical type context");
      prepared.insert(prepared.end(), retained_types_.begin(), retained_types_.end());
    }
    if (!program_->methods[method].queries.empty()) {
      if (!instance && !query_environment_) throw std::logic_error("native call lacks its declaration evidence context");
      prepared.push_back(instance ? b_.getInt32(program_->instances[*instance].query_environment) : query_environment_);
    }
    if (caller_destination(method)) { destination = slot(scalar_, "callable.result"); prepared.push_back(destination); }
    llvm::Value *result = b_.CreateCall(target, prepared);
    if (destination) result = b_.CreateInsertValue(result, load(destination), 0);
    return result;
  }
  llvm::Value *invoke_value(const Expression &expr, const std::vector<llvm::Value *> &prepared) {
    std::vector<Choice> cases;
    for (const auto *facts : facts_) {
      const auto &fact = facts->at(&expr);
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
        if (std::none_of(facts_.begin(), facts_.end(), [&](const auto *facts) {
              const auto &fact = facts->at(&expr);
              return fact.type != Type::never && fact.operand_type == value.type && (!fact.callable_choices || fact.callable_choices->contains(id));
            })) continue;
        auto *selected = block("callable.selected"); select->addCase(integer(id + 1), selected);
        b_.SetInsertPoint(selected);
        const auto &instance = program_->instances[value.instance];
        const auto method = instance.method;
        auto *target = specialize_ ? specialized_[value.instance] : general_[method];
        std::vector<llvm::Value *> arguments{prepared.begin() + 1, prepared.end()};
        for (size_t i = 0; i < arguments.size(); ++i) if (value.type.parameters[i] != instance.arguments[i]) {
          auto *original = arguments[i];
          arguments[i] = convert_value(original, value.type.parameters[i], instance.arguments[i]);
          if (const auto owned = value_owners_.find(original); owned != value_owners_.end()) value_owners_[arguments[i]] = owned->second;
        }
        auto *result = native_call(target, method, std::move(arguments), environment(prepared.front()), value.instance);
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
    if (expr.text == "!") return pack(b_.CreateNot(argument), type);
    if (expr.text == "+") return arguments.front();
    if (floating(type)) return pack(b_.CreateFNeg(argument), type);
    auto *zero = pack(llvm::ConstantInt::get(llvm::cast<llvm::IntegerType>(native_type(type)), 0), type);
    return binary(expr, zero, arguments.front(), Choice{{type, type}, choice.operation});
  }
  llvm::Value *invoke(const Expression &expr, const std::vector<llvm::Value *> &arguments) {
    if (facts_.front()->at(&expr).callable_call) return invoke_value(expr, arguments);
    std::vector<const Expression *> source_inputs;
    for (const auto &argument : expr.children) source_inputs.push_back(argument.get());
    const auto cases = choices(expr, source_inputs, true);
    const auto call = [&](llvm::Function *target, size_t method) { return native_call(target, method, arguments); };
    if (cases.front().operation->callee && (cases.size() > 1 || !specialize_)) {
      const auto method = program_->instances[*cases.front().operation->callee].method;
      if (std::all_of(cases.begin(), cases.end(), [&](const Choice &choice) {
            return choice.operation->callee && program_->instances[*choice.operation->callee].method == method;
          }))
        return call(general_[method], method);
    }
    return choose(expr, arguments, cases, outcome_, [&](const Choice &choice) -> llvm::Value * {
      if (choice.operation->callee) {
        const auto id = *choice.operation->callee;
        auto *target = specialize_ && cases.size() == 1 ? specialized_[id] : general_[program_->instances[id].method];
        return call(target, program_->instances[id].method);
      }
      if (choice.operation->builtin == BuiltinOperation::unary || choice.operation->builtin == BuiltinOperation::binary)
        return outcome(eager_builtin(expr, arguments, choice), integer(0), integer(0), null_pointer());
      if (choice.operation->builtin == BuiltinOperation::bits_swap) {
        auto *left = view_address(arguments[0]), *right = view_address(arguments[1]);
        auto *left_value = b_.CreateLoad(scalar_, left), *right_value = b_.CreateLoad(scalar_, right);
        b_.CreateStore(right_value, left); b_.CreateStore(left_value, right);
        return outcome(empty_scalar(), integer(0), integer(0), null_pointer());
      }
      if (choice.operation->builtin == BuiltinOperation::cause_at) {
        auto *payload = unpack(arguments[0], choice.inputs[0]);
        auto *buffer = slot(exception_, "exception.cause");
        b_.CreateCall(runtime("cod_lang_exception_cause", b_.getVoidTy(), {b_.getInt64Ty(), b_.getInt64Ty(), b_.getPtrTy()}),
            {b_.CreateExtractValue(payload, 4), unpack(arguments[1], Type::uint64), buffer});
        auto *loaded = load(buffer);
        llvm::Value *result = llvm::UndefValue::get(exception_);
        result = b_.CreateInsertValue(result, b_.CreateExtractValue(loaded, 4), 0);
        for (unsigned i = 0; i < 4; ++i) result = b_.CreateInsertValue(result, b_.CreateExtractValue(loaded, i), i + 1);
        return outcome(pack(result, exception_type(0)), integer(0), integer(0), null_pointer());
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
  llvm::Value *propagate(llvm::Value *result) {
    auto *bad = block("call.failed"), *good = block("call.completed");
    b_.CreateCondBr(failed(result), bad, good);
    b_.SetInsertPoint(bad); save_failure(result); b_.CreateBr(control_->exit);
    b_.SetInsertPoint(good);
    return b_.CreateExtractValue(result, 0);
  }
  void collect_deferred(const Expression &expr, Scope &scope) {
    if (expr.kind == Expression::Kind::block && expr.scoped) return;
    for (const auto &child : expr.children) collect_deferred(*child, scope);
    if (expr.kind != Expression::Kind::deferred) return;
    const auto &body = *expr.children.front();
    if (body.kind != Expression::Kind::block && std::all_of(facts_.begin(), facts_.end(),
        [&](const auto *facts) { return facts->at(&body).type == Type::never; })) return;
    std::set<std::set<size_t>> layouts;
    for (const auto *facts : facts_) layouts.insert(facts->at(&expr).captures);
    for (const auto &captures : layouts) {
      std::vector<llvm::Type *> fields{deferred_header_};
      fields.insert(fields.end(), body.kind == Expression::Kind::block ? captures.size() : prepared_count(body), scalar_);
      scope.deferred.push_back({&expr, llvm::StructType::get(context_, fields), captures});
    }
  }
  void complete_cleanup(const std::function<void()> &emit, bool handles_failure = false) {
    // Suspend the pending completion while cleanup executes its own control flow.
    std::map<llvm::AllocaInst *, llvm::Value *> pending;
    for (auto *place : {result_, category_, site_, fatal_, requested_, available_, cause_, exit_, returning_}) pending.emplace(place, load(place));
    if (environments_) b_.CreateStore(empty_scalar(), result_);
    auto *pending_failure = b_.CreateICmpEQ(pending.at(exit_), b_.getInt8(2));
    auto *entered_failure = handles_failure ? pending_failure : b_.getFalse();
    b_.CreateStore(b_.CreateSelect(entered_failure, b_.getInt8(2), b_.getInt8(0)), exit_);
    b_.CreateStore(b_.CreateSelect(entered_failure, pending.at(category_), integer(0)), category_);
    b_.CreateStore(b_.CreateSelect(entered_failure, pending.at(fatal_), null_pointer()), fatal_);
    b_.CreateStore(b_.getFalse(), returning_);
    Control action{control_, block("cleanup.completed")};
    control_ = &action;
    emit();
    if (reachable()) b_.CreateBr(action.exit);
    b_.SetInsertPoint(action.exit);
    control_ = action.parent;
    auto *restore = block("cleanup.preserve"), *replace = block("cleanup.replace"), *done = block("cleanup.done");
    b_.CreateCondBr(b_.CreateICmpEQ(load(exit_), b_.getInt8(0)), restore, replace);
    b_.SetInsertPoint(restore);
    if (environments_) clear_value(result_);
    for (const auto &[place, value] : pending) b_.CreateStore(value, place);
    if (handles_failure) {
      auto *resumed = b_.CreateSelect(pending.at(returning_), b_.getInt8(1), b_.getInt8(0));
      b_.CreateStore(b_.CreateSelect(pending_failure, resumed, pending.at(exit_)), exit_);
      b_.CreateStore(b_.CreateSelect(pending_failure, integer(0), pending.at(category_)), category_);
      b_.CreateStore(b_.CreateSelect(pending_failure, null_pointer(), pending.at(fatal_)), fatal_);
    }
    b_.CreateBr(done);
    b_.SetInsertPoint(replace);
    auto *retain = block("cleanup.retain.return"), *replaced = block("cleanup.replaced.return");
    b_.CreateCondBr(b_.CreateAnd(b_.CreateICmpEQ(load(exit_), b_.getInt8(2)), b_.CreateNot(load(returning_))), retain, replaced);
    b_.SetInsertPoint(retain);
    if (environments_) clear_value(result_);
    b_.CreateStore(pending.at(result_), result_);
    b_.CreateStore(pending.at(returning_), returning_);
    b_.CreateBr(done);
    b_.SetInsertPoint(replaced); release_value(pending.at(result_)); b_.CreateBr(done);
    b_.SetInsertPoint(done);
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
      const auto all_facts = facts_;
      const auto all_contexts = frame_contexts_;
      facts_.clear(); frame_contexts_.clear();
      for (size_t j = 0; j < all_facts.size(); ++j)
        if (all_facts[j]->at(descriptor.action).captures == descriptor.captures &&
            (body.kind == Expression::Kind::block || all_facts[j]->at(&body).type != Type::never)) {
          facts_.push_back(all_facts[j]);
          if (!all_contexts.empty()) frame_contexts_.push_back(all_contexts.at(j));
        }
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
        auto *pending_failure = b_.CreateICmpEQ(load(exit_), b_.getInt8(2));
        complete_cleanup([&] {
          if (descriptor.action->children.size() == 1) emit_expression(body, Projection::value, true);
          else emit_finally(*descriptor.action, Projection::value, true, pending_failure);
        }, descriptor.action->children.size() > 1);
        for (const auto &[id, place] : bindings) storage_[id] = place;
        facts_ = all_facts; frame_contexts_ = all_contexts;
        b_.CreateBr(scope.cleanup);
        continue;
      }
      std::vector<llvm::Value *> arguments;
      for (size_t j = 0; j < prepared_count(body); ++j)
        arguments.push_back(b_.CreateLoad(scalar_, b_.CreateStructGEP(descriptor.layout, record, j + 1)));
      // Prepared values and checked borrowed addresses do not refer into their record.
      b_.CreateCall(release, {record});
      auto *result = invoke(body, arguments);
      release_value(b_.CreateExtractValue(result, 0));
      if (facts_.front()->at(&body).callable_call) release_value(arguments.front());
      facts_ = all_facts; frame_contexts_ = all_contexts;
      auto *replace = block("defer.failed");
      b_.CreateCondBr(failed(result), replace, scope.cleanup);
      b_.SetInsertPoint(replace); save_failure(result); b_.CreateBr(scope.cleanup);
    }
    b_.SetInsertPoint(invalid);
    b_.CreateCall(release, {record});
    save_failure(outcome(empty_scalar(), integer(exception_id(ExceptionCategory::unsound_runtime_typing)), integer(0), null_pointer()));
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
  llvm::Value *full_scope(const std::vector<const Expression *> &expressions, bool callable,
                          const Location &at, llvm::Value *returns_value = nullptr, Projection projection = Projection::value, bool discard_last = false) {
    const bool read_last = projection == Projection::value;
    auto *outer_temporaries = temporaries_;
    auto *saved = !callable && read_last && environments_ ? owned_slot(*outer_temporaries, "scope.value")
        : slot(read_last ? scalar_ : static_cast<llvm::Type *>(b_.getPtrTy()), "scope.value");
    Scope scope{scope_, block("scope.cleanup"), slot(b_.getPtrTy(), "defer.head"), {}, {}};
    if (callable && environments_) scope.values.values = parameter_values_;
    Control control{control_, scope.cleanup};
    control_ = &control;
    scope_ = &scope;
    temporaries_ = &scope.values;
    b_.CreateStore(null_pointer(), scope.head);
    for (const auto *expr : expressions) collect_deferred(*expr, scope);
    if (!environments_ || !read_last || callable) b_.CreateStore(read_last ? empty_scalar() : null_pointer(), saved);
    auto *value = sequence(expressions, projection, discard_last);
    if (reachable()) {
      if (!callable && read_last && environments_) assign_value(saved, discard_last ? empty_scalar() : value, at);
      else b_.CreateStore(value, saved);
      b_.CreateBr(scope.cleanup);
    }
    drain(scope); release_values(scope.values);
    scope_ = scope.parent; control_ = control.parent; temporaries_ = outer_temporaries;
    if (callable) {
      if (returns_value) {
        auto *missing = block("return.missing");
        b_.CreateCondBr(b_.CreateAnd(returns_value, b_.CreateICmpEQ(load(exit_), b_.getInt8(0))), missing, finish_);
        b_.SetInsertPoint(missing);
        save_failure(outcome(empty_scalar(), integer(exception_id(ExceptionCategory::missing_return)), integer(site(at)), null_pointer()));
      }
      b_.CreateBr(finish_);
      return nullptr;
    }
    auto *continuation = block("scope.continue");
    b_.CreateCondBr(b_.CreateICmpEQ(load(exit_), b_.getInt8(0)), continuation, control_->exit);
    b_.SetInsertPoint(continuation);
    auto *result = load(saved);
    if (environments_ && read_last) value_owners_[result] = saved;
    return result;
  }
  llvm::Value *emit_finally(const Expression &expr, Projection projection, bool discard = false, llvm::Value *skip_body = nullptr) {
    const bool read = projection == Projection::value;
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
    auto *catchable = b_.CreateAnd(b_.CreateICmpNE(load(category_), integer(0)), b_.CreateICmpEQ(load(fatal_), null_pointer()));
    b_.CreateCondBr(catchable, dispatch, handled);
    b_.SetInsertPoint(dispatch);
    const Expression *finalizer = nullptr;
    for (size_t i = 1; i < expr.children.size(); ++i) {
      const auto &handler = *expr.children[i];
      if (handler.kind == Expression::Kind::block) { finalizer = &handler; break; }
      const auto &fact = facts_.front()->at(&handler);
      auto *selected = block("catch.selected"), *next = block("catch.next");
      auto *caught = choose(handler, {}, choices(handler, {}), scalar_, [&](const Choice &choice) -> llvm::Value * {
        const auto &type = choice.operation->type;
        auto *value = pack(active_exception(), type);
        return type.identity ? b_.CreateSelect(b_.CreateICmpEQ(load(category_), integer(type.identity)), value, empty_scalar()) : value;
      });
      b_.CreateCondBr(b_.CreateICmpNE(b_.CreateExtractValue(caught, 1), b_.getInt32(0)), selected, next);
      b_.SetInsertPoint(selected);
      b_.CreateStore(caught, storage_.at(*fact.place));
      b_.CreateStore(b_.getInt8(0), exit_); b_.CreateStore(integer(0), category_); b_.CreateStore(null_pointer(), fatal_);
      b_.CreateStore(b_.getFalse(), returning_);
      clear_value(result_);
      Control catching{control_, handled};
      control_ = &catching;
      auto *result = emit_expression(*handler.children.front(), projection, discard);
      if (reachable()) { if (read) assign_value(saved, discard ? empty_scalar() : result, handler.at); else b_.CreateStore(result, saved); b_.CreateBr(handled); }
      control_ = catching.parent;
      b_.SetInsertPoint(next);
    }
    b_.CreateBr(handled);
    b_.SetInsertPoint(handled);
    if (finalizer) complete_cleanup([&] { emit_expression(*finalizer); });
    auto *done = block("try.continue");
    b_.CreateCondBr(b_.CreateICmpEQ(load(exit_), b_.getInt8(0)), done, control_->exit);
    b_.SetInsertPoint(done);
    if (read && discard) return empty_scalar();
    if (facts_.front()->at(&expr).type == Type::never) { b_.CreateUnreachable(); return nullptr; }
    auto *result = load(saved);
    if (environments_ && read) value_owners_[result] = saved;
    return result;
  }
  llvm::Value *checked_address(llvm::Value *address, const Expression &expr, const Location &at, bool callable = false) {
    if (std::none_of(facts_.begin(), facts_.end(), [&](const auto *facts) {
          return callable ? facts->at(&expr).callable_nullable : facts->at(&expr).targets.contains(null_target);
        })) return address;
    auto *present = block("reference.present"), *absent = block("reference.absent");
    b_.CreateCondBr(b_.CreateICmpEQ(address, null_pointer()), absent, present);
    b_.SetInsertPoint(absent); fail_path(ExceptionCategory::null_pointer, at);
    b_.SetInsertPoint(present);
    return address;
  }
  size_t prepared_count(const Expression &expr) const {
    return expr.children.size() + (expr.kind == Expression::Kind::call && facts_.front()->at(&expr).callable_call ? 1 : 0);
  }
  static bool inert_argument(const Expression &expr) {
    using K = Expression::Kind;
    return expr.kind == K::name || expr.kind == K::integer || expr.kind == K::real || expr.kind == K::boolean || expr.kind == K::null_ref;
  }
  llvm::Value *prepare_value(const Expression &expr, size_t index, llvm::Value *value) {
    const auto &child = *expr.children[index];
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
    const auto all_facts = facts_;
    const auto all_contexts = frame_contexts_;
    auto *join = block(label + ".prepared");
    auto *saved = environments_ && result_type == scalar_ ? owned_slot(*temporaries_, label + ".value") : nullptr;
    std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> incoming;
    for (const auto &[key, indices] : alternatives) {
      auto *selected = block(label + ".prepare"), *next = block(label + ".next");
      llvm::Value *condition = b_.getFalse();
      facts_.clear(); frame_contexts_.clear();
      for (const auto i : indices) {
        condition = b_.CreateOr(condition, matches_context(*all_contexts.at(i)));
        facts_.push_back(all_facts[i]); frame_contexts_.push_back(all_contexts[i]);
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
    facts_ = all_facts; frame_contexts_ = all_contexts;
    fail_path(ExceptionCategory::unsound_runtime_typing, expr.at);
    b_.SetInsertPoint(join);
    if (incoming.empty()) { b_.CreateUnreachable(); return nullptr; }
    if (saved) { auto *value = load(saved); value_owners_[value] = saved; return value; }
    auto *value = b_.CreatePHI(result_type, incoming.size(), label + ".value");
    for (const auto &[prepared, from] : incoming) value->addIncoming(prepared, from);
    return value;
  }
  llvm::Value *prepare_argument(const Expression &expr, size_t index, bool copy) {
    const auto &child = *expr.children[index];
    const auto prepare = [&](ArgumentMode mode) -> llvm::Value * {
      auto *value = emit_expression(child, mode == ArgumentMode::value ? Projection::value : Projection::target);
      if (!reachable()) return nullptr;
      if (mode != ArgumentMode::value) {
        if (mode == ArgumentMode::borrowed_read && !facts_.front()->at(&expr).destination_arguments)
          value = checked_address(value, child, child.at);
        return pack_view(value, child);
      }
      value = prepare_value(expr, index, value);
      return copy ? copy_temporary(value, child.at) : value;
    };
    std::map<ArgumentMode, std::vector<size_t>> alternatives;
    for (size_t i = 0; i < facts_.size(); ++i)
      alternatives[facts_[i]->at(&expr).arguments.at(index).mode].push_back(i);
    return contextual_value(child, alternatives, "argument", prepare);
  }
  std::vector<llvm::Value *> prepare_arguments(const Expression &expr, bool deferred = false) {
    std::vector<llvm::Value *> arguments;
    const auto &fact = facts_.front()->at(&expr);
    const size_t offset = expr.kind == Expression::Kind::value_call ? 1 : 0;
    const bool protect = deferred || std::any_of(expr.children.begin() + offset, expr.children.end(),
                                               [](const auto &child) { return !inert_argument(*child); });
    if (expr.kind == Expression::Kind::call && fact.callable_call) {
      auto *address = binding_address(*fact.callable_binding, fact.callable_place);
      auto *value = b_.CreateLoad(scalar_, checked_address(address, expr, expr.at, true));
      arguments.push_back(protect ? copy_temporary(value, expr.at) : value);
    }

    for (size_t i = 0; i < expr.children.size(); ++i) {
      auto *value = prepare_argument(expr, i, !offset || i || protect);
      if (!reachable()) return {};
      arguments.push_back(value);
    }
    if (fact.destination_arguments) for (size_t i = 0; i < arguments.size(); ++i) {
      std::map<bool, std::vector<size_t>> alternatives;
      for (size_t j = 0; j < facts_.size(); ++j)
        alternatives[facts_[j]->at(&expr).arguments.at(i).mode == ArgumentMode::borrowed_read].push_back(j);
      contextual_value(*expr.children[i], alternatives, "destination.check", [&](bool read) {
        if (read) checked_address(view_address(arguments[i]), *expr.children[i], expr.at);
        return b_.getTrue();
      }, b_.getInt1Ty());
    }
    return arguments;
  }
  llvm::Value *captured_value(const CheckedOperation &fact) {
    if (fact.callable) return pack(integer(*fact.callable + 1), fact.type);
    auto *address = binding_address(*fact.binding, fact.place);
    return b_.CreateLoad(scalar_, address);
  }
  llvm::Value *materialize(const CheckedOperation &fact, const Type &type, const Location &at) {
    auto *value = pack(integer(*fact.callable + 1), type);
    if (fact.callable_captures.empty()) return value;
    const auto count = fact.callable_captures.size();
    auto fields = std::vector<llvm::Type *>{b_.getInt64Ty()}; fields.insert(fields.end(), count, scalar_);
    auto *layout = llvm::StructType::get(context_, fields);
    auto *children = slot(llvm::ArrayType::get(b_.getPtrTy(), count), "image.children");
    auto *offsets = slot(llvm::ArrayType::get(b_.getInt64Ty(), count), "image.offsets");
    std::vector<llvm::Value *> captures;
    const auto &data = module_.getDataLayout();
    for (size_t i = 0; i < count; ++i) {
      auto *captured = captured_value(fact.callable_captures[i]); captures.push_back(captured);
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
    std::map<bool, std::vector<size_t>> alternatives;
    for (size_t i = 0; i < facts_.size(); ++i) alternatives[facts_[i]->at(&declaration).view].push_back(i);
    return contextual_value(declaration, alternatives, "storage", [&](bool borrowed) {
      const auto &binding = facts_.front()->at(&declaration);
      llvm::Value *address = binding.place ? static_cast<llvm::Value *>(storage_.at(*binding.place)) : null_pointer();
      if (environments_ && binding.place && !borrowed) scope_->values.values.push_back(storage_.at(*binding.place));
      if (borrowed) {
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
      if (auto *slot = bindings_.at(*binding.binding)) b_.CreateStore(address, slot);
      return empty_scalar();
    });
  }
  llvm::Value *emit_expression(const Expression &expr, Projection projection = Projection::value, bool discard = false) {
    auto *value = emit_checked_expression(expr, projection, discard);
    if (std::all_of(facts_.begin(), facts_.end(), [&](const auto *facts) { return facts->at(&expr).type == Type::never; })) {
      if (reachable()) b_.CreateUnreachable();
      return nullptr;
    }
    return value;
  }
  llvm::Value *emit_checked_expression(const Expression &expr, Projection projection, bool discard) {
    const bool read = projection == Projection::value;
    using K = Expression::Kind;
    const auto &fact = facts_.front()->at(&expr);
    if (std::any_of(facts_.begin(), facts_.end(), [&](const auto *facts) {
          return facts->at(&expr).borrowed_result != fact.borrowed_result;
        })) {
      if (!read) implementation_gap(expr.at, "context-dependent value/view results without a common value-consuming boundary");
      std::map<bool, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < facts_.size(); ++i) alternatives[facts_[i]->at(&expr).borrowed_result].push_back(i);
      return contextual_value(expr, alternatives, "result", [&](bool) { return emit_expression(expr, projection, discard); });
    }
    if (std::all_of(facts_.begin(), facts_.end(), [&](const auto *facts) {
          const auto builtin = facts->at(&expr).builtin;
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
      const bool definition = expr.kind == K::callable && fact.place.has_value();
      const auto materialize = [&](const CheckedOperation &value) {
        return this->materialize(value, definition ? value.operand_type : value.type, expr.at);
      };
      llvm::Value *value;
      if (std::any_of(facts_.begin(), facts_.end(), [&](const auto *facts) { return facts->at(&expr).callable != fact.callable; })) {
        std::vector<Choice> cases;
        for (size_t i = 0; i < facts_.size(); ++i) cases.push_back({{}, &facts_[i]->at(&expr), {frame_contexts_[i]}});
        value = choose(expr, {}, cases, scalar_, [&](const Choice &choice) { return materialize(*choice.operation); });
      } else value = materialize(fact);
      if (definition) {
        auto *place = storage_.at(*fact.place);
        if (environments_) { scope_->values.values.push_back(place); clear_value(place); }
        b_.CreateStore(value, place);
      } else value = own_value(value);
      return definition ? empty_scalar() : value;
    }
    switch (expr.kind) {
    case K::null_ref: return null_pointer();
    case K::integer: case K::boolean:
      return pack(llvm::ConstantInt::get(llvm::cast<llvm::IntegerType>(native_type(fact.type)), fact.integer), fact.type);
    case K::real: {
      auto text = expr.text;
      if (text.ends_with("f32") || text.ends_with("f64")) text.resize(text.size() - 3);
      if (text.find_first_of(".eE") == std::string::npos) text += ".0";
      return pack(llvm::ConstantFP::get(native_type(fact.type), text), fact.type);
    }
    case K::name: {
      if (projection == Projection::slot) return binding_slot(*fact.binding);
      std::map<std::optional<size_t>, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < facts_.size(); ++i) alternatives[facts_[i]->at(&expr).captured_place].push_back(i);
      auto *address = contextual_value(expr, alternatives, "storage.address", [&](std::optional<size_t> captured) {
        const auto &selected = facts_.front()->at(&expr);
        auto *address = binding_address(*selected.binding, selected.place);
        if (captured && !binding_layout_->at(*selected.binding).value_binding)
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
      if (fact.builtin == BuiltinOperation::reference_swap) {
        auto *left = emit_expression(*expr.children[0], Projection::slot);
        if (!reachable()) return nullptr;
        auto *right = emit_expression(*expr.children[1], Projection::slot);
        if (!reachable()) return nullptr;
        auto *type = b_.getPtrTy();
        auto *left_value = b_.CreateLoad(type, left), *right_value = b_.CreateLoad(type, right);
        b_.CreateStore(right_value, left); b_.CreateStore(left_value, right);
        return empty_scalar();
      }
      if (fact.builtin == BuiltinOperation::is_null) {
        auto *address = emit_expression(*expr.children.front(), Projection::target);
        if (!reachable()) return nullptr;
        return pack(b_.CreateICmpEQ(address, null_pointer()), Type::boolean);
      }
      return temporary_scope(expr.at, !fact.borrowed_result || read ? scalar_ : static_cast<llvm::Type *>(b_.getPtrTy()), [&]() -> llvm::Value * {
        const auto arguments = prepare_arguments(expr);
        if (!reachable()) return nullptr;
        auto *value = propagate(invoke(expr, arguments));
        if (!fact.borrowed_result) return own_value(value);
        auto *address = view_address(value);
        return !read ? address : discard ? empty_scalar() : b_.CreateLoad(scalar_, checked_address(address, expr, expr.at));
      });
    }
    case K::conditional: {
      auto *condition = emit_expression(*expr.children[0]);
      if (!reachable()) return nullptr;
      auto *yes = block("if.then"), *no = block("if.else"), *join = block("if.join");
      b_.CreateCondBr(unpack(condition, Type::boolean), yes, no);
      std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> incoming;
      for (size_t i = 1; i < 3; ++i) {
        b_.SetInsertPoint(i == 1 ? yes : no);
        auto *value = emit_expression(*expr.children[i], projection, discard);
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
      auto *value = expr.scoped ? full_scope(expressions, false, expr.at, nullptr, projection, discard) : sequence(expressions, projection, discard);
      return value;
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
      auto *next = emit_expression(*expr.children[fold ? 3 : 2], Projection::value, !fold);
      if (reachable()) {
        if (fold) assign_value(accumulator, next, expr.at);
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
      if (!fold && expr.children.size() == 4) emit_expression(*expr.children[3], Projection::value, true);
      if (reachable()) b_.CreateBr(done);
      b_.SetInsertPoint(done);
      if (fact.type == Type::never) { b_.CreateUnreachable(); return nullptr; }
      return fold ? load(accumulator) : empty_scalar();
    }
    case K::alias: {
      const bool selected = fact.binding.has_value();
      const bool value = !selected || binding_layout_->at(*fact.binding).value_binding;
      auto *address = emit_expression(*expr.children.front(), value ? Projection::target : Projection::slot, true);
      if (reachable() && selected) b_.CreateStore(address, selections_.at(*fact.binding));
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
    case K::transfer: {
      auto *address = emit_expression(*expr.children[0], Projection::target);
      if (!reachable()) return nullptr;
      address = checked_address(address, *expr.children[0], expr.at);
      auto *value = emit_expression(*expr.children[1]);
      if (!reachable()) return nullptr;
      value = convert_result(expr, *expr.children[1], value);
      assign_value(address, value, expr.at);
      return read ? b_.CreateLoad(scalar_, address) : address;
    }
    case K::rebind: {
      auto *selected = emit_expression(*expr.children[0], Projection::slot);
      if (!reachable()) return nullptr;
      auto *address = emit_expression(*expr.children[1], Projection::target);
      if (!reachable()) return nullptr;
      b_.CreateStore(address, selected);
      if (projection == Projection::slot) return selected;
      return !read ? address : discard ? empty_scalar() : b_.CreateLoad(scalar_, checked_address(address, expr, expr.at));
    }
    case K::return_value: {
      llvm::Value *value = empty_scalar();
      if (!expr.children.empty()) {
        value = emit_expression(*expr.children.front(), fact.borrowed_result ? Projection::target : Projection::value);
        if (!reachable()) return nullptr;
        if (fact.borrowed_result) value = pack_view(value, expr, true);
        else value = convert_result(expr, *expr.children.front(), value);
      }
      assign_value(result_, value, expr.at); b_.CreateStore(integer(0), category_); b_.CreateStore(null_pointer(), fatal_);
      b_.CreateStore(b_.getTrue(), returning_);
      b_.CreateStore(b_.getInt8(1), exit_); b_.CreateBr(control_->exit);
      return nullptr;
    }
    case K::field: {
      auto *value = emit_expression(*expr.children[0]);
      if (!reachable()) return nullptr;
      const auto &type = facts_.front()->at(expr.children[0].get()).type;
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
      for (size_t i = 0; i < facts_.size(); ++i) alternatives[facts_[i]->at(&expr).captures].push_back(i);
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
  llvm::Function *declare(const std::string &name, size_t arity, bool destination, bool environment, size_t retained) {
    std::vector<llvm::Type *> parameters(arity, scalar_);
    if (environment) parameters.push_back(b_.getPtrTy());
    parameters.insert(parameters.end(), retained, b_.getInt32Ty());
    if (destination) parameters.push_back(b_.getPtrTy());
    return llvm::Function::Create(llvm::FunctionType::get(outcome_, parameters, false),
        llvm::GlobalValue::InternalLinkage, name, module_);
  }
  void begin(llvm::Function *function, const std::vector<CheckedStorage> &storage, const std::vector<CheckedBinding> &bindings, size_t arity = 0) {
    function_ = function; arguments_.clear(); retained_types_.clear(); parameter_values_.clear(); value_owners_.clear(); environment_ = nullptr; query_environment_ = nullptr; temporaries_ = nullptr;
    for (size_t i = 0; i < arity; ++i) arguments_.push_back(function->getArg(i));
    b_.SetInsertPoint(block("entry")); finish_ = block("completion"); scope_ = nullptr; control_ = nullptr;
    result_ = slot(scalar_, "return.value"); category_ = slot(b_.getInt64Ty(), "exception.category");
    site_ = slot(b_.getInt64Ty(), "exception.site"); fatal_ = slot(b_.getPtrTy(), "numeric.failure");
    requested_ = slot(b_.getInt64Ty(), "exception.requested"); available_ = slot(b_.getInt64Ty(), "exception.available"); cause_ = slot(b_.getInt64Ty(), "exception.cause");
    exit_ = slot(b_.getInt8Ty(), "completion.kind");
    returning_ = slot(b_.getInt1Ty(), "completion.has.return");
    b_.CreateStore(b_.getFalse(), returning_);
    b_.CreateStore(empty_scalar(), result_); b_.CreateStore(integer(0), category_);
    b_.CreateStore(integer(0), site_); b_.CreateStore(null_pointer(), fatal_); b_.CreateStore(b_.getInt8(0), exit_);
    for (auto *place : {requested_, available_, cause_}) b_.CreateStore(integer(0), place);
    storage_.clear();
    for (const auto &binding : storage) {
      auto *place = binding.borrowed ? nullptr : slot(scalar_, binding.name + ".storage");
      storage_.push_back(place);
      if (place && environments_) initialize_value(place);
    }
    bindings_.clear(); selections_.clear(); binding_layout_ = &bindings;
    for (const auto &binding : bindings) {
      bindings_.push_back(binding.value_binding || binding.alias || !binding.selection.empty() ? nullptr : slot(b_.getPtrTy(), "reference.target"));
      selections_.push_back(binding.selection.empty() ? nullptr : slot(b_.getPtrTy(), "alias.selection"));
    }
    for (size_t i = 0; i < arguments_.size(); ++i) {
      if (storage[i].borrowed) { b_.CreateStore(view_address(arguments_[i]), bindings_[i]); continue; }
      b_.CreateStore(arguments_[i], storage_[i]);
      if (environments_) parameter_values_.push_back(storage_[i]);
      if (bindings_[i]) b_.CreateStore(storage_[i], bindings_[i]);
    }
  }
  void body(llvm::Function *function, const CheckedMethod &method, std::optional<size_t> instance) {
    const auto &layout = program_->instances[instance ? *instance : method.instances.front()];
    const auto instances = instance ? std::vector<size_t>{*instance} : method.instances;
    std::vector<CheckedStorage> storage;
    std::map<size_t, size_t> places;
    std::deque<std::map<const Expression *, CheckedOperation>> operations;
    std::vector<const std::map<const Expression *, CheckedOperation> *> lowered;
    // A lexical declaration has one frame role even when a checked context
    // borrows an existing target and another constructs local payload storage.
    for (const auto id : instances) for (const auto &place : program_->instances[id].storage)
      if (places.emplace(place.binding, storage.size()).second) storage.push_back(place);
    for (const auto id : instances) {
      const auto &checked = program_->instances[id];
      const auto remap = [&](size_t place) { return place == null_target ? place : places.at(checked.storage.at(place).binding); };
      bool unchanged = true;
      for (size_t i = 0; i < checked.storage.size(); ++i) unchanged = unchanged && remap(i) == i;
      if (unchanged) { lowered.push_back(&checked.operations); continue; }
      const auto optional = [&](std::optional<size_t> &place) { if (place) place = remap(*place); };
      const auto set = [&](std::set<size_t> &source) {
        std::set<size_t> result;
        for (const auto place : source) result.insert(remap(place));
        source = std::move(result);
      };
      const auto lower = [&](auto &&self, CheckedOperation &operation) -> void {
        optional(operation.place); optional(operation.callable_place); optional(operation.accumulator); optional(operation.captured_place);
        set(operation.targets); set(operation.callable_targets); set(operation.captures);
        for (auto &capture : operation.callable_captures) self(self, capture);
      };
      operations.push_back(checked.operations);
      for (auto &[expr, operation] : operations.back()) lower(lower, operation);
      lowered.push_back(&operations.back());
    }
    begin(function, storage, layout.bindings, method.parameters.size()); facts_ = std::move(lowered); frame_contexts_.clear();
    for (size_t i = 0; i < instances.size(); ++i) {
      frame_contexts_.push_back(&program_->instances[instances[i]]);
    }
    if (!method.captures.empty()) {
      environment_ = function->getArg(method.parameters.size());
      auto fields = std::vector<llvm::Type *>{b_.getInt64Ty()}; fields.insert(fields.end(), method.captures.size(), scalar_);
      auto *image = llvm::StructType::get(context_, fields);
      for (size_t c = 0; c < method.captures.size(); ++c) {
        auto *value = b_.CreateLoad(scalar_, b_.CreateStructGEP(image, environment_, c + 1));
        auto *offset = b_.CreatePtrToInt(environment(value), b_.getInt64Ty());
        auto *nested = b_.CreateGEP(b_.getInt8Ty(), environment_, offset);
        auto *absolute = b_.CreateSelect(b_.CreateICmpEQ(offset, integer(0)), null_pointer(), nested);
        auto *captured = b_.CreateInsertValue(value, absolute, environment_field_);
        b_.CreateStore(captured, storage_.at(method.parameters.size() + c)); arguments_.push_back(captured);
      }
    }
    for (size_t i = 0; i < method.retained_types.size(); ++i) {
      auto *type = function->getArg(method.parameters.size() + !method.captures.empty() + i);
      retained_types_.push_back(type);
      arguments_.push_back(b_.CreateInsertValue(empty_scalar(), type, 1));
    }
    if (!method.queries.empty())
      query_environment_ = function->getArg(method.parameters.size() + !method.captures.empty() + method.retained_types.size());
    llvm::Value *returns_value = nullptr;
    if (instance) {
      if (program_->instances[*instance].result != Type::void_type) returns_value = b_.getTrue();
    }
    else {
      const bool all_return = std::all_of(method.instances.begin(), method.instances.end(),
          [&](size_t id) { return program_->instances[id].result != Type::void_type; });
      if (all_return) returns_value = b_.getTrue();
      llvm::Value *admitted = b_.getFalse();
      for (const auto id : method.instances) {
        auto *context = matches_context(program_->instances[id]);
        admitted = b_.CreateOr(admitted, context);
        if (!all_return && program_->instances[id].result != Type::void_type)
          returns_value = returns_value ? b_.CreateOr(returns_value, context) : context;
      }
      auto *valid = block("arguments.valid"), *invalid = block("arguments.invalid");
      b_.CreateCondBr(admitted, valid, invalid);
      b_.SetInsertPoint(invalid);
      for (auto *place : parameter_values_) clear_value(place);
      b_.CreateRet(outcome(empty_scalar(), integer(exception_id(ExceptionCategory::unsound_runtime_typing)),
          integer(site(method.source->at)), null_pointer()));
      b_.SetInsertPoint(valid);
    }
    full_scope({method.source->body.get()}, true, method.source->at, returns_value, Projection::value, true);
    b_.SetInsertPoint(finish_);
    if (environments_) {
      auto *failed = block("return.discard"), *ready = block("return.ready");
      b_.CreateCondBr(b_.CreateICmpEQ(load(exit_), b_.getInt8(2)), failed, ready);
      b_.SetInsertPoint(failed); clear_value(result_); b_.CreateBr(ready); b_.SetInsertPoint(ready);
    }
    const auto result_index = method.parameters.size() + !method.captures.empty() + method.retained_types.size() + !method.queries.empty();
    if (function->arg_size() > result_index) b_.CreateStore(load(result_), function->getArg(result_index));
    b_.CreateRet(outcome(load(result_), load(category_), load(site_), load(fatal_), load(requested_), load(available_), load(cause_)));
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
  std::pair<std::string, std::string> emit(const CheckedProgram &program, bool emit_text, bool emit_bitcode, bool specialize = true) {
    program_ = &program; specialize_ = specialize;
    environments_ = std::any_of(program.methods.begin(), program.methods.end(), [](const auto &method) { return !method.captures.empty(); });
    std::vector<llvm::Type *> fields{b_.getInt64Ty(), b_.getInt32Ty()};
    if (program.exception_values) fields.push_back(exception_);
    if (environments_) { environment_field_ = fields.size(); fields.push_back(b_.getPtrTy()); }
    scalar_->setBody(fields);
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
    general_.resize(program.methods.size()); specialized_.resize(program.instances.size());
    for (size_t i = 0; i < program.methods.size(); ++i) {
      const auto &method = program.methods[i]; if (method.instances.empty()) continue;
      const auto prefix = "cod.method." + std::to_string(i) + "." + method.source->name;
      if (method.source->type_parameters.empty() && method.captures.empty() && method.retained_types.empty() && method.queries.empty()) {
        general_[i] = declare(prefix + ".native", method.source->parameters.size(), caller_destination(i), false, 0);
        specialized_[method.instances.front()] = general_[i];
        continue;
      }
      general_[i] = declare(prefix + ".general", method.source->parameters.size(), caller_destination(i), !method.captures.empty(), method.retained_types.size() + !method.queries.empty());
      if (specialize) for (const auto id : method.instances) {
        std::string name = prefix + ".specialized";
        for (const auto &type : program.instances[id].context) name += "." + type_name(type);
        if (!method.queries.empty()) name += ".query." + std::to_string(program.instances[id].query_environment);
        specialized_[id] = declare(name, method.source->parameters.size(), caller_destination(i), !method.captures.empty(), method.retained_types.size() + !method.queries.empty());
      }
    }
    for (size_t i = 0; i < program.methods.size(); ++i) {
      const auto &method = program.methods[i]; if (method.instances.empty()) continue;
      if (method.source->type_parameters.empty() && method.captures.empty() && method.retained_types.empty() && method.queries.empty()) {
        body(general_[i], method, method.instances.front());
        continue;
      }
      body(general_[i], method, {});
      if (specialize) for (const auto id : method.instances) body(specialized_[id], method, id);
    }
    auto *main = llvm::Function::Create(llvm::FunctionType::get(b_.getInt32Ty(), false), llvm::GlobalValue::ExternalLinkage, "main", module_);
    begin(main, program.entry_storage, program.entry_bindings); facts_ = {&program.entry}; full_scope(program.initializers, true, Location{}, nullptr, Projection::value, true);
    b_.SetInsertPoint(finish_);
    auto *fatal = block("fatal"), *exception = block("exception"), *normal = block("normal");
    b_.CreateCondBr(b_.CreateICmpNE(load(fatal_), null_pointer()), fatal, exception);
    b_.SetInsertPoint(fatal);
    b_.CreateCall(runtime("cod_lang_fail", b_.getVoidTy(), {b_.getPtrTy()}), {load(fatal_)}); b_.CreateUnreachable();
    b_.SetInsertPoint(exception);
    auto *uncaught = block("uncaught");
    b_.CreateCondBr(b_.CreateICmpNE(load(category_), integer(0)), uncaught, normal);
    b_.SetInsertPoint(uncaught);
    b_.CreateCall(runtime("cod_lang_uncaught", b_.getVoidTy(), std::vector<llvm::Type *>(5, b_.getInt64Ty())),
        {load(category_), load(site_), load(requested_), load(available_), load(cause_)}); b_.CreateUnreachable();
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
