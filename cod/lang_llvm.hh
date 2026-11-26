//===--- lang_llvm.hh - Checked scalar native emission --------------===//
#pragma once

#include "lang_c_provider.hh"
#include "lang_checked_program.hh"
#include "lang_exceptions.hh"
#include "lang_llvm_c.h"
#include "runtime/cod_lang_runtime.h"

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/TargetOptions.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/MathExtras.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Host.h>

#include <array>
#include <cstddef>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <new>
#include <string>
#include <string_view>
#include <utility>

namespace cod::language {

class LLVMEmission {
  struct ResourceSession {
    cod_llvm_session handles{};
    ResourceSession() = default;
    ResourceSession(const ResourceSession &) = delete;
    ResourceSession &operator=(const ResourceSession &) = delete;
    ~ResourceSession() { cod_llvm_session_release(&handles); }
    llvm::LLVMContext &open() {
      if (!cod_llvm_session_open(&handles, "cod.program")) throw std::bad_alloc{};
      return *llvm::unwrap(handles.context);
    }
  };
  // Construct the owner before opening, so partial acquisition unwinds too.
  ResourceSession resources_;
  llvm::LLVMContext &context_ = resources_.open();
  llvm::Module &module_ = *llvm::unwrap(resources_.handles.module);
  llvm::IRBuilder<> &b_ = *llvm::unwrap(resources_.handles.builder);
  // Private execution-local value transport. Callable environments are complete
  // scope-owned images; they are not regional-storable bits.
  llvm::StructType *exception_ = llvm::StructType::get(context_, std::vector<llvm::Type *>(5, b_.getInt64Ty()));
  llvm::StructType *scalar_ = llvm::StructType::create(context_, {b_.getInt64Ty(), b_.getInt32Ty()}, "cod.scalar");
  llvm::StructType *value_pack_segment_ = llvm::StructType::get(context_, {b_.getPtrTy(), b_.getInt64Ty()});
  llvm::StructType *value_pack_ = llvm::StructType::get(context_, {b_.getPtrTy(), b_.getInt64Ty(), b_.getInt64Ty()});
  enum class ArrayBacking { inline_elements, foreign_loan };
  llvm::StructType *array_header_ =
      llvm::StructType::create(context_, {b_.getInt64Ty(), b_.getInt8Ty()}, "cod.array.header");
  llvm::StructType *borrowed_array_ =
      llvm::StructType::create(context_, {array_header_, b_.getPtrTy()}, "cod.array.borrowed");
  llvm::StructType *native_array_view_ =
      llvm::StructType::create(context_, {b_.getPtrTy(), b_.getInt64Ty()}, "cod.native.array.view");
  llvm::StructType *array_capability_ =
      llvm::StructType::create(context_, {b_.getInt1Ty(), b_.getPtrTy()}, "cod.array.allocation");
  llvm::StructType *text_view_ = llvm::StructType::create(context_, {b_.getPtrTy(), b_.getInt64Ty(), b_.getInt64Ty()}, "cod.text.view");
  llvm::StructType *outcome_ = llvm::StructType::create(context_,
      {scalar_, b_.getInt64Ty(), b_.getInt64Ty(), b_.getPtrTy(), b_.getInt64Ty(), b_.getInt64Ty(), b_.getInt64Ty()}, "cod.outcome");
  llvm::StructType *here_ = llvm::StructType::create(context_, {b_.getPtrTy(), b_.getPtrTy()}, "cod.here");
  llvm::StructType *here_transfer_ = llvm::StructType::create(context_, "cod.here.transfer");
  bool protected_outcomes_ = false;
  bool owned_failures_ = false;
  bool case_scope_outcomes_ = false;
  llvm::AllocaInst *case_scan_request_ = nullptr;
  unsigned case_scan_field() const { return 7 + protected_outcomes_ + owned_failures_; }
  unsigned failure_owner_field() const { return protected_outcomes_ ? 8 : 7; }
  const CheckedProgram *program_ = nullptr;
  // Transient native parameter roles shared by declarations, calls and entry.
  struct CallLayout {
    size_t operands = 0, owners = 0, captures = 0, retained = 0;
    bool declaration_context = false;
    bool callable_destination = false, native_borrow_destination = false;
    size_t here_formals = 0;
    bool here_group = false;
    size_t lexical_here = 0;
    std::vector<size_t> array_owners{};
    bool exception_result = false;
    size_t array_owners_begin() const { return operands + owners; }
    size_t captures_index() const { return array_owners_begin() + array_owners.size(); }
    size_t retained_begin() const { return captures_index() + (captures != 0); }
    size_t query_index() const { return retained_begin() + retained; }
    size_t result_index() const { return query_index() + declaration_context; }
    size_t native_result_index() const { return result_index() + callable_destination; }
    size_t exception_result_index() const { return native_result_index() + native_borrow_destination; }
    bool has_result_destination() const { return callable_destination || native_borrow_destination || exception_result; }
    size_t here_index() const { return exception_result_index() + exception_result; }
    size_t here_count() const { return here_formals + lexical_here; }
    size_t parameter_count() const { return here_index() + here_count() + here_group; }
  };
  std::vector<CallLayout> call_layouts_;
  bool bounds_checks_ = true;
  bool environments_ = false;
  unsigned environment_field_ = 0;
  std::vector<Type> callable_types_;
  std::map<size_t, llvm::StructType *> product_layouts_;
  std::map<size_t, llvm::StructType *> regional_product_layouts_;
  std::vector<llvm::Function *> methods_;
  struct HereEntry { llvm::Function *entry; llvm::StructType *activation; bool group; };
  std::map<size_t, HereEntry> here_entries_;
  std::map<size_t, llvm::Function *> prepared_source_entries_;
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
  std::vector<std::set<StorageFormat>> storage_formats_;
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
  std::map<size_t, llvm::Value *> array_capabilities_;
  llvm::AllocaInst *lexical_allocations_ = nullptr;
  bool owns_lexical_allocations_ = false;
  std::map<size_t, llvm::AllocaInst *> exception_holder_slots_;
  std::vector<llvm::AllocaInst *> exception_parameter_slots_;
  llvm::AllocaInst *exception_result_destination_ = nullptr;
  llvm::AllocaInst *pending_exception_result_ = nullptr;
  std::vector<llvm::AllocaInst *> parameter_values_;
  llvm::Value *query_environment_ = nullptr;
  std::vector<llvm::Value *> here_arguments_;
  std::map<const Expression *, llvm::AllocaInst *> pipeline_operands_;
  llvm::Value *here_destination_ = nullptr;
  struct SeenReturn {
    llvm::Value *present{}, *value{};
  } suspended_return_;
  llvm::Function *function_ = nullptr;
  llvm::BasicBlock *finish_ = nullptr;
  // Live slots and suspended values share the complete pending state. Native
  // call outcomes carry a protected destination independently of local return and failure.
  template <typename T> struct CompletionState {
    T value{}, category{}, site{}, fatal{}, requested{}, available{}, cause{}, kind{}, has_return{}, transfer{}, failure_owner{};
    auto fields() const { return std::array{value, category, site, fatal, requested, available, cause, kind, has_return, transfer, failure_owner}; }
    auto failure_fields() const { return std::array{category, site, fatal, requested, available, cause, failure_owner}; }
    template <typename Transform> auto transform(const Transform &apply) const {
      return CompletionState<decltype(apply(value))>{apply(value), apply(category), apply(site), apply(fatal),
          apply(requested), apply(available), apply(cause), apply(kind), apply(has_return), transfer ? apply(transfer) : nullptr,
          failure_owner ? apply(failure_owner) : nullptr};
    }
  };
  CompletionState<llvm::AllocaInst *> completion_;
  struct NamedExceptionSite {
    size_t category;
    std::string_view spelling;
  };
  struct FailureSite {
    Location location;
    std::optional<NamedExceptionSite> nominal;
  };
  std::vector<FailureSite> sites_;
  llvm::Function *named_exception_site_ = nullptr;
  enum class Projection { value, target, slot, receiver };
  static bool carries_value(Projection projection) { return projection == Projection::value || projection == Projection::receiver; }
  struct DeferredInputs {
    size_t count;
    std::map<size_t, size_t> handle_arguments;
    auto operator<=>(const DeferredInputs &) const = default;
  };
  struct CleanupAction {
    const Expression *action;
    llvm::StructType *layout;
    std::set<size_t> captures;
    std::optional<size_t> native_constructor = {};
    llvm::AllocaInst *activation_record = nullptr;
    std::optional<size_t> native_symbol = {};
    std::optional<DeferredInputs> inputs = {};
    bool region_release = false;
    bool exception_release = false;
    bool release_only() const { return native_constructor.has_value() || region_release || exception_release; }
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
    std::vector<CleanupAction> actions;
    Lifetime values;
  };
  llvm::StructType *cleanup_header_ = llvm::StructType::get(context_, {b_.getPtrTy(), b_.getInt64Ty()});
  Scope *scope_ = nullptr;
  Scope *native_effect_scope_ = nullptr;
  struct Choice {
    std::vector<Type> inputs;
    const CheckedOperation *operation;
    std::vector<const CheckedInstance *> contexts = {};
    std::optional<size_t> nominal_callee = {};
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
    const auto nominal_base = 8 + program_->exception_types.size();
    if (type.kind == Type::product || type.kind == Type::native_handle) return nominal_base + type.identity;
    const auto array_base = nominal_base + program_->schemas.size();
    if (type.kind == Type::array) return array_base + type.identity;
    const auto enum_base = array_base + program_->arrays.size();
    if (type.kind == Type::enumeration) return enum_base + type.identity;
    const auto sum_base = enum_base + program_->enums.size();
    if (type.kind == Type::sum) return sum_base + type.identity;
    const auto text_base = sum_base + program_->sums.size();
    if (type.kind == Type::string) return text_base;
    if (type.kind == Type::c_string) return text_base + 1;
    if (type.kind == Type::procedure) {
      const auto found = std::find(callable_types_.begin(), callable_types_.end(), type);
      const auto id = std::distance(callable_types_.begin(), found);
      if (found == callable_types_.end()) callable_types_.push_back(type);
      return text_base + 2 + id;
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
    case Type::enumeration: return native_type(program_->enums.at(type.identity).backing);
    default: throw std::logic_error("non-scalar native payload");
    }
  }
  struct SumLayout {
    struct Alternative {
      llvm::StructType *payload;
      std::uint64_t tag;
    };
    llvm::StructType *storage;
    std::vector<Alternative> alternatives;
  };
  std::map<size_t, SumLayout> sum_layouts_;
  llvm::Type *known_inline_layout(const Type &type) {
    if (type.kind == Type::product) return product_layouts_.at(type.identity);
    if (type.kind == Type::sum) return sum_layouts_.at(type.identity).storage;
    return native_type(type);
  }
  void build_product_layout(size_t id) {
    std::vector<llvm::Type *> fields;
    for (const auto &field : program_->schemas.at(id).fields)
      fields.push_back(field.stores_target() ? b_.getPtrTy() : known_inline_layout(field.type));
    // Fieldless products still need distinct addressable targets.
    if (fields.empty()) fields.push_back(b_.getInt8Ty());
    product_layouts_.emplace(id, llvm::StructType::get(context_, fields));
  }
  void build_sum_layout(size_t id) {
    const auto &data = module_.getDataLayout();
    std::vector<SumLayout::Alternative> alternatives;
    llvm::Type *alignment_type = b_.getInt8Ty();
    std::uint64_t bytes = 0;
    for (const auto &alternative : program_->sums.at(id).alternatives) {
      std::vector<llvm::Type *> fields;
      for (const auto &field : alternative.fields)
        fields.push_back(field.stores_target() ? b_.getInt64Ty() : known_inline_layout(field.type));
      auto *payload = llvm::StructType::get(context_, fields);
      if (data.getABITypeAlign(payload) > data.getABITypeAlign(alignment_type)) alignment_type = payload;
      bytes = std::max(bytes, data.getTypeAllocSize(payload).getFixedValue());
      alternatives.push_back({payload, alternatives.size()});
    }
    // A zero-length array supplies the alignment of the most strictly aligned
    // alternative. Unused payload bytes have no value.
    auto *payload = llvm::StructType::get(context_, {
        llvm::ArrayType::get(alignment_type, 0), llvm::ArrayType::get(b_.getInt8Ty(), bytes)});
    auto *storage = llvm::StructType::get(context_, {b_.getInt64Ty(), payload});
    sum_layouts_.emplace(id, SumLayout{storage, std::move(alternatives)});
  }
  void ensure_inline_layout(const Type &type) {
    const auto known = [&](const Type &item) {
      return item.kind == Type::product ? product_layouts_.contains(item.identity) :
          item.kind == Type::sum ? sum_layouts_.contains(item.identity) : true;
    };
    if (known(type)) return;
    std::vector<std::pair<Type, bool>> pending{{type, false}};
    while (!pending.empty()) {
      auto [item, ready] = std::move(pending.back());
      pending.pop_back();
      if (known(item)) continue;
      if (ready) {
        if (item.kind == Type::product) build_product_layout(item.identity);
        else build_sum_layout(item.identity);
      } else {
        pending.emplace_back(item, true);
        program_->visit_inline_fields(item, [&](const auto &field) {
          if (!known(field.type)) pending.emplace_back(field.type, false);
        });
      }
    }
  }
  llvm::StructType *product_layout(size_t id) {
    ensure_inline_layout(Type{Type::product, {}, id});
    return product_layouts_.at(id);
  }
  llvm::StructType *regional_product_layout(size_t id) {
    auto [position, inserted] = regional_product_layouts_.try_emplace(id);
    if (inserted) {
      std::vector<llvm::Type *> fields;
      for (const auto &field : program_->schemas.at(id).fields)
        fields.push_back(field.stores_target() ? b_.getInt64Ty() : native_type(field.type));
      if (fields.empty()) fields.push_back(b_.getInt8Ty());
      position->second = llvm::StructType::get(context_, fields);
    }
    return position->second;
  }
  const SumLayout &sum_layout(size_t id) {
    ensure_inline_layout(Type{Type::sum, {}, id});
    return sum_layouts_.at(id);
  }
  llvm::Type *inline_layout(const Type &type) {
    ensure_inline_layout(type);
    return known_inline_layout(type);
  }
  llvm::Value *sum_field_address(llvm::Value *storage, SumConstructor constructor, size_t field) {
    const auto &layout = sum_layout(constructor.sum);
    auto *payload = b_.CreateStructGEP(layout.storage, storage, 1);
    return b_.CreateStructGEP(layout.alternatives.at(constructor.alternative).payload, payload, field);
  }
  std::map<std::pair<Type::Kind, size_t>, llvm::Function *> bits_transfers_;
  std::vector<Type> pending_bits_transfers_;
  llvm::Function *bits_transfer(const Type &type) {
    const auto key = std::pair{type.kind, type.identity};
    if (const auto found = bits_transfers_.find(key); found != bits_transfers_.end()) return found->second;
    auto *signature = llvm::FunctionType::get(b_.getVoidTy(), {b_.getPtrTy(), b_.getPtrTy()}, false);
    const auto kind = type.kind == Type::product ? "product." : "sum.";
    auto *function = llvm::Function::Create(signature, llvm::GlobalValue::InternalLinkage,
        "cod.bits.transfer." + std::string(kind) + std::to_string(type.identity), module_);
    bits_transfers_.emplace(key, function);
    pending_bits_transfers_.push_back(type);
    return function;
  }
  void emit_bits_transfers() {
    for (size_t i = 0; i < pending_bits_transfers_.size(); ++i) {
      const auto type = pending_bits_transfers_[i];
      auto *function = bits_transfers_.at({type.kind, type.identity});
      llvm::IRBuilder<> body(llvm::BasicBlock::Create(context_, "entry", function));
      auto *destination = function->getArg(0);
      auto *source = function->getArg(1);
      const auto fields = [&](llvm::StructType *layout, const auto &declarations, llvm::Value *to, llvm::Value *from) {
        for (size_t field = 0; field < declarations.size(); ++field) {
          const auto &member = declarations[field].type;
          auto *into = body.CreateStructGEP(layout, to, field);
          auto *out_of = body.CreateStructGEP(layout, from, field);
          if (scalar_type(member)) body.CreateStore(body.CreateLoad(native_type(member), out_of), into);
          else body.CreateCall(bits_transfer(member), {into, out_of});
        }
      };
      if (type.kind == Type::product) {
        fields(product_layout(type.identity), program_->schemas.at(type.identity).fields, destination, source);
        body.CreateRetVoid();
        continue;
      }

      const auto &layout = sum_layout(type.identity);
      const auto &alternatives = program_->sums.at(type.identity).alternatives;
      auto *tag = body.CreateLoad(body.getInt64Ty(), body.CreateStructGEP(layout.storage, source, 0));
      auto *from = body.CreateStructGEP(layout.storage, source, 1);
      auto *to = body.CreateStructGEP(layout.storage, destination, 1);
      auto *invalid = llvm::BasicBlock::Create(context_, "invalid.tag", function);
      auto *done = llvm::BasicBlock::Create(context_, "payload.ready", function);
      auto *select = body.CreateSwitch(tag, invalid, alternatives.size());
      body.SetInsertPoint(invalid);
      body.CreateUnreachable();
      for (size_t n = 0; n < alternatives.size(); ++n) {
        const auto &alternative = layout.alternatives[n];
        auto *active = llvm::BasicBlock::Create(context_, "active.payload", function);
        select->addCase(body.getInt64(alternative.tag), active);
        body.SetInsertPoint(active);
        fields(alternative.payload, alternatives[n].fields, to, from);
        body.CreateBr(done);
      }
      body.SetInsertPoint(done);
      body.CreateStore(tag, body.CreateStructGEP(layout.storage, destination, 0));
      body.CreateRetVoid();
    }
  }
  std::uint64_t array_prefix(llvm::Type *element) const {
    const auto &layout = module_.getDataLayout();
    return llvm::alignTo(layout.getTypeAllocSize(array_header_).getFixedValue(), layout.getABITypeAlign(element));
  }
  llvm::Value *array_length(llvm::Value *array) {
    return b_.CreateLoad(b_.getInt64Ty(), b_.CreateStructGEP(array_header_, array, 0), "array.length");
  }
  llvm::Value *array_data(llvm::Value *array, llvm::Type *element) {
    auto *kind = b_.CreateLoad(b_.getInt8Ty(), b_.CreateStructGEP(array_header_, array, 1), "array.backing");
    auto *direct = block("array.inline"), *loan = block("array.external"), *join = block("array.data");
    b_.CreateCondBr(b_.CreateICmpEQ(kind, b_.getInt8(static_cast<unsigned>(ArrayBacking::inline_elements))), direct, loan);
    b_.SetInsertPoint(direct);
    auto *inline_data = b_.CreateGEP(b_.getInt8Ty(), array, integer(array_prefix(element)));
    b_.CreateBr(join);
    b_.SetInsertPoint(loan);
    auto *external = b_.CreateLoad(b_.getPtrTy(), b_.CreateStructGEP(borrowed_array_, array, 1));
    b_.CreateBr(join);
    b_.SetInsertPoint(join);
    auto *data = b_.CreatePHI(b_.getPtrTy(), 2);
    data->addIncoming(inline_data, direct);
    data->addIncoming(external, loan);
    return data;
  }
  llvm::Value *native_array_loan(llvm::Value *array, const Type &type) {
    auto *loan = slot(native_array_view_, "native.array.loan");
    auto *element = inline_layout(program_->arrays.at(type.identity).element);
    b_.CreateStore(array_data(array, element), b_.CreateStructGEP(native_array_view_, loan, 0));
    b_.CreateStore(array_length(array), b_.CreateStructGEP(native_array_view_, loan, 1));
    return loan;
  }
  llvm::Value *array_capability(const CheckedOperation::ArrayAllocator &allocator) {
    using Allocation = CheckedOperation::ArrayAllocator;
    if (allocator.kind == Allocation::input) return array_capabilities_.at(allocator.place);
    const bool region = allocator.kind == Allocation::region;
    auto *backing = region ? load(storage_.at(allocator.place)) : lexical_allocation_owner();
    auto *capability = b_.CreateInsertValue(llvm::PoisonValue::get(array_capability_), b_.getInt1(region), 0);
    return b_.CreateInsertValue(capability, backing, 1);
  }
  llvm::Value *reserve_region_extent(llvm::Value *backing, llvm::Value *bytes, std::uint64_t alignment,
                                     const Location &at) {
    auto reserve = runtime("cod_lang_region_reserve", b_.getInt64Ty(),
        {b_.getPtrTy(), b_.getInt64Ty(), b_.getInt64Ty()});
    auto *offset = b_.CreateCall(reserve, {backing, bytes, integer(alignment)});
    auto *reserved = block("region.extent.reserved"), *full = block("region.extent.full");
    b_.CreateCondBr(b_.CreateICmpNE(offset, integer(UINT64_MAX)), reserved, full);
    b_.SetInsertPoint(full);
    auto remaining = runtime("cod_lang_region_remaining", b_.getInt64Ty(), {b_.getPtrTy()});
    auto *available = b_.CreateCall(remaining, {backing});
    save_abrupt(outcome(empty_scalar(), integer(exception_id(ExceptionCategory::region_capacity_exceeded)),
                        integer(site(at)), null_pointer(), bytes, available));
    b_.CreateBr(control_->exit);
    b_.SetInsertPoint(reserved);
    return offset;
  }
  llvm::Value *allocate_array(llvm::Value *capability, llvm::Value *bytes, std::uint64_t alignment, const Location &at) {
    auto *regional = b_.CreateExtractValue(capability, 0);
    auto *backing = b_.CreateExtractValue(capability, 1);
    auto *region = block("array.region"), *lexical = block("array.lexical"), *ready = block("array.allocation.ready");
    b_.CreateCondBr(regional, region, lexical);
    b_.SetInsertPoint(region);
    auto *offset = reserve_region_extent(backing, bytes, alignment, at);
    auto *regional_storage = region_address(backing, offset, bytes);
    auto *regional_predecessor = b_.GetInsertBlock();
    b_.CreateBr(ready);

    b_.SetInsertPoint(lexical);
    auto allocate = runtime("cod_lang_lexical_extent_allocate", b_.getPtrTy(),
        {b_.getPtrTy(), b_.getInt64Ty(), b_.getInt64Ty()});
    auto *lexical_storage = b_.CreateCall(allocate, {backing, bytes, integer(alignment)});
    auto *unavailable = block("array.lexical.unavailable");
    b_.CreateCondBr(b_.CreateIsNotNull(lexical_storage), ready, unavailable);
    b_.SetInsertPoint(unavailable);
    fail_path(ExceptionCategory::out_of_memory, at);
    b_.SetInsertPoint(ready);
    auto *array = b_.CreatePHI(b_.getPtrTy(), 2);
    array->addIncoming(regional_storage, regional_predecessor);
    array->addIncoming(lexical_storage, lexical);
    return array;
  }
  llvm::Value *emit_array_construction(const Expression &expr, bool discard) {
    const auto &length = *expr.children.front();
    const auto &fill = *expr.children.back();
    auto *count_value = emit_expression(length);
    if (!reachable()) return nullptr;

    using Plan = std::tuple<CheckedOperation::ArrayAllocator, size_t, StorageFormat, StorageFormat>;
    std::map<Plan, std::vector<size_t>> alternatives;
    for (size_t i = 0; i < contexts_.size(); ++i) {
      const auto &context = contexts_[i];
      const auto &fact = context.at(&expr);
      if (context.at(&length).type == Type::never) continue;
      alternatives[{*fact.array_allocator, fact.operand_type.identity, StorageFormat{context.at(&length).type},
                    StorageFormat{context.at(&fill).type}}].push_back(i);
    }
    auto *view = contextual_value(expr, alternatives, "array.construct", [&](const auto &selection) -> llvm::Value * {
      const auto &[allocator, schema, length_format, fill_format] = selection;
      const auto &array = program_->arrays.at(schema);
      const bool aggregate = inline_aggregate_type(array.element);
      auto *fill_value = emit_expression(fill, aggregate ? Projection::target : Projection::value);
      if (!reachable()) return nullptr;
      if (aggregate) fill_value = checked_address(fill_value, fill, fill.at);
      auto *element = inline_layout(array.element);
      const auto &layout = module_.getDataLayout();
      const auto element_alignment = layout.getABITypeAlign(element);
      const auto alignment = std::max(element_alignment, layout.getABITypeAlign(array_header_));
      const auto stride = layout.getTypeAllocSize(element).getFixedValue();
      const auto prefix = array_prefix(element);
      constexpr auto limit = std::uint64_t{std::numeric_limits<std::ptrdiff_t>::max()};
      if (alignment.value() > alignof(std::max_align_t) || !stride || prefix > limit)
        implementation_gap(expr.at, "target layout for array backing");

      auto *count = convert(count_value, length_format.type(), Type::uint64);
      auto *capability = array_capability(allocator);
      auto *fits = block("array.size.valid"), *overflow = block("array.size.overflow");
      b_.CreateCondBr(b_.CreateICmpULE(count, integer((limit - prefix) / stride)), fits, overflow);
      b_.SetInsertPoint(overflow);
      auto *invalid = block("array.region.unrepresentable"), *unavailable = block("array.lexical.unrepresentable");
      b_.CreateCondBr(b_.CreateExtractValue(capability, 0), invalid, unavailable);
      b_.SetInsertPoint(invalid);
      fail_path(ExceptionCategory::invalid_capacity, expr.at);
      b_.SetInsertPoint(unavailable);
      fail_path(ExceptionCategory::out_of_memory, expr.at);
      b_.SetInsertPoint(fits);
      auto *bytes = b_.CreateAdd(integer(prefix), b_.CreateMul(count, integer(stride)));
      auto *storage = allocate_array(capability, bytes, alignment.value(), expr.at);
      auto *ready = b_.GetInsertBlock();
      auto *data = b_.CreateGEP(b_.getInt8Ty(), storage, integer(prefix));
      auto *value = aggregate ? fill_value : fill_format.kind == Type::enumeration ? unpack(fill_value, array.element)
          : convert(fill_value, fill_format.type(), array.element);
      auto *test = block("array.fill.test"), *store = block("array.fill.store"), *complete = block("array.initialized");
      b_.CreateBr(test);
      b_.SetInsertPoint(test);
      auto *index = b_.CreatePHI(b_.getInt64Ty(), 2, "array.fill.index");
      index->addIncoming(integer(0), ready);
      b_.CreateCondBr(b_.CreateICmpULT(index, count), store, complete);
      b_.SetInsertPoint(store);
      auto *place = b_.CreateGEP(element, data, index);
      if (aggregate) b_.CreateCall(bits_transfer(array.element), {place, value});
      else b_.CreateStore(value, place);
      auto *next = b_.CreateAdd(index, integer(1));
      index->addIncoming(next, store);
      b_.CreateBr(test);
      b_.SetInsertPoint(complete);
      b_.CreateStore(count, b_.CreateStructGEP(array_header_, storage, 0));
      b_.CreateStore(b_.getInt8(static_cast<unsigned>(ArrayBacking::inline_elements)),
                     b_.CreateStructGEP(array_header_, storage, 1));
      return storage;
    }, b_.getPtrTy());
    return discard ? empty_scalar() : view;
  }
  llvm::Value *pack(llvm::IRBuilder<> &builder, llvm::Value *value, const Type &type) {
    if (type.kind == Type::exception) {
      auto *result = builder.CreateInsertValue(empty_scalar(), value, 2);
      return builder.CreateInsertValue(result, builder.getInt32(tag(type)), 1);
    }
    if (floating(type)) value = builder.CreateBitCast(value, builder.getIntNTy(type == Type::float32 ? 32 : 64));
    value = builder.CreateZExtOrTrunc(value, builder.getInt64Ty());
    auto *result = builder.CreateInsertValue(empty_scalar(), value, 0);
    return builder.CreateInsertValue(result, builder.getInt32(tag(type)), 1);
  }
  llvm::Value *pack(llvm::Value *value, const Type &type) { return pack(b_, value, type); }
  llvm::Value *unpack(llvm::Value *value, const Type &type) {
    if (type.kind == Type::exception) return b_.CreateExtractValue(value, 2);
    value = b_.CreateExtractValue(value, 0);
    if (floating(type)) {
      value = b_.CreateTruncOrBitCast(value, b_.getIntNTy(type == Type::float32 ? 32 : 64));
      return b_.CreateBitCast(value, native_type(type));
    }
    return b_.CreateTruncOrBitCast(value, native_type(type));
  }
  // Views address the payload. Only non-primitive lexical values keep their
  // transport envelope in storage. A polymorphic slot can reserve space for
  // several formats, but each admitted activation accesses one checked type.
  llvm::Value *load_place(llvm::IRBuilder<> &builder, llvm::Value *address, StorageFormat format) {
    if (format.kind == Type::void_type) return builder.CreateLoad(scalar_, address);
    const auto type = format.type();
    return pack(builder, builder.CreateLoad(native_type(type), address), type);
  }
  void store_place(llvm::Value *address, llvm::Value *value, StorageFormat format) {
    b_.CreateStore(format.kind == Type::void_type ? value : unpack(value, format.type()), address);
  }
  void store_incoming(size_t place, llvm::Value *value) {
    auto *address = storage_.at(place);
    const auto &formats = storage_formats_.at(place);
    if (formats.size() == 1) {
      store_place(address, value, *formats.begin());
      return;
    }
    auto *done = block("storage.initialized");
    auto *fallback = formats.contains(StorageFormat{}) ? block("storage.managed") : done;
    auto *select = b_.CreateSwitch(b_.CreateExtractValue(value, 1), fallback,
                                   formats.size() - formats.contains(StorageFormat{}));
    for (const auto format : formats) if (format.kind != Type::void_type) {
      auto *selected = block("storage.bits");
      select->addCase(b_.getInt32(tag(format.type())), selected);
      b_.SetInsertPoint(selected);
      store_place(address, value, format);
      b_.CreateBr(done);
    }
    if (fallback != done) {
      b_.SetInsertPoint(fallback);
      store_place(address, value, StorageFormat{});
      b_.CreateBr(done);
    }
    b_.SetInsertPoint(done);
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
    if (from == to || from.kind == Type::exception || supplies_enum_code(from, to)) return value;
    if (floating(from)) return b_.CreateFPExt(value, native_type(to));
    if (floating(to)) return from == Type::integer ? b_.CreateSIToFP(value, native_type(to)) : b_.CreateUIToFP(value, native_type(to));
    return b_.CreateZExt(value, native_type(to));
  }
  llvm::FunctionCallee runtime(const std::string &name, llvm::Type *result, std::vector<llvm::Type *> arguments) {
    return module_.getOrInsertFunction(name, llvm::FunctionType::get(result, arguments, false));
  }
  std::uint64_t site(const Location &at, std::optional<NamedExceptionSite> nominal = {}) {
    sites_.push_back({at, nominal});
    return sites_.size();
  }
  llvm::Value *take_failure_owner(llvm::IRBuilder<> &builder, llvm::Value *place) {
    auto *owner = builder.CreateLoad(builder.getPtrTy(), place);
    builder.CreateStore(null_pointer(), place);
    return owner;
  }
  void release_failure_owner(llvm::IRBuilder<> &builder, llvm::Value *owner) {
    if (owner) builder.CreateCall(runtime("cod_lang_release_exception_region", builder.getVoidTy(), {builder.getPtrTy()}), {owner});
  }
  llvm::Value *exception_payload(llvm::Value *owner) {
    return b_.CreateCall(runtime("cod_lang_exception_payload", b_.getPtrTy(), {b_.getPtrTy()}), {owner});
  }
  llvm::StructType *exception_input_layout() {
    return llvm::StructType::get(context_, {b_.getPtrTy(), b_.getPtrTy()});
  }
  void clear_failure_owner() {
    if (owned_failures_) release_failure_owner(b_, take_failure_owner(b_, completion_.failure_owner));
  }
  llvm::Value *here_payload_slot(llvm::IRBuilder<> &builder, llvm::Value *transfer) {
    return builder.CreateStructGEP(here_transfer_, transfer, 1);
  }
  llvm::Value *transfer_failure_slot(llvm::IRBuilder<> &builder, llvm::Value *transfer) {
    return builder.CreateStructGEP(outcome_, here_payload_slot(builder, transfer), failure_owner_field());
  }
  void discard_transfer_failure(llvm::Value *transfer, llvm::Value *retained) {
    if (!owned_failures_) return;
    auto *release = block("here.release.failure"), *done = block("here.failure.released");
    b_.CreateCondBr(b_.CreateAnd(b_.CreateIsNotNull(transfer), b_.CreateICmpNE(transfer, retained)), release, done);
    b_.SetInsertPoint(release);
    release_failure_owner(b_, take_failure_owner(b_, transfer_failure_slot(b_, transfer)));
    b_.CreateBr(done);
    b_.SetInsertPoint(done);
  }
  llvm::Value *outcome(llvm::Value *value, llvm::Value *category, llvm::Value *where, llvm::Value *fatal,
      llvm::Value *requested = nullptr, llvm::Value *available = nullptr, llvm::Value *cause = nullptr,
      llvm::Value *transfer = nullptr, llvm::Value *failure_owner = nullptr, llvm::Value *resumes_case = nullptr) {
    llvm::Value *result = llvm::UndefValue::get(outcome_);
    result = b_.CreateInsertValue(result, value, 0);
    result = b_.CreateInsertValue(result, category, 1);
    result = b_.CreateInsertValue(result, where, 2);
    result = b_.CreateInsertValue(result, fatal, 3);
    result = b_.CreateInsertValue(result, requested ? requested : integer(0), 4);
    result = b_.CreateInsertValue(result, available ? available : integer(0), 5);
    result = b_.CreateInsertValue(result, cause ? cause : integer(0), 6);
    if (protected_outcomes_) result = b_.CreateInsertValue(result, transfer ? transfer : null_pointer(), 7);
    if (owned_failures_) result = b_.CreateInsertValue(result, failure_owner ? failure_owner : null_pointer(), failure_owner_field());
    if (case_scope_outcomes_) result = b_.CreateInsertValue(result, resumes_case ? resumes_case : b_.getFalse(), case_scan_field());
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
    auto *previous = outer;
    auto *kind = load(completion_.kind);
    if (here_destination_) {
      // An explicit continue in the group's own cleanup completes that
      // invocation before its earlier transfer has left the group.
      auto *local_feedback = b_.CreateAnd(b_.CreateICmpEQ(kind, b_.getInt8(1)), b_.CreateICmpEQ(outer, here_destination_));
      outer = b_.CreateSelect(local_feedback, null_pointer(), outer);
    }
    auto *inner = load(completion_.transfer);
    auto *pending = b_.CreateSelect(b_.CreateIsNull(inner), outer, inner);
    discard_transfer_failure(previous, pending);
    b_.CreateStore(pending, completion_.transfer);
    auto *leaves = b_.CreateAnd(b_.CreateIsNotNull(pending), b_.CreateICmpNE(kind, b_.getInt8(2)));
    b_.CreateStore(b_.CreateSelect(leaves, b_.getInt8(3), kind), completion_.kind);
  }
  void save_abrupt(llvm::Value *result) {
    if (owned_failures_) {
      clear_failure_owner();
      b_.CreateStore(b_.CreateExtractValue(result, failure_owner_field()), completion_.failure_owner);
    }
    b_.CreateStore(b_.CreateExtractValue(result, 1), completion_.category);
    b_.CreateStore(b_.CreateExtractValue(result, 2), completion_.site);
    b_.CreateStore(b_.CreateExtractValue(result, 3), completion_.fatal);
    b_.CreateStore(b_.CreateExtractValue(result, 4), completion_.requested);
    b_.CreateStore(b_.CreateExtractValue(result, 5), completion_.available);
    b_.CreateStore(b_.CreateExtractValue(result, 6), completion_.cause);
    if (protected_outcomes_) {
      auto *incoming = b_.CreateExtractValue(result, 7);
      auto *previous = load(completion_.transfer);
      auto *retained = b_.CreateSelect(b_.CreateIsNotNull(incoming), incoming, previous);
      discard_transfer_failure(previous, retained);
      b_.CreateStore(retained, completion_.transfer);
      b_.CreateStore(b_.CreateSelect(failed(result), b_.getInt8(2), b_.getInt8(3)), completion_.kind);
    } else b_.CreateStore(b_.getInt8(2), completion_.kind);
  }
  void save_local_completion(llvm::Value *value, const Location &at, unsigned kind, bool returned) {
    assign_value(completion_.value, value, at);
    clear_failure_owner();
    b_.CreateStore(integer(0), completion_.category);
    b_.CreateStore(null_pointer(), completion_.fatal);
    b_.CreateStore(b_.getInt1(returned), completion_.has_return);
    b_.CreateStore(b_.getInt8(kind), completion_.kind);
  }
  void save_return(llvm::Value *value, const Location &at) {
    save_local_completion(value, at, 1, !here_destination_);
  }
  llvm::Value *observed_return() {
    if (!suspended_return_.present) throw std::logic_error("return inspection has no suspended completion");
    return b_.CreateOr(suspended_return_.present, load(completion_.has_return));
  }
  llvm::StructType *here_return_header() {
    return llvm::StructType::get(context_, {here_transfer_, scalar_});
  }
  llvm::Value *protected_return_value() {
    auto *packet = b_.CreateLoad(outcome_, b_.CreateStructGEP(here_transfer_, here_destination_, 1));
    return b_.CreateExtractValue(packet, 0);
  }
  llvm::Value *observed_return_value() {
    if (!suspended_return_.value) throw std::logic_error("return binding has no suspended result");
    auto *prepared = here_destination_ ? protected_return_value() : load(completion_.value);
    return b_.CreateSelect(load(completion_.has_return), prepared, suspended_return_.value);
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
  void raise_exception(llvm::Value *value, llvm::Value *owner = nullptr) {
    save_abrupt(outcome(empty_scalar(), b_.CreateExtractValue(value, 0), b_.CreateExtractValue(value, 1), null_pointer(),
        b_.CreateExtractValue(value, 2), b_.CreateExtractValue(value, 3), b_.CreateExtractValue(value, 4), nullptr, owner));
    b_.CreateBr(control_->exit);
  }
  template <typename Initialize>
  void raise_constructed_exception(const Location &at, size_t category, const Initialize &emit_fields,
                                   std::string_view spelling = {}) {
    auto *layout = product_layout(*program_->exception_types.at(category).schema);
    auto *capacity = integer(COD_LANG_EXCEPTION_REGION_DEFAULT);
    auto *size = llvm::ConstantExpr::getSizeOf(layout);
    auto *alignment = integer(module_.getDataLayout().getABITypeAlign(layout).value());
    auto *allocate = block("exception.allocate"), *insufficient = block("exception.capacity");
    b_.CreateCondBr(b_.CreateICmpULE(size, capacity), allocate, insufficient);
    b_.SetInsertPoint(insufficient);
    save_abrupt(outcome(empty_scalar(), integer(exception_id(ExceptionCategory::region_capacity_exceeded)),
                        integer(site(at)), null_pointer(), size, capacity));
    b_.CreateBr(control_->exit);
    b_.SetInsertPoint(allocate);
    auto acquire = runtime("cod_lang_acquire_exception_region", b_.getPtrTy(), std::vector<llvm::Type *>(3, b_.getInt64Ty()));
    auto *owner = b_.CreateCall(acquire, {capacity, size, alignment}, "exception.owner");
    auto *initialize = block("exception.initialize"), *unavailable = block("exception.unavailable");
    b_.CreateCondBr(b_.CreateIsNotNull(owner), initialize, unavailable);
    b_.SetInsertPoint(unavailable);
    fail_path(ExceptionCategory::out_of_memory, at);
    b_.SetInsertPoint(initialize);
    Control construction{control_, block("exception.abandoned")};
    control_ = &construction;
    emit_fields(layout, exception_payload(owner));
    control_ = construction.parent;
    if (reachable()) {
      const auto origin = spelling.empty() ? site(at) : site(at, NamedExceptionSite{category, spelling});
      save_abrupt(outcome(empty_scalar(), integer(category), integer(origin), null_pointer(),
                          nullptr, nullptr, nullptr, nullptr, owner));
      b_.CreateBr(control_->exit);
    }
    b_.SetInsertPoint(construction.exit);
    release_failure_owner(b_, owner);
    b_.CreateBr(control_->exit);
  }
  llvm::Value *construct_exception(const Expression &expr, size_t category) {
    raise_constructed_exception(expr.at, category, [&](llvm::StructType *layout, llvm::Value *payload) {
      for (const auto &initializer : expr.children) {
        const auto &input = *initializer->children.front();
        auto *value = emit_expression(input);
        if (!reachable()) break;
        const auto index = *contexts_.front().at(initializer.get()).product_field;
        assign_place(b_.CreateStructGEP(layout, payload, index), convert_result(*initializer, input, value),
                     *initializer, &CheckedOperation::operand_type);
      }
    }, expr.annotation->name);
    return nullptr;
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
    const auto add = [&](Choice choice, const CheckedInstance *instance) {
      const auto same = [&](const Choice &other) {
        return other.inputs == choice.inputs && other.nominal_callee == choice.nominal_callee &&
            other.operation->callee == choice.operation->callee &&
            other.operation->symbol == choice.operation->symbol &&
            other.operation->type == choice.operation->type && other.operation->operand_type == choice.operation->operand_type &&
            other.operation->arguments == choice.operation->arguments && other.operation->builtin == choice.operation->builtin &&
            other.operation->builtin_name == choice.operation->builtin_name;
      };
      auto found = std::find_if(result.begin(), result.end(), same);
      if (found == result.end()) { result.push_back(std::move(choice)); found = std::prev(result.end()); }
      if (instance) found->contexts.push_back(instance);
    };
    for (const auto &context : contexts_) {
      if (std::any_of(inputs.begin(), inputs.end(), [&](const auto *input) { return context.at(input).type == Type::never; })) continue;
      const auto &operation = context.at(&expr);
      if (prepared && !operation.nominal_calls.empty()) {
        for (const auto &call : operation.nominal_calls)
          add(Choice{call.inputs, &operation, {}, call.callee}, context.instance);
        continue;
      }
      Choice choice{{}, &operation};
      for (size_t p = 0; p < inputs.size(); ++p)
        choice.inputs.push_back(prepared ? choice.operation->arguments.at(p).type : context.at(inputs[p]).type);
      add(std::move(choice), context.instance);
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
  using PlaceType = Type CheckedOperation::*;
  template <typename Access>
  llvm::Value *access_place(const Expression &expr, PlaceType type, llvm::Type *result, const Access &access) {
    auto cases = choices(expr, {});
    std::erase_if(cases, [&](const auto &choice) { return choice.operation->*type == Type::never; });
    if (cases.empty()) throw std::logic_error("no checked payload type for reachable storage access");
    const auto format = [&](const auto &choice) { return StorageFormat{choice.operation->*type}; };
    const auto first = format(cases.front());
    if (std::all_of(cases.begin(), cases.end(), [&](const auto &choice) { return format(choice) == first; }))
      return access(first);
    return choose(expr, {}, cases, result, [&](const auto &choice) { return access(format(choice)); });
  }
  llvm::Value *load_place(llvm::Value *address, const Expression &expr, PlaceType type = &CheckedOperation::type) {
    return access_place(expr, type, scalar_, [&](auto format) { return load_place(b_, address, format); });
  }
  void assign_place(llvm::Value *address, llvm::Value *value, const Expression &expr,
                    PlaceType type = &CheckedOperation::type) {
    access_place(expr, type, b_.getInt1Ty(), [&](auto format) {
      if (format.kind == Type::void_type) assign_value(address, value, expr.at);
      else store_place(address, value, format);
      return b_.getTrue();
    });
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
      auto *checked = llvm::unwrap(cod_llvm_build_checked_integer(
          llvm::wrap(&b_), llvm::wrap(&module_), intrinsic, llvm::wrap(left), llvm::wrap(right)));
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
  llvm::AllocaInst *lexical_allocation_owner() {
    if (!lexical_allocations_) {
      lexical_allocations_ = slot(b_.getPtrTy(), "owner.allocations");
      llvm::IRBuilder<> entry(lexical_allocations_->getParent(), std::next(lexical_allocations_->getIterator()));
      entry.CreateStore(null_pointer(), lexical_allocations_);
    }
    return lexical_allocations_;
  }
  llvm::Value *owner_identity(size_t owner, const Location &at) {
    // In a program with owner-directed allocation, the full scope's identity also
    // carries its allocation head to calls with a proved lexical destination.
    if (owner == 0 && owns_lexical_allocations_) return lexical_allocation_owner();
    if (const auto found = owner_identities_.find(owner); found != owner_identities_.end()) return found->second;
    if (is_symbol_owner(owner, program_->symbols.size())) {
      // A selected incoming view can survive a shadow in this activation.
      auto *binding = symbols_.at(owner - first_symbol_owner);
      llvm::IRBuilder<> entry(&function_->getEntryBlock(), function_->getEntryBlock().begin());
      auto *selected = entry.CreateLoad(binding->getValueType(), binding, "effect.incoming");
      auto *identity = entry.CreateExtractValue(selected, 1, "effect.owner");
      owner_identities_.emplace(owner, identity);
      return identity;
    }
    if (owner > null_target / 2) implementation_gap(at, "runtime identity for an owner without an input-bound actual context");
    // Admitted lexical cells have stable native storage. One activation-local
    // address names their owner; it is shared across all views and forwarded calls.
    auto *identity = slot(b_.getInt8Ty(), "owner.identity");
    owner_identities_.emplace(owner, identity);
    return identity;
  }
  llvm::Value *selected_owner(const Expression &expr, const Expression *call = nullptr,
                              std::optional<size_t> method = {}, const Type *callable_view = nullptr,
                              std::optional<size_t> argument = {}) {
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
      if (argument) {
        const auto &storage = contexts_[i].at(call).arguments.at(*argument).reference_storage;
        if (storage) {
          alternatives[storage->owner].push_back(i);
          continue;
        }
      }
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
    if (!entry.group && body.places.empty() && body.owners.empty() && body.lexical_here.empty())
      return b_.CreateInsertValue(value, null_pointer(), 1);
    auto *activation = slot(entry.activation, "here.activation");
    const auto &method = program_->methods[program_->instances[body.instance].method];
    if (entry.group && owned_failures_) b_.CreateStore(llvm::Constant::getNullValue(here_transfer_), activation);
    if (method.here_group && method.here_group->binds_return_payload) {
      if (!owned_failures_) b_.CreateStore(llvm::Constant::getNullValue(here_transfer_), activation);
      b_.CreateStore(observed_return_value(), b_.CreateStructGEP(here_return_header(), activation, 1));
    }
    const auto &inputs = *method.scope_inputs;
    const auto first_capture = inputs.here_reception->selected.size();
    for (size_t i = 0; i < body.places.size(); ++i) {
      const auto &place = *body.places[i];
      const auto projection = inputs.live_bindings.contains(first_capture + i) ? Projection::slot : Projection::target;
      auto *address = emit_expression(place, projection);
      b_.CreateStore(pack_view(address, place), b_.CreateStructGEP(entry.activation, activation, entry.group + i));
    }
    for (size_t i = 0; i < body.owners.size(); ++i)
      b_.CreateStore(owner_identity(body.owners[i], at), b_.CreateStructGEP(entry.activation, activation, entry.group + body.places.size() + i));
    const auto here_begin = entry.group + body.places.size() + body.owners.size();
    for (size_t i = 0; i < body.lexical_here.size(); ++i)
      b_.CreateStore(here_arguments_.at(body.lexical_here[i]), b_.CreateStructGEP(entry.activation, activation, here_begin + i));
    return b_.CreateInsertValue(value, activation, 1);
  }
  llvm::StructType *native_handle_pair() {
    return llvm::StructType::get(context_, {b_.getInt64Ty(), b_.getInt64Ty()});
  }
  llvm::AllocaInst *native_reborrow(llvm::Value *resource) {
    auto *pair = native_handle_pair();
    auto *view = slot(pair, "native.reborrow");
    for (unsigned word = 0; word < 2; ++word) b_.CreateStore(resource, b_.CreateStructGEP(pair, view, word));
    return view;
  }
  llvm::Value *native_scalar_argument(const Expression &operand, const Type &type) {
    auto *value = emit_expression(operand);
    if (!reachable()) return nullptr;
    auto *boundary = c_scalar_type(type);
    return choose(operand, {value}, choices(operand, {&operand}), boundary, [&](const Choice &choice) {
      auto *converted = convert(value, choice.inputs.front(), type);
      return type == Type::boolean ? b_.CreateZExt(converted, boundary) : converted;
    });
  }
  llvm::Value *emit_native_call(const Expression &expr, const std::vector<llvm::Value *> *prepared = nullptr) {
    std::map<size_t, std::vector<size_t>> alternatives;
    for (size_t i = 0; i < contexts_.size(); ++i)
      alternatives[*contexts_[i].at(&expr).native_call].push_back(i);
    return contextual_value(expr, alternatives, "native.call", [&](size_t id) -> llvm::Value * {
      const auto &native = program_->native_operations.at(id);
      std::vector<llvm::Value *> arguments;
      std::vector<llvm::Type *> types;
      for (size_t i = 0; i < native.inputs.size(); ++i) {
        const auto &input = native.inputs[i];
        const auto &operand = *expr.children[i];
        llvm::Value *argument;
        if (prepared) {
          argument = input.owner ? view_address(prepared->at(i)) : unpack(prepared->at(i), input.type);
          if (input.type == Type::boolean) argument = b_.CreateZExt(argument, b_.getInt8Ty());
        } else if (input.owner) {
          argument = emit_expression(operand, Projection::target);
          if (!reachable()) return nullptr;
        } else {
          argument = native_scalar_argument(operand, input.type);
          if (!reachable()) return nullptr;
        }
        if (input.type.kind == Type::array)
          argument = native_array_loan(checked_address(argument, operand, operand.at), input.type);
        arguments.push_back(argument);
        types.push_back(argument->getType());
      }
      llvm::AllocaInst *value = nullptr;
      const auto &result = *native.contract.result.type;
      if (result != Type::void_type) {
        value = slot(c_scalar_type(result), "native.call.value");
        arguments.push_back(value);
        types.push_back(b_.getPtrTy());
      }
      auto *status = b_.CreateCall(runtime(native_operation_name(native) + "_call", b_.getInt32Ty(), types), arguments);
      for (size_t i = 0; i < native.inputs.size(); ++i)
        if (types[i] == b_.getInt8Ty()) status->addParamAttr(i, llvm::Attribute::ZExt);
      auto *ready = block("native.call.success"), *invalid = block("native.call.invalid");
      b_.CreateCondBr(b_.CreateICmpEQ(status, b_.getInt32(static_cast<unsigned>(NativeStatus::ok))), ready, invalid);
      b_.SetInsertPoint(invalid);
      fail_path(ExceptionCategory::system, expr.at, b_.CreateGlobalString(location_text(expr.at) + ": invalid native provider status"));
      b_.SetInsertPoint(ready);
      if (!value) return empty_scalar();
      auto *native_result = load(value);
      if (result == Type::boolean) return pack(b_.CreateTrunc(native_result, b_.getInt1Ty()), result);
      return pack(native_result, result);
    });
  }
  llvm::Function *native_here_bridge(const CheckedNativeOperation &native, llvm::StructType *frame) {
    const auto name = native_operation_name(native) + "_bridge";
    if (auto *existing = module_.getFunction(name)) return existing;
    auto *pointer = b_.getPtrTy();
    auto *bridge = llvm::Function::Create(llvm::FunctionType::get(b_.getInt32Ty(), {pointer, pointer, pointer}, false),
        llvm::GlobalValue::InternalLinkage, name, module_);
    llvm::IRBuilder<> body(llvm::BasicBlock::Create(context_, "entry", bridge));
    auto *state = bridge->getArg(0);
    auto *views = bridge->getArg(1);
    auto *arrays = llvm::StructType::get(context_, std::vector<llvm::Type *>(native.arrays().size(), native_array_view_));
    auto *callback = body.CreateLoad(here_, body.CreateStructGEP(frame, state, 0));
    std::vector<llvm::Value *> arguments;
    for (size_t i = 0; i < native.arrays().size(); ++i) {
      auto *native_view = body.CreateStructGEP(arrays, views, i);
      auto *external = body.CreateAlloca(borrowed_array_, nullptr, "native.array.header");
      auto *header = body.CreateStructGEP(borrowed_array_, external, 0);
      auto *data = body.CreateLoad(pointer, body.CreateStructGEP(native_array_view_, native_view, 0));
      auto *count = body.CreateLoad(body.getInt64Ty(), body.CreateStructGEP(native_array_view_, native_view, 1));
      body.CreateStore(count, body.CreateStructGEP(array_header_, header, 0));
      body.CreateStore(body.getInt8(static_cast<unsigned>(ArrayBacking::foreign_loan)), body.CreateStructGEP(array_header_, header, 1));
      body.CreateStore(data, body.CreateStructGEP(borrowed_array_, external, 1));
      auto *view = body.CreateInsertValue(empty_scalar(), body.CreatePtrToInt(external, body.getInt64Ty()), 0);
      arguments.push_back(body.CreateInsertValue(view, body.getInt32(tag(native.arrays()[i].type)), 1));
    }
    arguments.push_back(body.CreateLoad(pointer, body.CreateStructGEP(frame, state, 1)));
    arguments.push_back(body.CreateExtractValue(callback, 1));
    std::vector<llvm::Type *> types(native.arrays().size(), scalar_);
    types.insert(types.end(), 2, pointer);
    auto *outcome = body.CreateCall(llvm::FunctionType::get(outcome_, types, false), body.CreateExtractValue(callback, 0), arguments);
    auto *abrupt = body.CreateOr(body.CreateICmpNE(body.CreateExtractValue(outcome, 1), integer(0)),
                                  body.CreateIsNotNull(body.CreateExtractValue(outcome, 3)));
    if (protected_outcomes_) abrupt = body.CreateOr(abrupt, body.CreateIsNotNull(body.CreateExtractValue(outcome, 7)));
    auto *forward = llvm::BasicBlock::Create(context_, "callback.protected", bridge);
    auto *normal = llvm::BasicBlock::Create(context_, "callback.value", bridge);
    body.CreateCondBr(abrupt, forward, normal);
    body.SetInsertPoint(forward);
    body.CreateStore(outcome, body.CreateStructGEP(frame, state, 2));
    body.CreateRet(body.getInt32(static_cast<unsigned>(NativeHereStep::protected_completion)));
    body.SetInsertPoint(normal);
    body.CreateStore(body.CreateTruncOrBitCast(body.CreateExtractValue(outcome, {0, 0}),
                                             c_scalar_type(*native.contract.result.type)), bridge->getArg(2));
    body.CreateRet(body.getInt32(static_cast<unsigned>(NativeHereStep::value)));
    return bridge;
  }
  llvm::Value *emit_native_here(const Expression &expr) {
    using Call = std::pair<size_t, std::optional<CheckedOperation::SuppliedHere>>;
    std::map<Call, std::vector<size_t>> alternatives;
    for (size_t i = 0; i < contexts_.size(); ++i) {
      const auto &fact = contexts_[i].at(&expr);
      alternatives[{*fact.native_call, fact.supplied_here}].push_back(i);
    }
    return contextual_value(expr, alternatives, "native.here", [&](const auto &selected) -> llvm::Value * {
      const auto &[id, supplied] = selected;
      const auto &operand = *expr.children.front();
      auto *borrow = emit_expression(operand, Projection::target);
      if (!reachable()) return nullptr;
      const auto &body = supplied.value();
      const auto &native = program_->native_operations.at(id);
      auto *callback = prepare_here(body, expr.at);
      auto *frame = llvm::StructType::get(context_, {here_, b_.getPtrTy(), outcome_});
      auto *state = slot(frame, "native.here.context");
      b_.CreateStore(callback, b_.CreateStructGEP(frame, state, 0));
      b_.CreateStore(selected_owner(operand), b_.CreateStructGEP(frame, state, 1));
      auto *completion = slot(b_.getInt32Ty(), "native.here.completion");
      b_.CreateStore(b_.getInt32(static_cast<unsigned>(NativeHereStep::invalid)), completion);
      const auto &result_type = *native.contract.result.type;
      auto *value = slot(c_scalar_type(result_type), "native.here.value");
      auto *status = b_.CreateCall(runtime(native_operation_name(native) + "_call", b_.getInt32Ty(),
          std::vector<llvm::Type *>(5, b_.getPtrTy())), {borrow, native_here_bridge(native, frame), state, completion, value});
      auto *step = load(completion);
      auto *abrupt = block("native.here.protected"), *ordinary = block("native.here.ordinary"), *done = block("native.here.outcome");
      b_.CreateCondBr(b_.CreateICmpEQ(step, b_.getInt32(static_cast<unsigned>(NativeHereStep::protected_completion))), abrupt, ordinary);
      b_.SetInsertPoint(abrupt);
      auto *protected_result = b_.CreateLoad(outcome_, b_.CreateStructGEP(frame, state, 2));
      b_.CreateBr(done);
      b_.SetInsertPoint(ordinary);
      auto *success = block("native.here.success"), *invalid = block("native.here.invalid");
      b_.CreateCondBr(b_.CreateAnd(b_.CreateICmpEQ(step, b_.getInt32(static_cast<unsigned>(NativeHereStep::value))),
          b_.CreateICmpEQ(status, b_.getInt32(static_cast<unsigned>(NativeStatus::ok)))), success, invalid);
      b_.SetInsertPoint(invalid);
      fail_path(ExceptionCategory::system, expr.at, b_.CreateGlobalString(location_text(expr.at) + ": invalid native callback or provider status"));
      b_.SetInsertPoint(success);
      auto *ordinary_result = outcome(pack(load(value), result_type), integer(0), integer(0), null_pointer());
      b_.CreateBr(done);
      b_.SetInsertPoint(done);
      auto *result = b_.CreatePHI(outcome_, 2);
      result->addIncoming(protected_result, abrupt);
      result->addIncoming(ordinary_result, success);
      llvm::Value *completed = result;
      if (here_entries_.at(body.instance).group)
        completed = consume_here_transfer(result, b_.CreateExtractValue(callback, 1), expr.at);
      return propagate(completed);
    });
  }
  llvm::Value *native_call(const Expression &expr, llvm::Function *target, size_t method, std::vector<llvm::Value *> prepared,
                           const std::vector<llvm::Value *> &captured = {}, std::optional<size_t> instance = {},
                           const Type *callable_view = nullptr, llvm::AllocaInst *here_site = nullptr,
                           const std::vector<llvm::Value *> &here_values = {}, bool nominal = false,
                           std::span<llvm::Value *const> current_owners = {}) {
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
      const auto &callee = program_->methods[method];
      if (callee.lexical_result && owner == null_target - 1 - callee.lexical_result->owner) {
        std::map<size_t, std::vector<size_t>> alternatives;
        for (size_t i = 0; i < contexts_.size(); ++i) {
          const auto &call = contexts_[i].at(&expr);
          if (call.lexical_result_owner && call.callee && program_->instances[*call.callee].method == method)
            alternatives[*call.lexical_result_owner].push_back(i);
        }
        prepared.push_back(contextual_value(expr, alternatives, "result.owner", [&](size_t actual) {
          return owner_identity(actual, expr.at);
        }, b_.getPtrTy()));
      } else if (callee.scope_inputs) {
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
        const auto index = input + offset;
        const auto &operand = invocation.argument(expr, index);
        if (!current_owners.empty() && current_owners[index]) prepared.push_back(current_owners[index]);
        else prepared.push_back(nominal ? selected_owner(operand) : selected_owner(operand, &expr, method, callable_view, index));
      }
    }
    for (const auto role : layout.array_owners) {
      using Allocator = CheckedOperation::ArrayAllocator;
      std::map<std::optional<Allocator>, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < contexts_.size(); ++i) {
        const auto &call = contexts_[i].at(&expr);
        if (!call.callee) continue;
        const auto &callee = program_->instances[*call.callee];
        if (callee.method != method) continue;
        std::optional<Allocator> selected;
        if (call.array_allocator_arguments) {
          const auto found = call.array_allocator_arguments->find(role);
          if (found != call.array_allocator_arguments->end()) selected = found->second;
        }
        if (!selected && callee.array_allocator_roles->contains(role))
          implementation_gap(expr.at, "call without a prepared array allocator capability");
        alternatives[selected].push_back(i);
      }
      prepared.push_back(contextual_value(expr, alternatives, "array.owner",
          [&](const auto &selected) -> llvm::Value * {
            return selected ? array_capability(*selected) : llvm::Constant::getNullValue(array_capability_);
          }, array_capability_));
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
    if (layout.callable_destination) {
      destination = slot(scalar_, "callable.result");
      prepared.push_back(destination);
    }
    if (layout.native_borrow_destination) prepared.push_back(slot(native_handle_pair(), "native.call.result"));
    if (layout.exception_result) {
      if (!exception_result_destination_) throw std::logic_error("owning call without its checked exception result destination");
      prepared.push_back(exception_result_destination_);
    }
    llvm::Value *callee = target;
    if (here_values.size() != layout.here_formals)
      throw std::logic_error("native call disagrees with its here parameter roles");
    if (invocation.supplied_here) {
      std::map<CheckedOperation::SuppliedHere, std::vector<size_t>> entries;
      for (size_t i = 0; i < contexts_.size(); ++i) {
        const auto &call = contexts_[i].at(&expr);
        if (call.callee && program_->instances[*call.callee].method == method)
          entries[call.supplied_here.value()].push_back(i);
      }
      llvm::Value *argument;
      if (std::ranges::any_of(entries, [](const auto &entry) { return entry.first.returning.has_value(); })) {
        auto *returning = observed_return();
        auto *dispatch_type = llvm::StructType::get(context_, {here_, b_.getPtrTy()});
        auto *dispatch = contextual_value(expr, entries, "here.dispatch", [&](const auto &supplied) {
          llvm::Value *receiver = target;
          if (supplied.returning) {
            const auto id = supplied.returning->receiver;
            auto *alternate = methods_.at(program_->instances[id].method);
            if (!alternate || target->getFunctionType() != alternate->getFunctionType())
              throw std::logic_error("supplied here return state changed the receiver layout");
            receiver = b_.CreateSelect(returning, alternate, target);
          }
          auto *value = b_.CreateInsertValue(llvm::UndefValue::get(dispatch_type), prepare_here(supplied, expr.at, returning), 0);
          return b_.CreateInsertValue(value, receiver, 1);
        }, dispatch_type);
        argument = b_.CreateExtractValue(dispatch, 0);
        callee = b_.CreateExtractValue(dispatch, 1);
      } else argument = contextual_value(expr, entries, "here.argument", [&](const auto &supplied) {
        return prepare_here(supplied, expr.at);
      }, here_);
      if (here_site) b_.CreateStore(b_.CreateExtractValue(argument, 1), here_site);
      prepared.push_back(argument);
    }
    prepared.insert(prepared.end(), here_values.begin() + invocation.supplied_here.has_value(), here_values.end());
    for (size_t slot = 0; slot < layout.lexical_here; ++slot) {
      std::map<size_t, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < contexts_.size(); ++i) {
        const auto &call = contexts_[i].at(&expr);
        if (call.callee && program_->instances[*call.callee].method == method)
          alternatives[call.scope_here.at(slot)].push_back(i);
      }
      prepared.push_back(contextual_value(expr, alternatives, "here.lexical", [&](size_t source) {
        return here_arguments_.at(source);
      }, here_));
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
    auto *target = methods_[method];
    for (size_t i = 0; i < arguments.size(); ++i) if (actual[i] != instance.arguments[i]) {
      auto *original = arguments[i];
      arguments[i] = convert_value(original, actual[i], instance.arguments[i]);
      if (const auto owned = value_owners_.find(original); owned != value_owners_.end()) value_owners_[arguments[i]] = owned->second;
    }
    std::vector<std::pair<size_t, llvm::Value *>> saved;
    auto *image = environment(callee);
    const auto &symbols = program_->captured_symbols(value);
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
                                      b_.CreateICmpULT(category, integer(program_->exception_types.size())));
            valid = b_.CreateAnd(valid, b_.CreateICmpNE(category, integer(error_root_id)));
            auto *identity = b_.CreateAdd(b_.CreateTrunc(category, b_.getInt32Ty()), b_.getInt32(tag(program_->exception_type(0))));
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
  llvm::Value *swap_builtin(const std::vector<llvm::Value *> &arguments, BuiltinOperation operation, Type bits = {}) {
    auto *left = view_address(arguments[0]), *right = view_address(arguments[1]);
    auto *type = operation == BuiltinOperation::reference_swap ? static_cast<llvm::Type *>(b_.getPtrTy()) :
        scalar_type(bits) ? native_type(bits) : scalar_;
    auto *left_value = b_.CreateLoad(type, left), *right_value = b_.CreateLoad(type, right);
    b_.CreateStore(right_value, left); b_.CreateStore(left_value, right);
    return outcome(empty_scalar(), integer(0), integer(0), null_pointer());
  }
  llvm::Value *invoke(const Expression &expr, const std::vector<llvm::Value *> &arguments, llvm::AllocaInst *here_site = nullptr,
                      const std::vector<llvm::Value *> &here_values = {},
                      std::span<llvm::Value *const> current_owners = {}) {
    const auto &fact = contexts_.front().at(&expr);
    if (fact.invokes_here) {
      auto *argument = here_arguments_.at(*fact.invokes_here);
      if (fact.opened_pipeline_stages) {
        const auto count = *fact.opened_pipeline_stages;
        auto *backing = b_.CreateExtractValue(argument, 1);
        auto *layout = llvm::ArrayType::get(here_, count);
        argument = here_values.at(0);
        for (size_t i = 0; i < count; ++i) {
          auto *address = b_.CreateInBoundsGEP(layout, backing, {b_.getInt32(0), b_.getInt64(i)});
          argument = connect_prepared_stage(b_.CreateLoad(here_, address), argument);
        }
      }
      llvm::Value *entry = nullptr;
      auto prepared = arguments;
      std::vector<llvm::Type *> parameters(arguments.size(), scalar_);
      for (const auto input : here_owner_inputs(fact.operand_type)) {
        prepared.push_back(selected_owner(fact.argument(expr, input)));
        parameters.push_back(b_.getPtrTy());
      }
      if (fact.supplied_here) {
        std::map<CheckedOperation::SuppliedHere, std::vector<size_t>> entries;
        for (size_t i = 0; i < contexts_.size(); ++i) entries[contexts_[i].at(&expr).supplied_here.value()].push_back(i);
        auto *supplied = contextual_value(expr, entries, "here.consumer", [&](const auto &body) {
          return prepare_here(body, expr.at);
        }, here_);
        if (here_site) b_.CreateStore(b_.CreateExtractValue(supplied, 1), here_site);
        prepared.push_back(supplied);
        parameters.push_back(here_);
        std::map<size_t, std::vector<size_t>> sources;
        for (size_t i = 0; i < contexts_.size(); ++i) sources[contexts_[i].at(&expr).opened_source.value()].push_back(i);
        entry = contextual_value(expr, sources, "here.open", [&](size_t source) -> llvm::Value * {
          return prepared_source_entries_.at(source);
        }, b_.getPtrTy());
      }
      prepared.push_back(b_.CreateExtractValue(argument, 1));
      if (!entry) entry = b_.CreateExtractValue(argument, 0);
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
      auto *left = view_address(arguments[0]);
      auto *right = view_address(arguments[1]);
      auto *equal = b_.CreateAnd(b_.CreateICmpEQ(left_owner, right_owner),
          b_.CreateICmpEQ(left, right));
      std::map<bool, std::vector<size_t>> distinct_subobjects;
      for (size_t i = 0; i < contexts_.size(); ++i) {
        const auto &a = contexts_[i].at(expr.children[0].get()).type;
        const auto &b = contexts_[i].at(expr.children[1].get()).type;
        distinct_subobjects[a != b && (a.kind == Type::product || b.kind == Type::product)].push_back(i);
      }
      if (distinct_subobjects.contains(true)) {
        auto *same_locator = contextual_value(expr, distinct_subobjects, "identity.subobject",
            [&](bool distinct) -> llvm::Value * { return distinct ? b_.CreateIsNull(left) : b_.getTrue(); }, b_.getInt1Ty());
        equal = b_.CreateAnd(equal, same_locator);
      }
      if (expr.text == "is not") equal = b_.CreateNot(equal);
      return outcome(pack(equal, Type::boolean), integer(0), integer(0), null_pointer());
    }
    std::vector<const Expression *> source_inputs;
    for (size_t i = 0; i < fact.argument_count(expr); ++i) source_inputs.push_back(&fact.argument(expr, i));
    const auto cases = choices(expr, source_inputs, true);
    const std::vector<llvm::Value *> inputs(arguments.begin(), arguments.begin() + source_inputs.size());
    const std::vector<llvm::Value *> captured(arguments.begin() + source_inputs.size(), arguments.end());
    return choose(expr, inputs, cases, outcome_, [&](const Choice &choice) -> llvm::Value * {
      if (choice.nominal_callee) {
        const auto id = *choice.nominal_callee;
        const auto method = program_->instances[id].method;
        auto *target = methods_[method];
        return native_call(expr, target, method, inputs, captured, id, nullptr, nullptr, {}, true);
      }
      if (choice.operation->callee) {
        const auto id = *choice.operation->callee;
        auto *target = methods_[program_->instances[id].method];
        return native_call(expr, target, program_->instances[id].method, inputs, captured, id, nullptr, here_site,
                           here_values, false, current_owners);
      }
      if (choice.operation->builtin == BuiltinOperation::unary || choice.operation->builtin == BuiltinOperation::binary)
        return outcome(eager_builtin(expr, arguments, choice), integer(0), integer(0), null_pointer());
      if (choice.operation->builtin == BuiltinOperation::bits_swap || choice.operation->builtin == BuiltinOperation::reference_swap)
        return swap_builtin(arguments, choice.operation->builtin, choice.inputs.front());
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
    if (owned_failures_) release_failure_owner(b_, take_failure_owner(b_, transfer_failure_slot(b_, target)));
    auto *cleanup_failure = b_.CreateInsertValue(result, null_pointer(), 7);
    b_.CreateBr(join);
    b_.SetInsertPoint(deliver);
    auto *kind = b_.CreateLoad(b_.getInt8Ty(), b_.CreateStructGEP(here_transfer_, target, 0));
    auto *payload = b_.CreateLoad(outcome_, here_payload_slot(b_, target));
    if (owned_failures_) b_.CreateStore(null_pointer(), transfer_failure_slot(b_, target));
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
  void collect_cleanup(const Expression &expr, Scope &scope, bool only_native_effects = false,
                       bool nested_iteration = false) {
    using K = Expression::Kind;
    if (expr.kind != K::deferred || !only_native_effects)
      for (size_t i = 0; i < expr.children.size(); ++i)
        collect_cleanup(*expr.children[i], scope, only_native_effects,
                        nested_iteration || (expr.kind == K::range_for && i >= 2));
    if (expr.kind == K::effect && !nested_iteration) {
      std::set<std::pair<size_t, size_t>> installations;
      for (const auto &context : contexts_) {
        const auto effect = context.operations->find(&expr);
        if (effect != context.operations->end() && effect->second.native_acquisition)
          installations.emplace(*effect->second.symbol, effect->second.native_acquisition->constructor);
      }
      for (const auto &[symbol, constructor] : installations) {
        auto *layout = llvm::StructType::get(context_, {cleanup_header_, native_handle_pair(), scalar_});
        scope.actions.push_back({&expr, layout, {}, constructor, slot(layout, "native.effect.record"), symbol});
      }
    }
    if (only_native_effects) return;
    if (expr.kind == K::region_binding) {
      auto *layout = llvm::StructType::get(context_, {cleanup_header_, b_.getPtrTy()});
      scope.actions.push_back({&expr, layout, {}, {}, slot(layout, "region.cleanup"), {}, {}, true});
      return;
    }
    if (expr.kind == K::exception_binding) {
      auto *layout = llvm::StructType::get(context_, {cleanup_header_, b_.getPtrTy()});
      scope.actions.push_back({&expr, layout, {}, {}, slot(layout, "exception.cleanup"), {}, {}, false, true});
      return;
    }
    if (expr.kind == Expression::Kind::native_handle_binding && expr.binding == "<-") {
      std::set<size_t> constructors;
      for (const auto &context : contexts_) {
        const auto operation = context.operations->find(&expr);
        if (operation != context.operations->end() && operation->second.native_acquisition)
          constructors.insert(operation->second.native_acquisition->constructor);
      }
      for (const auto id : constructors)
        scope.actions.push_back({&expr, llvm::StructType::get(context_, {cleanup_header_, b_.getInt64Ty()}), {}, id});
      return;
    }
    if (expr.kind != Expression::Kind::deferred) return;
    const auto &body = *expr.children.front();
    if (body.kind != Expression::Kind::block && std::all_of(contexts_.begin(), contexts_.end(),
        [&](const auto &context) { return context.at(&body).type == Type::never; })) return;
    std::set<std::pair<std::set<size_t>, std::optional<DeferredInputs>>> layouts;
    for (const auto &context : contexts_) {
      auto inputs = deferred_inputs(body, context.at(&body));
      if (body.kind == K::block || inputs) layouts.emplace(context.at(&expr).captures, std::move(inputs));
    }
    for (const auto &[captures, inputs] : layouts) {
      std::vector<llvm::Type *> fields{cleanup_header_, deferred_failure_layout()};
      if (inputs) {
        for (size_t i = 0; i < inputs->count; ++i)
          fields.push_back(inputs->handle_arguments.contains(i) ? native_handle_pair() : scalar_);
      } else fields.insert(fields.end(), captures.size(), scalar_);
      auto *layout = llvm::StructType::get(context_, fields);
      auto *backing = expr.repeated_defer_registration ? nullptr : slot(layout, "defer.record");
      scope.actions.push_back({&expr, layout, captures, {}, backing, {}, inputs});
    }
  }
  void prepare_cleanup_record(Scope &scope, llvm::Value *record, size_t action) {
    b_.CreateStore(load(scope.head), b_.CreateStructGEP(cleanup_header_, record, 0));
    b_.CreateStore(integer(action), b_.CreateStructGEP(cleanup_header_, record, 1));
  }
  llvm::Value *complete_cleanup(const std::function<void()> &emit, CleanupBoundary boundary,
                                llvm::Value *entered_failure = nullptr) {
    // Suspend the pending completion while cleanup executes its own control flow.
    auto pending = completion_.transform([&](auto *place) { return load(place); });
    if (environments_) b_.CreateStore(empty_scalar(), completion_.value);
    const bool handles_failure = entered_failure != nullptr;
    if (!entered_failure) entered_failure = b_.getFalse();
    b_.CreateStore(b_.CreateSelect(entered_failure, b_.getInt8(2), b_.getInt8(0)), completion_.kind);
    b_.CreateStore(b_.CreateSelect(entered_failure, pending.category, integer(0)), completion_.category);
    b_.CreateStore(b_.CreateSelect(entered_failure, pending.fatal, null_pointer()), completion_.fatal);
    if (owned_failures_) {
      // A guarding action receives the failure. Otherwise its owner remains suspended.
      b_.CreateStore(b_.CreateSelect(entered_failure, pending.failure_owner, null_pointer()), completion_.failure_owner);
      pending.failure_owner = b_.CreateSelect(entered_failure, null_pointer(), pending.failure_owner);
    }
    b_.CreateStore(b_.getFalse(), completion_.has_return);
    if (protected_outcomes_) b_.CreateStore(null_pointer(), completion_.transfer);
    Control action{control_, block("cleanup.completed")};
    control_ = &action;
    const auto outer_return = suspended_return_;
    auto *prepared = pending.value;
    if (here_destination_) prepared = outer_return.value ? protected_return_value() : nullptr;
    suspended_return_ = {
        outer_return.present ? b_.CreateOr(outer_return.present, pending.has_return) : pending.has_return,
        outer_return.value ? b_.CreateSelect(pending.has_return, prepared, outer_return.value) : prepared};
    emit();
    suspended_return_ = outer_return;
    if (reachable()) b_.CreateBr(action.exit);
    b_.SetInsertPoint(action.exit);
    control_ = action.parent;
    auto *restore = block("cleanup.preserve"), *replace = block("cleanup.replace"), *done = block("cleanup.done");
    auto *kind = load(completion_.kind);
    auto *normal = b_.CreateICmpEQ(kind, b_.getInt8(0));
    auto *replaces_body = b_.CreateOr(load(completion_.has_return),
        b_.CreateAnd(b_.CreateNot(normal), b_.CreateICmpNE(kind, b_.getInt8(2))));
    b_.CreateCondBr(normal, restore, replace);
    b_.SetInsertPoint(restore);
    if (environments_) clear_value(completion_.value);
    const auto places = completion_.fields();
    const auto values = pending.fields();
    for (size_t i = 0; i < places.size(); ++i) if (places[i]) b_.CreateStore(values[i], places[i]);
    if (handles_failure) {
      auto *resumed = b_.CreateSelect(pending.has_return, b_.getInt8(1), b_.getInt8(0));
      b_.CreateStore(b_.CreateSelect(entered_failure, resumed, pending.kind), completion_.kind);
      b_.CreateStore(b_.CreateSelect(entered_failure, integer(0), pending.category), completion_.category);
      b_.CreateStore(b_.CreateSelect(entered_failure, null_pointer(), pending.fatal), completion_.fatal);
    }
    b_.CreateBr(done);
    b_.SetInsertPoint(replace);
    release_failure_owner(b_, pending.failure_owner);
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
      discard_transfer_failure(pending.transfer, transfer);
    }
    preserve_transfer(transfer);
    return replaces_body;
  }
  struct DeferredFailures {
    llvm::AllocaInst *head, *tail;
  };
  enum DeferredFailureField { failure_next, failure_backing, failure_outcome };
  static constexpr unsigned first_deferred_operand = 2;
  llvm::StructType *deferred_failure_layout() {
    return llvm::StructType::get(context_, {b_.getPtrTy(), b_.getPtrTy(), outcome_});
  }
  llvm::Value *deferred_failure_field(llvm::Value *node, unsigned index) {
    return b_.CreateStructGEP(deferred_failure_layout(), node, index);
  }
  void clear_deferred_failures(const DeferredFailures &queue) {
    b_.CreateStore(null_pointer(), queue.head);
    b_.CreateStore(null_pointer(), queue.tail);
  }
  DeferredFailures deferred_failures() {
    DeferredFailures queue{slot(b_.getPtrTy(), "defer.failures.head"), slot(b_.getPtrTy(), "defer.failures.tail")};
    clear_deferred_failures(queue);
    return queue;
  }
  void append_deferred_failures(const DeferredFailures &queue, llvm::Value *first, llvm::Value *last) {
    auto *extend = block("defer.failures.extend"), *start = block("defer.failures.start"), *done = block("defer.failures.appended");
    auto *tail = load(queue.tail);
    b_.CreateCondBr(b_.CreateIsNull(tail), start, extend);
    b_.SetInsertPoint(start);
    b_.CreateStore(first, queue.head);
    b_.CreateBr(done);
    b_.SetInsertPoint(extend);
    b_.CreateStore(first, deferred_failure_field(tail, failure_next));
    b_.CreateBr(done);
    b_.SetInsertPoint(done);
    b_.CreateStore(last, queue.tail);
  }
  void retire_deferred_failure(llvm::Value *node) {
    auto *backing = b_.CreateLoad(b_.getPtrTy(), deferred_failure_field(node, failure_backing));
    auto *release = block("defer.record.release"), *done = block("defer.record.released");
    b_.CreateCondBr(b_.CreateIsNull(backing), done, release);
    b_.SetInsertPoint(release);
    b_.CreateCall(runtime("cod_lang_defer_free", b_.getVoidTy(), {b_.getPtrTy()}), {backing});
    b_.CreateBr(done);
    b_.SetInsertPoint(done);
  }
  void finish_deferred_phase(const DeferredFailures &queue, llvm::Value *node) {
    auto *failed = block("defer.failure.save"), *normal = block("defer.phase.normal"), *done = block("defer.phase.done");
    b_.CreateCondBr(b_.CreateICmpEQ(load(completion_.kind), b_.getInt8(2)), failed, normal);
    b_.SetInsertPoint(failed);
    auto *owner = owned_failures_ ? take_failure_owner(b_, completion_.failure_owner) : nullptr;
    auto *failure = outcome(empty_scalar(), load(completion_.category), load(completion_.site), load(completion_.fatal),
                            load(completion_.requested), load(completion_.available), load(completion_.cause), nullptr, owner);
    b_.CreateStore(failure, deferred_failure_field(node, failure_outcome));
    b_.CreateStore(null_pointer(), deferred_failure_field(node, failure_next));
    auto *kind = b_.CreateSelect(load(completion_.has_return), b_.getInt8(1), b_.getInt8(0));
    if (protected_outcomes_)
      kind = b_.CreateSelect(b_.CreateIsNotNull(load(completion_.transfer)), b_.getInt8(3), kind);
    b_.CreateStore(kind, completion_.kind);
    b_.CreateStore(integer(0), completion_.category);
    b_.CreateStore(null_pointer(), completion_.fatal);
    append_deferred_failures(queue, node, node);
    b_.CreateBr(done);
    b_.SetInsertPoint(normal);
    retire_deferred_failure(node);
    b_.CreateBr(done);
    b_.SetInsertPoint(done);
  }
  llvm::Value *matches_deferred_handler(const Expression &entry, llvm::Value *failure) {
    auto *category = b_.CreateExtractValue(failure, 1);
    llvm::Value *matches = b_.getFalse();
    for (const auto &clause : std::span{entry.children}.subspan(1)) {
      if (clause->kind != Expression::Kind::handler) break;
      auto *selected = choose(*clause, {}, choices(*clause, {}), b_.getInt1Ty(), [&](const Choice &choice) {
        return catch_matches(category, choice.operation->type.identity);
      });
      matches = b_.CreateOr(matches, selected);
    }
    return b_.CreateAnd(matches, b_.CreateAnd(b_.CreateICmpNE(category, integer(0)),
                        b_.CreateIsNull(b_.CreateExtractValue(failure, 3))));
  }
  void visit_deferred_failures(const Expression &entry, const DeferredFailures &queue, const auto &run_action) {
    auto *incoming = slot(b_.getPtrTy(), "defer.visit.incoming");
    b_.CreateStore(load(queue.head), incoming);
    clear_deferred_failures(queue);
    const auto outgoing = deferred_failures();
    auto *scan = block("defer.visit.next"), *test = block("defer.visit.match"), *done = block("defer.visit.done");
    b_.CreateBr(scan);
    b_.SetInsertPoint(scan);
    auto *node = load(incoming);
    b_.CreateCondBr(b_.CreateIsNull(node), done, test);
    b_.SetInsertPoint(test);
    b_.CreateStore(b_.CreateLoad(b_.getPtrTy(), deferred_failure_field(node, failure_next)), incoming);
    auto *failure = b_.CreateLoad(outcome_, deferred_failure_field(node, failure_outcome));
    auto *handle = block("defer.visit.handle"), *keep = block("defer.visit.keep");
    b_.CreateCondBr(matches_deferred_handler(entry, failure), handle, keep);
    b_.SetInsertPoint(keep);
    b_.CreateStore(null_pointer(), deferred_failure_field(node, failure_next));
    append_deferred_failures(queue, node, node);
    b_.CreateBr(scan);
    b_.SetInsertPoint(handle);
    save_abrupt(failure);
    run_action([&] { emit_finally(entry, Projection::value, true); }, b_.getTrue());
    finish_deferred_phase(outgoing, node);
    b_.CreateBr(scan);
    b_.SetInsertPoint(done);
    auto *first = load(outgoing.head);
    auto *append = block("defer.visit.outputs"), *next = block("defer.visit.completed");
    b_.CreateCondBr(b_.CreateIsNull(first), next, append);
    b_.SetInsertPoint(append);
    append_deferred_failures(queue, first, load(outgoing.tail));
    b_.CreateBr(next);
    b_.SetInsertPoint(next);
  }
  void finish_deferred_residuals(const DeferredFailures &queue, const Location &at, llvm::AllocaInst *body_pending,
                                 const CompletionState<llvm::Value *> &body) {
    auto *current = slot(b_.getPtrTy(), "defer.residual.current");
    auto *terminal = slot(b_.getPtrTy(), "defer.residual.terminal");
    auto *count = slot(b_.getInt64Ty(), "defer.residual.count");
    auto *aggregate = slot(b_.getPtrTy(), "defer.residual.aggregate");
    b_.CreateStore(load(queue.head), current);
    b_.CreateStore(null_pointer(), terminal);
    b_.CreateStore(null_pointer(), aggregate);
    b_.CreateStore(integer(0), count);
    auto *scan = block("defer.residual.scan"), *inspect = block("defer.residual.inspect"), *classify = block("defer.residual.classify");
    auto *retire = block("defer.residual.retire"), *release = block("defer.residual.release"), *done = block("defer.residual.done");
    b_.CreateCondBr(b_.CreateIsNull(load(current)), done, scan);
    b_.SetInsertPoint(scan);
    auto *node = load(current);
    b_.CreateCondBr(b_.CreateIsNull(node), classify, inspect);
    b_.SetInsertPoint(inspect);
    auto *failure = b_.CreateLoad(outcome_, deferred_failure_field(node, failure_outcome));
    auto *fatal = b_.CreateIsNotNull(b_.CreateExtractValue(failure, 3));
    b_.CreateStore(b_.CreateSelect(fatal, node, load(terminal)), terminal);
    b_.CreateStore(b_.CreateAdd(load(count), b_.CreateZExt(b_.CreateNot(fatal), b_.getInt64Ty())), count);
    b_.CreateStore(b_.CreateLoad(b_.getPtrTy(), deferred_failure_field(node, failure_next)), current);
    b_.CreateBr(scan);

    b_.SetInsertPoint(classify);
    auto *last_terminal = load(terminal);
    auto *deliver_fatal = block("defer.residual.fatal"), *test_body = block("defer.residual.body");
    b_.CreateCondBr(b_.CreateIsNotNull(last_terminal), deliver_fatal, test_body);
    b_.SetInsertPoint(deliver_fatal);
    save_abrupt(b_.CreateLoad(outcome_, deferred_failure_field(last_terminal, failure_outcome)));
    b_.CreateBr(retire);
    b_.SetInsertPoint(test_body);
    auto *publish = block("defer.residual.publish");
    auto *local_body = load(body_pending);
    auto *terminal_body = b_.CreateAnd(local_body, b_.CreateIsNotNull(body.fatal));
    llvm::Value *body_transfer = nullptr;
    if (protected_outcomes_) {
      // The supplier failure bypasses deferred catches. Only the completed
      // owning pass can retain it as the aggregate's body exception.
      auto *test_transfer = block("defer.residual.transfer"), *inspect_transfer = block("defer.residual.inspect.transfer");
      b_.CreateCondBr(terminal_body, retire, test_transfer);
      b_.SetInsertPoint(test_transfer);
      auto *transfer = load(completion_.transfer);
      b_.CreateCondBr(b_.CreateAnd(b_.CreateNot(local_body), b_.CreateIsNotNull(transfer)), inspect_transfer, publish);
      b_.SetInsertPoint(inspect_transfer);
      auto *kind = b_.CreateLoad(b_.getInt8Ty(), b_.CreateStructGEP(here_transfer_, transfer, 0));
      auto *is_failure = b_.CreateICmpEQ(kind, b_.getInt8(static_cast<unsigned>(HereExit::failure)));
      auto *payload = b_.CreateLoad(outcome_, here_payload_slot(b_, transfer));
      auto *is_terminal = b_.CreateAnd(is_failure, b_.CreateIsNotNull(b_.CreateExtractValue(payload, 3)));
      auto *selected = b_.CreateSelect(is_failure, transfer, null_pointer());
      b_.CreateCondBr(is_terminal, retire, publish);
      b_.SetInsertPoint(publish);
      auto *incoming = b_.CreatePHI(b_.getPtrTy(), 2);
      incoming->addIncoming(null_pointer(), test_transfer);
      incoming->addIncoming(selected, inspect_transfer);
      body_transfer = incoming;
    } else b_.CreateCondBr(terminal_body, retire, publish);

    const auto cancel_transfer = [&] {
      if (!protected_outcomes_) return;
      discard_transfer_failure(load(completion_.transfer), null_pointer());
      b_.CreateStore(null_pointer(), completion_.transfer);
    };

    const auto retain = [&](llvm::Value *owner, bool member, llvm::Value *category, llvm::Value *where,
                            llvm::Value *requested, llvm::Value *available, llvm::Value *cause, llvm::Value *child) {
      std::vector<llvm::Type *> types{b_.getPtrTy()};
      types.insert(types.end(), 6, b_.getInt64Ty());
      types.push_back(b_.getPtrTy());
      b_.CreateCall(runtime("cod_lang_retain_deferred_failure", b_.getVoidTy(), types),
                    {owner, integer(member), category, where, requested, available, cause, child});
    };
    b_.SetInsertPoint(publish);
    if (program_->deferred_aggregates) {
      auto *status = slot(b_.getInt64Ty(), "defer.residual.status");
      auto acquire = runtime("cod_lang_acquire_deferred_aggregate", b_.getPtrTy(), {b_.getInt64Ty(), b_.getPtrTy()});
      auto *owner = b_.CreateCall(acquire, {load(count), status});
      auto *ready = block("defer.residual.ready"), *unavailable = block("defer.residual.unavailable");
      auto *where = integer(site(at));
      b_.CreateCondBr(b_.CreateIsNotNull(owner), ready, unavailable);
      b_.SetInsertPoint(unavailable);
      cancel_transfer();
      save_abrupt(outcome(empty_scalar(), load(status), where, null_pointer()));
      b_.CreateBr(retire);
      b_.SetInsertPoint(ready);
      b_.CreateStore(owner, aggregate);
      auto *attach_body = block("defer.residual.attach.body"), *deliver = block("defer.residual.deliver");
      auto *attach_supplier = body_transfer ? block("defer.residual.supplier") : deliver;
      b_.CreateCondBr(local_body, attach_body, attach_supplier);
      b_.SetInsertPoint(attach_body);
      retain(owner, false, body.category, body.site, body.requested, body.available, body.cause, body.failure_owner);
      b_.CreateStore(b_.getFalse(), body_pending);
      b_.CreateBr(deliver);
      if (body_transfer) {
        b_.SetInsertPoint(attach_supplier);
        auto *attach = block("defer.residual.attach.supplier");
        b_.CreateCondBr(b_.CreateIsNotNull(body_transfer), attach, deliver);
        b_.SetInsertPoint(attach);
        auto *payload = b_.CreateLoad(outcome_, here_payload_slot(b_, body_transfer));
        auto *child = take_failure_owner(b_, transfer_failure_slot(b_, body_transfer));
        retain(owner, false, b_.CreateExtractValue(payload, 1), b_.CreateExtractValue(payload, 2), b_.CreateExtractValue(payload, 4),
               b_.CreateExtractValue(payload, 5), b_.CreateExtractValue(payload, 6), child);
        // A later receiver that catches the aggregate must not resume this transfer.
        b_.CreateStore(null_pointer(), completion_.transfer);
        b_.CreateBr(deliver);
      }
      b_.SetInsertPoint(deliver);
      cancel_transfer();
      save_abrupt(outcome(empty_scalar(), integer(deferred_aggregate_id), where, null_pointer(),
                          nullptr, nullptr, nullptr, nullptr, owner));
    } else save_abrupt(outcome(empty_scalar(), integer(exception_id(ExceptionCategory::unsound_runtime_typing)), integer(0), null_pointer()));
    b_.CreateBr(retire);

    b_.SetInsertPoint(retire);
    auto *pending = load(queue.head);
    b_.CreateCondBr(b_.CreateIsNull(pending), done, release);
    b_.SetInsertPoint(release);
    b_.CreateStore(b_.CreateLoad(b_.getPtrTy(), deferred_failure_field(pending, failure_next)), queue.head);
    if (owned_failures_) {
      auto *old = b_.CreateLoad(outcome_, deferred_failure_field(pending, failure_outcome));
      auto *child = b_.CreateExtractValue(old, failure_owner_field());
      if (program_->deferred_aggregates) {
        auto *owner = load(aggregate);
        auto *retain_member = block("defer.residual.attach.member"), *drop = block("defer.residual.drop");
        auto *retained = block("defer.residual.retained");
        b_.CreateCondBr(b_.CreateIsNotNull(owner), retain_member, drop);
        b_.SetInsertPoint(retain_member);
        retain(owner, true, b_.CreateExtractValue(old, 1), b_.CreateExtractValue(old, 2), b_.CreateExtractValue(old, 4),
               b_.CreateExtractValue(old, 5), b_.CreateExtractValue(old, 6), child);
        b_.CreateBr(retained);
        b_.SetInsertPoint(drop);
        release_failure_owner(b_, child);
        b_.CreateBr(retained);
        b_.SetInsertPoint(retained);
      } else release_failure_owner(b_, child);
    }
    retire_deferred_failure(pending);
    b_.CreateBr(retire);
    b_.SetInsertPoint(done);
  }
  void drain(Scope &scope, const Location &at) {
    b_.SetInsertPoint(scope.cleanup);
    if (scope.actions.empty()) return;
    auto *next_entry = scope.cleanup;
    llvm::AllocaInst *body_pending = nullptr;
    CompletionState<llvm::Value *> body_outcome;
    DeferredFailures failures{};
    if (std::ranges::any_of(scope.actions, [](const auto &entry) { return !entry.release_only(); })) {
      // Suspend the ordinary failure for the whole pass. The active failure
      // during the pass always comes from an invoked cleanup action.
      body_outcome = completion_.transform([&](auto *place) { return load(place); });
      body_pending = slot(b_.getInt1Ty(), "defer.body.pending");
      auto *failed = b_.CreateICmpEQ(body_outcome.kind, b_.getInt8(2));
      b_.CreateStore(failed, body_pending);
      b_.CreateStore(b_.CreateSelect(failed, b_.getInt8(0), body_outcome.kind), completion_.kind);
      b_.CreateStore(integer(0), completion_.category);
      b_.CreateStore(null_pointer(), completion_.fatal);
      if (owned_failures_) b_.CreateStore(null_pointer(), completion_.failure_owner);
      failures = deferred_failures();
      next_entry = block("defer.next");
      b_.CreateBr(next_entry);
      b_.SetInsertPoint(next_entry);
    }
    const auto run_action = [&](const auto &emit, llvm::Value *arrives = nullptr) {
      auto *replaces_body = complete_cleanup(emit, CleanupBoundary::deferred, arrives);
      auto *pending = load(body_pending);
      if (owned_failures_)
        release_failure_owner(b_, b_.CreateSelect(b_.CreateAnd(pending, replaces_body), body_outcome.failure_owner, null_pointer()));
      b_.CreateStore(b_.CreateAnd(pending, b_.CreateNot(replaces_body)), body_pending);
    };
    auto *pop = block("defer.pop"), *done = block("defer.done"), *invalid = block("defer.invalid");
    auto *record = load(scope.head);
    b_.CreateCondBr(b_.CreateICmpEQ(record, null_pointer()), done, pop);
    b_.SetInsertPoint(pop);
    auto *previous = b_.CreateLoad(b_.getPtrTy(), b_.CreateStructGEP(cleanup_header_, record, 0));
    auto *action = b_.CreateLoad(b_.getInt64Ty(), b_.CreateStructGEP(cleanup_header_, record, 1));
    b_.CreateStore(previous, scope.head);
    auto *dispatch = b_.CreateSwitch(action, invalid, scope.actions.size());
    const auto release_heap_record = [&] {
      b_.CreateCall(runtime("cod_lang_defer_free", b_.getVoidTy(), {b_.getPtrTy()}), {record});
    };
    for (size_t i = 0; i < scope.actions.size(); ++i) {
      const auto &descriptor = scope.actions[i];
      if (descriptor.region_release) {
        auto *destroy = block("region.release");
        dispatch->addCase(integer(i), destroy);
        b_.SetInsertPoint(destroy);
        auto *owner = b_.CreateLoad(b_.getPtrTy(), b_.CreateStructGEP(descriptor.layout, record, 1));
        b_.CreateCall(runtime("cod_lang_region_release", b_.getVoidTy(), {b_.getPtrTy()}), {owner});
        b_.CreateBr(next_entry);
        continue;
      }
      if (descriptor.exception_release) {
        auto *destroy = block("exception.release");
        dispatch->addCase(integer(i), destroy);
        b_.SetInsertPoint(destroy);
        auto *holder = b_.CreateLoad(b_.getPtrTy(), b_.CreateStructGEP(descriptor.layout, record, 1));
        release_failure_owner(b_, take_failure_owner(b_, holder));
        b_.CreateBr(next_entry);
        continue;
      }
      if (descriptor.native_constructor) {
        auto *destroy = block("native.destroy");
        dispatch->addCase(integer(i), destroy);
        b_.SetInsertPoint(destroy);
        auto *resource_address = b_.CreateStructGEP(descriptor.layout, record, 1);
        if (descriptor.native_symbol) {
          auto *previous = b_.CreateLoad(scalar_, b_.CreateStructGEP(descriptor.layout, record, 2));
          b_.CreateStore(previous, symbols_.at(*descriptor.native_symbol));
          resource_address = b_.CreateStructGEP(native_handle_pair(), resource_address, 0);
        }
        auto *resource = b_.CreateLoad(b_.getInt64Ty(), resource_address);
        const auto &handle = program_->native_constructors.at(*descriptor.native_constructor).handle;
        b_.CreateCall(runtime(native_destroy_name(handle), b_.getVoidTy(), {b_.getInt64Ty()}), {resource});
        b_.CreateBr(next_entry);
        continue;
      }
      const auto &body = *descriptor.action->children.front();
      auto *run = block("defer.invoke");
      dispatch->addCase(integer(i), run);
      b_.SetInsertPoint(run);
      auto *failure_node = b_.CreateStructGEP(descriptor.layout, record, 1);
      b_.CreateStore(descriptor.activation_record ? null_pointer() : record, deferred_failure_field(failure_node, failure_backing));
      const auto run_entry = [&](const auto &emit) {
        run_action(emit);
        finish_deferred_phase(failures, failure_node);
        b_.CreateBr(next_entry);
      };
      const ContextSelection selection(contexts_);
      for (const auto &context : selection.all())
        if (context.at(descriptor.action).captures == descriptor.captures &&
            deferred_inputs(body, context.at(&body)) == descriptor.inputs &&
            (body.kind == Expression::Kind::block || descriptor.inputs))
          contexts_.push_back(context);
      if (body.kind == Expression::Kind::block) {
        std::vector<std::pair<size_t, llvm::AllocaInst *>> bindings;
        size_t field = first_deferred_operand;
        for (const auto id : descriptor.captures) {
          auto *captured = slot(scalar_, "defer.caught");
          b_.CreateStore(b_.CreateLoad(scalar_, b_.CreateStructGEP(descriptor.layout, record, field++)), captured);
          bindings.emplace_back(id, storage_.at(id));
          storage_[id] = captured;
        }
        if (descriptor.action->deferred_catch()) {
          visit_deferred_failures(*descriptor.action, failures, run_action);
          if (const auto *finalizer = descriptor.action->deferred_finalizer())
            run_entry([&] { emit_expression(*finalizer, Projection::value, true); });
          else {
            retire_deferred_failure(failure_node);
            b_.CreateBr(next_entry);
          }
        } else run_entry([&] {
          if (descriptor.action->children.size() == 1) emit_expression(body, Projection::value, true);
          else emit_finally(*descriptor.action, Projection::value, true);
        });
        for (const auto &[id, place] : bindings) storage_[id] = place;
        continue;
      }
      std::vector<llvm::Value *> arguments;
      for (size_t j = 0; j < descriptor.inputs->count; ++j) {
        auto *field = descriptor.layout->getElementType(j + first_deferred_operand);
        auto *value = b_.CreateLoad(field, b_.CreateStructGEP(descriptor.layout, record, j + first_deferred_operand));
        if (field == native_handle_pair()) {
          auto *loan = slot(field, "defer.native.loan");
          b_.CreateStore(value, loan);
          const auto argument = descriptor.inputs->handle_arguments.at(j);
          arguments.push_back(pack_view(loan, contexts_.front().at(&body).argument(body, argument)));
        } else arguments.push_back(value);
      }
      if (contexts_.front().at(&body).native_call) {
        run_entry([&] { emit_native_call(body, &arguments); });
        continue;
      }
      if (body.kind == Expression::Kind::transfer || body.kind == Expression::Kind::rebind) {
        if (body.kind == Expression::Kind::transfer) arguments[1] = own_value(arguments[1]);
        run_entry([&] { assign_prepared(body, arguments); });
        continue;
      }
      run_entry([&] {
        if (std::any_of(contexts_.begin(), contexts_.end(), [&](const auto &context) {
              const auto &call = context.at(&body);
              return call.destination_arguments && std::any_of(call.arguments.begin(), call.arguments.end(),
                  [](const auto &argument) { return argument.binding_slot && argument.mode == ArgumentMode::borrowed_read; });
            })) check_destination_arguments(body, arguments, true);
        auto *result = invoke(body, arguments);
        release_value(b_.CreateExtractValue(result, 0));
        if (contexts_.front().at(&body).callable_call) release_value(arguments.front());
        if (contexts_.front().at(&body).callee)
          for (size_t i = contexts_.front().at(&body).argument_count(body); i < arguments.size(); ++i)
            release_value(arguments[i]);
        auto *replace = block("defer.failed"), *done = block("defer.completed");
        b_.CreateCondBr(abrupt_outcome(result), replace, done);
        b_.SetInsertPoint(replace); save_abrupt(result); b_.CreateBr(done);
        b_.SetInsertPoint(done);
      });
    }
    b_.SetInsertPoint(invalid);
    if (std::ranges::none_of(scope.actions, [](const auto &action) { return action.native_constructor || action.activation_record; }))
      release_heap_record();
    save_abrupt(outcome(empty_scalar(), integer(exception_id(ExceptionCategory::unsound_runtime_typing)), integer(0), null_pointer()));
    b_.CreateBr(next_entry);
    b_.SetInsertPoint(done);
    if (body_pending) {
      finish_deferred_residuals(failures, at, body_pending, body_outcome);
      auto *resume = block("defer.resume.body"), *discard = block("defer.discard.body"), *completed = block("defer.completed");
      auto *deferred_failure = b_.CreateICmpEQ(load(completion_.kind), b_.getInt8(2));
      b_.CreateCondBr(b_.CreateAnd(load(body_pending), b_.CreateNot(deferred_failure)), resume, discard);
      b_.SetInsertPoint(resume);
      const auto places = completion_.failure_fields();
      const auto values = body_outcome.failure_fields();
      for (size_t i = 0; i < places.size(); ++i) if (places[i]) b_.CreateStore(values[i], places[i]);
      b_.CreateStore(b_.getInt8(2), completion_.kind);
      b_.CreateBr(completed);
      b_.SetInsertPoint(discard);
      if (owned_failures_)
        release_failure_owner(b_, b_.CreateSelect(load(body_pending), body_outcome.failure_owner, null_pointer()));
      b_.CreateBr(completed);
      b_.SetInsertPoint(completed);
    }
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
      for (const auto &context : contexts_) {
        const auto effect = context.operations->find(&expr);
        if (effect != context.operations->end() && effect->second.publishes_effect && !effect->second.native_acquisition)
          symbols.insert(*effect->second.symbol);
      }
    }
    const size_t children = expr.kind == K::range_for ? 2 : expr.children.size();
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
      auto *binding = symbols_.at(id);
      SavedEffect effect{id, b_.CreateLoad(binding->getValueType(), binding, "effect.previous")};
      if (program_->symbols[id].type->kind == Type::procedure && !program_->symbols[id].live_scope) {
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
    Scope native{nullptr, nullptr, nullptr, {}, {}};
    collect_cleanup(expr, native, true);
    const auto saved = save_effects({&expr});
    if (saved.empty() && native.actions.empty()) return emit_expression(expr, Projection::value, discard);
    Control control{control_, block("iteration.restore")};
    control_ = &control;
    auto *outer_native = native_effect_scope_;
    if (!native.actions.empty()) {
      native.cleanup = control.exit;
      native.head = slot(b_.getPtrTy(), "iteration.native.head");
      b_.CreateStore(null_pointer(), native.head);
      native_effect_scope_ = &native;
    }
    auto *result = slot(scalar_, "iteration.value");
    auto *value = emit_expression(expr, Projection::value, discard);
    if (reachable()) { b_.CreateStore(value, result); b_.CreateBr(control.exit); }
    b_.SetInsertPoint(control.exit);
    if (!native.actions.empty()) drain(native, expr.at);
    native_effect_scope_ = outer_native;
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
                  llvm::Value *requires_return = nullptr, const Type *feedback = nullptr,
                  llvm::Value *returns_void_at_end = nullptr) {
    auto *outer_temporaries = temporaries_;
    auto *outer_native = native_effect_scope_;
    Scope scope{scope_, block("scope.cleanup"), slot(b_.getPtrTy(), "defer.head"), {}, {}};
    if (environments_) scope.values.values = parameter_values_;
    Control control{control_, scope.cleanup};
    control_ = &control;
    scope_ = &scope;
    native_effect_scope_ = &scope;
    temporaries_ = &scope.values;
    b_.CreateStore(null_pointer(), scope.head);
    for (const auto *expr : expressions) collect_cleanup(*expr, scope);
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
    if (reachable()) {
      if (const auto *known = llvm::dyn_cast_or_null<llvm::ConstantInt>(returns_void_at_end)) {
        if (known->isOne()) save_return(empty_scalar(), at);
      } else if (returns_void_at_end) {
        auto *returned = block("return.void");
        b_.CreateCondBr(returns_void_at_end, returned, scope.cleanup);
        b_.SetInsertPoint(returned);
        save_return(empty_scalar(), at);
      }
      b_.CreateBr(scope.cleanup);
    }
    if (case_scan_request_) {
      b_.SetInsertPoint(scope.cleanup);
      auto *kind = load(completion_.kind);
      auto *scan = b_.CreateICmpEQ(kind, b_.getInt8(Completion::case_scan));
      b_.CreateStore(scan, case_scan_request_);
      b_.CreateStore(b_.CreateSelect(scan, b_.getInt8(0), kind), completion_.kind);
      // The drain loops through its own header for every action. Only
      // arrival from the body can set the pending scan request.
      auto *draining = block("scope.drain");
      b_.CreateBr(draining);
      scope.cleanup = draining;
    }
    drain(scope, at); release_values(scope.values); restore_effects(effects);
    if (lexical_allocations_) {
      auto *head = load(lexical_allocations_);
      auto *release_storage = block("storage.release"), *done = block("storage.released");
      b_.CreateCondBr(b_.CreateIsNotNull(head), release_storage, done);
      b_.SetInsertPoint(release_storage);
      auto release = runtime("cod_lang_lexical_allocations_release", b_.getVoidTy(), {b_.getPtrTy()});
      b_.CreateCall(release, {head});
      b_.CreateBr(done);
      b_.SetInsertPoint(done);
    }
    scope_ = scope.parent; native_effect_scope_ = outer_native; control_ = control.parent; temporaries_ = outer_temporaries;
    if (requires_return) {
      auto *missing = block("return.missing");
      auto *unprepared = b_.CreateICmpEQ(load(completion_.kind), b_.getInt8(0));
      if (case_scan_request_) unprepared = b_.CreateAnd(unprepared, b_.CreateNot(load(case_scan_request_)));
      b_.CreateCondBr(b_.CreateAnd(requires_return, unprepared), missing, finish_);
      b_.SetInsertPoint(missing);
      save_abrupt(outcome(empty_scalar(), integer(exception_id(ExceptionCategory::missing_return)), integer(site(at)), null_pointer()));
    }
    b_.CreateBr(finish_);
  }
  bool call_native_constructor(const CheckedOperation::NativeAcquisition &plan,
                               llvm::Value *resource, llvm::Value *parent = nullptr) {
    const auto &expr = *plan.initializer;
    const auto &constructor = program_->native_constructors.at(plan.constructor);
    std::vector<llvm::Type *> types;
    std::vector<llvm::Value *> arguments;
    if (parent) {
      types.push_back(b_.getPtrTy());
      arguments.push_back(parent);
    }
    for (size_t i = 0; i < constructor.inputs.size(); ++i) {
      const auto &input = constructor.inputs[i];
      if (input.type.kind == Type::c_type_marker) {
        auto *value = emit_expression(*expr.children[i], Projection::target);
        if (!reachable()) return false;
        arguments.push_back(value);
        types.push_back(b_.getPtrTy());
      } else {
        auto *value = native_scalar_argument(*expr.children[i], input.type);
        if (!reachable()) return false;
        arguments.push_back(value);
        types.push_back(value->getType());
      }
    }
    arguments.push_back(resource);
    types.push_back(b_.getPtrTy());
    llvm::AllocaInst *error = nullptr;
    const CheckedSchema *failure = nullptr;
    if (constructor.failure) {
      failure = &program_->schemas.at(*program_->exception_types.at(constructor.failure->identity).schema);
      std::vector<llvm::Type *> fields;
      for (const auto &field : failure->fields) fields.push_back(c_scalar_type(field.type));
      if (fields.empty()) fields.push_back(b_.getInt8Ty());
      error = slot(llvm::StructType::get(context_, fields), "native.failure.payload");
      types.push_back(b_.getPtrTy());
      arguments.push_back(error);
    }
    auto *status = b_.CreateCall(runtime(native_construct_name(constructor.handle), b_.getInt32Ty(), types), arguments);
    for (size_t i = parent ? 1 : 0, end = i + constructor.inputs.size(); i < end; ++i)
      if (types[i] == b_.getInt8Ty()) status->addParamAttr(i, llvm::Attribute::ZExt);
    auto *ready = block("native.acquired"), *invalid = block("native.invalid");
    auto *select = b_.CreateSwitch(status, invalid, 1 + bool(failure));
    select->addCase(b_.getInt32(static_cast<unsigned>(NativeStatus::ok)), ready);
    if (failure) {
      auto *failed = block("native.failure");
      select->addCase(b_.getInt32(static_cast<unsigned>(NativeStatus::failure)), failed);
      b_.SetInsertPoint(failed);
      auto *native_layout = llvm::cast<llvm::StructType>(error->getAllocatedType());
      raise_constructed_exception(expr.at, constructor.failure->identity, [&](llvm::StructType *layout, llvm::Value *payload) {
        for (size_t i = 0; i < failure->fields.size(); ++i) {
          const auto &type = failure->fields[i].type;
          llvm::Value *value = b_.CreateLoad(c_scalar_type(type), b_.CreateStructGEP(native_layout, error, i));
          if (type == Type::boolean) value = b_.CreateTrunc(value, b_.getInt1Ty());
          b_.CreateStore(value, b_.CreateStructGEP(layout, payload, i));
        }
      });
    }
    b_.SetInsertPoint(invalid);
    fail_path(ExceptionCategory::system, expr.at, b_.CreateGlobalString(location_text(expr.at) + ": invalid C constructor completion"));
    b_.SetInsertPoint(ready);
    return true;
  }
  llvm::Value *acquire_native_handle(const Expression &binding, size_t place,
                                     const CheckedOperation::NativeAcquisition &plan) {
    auto *resource = storage_.at(place);
    if (!call_native_constructor(plan, resource)) return nullptr;
    const auto action = std::ranges::find_if(scope_->actions, [&](const auto &item) {
      return item.action == &binding && item.native_constructor == plan.constructor;
    });
    if (action == scope_->actions.end()) throw std::logic_error("uncollected native handle release");
    // Direct declarations execute once per owning scope activation. Their
    // cleanup record is already reserved when native acquisition succeeds.
    auto *record = slot(action->layout, "native.cleanup");
    prepare_cleanup_record(*scope_, record, std::distance(scope_->actions.begin(), action));
    b_.CreateStore(load(resource), b_.CreateStructGEP(action->layout, record, 1));
    b_.CreateStore(record, scope_->head);
    return empty_scalar();
  }
  llvm::StructType *scoped_effect_layout(const CheckedInstance &instance) {
    const auto &target = program_->methods.at(instance.method);
    const auto &call = call_layouts_.at(instance.method);
    const auto explicit_inputs = target.scope_inputs->named->explicit_parameters;
    std::vector<llvm::Type *> fields(call.operands - explicit_inputs, scalar_);
    fields.insert(fields.end(), call.owners, b_.getPtrTy());
    fields.insert(fields.end(), call.array_owners.size(), array_capability_);
    for (const auto symbol : instance.scoped_replay_bindings)
      fields.push_back(symbols_.at(symbol)->getValueType());
    return llvm::StructType::get(context_, fields);
  }
  llvm::Value *install_scoped_effect(const Expression &expr, const CheckedOperation &operation) {
    if (!operation.publishes_effect) return empty_scalar();
    const auto &instance = program_->instances.at(*operation.scoped_effect);
    const auto method = instance.method;
    auto *layout = scoped_effect_layout(instance);
    auto *record = slot(layout, "scoped.effect.activation");
    unsigned field = 0;
    for (size_t i = 0; i < operation.arguments.size(); ++i) {
      const auto &input = operation.argument(expr, i);
      const auto projection = operation.arguments[i].binding_slot ? Projection::slot : Projection::target;
      auto *address = emit_expression(input, projection);
      if (!reachable()) return nullptr;
      b_.CreateStore(pack_view(address, input), b_.CreateStructGEP(layout, record, field++));
    }
    for (const auto &[owner, input] : input_frame(method).owners) {
      auto *identity = owner_identity(operation.scope_owners.at(null_target - 1 - owner), expr.at);
      b_.CreateStore(identity, b_.CreateStructGEP(layout, record, field++));
    }
    for (const auto role : call_layouts_.at(method).array_owners) {
      auto *capability = array_capability(operation.array_allocator_arguments->at(role));
      b_.CreateStore(capability, b_.CreateStructGEP(layout, record, field++));
    }
    for (const auto symbol : instance.scoped_replay_bindings) {
      auto *binding = symbols_.at(symbol);
      auto *saved = b_.CreateLoad(binding->getValueType(), binding, "scoped.replay.capture");
      b_.CreateStore(saved, b_.CreateStructGEP(layout, record, field++));
    }
    if (field != layout->getNumElements()) throw std::logic_error("invalid scoped effect activation layout");
    b_.CreateStore(record, symbols_.at(*operation.symbol));
    return empty_scalar();
  }
  llvm::Value *invoke_scoped_effect(const Expression &expr, size_t symbol) {
    auto *record = b_.CreateLoad(b_.getPtrTy(), symbols_.at(symbol), "scoped.effect.selected");
    return temporary_scope(expr.at, scalar_, [&]() -> llvm::Value * {
      std::vector<llvm::Value *> arguments;
      for (size_t i = 0; i < expr.children.size(); ++i) {
        arguments.push_back(prepare_argument(expr, i, true));
        if (!reachable()) return nullptr;
      }
      const auto target = program_->symbols.at(symbol).scoped_instance.value();
      const auto &instance = program_->instances.at(target);
      const auto method = instance.method;
      auto *callee = methods_.at(method);
      auto *valid = block("scoped.effect.installed"), *missing = block("scoped.effect.unbound");
      b_.CreateCondBr(b_.CreateIsNotNull(record), valid, missing);
      b_.SetInsertPoint(missing);
      fail_path(ExceptionCategory::unsound_runtime_typing, expr.at);
      b_.SetInsertPoint(valid);
      auto *layout = scoped_effect_layout(instance);
      const auto &replay = instance.scoped_replay_bindings;
      const auto inputs = layout->getNumElements() - replay.size();
      for (unsigned field = 0; field < inputs; ++field)
        arguments.push_back(b_.CreateLoad(layout->getElementType(field), b_.CreateStructGEP(layout, record, field)));
      if (callee->arg_size() != arguments.size()) throw std::logic_error("invalid scoped effect invocation layout");
      SavedEffects previous;
      if (expr.kind == Expression::Kind::perform) {
        auto field = inputs;
        for (const auto symbol : replay) {
          auto *binding = symbols_.at(symbol);
          auto *type = binding->getValueType();
          previous.push_back({symbol, b_.CreateLoad(type, binding, "scoped.replay.previous")});
          auto *saved = b_.CreateLoad(type, b_.CreateStructGEP(layout, record, field++));
          b_.CreateStore(saved, binding);
        }
      }
      // The raw outcome returns after the selected body's cleanup has run.
      // Restore the caller before propagating any abrupt outcome.
      auto *result = b_.CreateCall(callee, arguments);
      restore_effects(previous);
      return propagate(result);
    });
  }
  llvm::Value *region_address(llvm::Value *owner, llvm::Value *offset, llvm::Value *size) {
    auto address = runtime("cod_lang_region_address", b_.getPtrTy(),
        {b_.getPtrTy(), b_.getInt64Ty(), b_.getInt64Ty()});
    return b_.CreateCall(address, {owner, offset, size});
  }
  llvm::Value *regional_scalar_locator(llvm::Value *owner, llvm::Value *address) {
    auto *base = region_address(owner, integer(0), integer(0));
    auto *offset = b_.CreateSub(b_.CreatePtrToInt(address, b_.getInt64Ty()),
                               b_.CreatePtrToInt(base, b_.getInt64Ty()));
    return b_.CreateSelect(b_.CreateIsNull(address), integer(UINT64_MAX), offset);
  }
  llvm::Value *regional_scalar_target(llvm::Value *owner, llvm::Value *offset, llvm::Type *layout) {
    auto *entry = b_.GetInsertBlock();
    auto *present = block("region.field.present"), *join = block("region.field.target");
    b_.CreateCondBr(b_.CreateICmpNE(offset, integer(UINT64_MAX)), present, join);
    b_.SetInsertPoint(present);
    const auto bytes = module_.getDataLayout().getTypeAllocSize(layout).getFixedValue();
    auto *address = region_address(owner, offset, integer(bytes));
    b_.CreateBr(join);
    b_.SetInsertPoint(join);
    auto *target = b_.CreatePHI(b_.getPtrTy(), 2);
    target->addIncoming(null_pointer(), entry);
    target->addIncoming(address, present);
    return target;
  }
  llvm::Value *reserve_regional_row(llvm::Value *owner, llvm::Type *layout, const Location &at) {
    const auto &target = module_.getDataLayout();
    const auto bytes = target.getTypeAllocSize(layout).getFixedValue();
    auto *offset = reserve_region_extent(owner, integer(bytes), target.getABITypeAlign(layout).value(), at);
    return region_address(owner, offset, integer(bytes));
  }
  llvm::Value *construct_region(const Expression &make) {
    const auto &input = *make.children.front();
    auto *capacity_value = emit_expression(input);
    if (!reachable()) return nullptr;
    auto *capacity = choose(input, {capacity_value}, choices(input, {&input}), b_.getInt64Ty(),
        [&](const Choice &choice) { return convert(capacity_value, choice.inputs.front(), Type::uint64); });
    using Plan = std::pair<size_t, std::vector<size_t>>;
    std::map<Plan, std::vector<size_t>> plans;
    for (size_t i = 0; i < contexts_.size(); ++i) {
      const auto &fact = contexts_[i].at(&make);
      if (contexts_[i].at(&input).type != Type::never)
        plans[{fact.operand_type.identity, fact.region_initializers}].push_back(i);
    }
    return contextual_value(make, plans, "region.acquire", [&](const Plan &plan) -> llvm::Value * {
      const auto &[schema_id, order] = plan;
      auto *layout = product_layout(schema_id);
      const auto &roots = program_->schemas.at(schema_id).fields;
      const auto &target = module_.getDataLayout();
      const auto bytes = roots.empty() ? 0 : target.getTypeAllocSize(layout).getFixedValue();
      const auto alignment = target.getABITypeAlign(layout).value();
      constexpr auto limit = std::uint64_t{std::numeric_limits<std::ptrdiff_t>::max()};
      if (bytes > limit || alignment > alignof(std::max_align_t))
        implementation_gap(make.at, "target layout for region roots");

      auto *valid = block("region.capacity.valid"), *invalid = block("region.capacity.invalid");
      b_.CreateCondBr(b_.CreateICmpULE(capacity, integer(limit)), valid, invalid);
      b_.SetInsertPoint(invalid);
      fail_path(ExceptionCategory::invalid_capacity, make.at);
      b_.SetInsertPoint(valid);
      auto *allocate = block("region.allocate"), *full = block("region.capacity.full");
      b_.CreateCondBr(b_.CreateICmpULE(integer(bytes), capacity), allocate, full);
      b_.SetInsertPoint(full);
      save_abrupt(outcome(empty_scalar(), integer(exception_id(ExceptionCategory::region_capacity_exceeded)),
                          integer(site(make.at)), null_pointer(), integer(bytes), capacity));
      b_.CreateBr(control_->exit);
      b_.SetInsertPoint(allocate);
      auto acquire = runtime("cod_lang_region_acquire", b_.getPtrTy(), {b_.getInt64Ty()});
      auto *owner = b_.CreateCall(acquire, {capacity}, "region.owner");
      auto *ready = block("region.acquired"), *unavailable = block("region.unavailable");
      b_.CreateCondBr(b_.CreateIsNotNull(owner), ready, unavailable);
      b_.SetInsertPoint(unavailable);
      fail_path(ExceptionCategory::out_of_memory, make.at);
      b_.SetInsertPoint(ready);

      Control construction{control_, block("region.abandoned")};
      control_ = &construction;
      auto reserve = runtime("cod_lang_region_reserve", b_.getInt64Ty(),
          {b_.getPtrTy(), b_.getInt64Ty(), b_.getInt64Ty()});
      auto *offset = b_.CreateCall(reserve, {owner, integer(bytes), integer(alignment)}, "region.root.offset");
      auto *reserved = block("region.root.reserved"), *failed = block("region.root.failed");
      b_.CreateCondBr(b_.CreateICmpEQ(offset, integer(0)), reserved, failed);
      b_.SetInsertPoint(failed);
      fail_path(ExceptionCategory::unsound_runtime_typing, make.at);
      b_.SetInsertPoint(reserved);
      const auto items = make.initializer_items();
      if (!items.empty()) {
        auto *root = region_address(owner, offset, integer(bytes));
        for (size_t i = 0; i < order.size(); ++i) {
          const auto &value = *items[i]->children.back();
          const auto &type = roots[order[i]].type;
          const bool aggregate = inline_aggregate_type(type);
          auto *operand = emit_expression(value, aggregate ? Projection::target : Projection::value);
          if (!reachable()) break;
          auto *destination = b_.CreateStructGEP(layout, root, order[i]);
          if (aggregate) b_.CreateCall(bits_transfer(type), {destination, checked_address(operand, value, value.at)});
          else {
            auto *prepared = choose(value, {operand}, choices(value, {&value}), scalar_, [&](const Choice &choice) {
              return convert_value(operand, choice.inputs.front(), type);
            });
            store_place(destination, prepared, StorageFormat{type});
          }
        }
      }
      control_ = construction.parent;
      auto *completed = reachable() ? block("region.initialized") : nullptr;
      if (completed) b_.CreateBr(completed);
      b_.SetInsertPoint(construction.exit);
      b_.CreateCall(runtime("cod_lang_region_release", b_.getVoidTy(), {b_.getPtrTy()}), {owner});
      b_.CreateBr(control_->exit);
      if (!completed) return nullptr;
      b_.SetInsertPoint(completed);

      return owner;
    }, b_.getPtrTy());
  }
  llvm::Value *acquire_region(const Expression &binding) {
    const auto *initializer = binding.children.front().get();
    while (initializer->kind == Expression::Kind::grouped) initializer = initializer->children.front().get();
    llvm::Value *owner;
    if (initializer->kind == Expression::Kind::make_region) {
      owner = construct_region(*initializer);
    } else {
      auto *result = emit_expression(*initializer);
      if (!reachable()) return nullptr;
      owner = b_.CreateIntToPtr(b_.CreateExtractValue(result, 0), b_.getPtrTy());
    }
    if (!reachable()) return nullptr;
    std::map<size_t, std::vector<size_t>> holders;
    for (size_t i = 0; i < contexts_.size(); ++i) {
      const auto &fact = contexts_[i].at(&binding);
      if (fact.region_holder) holders[*fact.region_holder].push_back(i);
    }
    return contextual_value(binding, holders, "region.enroll", [&](size_t holder) -> llvm::Value * {
      const auto action = std::ranges::find_if(scope_->actions, [&](const auto &entry) {
        return entry.region_release && entry.action == &binding;
      });
      if (action == scope_->actions.end()) throw std::logic_error("uncollected region release");
      auto *record = action->activation_record;
      prepare_cleanup_record(*scope_, record, std::distance(scope_->actions.begin(), action));
      b_.CreateStore(owner, b_.CreateStructGEP(action->layout, record, 1));
      b_.CreateStore(record, scope_->head);
      b_.CreateStore(owner, storage_.at(holder));
      return empty_scalar();
    });
  }
  llvm::Value *receive_exception_holder(const Expression &binding) {
    using Reception = std::pair<size_t, std::optional<size_t>>;
    std::map<std::optional<Reception>, std::vector<size_t>> choices;
    for (size_t i = 0; i < contexts_.size(); ++i) {
      const auto &fact = contexts_[i].at(&binding);
      const auto plan = fact.type == Type::never ? std::optional<Reception>{}
          : std::optional{Reception{fact.place.value(), fact.exception_owner}};
      choices[plan].push_back(i);
    }
    return contextual_value(binding, choices, "exception.receive", [&](const auto &selected) {
      if (!selected) return emit_expression(*binding.children.front());
      const auto [place, source] = *selected;
      auto [destination, inserted] = exception_holder_slots_.try_emplace(place, nullptr);
      if (inserted) destination->second = slot(b_.getPtrTy(), "exception.owner");
      llvm::Value *value;
      if (source) {
        value = load(storage_.at(*source));
        auto *owner = take_failure_owner(b_, exception_holder_slots_.at(*source));
        b_.CreateStore(owner, destination->second);
      } else {
        b_.CreateStore(null_pointer(), destination->second);
        auto *previous = std::exchange(exception_result_destination_, destination->second);
        value = emit_expression(*binding.children.front());
        exception_result_destination_ = previous;
        if (!reachable()) return static_cast<llvm::Value *>(nullptr);
      }
      b_.CreateStore(value, storage_.at(place));
      const auto action = std::ranges::find_if(scope_->actions, [&](const auto &entry) {
        return entry.exception_release && entry.action == &binding;
      });
      if (action == scope_->actions.end()) throw std::logic_error("uncollected exception holder release");
      auto *record = action->activation_record;
      prepare_cleanup_record(*scope_, record, std::distance(scope_->actions.begin(), action));
      b_.CreateStore(destination->second, b_.CreateStructGEP(action->layout, record, 1));
      b_.CreateStore(record, scope_->head);
      return static_cast<llvm::Value *>(empty_scalar());
    });
  }
  llvm::Value *install_native_effect(const Expression &effect) {
    using Plan = std::pair<size_t, CheckedOperation::NativeAcquisition>;
    std::map<Plan, std::vector<size_t>> plans;
    for (size_t i = 0; i < contexts_.size(); ++i) {
      const auto &operation = contexts_[i].at(&effect);
      plans[{*operation.symbol, *operation.native_acquisition}].push_back(i);
    }
    return contextual_value(effect, plans, "native.effect", [&](const Plan &plan) {
      return temporary_scope(effect.at, scalar_, [&]() -> llvm::Value * {
        const auto &[symbol, acquisition] = plan;
        auto &scope = *native_effect_scope_;
        const auto action = std::ranges::find_if(scope.actions, [&](const auto &item) {
          return item.action == &effect && item.native_constructor == acquisition.constructor && item.native_symbol == symbol;
        });
        if (action == scope.actions.end()) throw std::logic_error("uncollected native effect release");
        auto *record = action->activation_record;
        auto *pair_type = native_handle_pair();
        auto *pair = b_.CreateStructGEP(action->layout, record, 1);
        auto *resource = b_.CreateStructGEP(pair_type, pair, 0);
        if (!call_native_constructor(acquisition, resource)) return nullptr;
        b_.CreateStore(b_.CreateLoad(b_.getInt64Ty(), resource), b_.CreateStructGEP(pair_type, pair, 1));
        auto *binding = symbols_.at(symbol);
        b_.CreateStore(b_.CreateLoad(scalar_, binding), b_.CreateStructGEP(action->layout, record, 2));
        prepare_cleanup_record(scope, record, std::distance(scope.actions.begin(), action));
        auto *selected = b_.CreateInsertValue(empty_scalar(), b_.CreatePtrToInt(pair, b_.getInt64Ty()), 0);
        b_.CreateStore(selected, binding);
        b_.CreateStore(record, scope.head);
        return empty_scalar();
      });
    });
  }
  llvm::Value *emit_attached_native_handle(const Expression &expr) {
    std::map<CheckedOperation::NativeAcquisition, std::vector<size_t>> plans;
    for (size_t i = 0; i < contexts_.size(); ++i)
      plans[*contexts_[i].at(&expr).native_acquisition].push_back(i);
    return contextual_value(expr, plans, "native.attach", [&](const auto &plan) {
      return temporary_scope(expr.at, b_.getPtrTy(), [&]() -> llvm::Value * {
        auto *owner = plan.parent_borrow ? binding_address(*plan.parent_borrow, plan.parent_place)
                                        : native_reborrow(load(storage_.at(*plan.parent_place)));
        auto *pair = native_handle_pair();
        auto *owner_word = b_.CreateLoad(b_.getInt64Ty(), b_.CreateStructGEP(pair, owner, 1));
        auto *child = slot(pair, "native.child.borrow");
        if (!call_native_constructor(plan, b_.CreateStructGEP(pair, child, 0), owner)) return nullptr;
        b_.CreateStore(owner_word, b_.CreateStructGEP(pair, child, 1));
        return child;
      });
    }, b_.getPtrTy());
  }
  llvm::Value *catch_matches(llvm::Value *category, size_t identity) {
    if (!identity) return b_.getTrue();
    if (identity != error_root_id) return b_.CreateICmpEQ(category, integer(identity));
    llvm::Value *matched = b_.getFalse();
    for (size_t i = 1; i < program_->exception_types.size(); ++i)
      if (matches_failure(program_->exception_types, i, identity))
        matched = b_.CreateOr(matched, b_.CreateICmpEQ(category, integer(i)));
    return matched;
  }
  llvm::Value *emit_finally(const Expression &expr, Projection projection, bool discard = false) {
    const bool read = carries_value(projection);
    Control guarded{control_, block("try.completed")};
    control_ = &guarded;
    auto *saved = read && environments_ ? owned_slot(*temporaries_, "try.value")
        : slot(read ? scalar_ : static_cast<llvm::Type *>(b_.getPtrTy()), "try.value");
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
      if (handler.kind == Expression::Kind::block) {
        if (!expr.deferred_catch()) finalizer = &handler;
        break;
      }
      const auto &fact = contexts_.front().at(&handler);
      auto *selected = block("catch.selected"), *next = block("catch.next");
      auto *caught = choose(handler, {}, choices(handler, {}), scalar_, [&](const Choice &choice) -> llvm::Value * {
        const auto &type = choice.operation->type;
        auto *value = pack(active_exception(), type);
        return b_.CreateSelect(catch_matches(load(completion_.category), type.identity), value, empty_scalar());
      });
      b_.CreateCondBr(b_.CreateICmpNE(b_.CreateExtractValue(caught, 1), b_.getInt32(0)), selected, next);
      b_.SetInsertPoint(selected);
      b_.CreateStore(caught, storage_.at(*fact.place));
      auto *pending_transfer = protected_outcomes_ ? load(completion_.transfer) : nullptr;
      auto *caught_owner = owned_failures_ ? take_failure_owner(b_, completion_.failure_owner) : nullptr;
      llvm::AllocaInst *holder = nullptr;
      if (owned_failures_) {
        holder = slot(b_.getPtrTy(), "catch.owner");
        b_.CreateStore(caught_owner, holder);
        exception_holder_slots_.emplace(*fact.place, holder);
      }
      if (protected_outcomes_) b_.CreateStore(null_pointer(), completion_.transfer);
      b_.CreateStore(b_.getInt8(0), completion_.kind); b_.CreateStore(integer(0), completion_.category); b_.CreateStore(null_pointer(), completion_.fatal);
      b_.CreateStore(b_.getFalse(), completion_.has_return);
      clear_value(completion_.value);
      Control catching{control_, protected_outcomes_ || owned_failures_ ? block("catch.completed") : handled};
      control_ = &catching;
      auto *result = emit_expression(*handler.children.front(), projection, discard);
      if (reachable()) { if (read) assign_value(saved, discard ? empty_scalar() : result, handler.at); else b_.CreateStore(result, saved); b_.CreateBr(catching.exit); }
      if (protected_outcomes_ || owned_failures_) {
        b_.SetInsertPoint(catching.exit);
        if (holder) release_failure_owner(b_, take_failure_owner(b_, holder));
        preserve_transfer(pending_transfer);
        b_.CreateBr(handled);
      }
      if (holder) exception_holder_slots_.erase(*fact.place);
      control_ = catching.parent;
      b_.SetInsertPoint(next);
    }
    b_.CreateBr(handled);
    b_.SetInsertPoint(handled);
    if (finalizer) complete_cleanup([&] { emit_expression(*finalizer); }, CleanupBoundary::ordinary_finally);
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
  static std::optional<DeferredInputs> deferred_inputs(const Expression &expr, const CheckedOperation &fact) {
    if (expr.kind == Expression::Kind::block || fact.type == Type::never) return {};
    const size_t prefix = expr.kind != Expression::Kind::value_call && fact.callable_call ? 1 : 0;
    DeferredInputs result{fact.argument_count(expr) + prefix +
        (fact.callee ? fact.callable_captures.size() : 0), {}};
    for (size_t i = 0; i < fact.arguments.size(); ++i)
      if (fact.arguments[i].mode == ArgumentMode::borrowed_handle)
        result.handle_arguments.emplace(prefix + i, i);
    return result;
  }
  static bool inert_argument(const Expression &expr) {
    using K = Expression::Kind;
    return !expr.checked_ref_argument && (expr.kind == K::name || expr.kind == K::integer || expr.kind == K::real ||
                                         expr.kind == K::boolean || expr.kind == K::null_ref);
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
  llvm::Value *prepare_argument(const Expression &expr, size_t index, bool copy,
                                std::vector<llvm::Value *> *current_owners = nullptr) {
    const auto &call = contexts_.front().at(&expr);
    const auto &child = call.argument(expr, index);
    if (const auto symbol = call.arguments.at(index).current_reference_symbol) {
      if (!current_owners || std::ranges::any_of(contexts_, [&](const auto &context) {
            return context.at(&expr).arguments.at(index).current_reference_symbol != symbol;
          })) throw std::logic_error("current reference argument lacks a uniform owner preparation");
      // Both projections must survive preparation of all later operands.
      auto *binding = symbols_.at(*symbol);
      auto *selected = b_.CreateLoad(binding->getValueType(), binding, "effect.current.argument");
      if (current_owners->empty()) current_owners->resize(call.argument_count(expr));
      (*current_owners)[index] = b_.CreateExtractValue(selected, 1, "effect.argument.owner");
      return b_.CreateExtractValue(selected, 0, "effect.argument.selection");
    }
    const auto consumes_exception = [&](const auto &context) {
      return context.at(&expr).arguments.at(index).mode == ArgumentMode::owning_exception;
    };
    const bool owning = consumes_exception(contexts_.front());
    if (std::ranges::any_of(contexts_, [&](const auto &context) { return consumes_exception(context) != owning; })) {
      std::map<bool, std::vector<size_t>> modes;
      for (size_t i = 0; i < contexts_.size(); ++i) modes[consumes_exception(contexts_[i])].push_back(i);
      return contextual_value(child, modes, "exception.operand.mode", [&](bool) { return prepare_argument(expr, index, copy); });
    }
    if (owning) {
      std::map<std::pair<size_t, unsigned>, std::vector<size_t>> holders;
      for (size_t i = 0; i < contexts_.size(); ++i) {
        const auto &argument = contexts_[i].at(&expr).arguments.at(index);
        const auto &operand = contexts_[i].at(&child);
        if (!operand.exception_owner)
          throw std::logic_error("an owning exception argument lacks its checked holder");
        holders[{*operand.exception_owner, tag(argument.type)}].push_back(i);
      }
      return contextual_value(child, holders, "exception.argument", [&](const auto &selected) {
        const auto &[holder, type_tag] = selected;
        auto *layout = exception_input_layout();
        auto *input = slot(layout, "exception.input");
        b_.CreateStore(storage_.at(holder), b_.CreateStructGEP(layout, input, 0));
        b_.CreateStore(exception_holder_slots_.at(holder), b_.CreateStructGEP(layout, input, 1));
        auto *carrier = b_.CreateInsertValue(empty_scalar(), b_.CreatePtrToInt(input, b_.getInt64Ty()), 0);
        return b_.CreateInsertValue(carrier, b_.getInt32(type_tag), 1);
      });
    }
    using Preparation = std::tuple<ArgumentMode, bool, std::optional<size_t>, bool>;
    const auto prepare = [&](const Preparation &plan) -> llvm::Value * {
      const auto [mode, binding_slot, reference_storage, mixed] = plan;
      if (reference_storage) {
        auto *storage = storage_.at(*reference_storage);
        const auto construct = [&](llvm::Value *value) { store_incoming(*reference_storage, value); };
        llvm::Value *target = storage;
        if (mixed)
          target = initialize_reference_target(child, storage, construct);
        else {
          auto *value = emit_expression(child);
          if (!reachable()) return nullptr;
          construct(value);
        }
        return reachable() ? pack_view(target, child) : nullptr;
      }
      if (contexts_.front().at(&child).type.kind == Type::abstract_type) {
        const auto *source = &child;
        while (source->kind == Expression::Kind::grouped) source = source->children.front().get();
        auto *binding = symbols_.at(*contexts_.front().at(source).symbol);
        auto *selected = b_.CreateLoad(binding->getValueType(), binding, "effect.current");
        return b_.CreateExtractValue(selected, 0, "effect.selection");
      }
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
    std::map<Preparation, std::vector<size_t>> alternatives;
    for (size_t i = 0; i < contexts_.size(); ++i) {
      const auto &argument = contexts_[i].at(&expr).arguments.at(index);
      const auto &storage = argument.reference_storage;
      alternatives[{argument.mode, argument.binding_slot, storage ? std::optional{storage->place} : std::nullopt,
                    storage && contexts_[i].at(&child).mixed_view}].push_back(i);
    }
    return contextual_value(child, alternatives, "argument", prepare);
  }
  std::vector<llvm::Value *> prepare_arguments(const Expression &expr, bool deferred = false,
                                               std::span<llvm::Value *> here_values = {}, llvm::Value *first_place = nullptr,
                                               std::vector<llvm::Value *> *current_owners = nullptr) {
    using Preparation = CheckedOperation::OperandPreparation;
    std::vector<llvm::Value *> arguments;
    const auto &fact = contexts_.front().at(&expr);
    const size_t offset = expr.kind == Expression::Kind::value_call && fact.callable_call ? 1 : 0;
    bool protect = deferred || std::ranges::any_of(fact.here_arguments, [](const auto &input) { return input.source || input.traversal; });
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
    const auto &schedule = fact.operand_preparation;
    const auto count = schedule.empty() ? fact.argument_count(expr) : schedule.size();
    for (size_t written = 0; written < count; ++written) {
      const auto [kind, slot] = schedule.empty() ? Preparation{Preparation::Kind::ordinary, written} : schedule[written];
      if (kind == Preparation::Kind::here) {
        if (slot >= here_values.size()) throw std::logic_error("missing checked here operand slot");
        here_values[slot] = prepare_here_argument(expr, slot);
      } else if (first_place && slot == 0) arguments[first] = pack_view(first_place, fact.argument(expr, 0));
      else arguments[first + slot] = prepare_argument(expr, slot, !offset || slot || protect, current_owners);
      if (!reachable()) return {};
    }
    check_destination_arguments(expr, arguments);
    arguments.insert(arguments.end(), captured.begin(), captured.end());
    return arguments;
  }
  llvm::Value *emit_text_transfer(const Expression &expr, llvm::Value *destination = nullptr) {
    return temporary_scope(expr.at, scalar_, [&]() -> llvm::Value * {
      const auto arguments = prepare_arguments(expr, false, {}, destination);
      return reachable() ? propagate(invoke(expr, arguments)) : nullptr;
    });
  }
  llvm::StructType *prepared_producer_layout(size_t operands, size_t owners) {
    std::vector<llvm::Type *> fields(operands, scalar_);
    fields.insert(fields.end(), owners, b_.getPtrTy());
    return llvm::StructType::get(context_, fields);
  }
  llvm::StructType *prepared_connection_layout() {
    return llvm::StructType::get(context_, {b_.getPtrTy(), here_});
  }
  template <typename Emit>
  llvm::Value *with_prepared_producer(const Expression &expr, const Emit &emit) {
    std::map<CheckedOperation::PreparedProducer, std::vector<size_t>> producers;
    for (size_t i = 0; i < contexts_.size(); ++i)
      producers[contexts_[i].at(&expr).prepared_producer.value()].push_back(i);
    return contextual_value(expr, producers, "here.producer", [&](const auto &producer) {
      const auto method = program_->instances[producer.instance].method;
      return emit(producer, input_frame(method));
    }, here_);
  }
  llvm::Value *prepare_source(const Expression &expr) {
    if (contexts_.front().at(&expr).open_pipeline_operands) {
      const auto &operands = *contexts_.front().at(&expr).open_pipeline_operands;
      auto *layout = llvm::ArrayType::get(here_, operands.size());
      auto *backing = slot(layout, "here.open.stages");
      for (size_t i = 0; i < operands.size(); ++i) {
        auto *address = b_.CreateInBoundsGEP(layout, backing, {b_.getInt32(0), b_.getInt64(i)});
        b_.CreateStore(load(pipeline_operand_slot(*operands[i])), address);
      }
      return b_.CreateInsertValue(llvm::Constant::getNullValue(here_), backing, 1);
    }
    if (contexts_.front().at(&expr).pipeline_preparation) {
      std::map<CheckedOperation::PipelinePreparation, std::vector<size_t>> plans;
      for (size_t i = 0; i < contexts_.size(); ++i)
        plans[contexts_[i].at(&expr).pipeline_preparation.value()].push_back(i);
      return contextual_value(expr, plans, "here.saved.traversal", [&](const auto &plan) {
        return prepare_traversal(plan);
      }, here_);
    }
    return with_prepared_producer(expr, [&](const auto &producer, const auto &frame) -> llvm::Value * {
      const auto operands = prepare_arguments(expr);
      if (!reachable()) return nullptr;
      const auto &owners = frame.owners;
      llvm::Value *activation = null_pointer();
      if (!operands.empty() || !owners.empty()) {
        auto *storage = prepared_producer_layout(operands.size(), owners.size());
        activation = slot(storage, "here.source.operands");
        for (size_t i = 0; i < operands.size(); ++i)
          b_.CreateStore(operands[i], b_.CreateStructGEP(storage, activation, i));
        auto i = operands.size();
        for (const auto &[owner, input] : owners) {
          const auto actual = producer.owners.at(null_target - 1 - owner);
          b_.CreateStore(owner_identity(actual, expr.at), b_.CreateStructGEP(storage, activation, i++));
        }
      }
      // The opening call selects the checked entry for its actual consumer.
      return b_.CreateInsertValue(llvm::Constant::getNullValue(here_), activation, 1);
    });
  }
  llvm::Value *connect_prepared_stage(llvm::Value *stage, llvm::Value *upstream) {
    auto *layout = prepared_connection_layout();
    auto *connection = slot(layout, "here.connection");
    b_.CreateStore(b_.CreateExtractValue(stage, 1), b_.CreateStructGEP(layout, connection, 0));
    b_.CreateStore(upstream, b_.CreateStructGEP(layout, connection, 1));
    return b_.CreateInsertValue(stage, connection, 1);
  }
  llvm::Value *connect_prepared_pipeline(std::span<const Expression *const> traversal, const auto &operand) {
    auto *upstream = operand(traversal.front());
    for (const auto *stage : traversal.subspan(1)) upstream = connect_prepared_stage(operand(stage), upstream);
    return upstream;
  }
  llvm::Value *prepare_traversal(const CheckedOperation::PipelinePreparation &plan) {
    std::map<const Expression *, llvm::Value *> prepared;
    for (const auto *operand : plan.written) {
      auto *value = plan.lexical_source && operand == plan.traversal.front()
          ? here_arguments_.at(*plan.lexical_source) : prepare_source(*operand);
      if (!reachable()) return nullptr;
      prepared.emplace(operand, value);
    }
    return connect_prepared_pipeline(plan.traversal, [&](const Expression *operand) {
      const auto found = prepared.find(operand);
      return found == prepared.end() ? load(pipeline_operand_slot(*operand)) : found->second;
    });
  }
  llvm::AllocaInst *pipeline_operand_slot(const Expression &expr) {
    auto [position, inserted] = pipeline_operands_.try_emplace(&expr, nullptr);
    if (inserted) position->second = slot(here_, "here.saved.operand");
    return position->second;
  }
  std::vector<llvm::Value *> load_prepared_operands(const Expression &expr, llvm::Value *prepared) {
    // The consumer always runs in the supplying activation, which has the original owner evidence.
    const auto &fact = contexts_.front().at(&expr);
    auto *backing = b_.CreateExtractValue(prepared, 1);
    std::vector<llvm::Value *> operands(fact.argument_count(expr));
    auto *layout = prepared_producer_layout(operands.size(), 0);
    for (size_t i = 0; i < operands.size(); ++i) {
      auto *address = b_.CreateStructGEP(layout, backing, i);
      operands[i] = b_.CreateLoad(scalar_, address);
    }
    return operands;
  }
  llvm::Value *save_pipeline_operands(const Expression &expr, const CheckedOperation::PipelinePreparation &plan) {
    return temporary_scope(expr.at, scalar_, [&]() -> llvm::Value * {
      for (const auto *operand : plan.written) {
        auto *value = prepare_source(*operand);
        if (!reachable()) return nullptr;
        b_.CreateStore(value, pipeline_operand_slot(*operand));
      }
      return empty_scalar();
    });
  }
  std::vector<llvm::Value *> prepare_pipeline(const Expression &expr,
      const CheckedOperation::PipelinePreparation &plan, std::span<llvm::Value *> here_values) {
    const auto *receiver = plan.traversal.back();
    std::map<const Expression *, llvm::Value *> prepared;
    std::vector<llvm::Value *> arguments;
    bool fresh_receiver = false;
    for (const auto *operand : plan.written) {
      if (operand == receiver) { arguments = prepare_arguments(expr); fresh_receiver = true; }
      else if (operand == plan.traversal.front() && plan.lexical_source) prepared.emplace(operand, here_arguments_.at(*plan.lexical_source));
      else prepared.emplace(operand, prepare_source(*operand));
      if (!reachable()) return {};
    }
    const auto stored = [&](const Expression *operand) {
      const auto found = prepared.find(operand);
      return found == prepared.end() ? load(pipeline_operand_slot(*operand)) : found->second;
    };
    if (!fresh_receiver) arguments = load_prepared_operands(expr, stored(receiver));
    if (plan.traversal.size() > 1) {
      const auto upstream = std::span{plan.traversal}.first(plan.traversal.size() - 1);
      here_values[1] = connect_prepared_pipeline(upstream, stored);
    }
    return arguments;
  }
  llvm::Value *prepare_here_argument(const Expression &expr, size_t slot) {
    std::map<CheckedOperation::HereArgument, std::vector<size_t>> arguments;
    for (size_t i = 0; i < contexts_.size(); ++i)
      arguments[contexts_[i].at(&expr).here_arguments.at(slot)].push_back(i);
    return contextual_value(expr, arguments, "here.input", [&](const auto &input) {
      if (input.traversal) return prepare_traversal(*input.traversal);
      if (input.source) return prepare_source(*input.source);
      return here_arguments_.at(input.lexical.value());
    }, here_);
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
      assign_place(destination, arguments[1], expr);
      return destination;
    }
    auto *target = view_address(arguments[1]);
    std::map<std::optional<size_t>, std::vector<size_t>> owners;
    for (size_t i = 0; i < contexts_.size(); ++i)
      owners[contexts_[i].at(expr.children.front().get()).regional_product_holder].push_back(i);
    return contextual_value(expr, owners, "reference.store", [&](const auto &holder) {
      if (holder) b_.CreateStore(regional_scalar_locator(load(storage_.at(*holder)), target), destination);
      else b_.CreateStore(target, destination);
      return target;
    }, b_.getPtrTy());
  }
  llvm::Value *captured_value(const CheckedOperation &fact, const Location &at) {
    if (fact.callable) return own_value(materialize(fact, fact.type, at));
    auto *address = binding_address(*fact.binding, fact.place);
    return load_place(b_, address, StorageFormat{fact.type});
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
    const auto &symbols = program_->captured_symbols(program_->callables[*fact.callable]);
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
  template <typename Construct>
  llvm::Value *initialize_reference_target(const Expression &source, llvm::Value *storage, const Construct &initialize) {
    auto *value = emit_expression(source, Projection::receiver);
    if (!reachable()) return nullptr;
    auto *existing = b_.GetInsertBlock(), *construct = block("storage.construct"), *ready = block("storage.ready");
    auto *target = view_address(value);
    b_.CreateCondBr(b_.CreateICmpEQ(b_.CreateExtractValue(value, 1), b_.getInt32(0)), ready, construct);
    b_.SetInsertPoint(construct);
    initialize(value);
    auto *constructed = b_.GetInsertBlock(); b_.CreateBr(ready);
    b_.SetInsertPoint(ready);
    auto *selected = b_.CreatePHI(b_.getPtrTy(), 2, "storage.target");
    selected->addIncoming(target, existing); selected->addIncoming(storage, constructed);
    return selected;
  }
  llvm::Value *declare_local(const Expression &declaration) {
    std::map<std::tuple<bool, bool, bool>, std::vector<size_t>> alternatives;
    for (size_t i = 0; i < contexts_.size(); ++i) {
      const auto &binding = contexts_[i].at(&declaration);
      const auto &source = contexts_[i].at(declaration.children.front().get());
      const bool same_place = binding.place && binding.place == source.place;
      alternatives[{binding.view, binding.mixed_view, same_place}].push_back(i);
    }
    return contextual_value(declaration, alternatives, "storage", [&](auto mode) {
      const auto [borrowed, mixed, same_place] = mode;
      const auto &binding = contexts_.front().at(&declaration);
      llvm::Value *address = binding.place ? static_cast<llvm::Value *>(storage_.at(*binding.place)) : null_pointer();
      if (environments_ && binding.place && storage_.at(*binding.place)->getAllocatedType() == scalar_)
        scope_->values.values.push_back(storage_.at(*binding.place));
      if (mixed) {
        address = initialize_reference_target(*declaration.children.front(), address, [&](llvm::Value *value) {
          assign_place(address, value, declaration, &CheckedOperation::operand_type);
        });
        if (!reachable()) return static_cast<llvm::Value *>(nullptr);
      } else if (borrowed || same_place) {
        address = emit_expression(*declaration.children.front(), Projection::target);
        if (!reachable()) return static_cast<llvm::Value *>(nullptr);
      } else {
        const auto *initializer = declaration.children.front().get();
        auto *value = emit_expression(*initializer);
        if (!reachable()) return static_cast<llvm::Value *>(nullptr);
        value = convert_result(declaration, *initializer, value);
        assign_place(address, value, declaration, &CheckedOperation::operand_type);
      }
      if (auto *slot = bindings_.at(*binding.binding).reference_slot) b_.CreateStore(address, slot);
      return empty_scalar();
    });
  }
  llvm::Value *emit_text_access(const Expression &expr, BuiltinOperation operation, bool discard) {
    auto *view = emit_expression(*expr.children[0], Projection::target);
    if (!reachable()) return nullptr;
    if (operation == BuiltinOperation::text_size) {
      auto *size = b_.CreateLoad(b_.getInt64Ty(), b_.CreateStructGEP(text_view_, view, 1));
      return pack(size, Type::uint64);
    }
    auto *first = emit_expression(*expr.children[1]);
    if (!reachable()) return nullptr;
    first = unpack(first, Type::uint64);
    if (operation == BuiltinOperation::text_byte) {
      auto *size = b_.CreateLoad(b_.getInt64Ty(), b_.CreateStructGEP(text_view_, view, 1));
      auto *ready = block("text.byte.ready"), *invalid = block("text.byte.invalid");
      b_.CreateCondBr(b_.CreateICmpULT(first, size), ready, invalid);
      b_.SetInsertPoint(invalid);
      save_abrupt(outcome(empty_scalar(), integer(exception_id(ExceptionCategory::index_out_of_bounds)),
                         integer(site(expr.at)), null_pointer(), first, size));
      b_.CreateBr(control_->exit);
      b_.SetInsertPoint(ready);
      auto *bytes = b_.CreateLoad(b_.getPtrTy(), b_.CreateStructGEP(text_view_, view, 0));
      auto *byte = b_.CreateLoad(b_.getInt8Ty(), b_.CreateGEP(b_.getInt8Ty(), bytes, first));
      return pack(byte, Type::uint8);
    }
    auto *past = emit_expression(*expr.children[2]);
    if (!reachable()) return nullptr;
    past = unpack(past, Type::uint64);
    auto *result = slot(b_.getPtrTy(), "text.slice");
    std::map<size_t, std::vector<size_t>> owners;
    for (size_t i = 0; i < contexts_.size(); ++i) {
      const auto &access = contexts_[i].at(&expr);
      if (access.type != Type::never) owners[access.lexical_allocation->owner].push_back(i);
    }
    auto *owner = contextual_value(expr, owners, "text.slice.owner", [&](size_t actual) {
      return owner_identity(actual, expr.at);
    }, b_.getPtrTy());
    auto slice = runtime("cod_lang_lexical_text_slice", b_.getInt32Ty(),
        {b_.getPtrTy(), b_.getPtrTy(), b_.getInt64Ty(), b_.getInt64Ty(), b_.getPtrTy()});
    auto *status = b_.CreateCall(slice, {owner, view, first, past, result});
    auto *ready = block("text.slice.ready"), *invalid = block("text.slice.invalid"), *failed = block("text.slice.failed");
    auto *completion = b_.CreateSwitch(status, failed, 2);
    completion->addCase(b_.getInt32(COD_TEXT_SLICE_READY), ready);
    completion->addCase(b_.getInt32(COD_TEXT_SLICE_INVALID), invalid);
    b_.SetInsertPoint(invalid);
    fail_path(ExceptionCategory::type_assertion_failed, expr.at);
    b_.SetInsertPoint(failed);
    fail_path(ExceptionCategory::out_of_memory, expr.at);
    b_.SetInsertPoint(ready);
    return discard ? empty_scalar() : load(result);
  }
  llvm::Value *emit_expression(const Expression &expr, Projection projection = Projection::value, bool discard = false) {
    if (discard && projection == Projection::value &&
        std::ranges::any_of(contexts_, [&](const auto &context) { return context.at(&expr).discarded_view; })) {
      std::map<bool, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < contexts_.size(); ++i)
        alternatives[contexts_[i].at(&expr).discarded_view].push_back(i);
      return contextual_value(expr, alternatives, "discard.view", [&](bool view_only) -> llvm::Value * {
        emit_expression(expr, view_only ? Projection::target : Projection::value, true);
        return reachable() ? empty_scalar() : nullptr;
      });
    }
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
    if (expr.checked_ref_argument && projection == Projection::target && reachable()) {
      using Presence = CheckedOperation::CheckedRefPresence;
      std::map<Presence, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < contexts_.size(); ++i)
        alternatives[contexts_[i].at(&expr).checked_ref_presence].push_back(i);
      return contextual_value(expr, alternatives, "ref.selection", [&](Presence presence) -> llvm::Value * {
        if (presence == Presence::definite_absence) {
          fail_path(ExceptionCategory::null_pointer, *expr.checked_ref_argument);
          return nullptr;
        }
        return checked_address(value, presence == Presence::possible_absence, *expr.checked_ref_argument);
      }, b_.getPtrTy());
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
    if (source == Projection::target) return load_place(checked_address(result, expr, expr.at), expr);
    // Carry bits or the selected target through the operand's own cleanup.
    // The receiver encoding reserves tag zero for a target address.
    auto *saved = environments_ ? owned_slot(*temporaries_, "projection.value") : nullptr;
    auto *direct = block("projection.direct"), *borrowed = block("projection.target"), *done = block("projection.ready");
    b_.CreateCondBr(b_.CreateICmpEQ(b_.CreateExtractValue(result, 1), b_.getInt32(0)), borrowed, direct);
    b_.SetInsertPoint(direct);
    if (saved) assign_value(saved, result, expr.at);
    auto *value_path = b_.GetInsertBlock(); b_.CreateBr(done);
    b_.SetInsertPoint(borrowed);
    auto *value = load_place(checked_address(view_address(result), expr, expr.at), expr);
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
  llvm::Value *emit_joined_bits(const Expression &join, const Expression &arm, bool discard) {
    std::map<Projection, std::vector<size_t>> alternatives;
    for (size_t i = 0; i < contexts_.size(); ++i) {
      const auto &fact = contexts_[i].at(&arm);
      const auto source = contexts_[i].at(&join).scalar_value_join ? result_source(fact) : Projection::value;
      alternatives[source].push_back(i);
    }
    return contextual_value(arm, alternatives, "join.arm", [&](Projection source) {
      return emit_projected_value(arm, source, discard);
    });
  }
  llvm::Value *emit_case_scope(const Expression &body) {
    return temporary_scope(body.at, outcome_, [&]() -> llvm::Value * {
      const auto arguments = prepare_arguments(body);
      if (!reachable()) return nullptr;
      auto *result = invoke(body, arguments);
      propagate(result);
      return result;
    });
  }
  void emit_case_item(llvm::BasicBlock *breaking, llvm::BasicBlock *scanning, const std::function<void()> &emit) {
    if (!breaking && !scanning) { emit(); return; }
    Control item{control_, block("case.transfer")};
    control_ = &item;
    emit();
    control_ = item.parent;
    auto *normal = b_.GetInsertBlock();
    b_.SetInsertPoint(item.exit);
    auto *dispatch = b_.CreateSwitch(load(completion_.kind), control_->exit, bool(breaking) + bool(scanning));
    if (breaking) dispatch->addCase(b_.getInt8(Completion::case_break), breaking);
    if (scanning) {
      auto *resume = block("case.resume");
      dispatch->addCase(b_.getInt8(Completion::case_scan), resume);
      b_.SetInsertPoint(resume);
      clear_value(completion_.value);
      b_.CreateStore(b_.getInt8(0), completion_.kind);
      b_.CreateBr(scanning);
    }
    b_.SetInsertPoint(normal);
  }
  llvm::Value *case_view(llvm::Value *address) {
    return b_.CreateInsertValue(empty_scalar(), b_.CreatePtrToInt(address, b_.getInt64Ty()), 0);
  }
  llvm::Value *emit_case(const Expression &expr, const Type &subject_type, bool borrowed, bool may_be_null,
                         bool reaches_end, bool result_view, bool projects, Projection projection, bool discard) {
    using K = Expression::Kind;
    const auto &subject = *expr.children.front();
    auto *selected = emit_expression(subject, borrowed ? Projection::target : Projection::value);
    if (!reachable()) return nullptr;
    auto *join = block("case.join");
    auto *breaking = expr.case_breaks ? block("case.break") : nullptr;
    std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> incoming;
    for (const auto &entry : std::span(expr.children).subspan(1)) {
      if (!reachable()) break;
      const auto &item = *entry;
      if (item.kind != K::case_arm) {
        emit_case_item(breaking, nullptr, [&] {
          temporary_scope(item.at, scalar_, [&] { return emit_expression(item, Projection::value, true); });
        });
        continue;
      }

      auto *selected_arm = block("case.matched"), *next = block("case.next");
      const auto &checked = contexts_.front().at(&item);
      const auto pattern = checked.case_pattern;
      using P = CasePattern::Kind;
      switch (pattern.kind) {
      case P::any:
        b_.CreateBr(selected_arm);
        break;
      case P::null:
        if (may_be_null) b_.CreateCondBr(b_.CreateIsNull(selected), selected_arm, next);
        else b_.CreateBr(next);
        break;
      case P::code:
        if (may_be_null) {
          auto *present = block("case.present");
          b_.CreateCondBr(b_.CreateIsNull(selected), next, present);
          b_.SetInsertPoint(present);
        }
        if (pattern.constructor) {
          const auto constructor = *pattern.constructor;
          const auto &layout = sum_layout(constructor.sum);
          auto *tag = b_.CreateLoad(b_.getInt64Ty(), b_.CreateStructGEP(layout.storage, selected, 0));
          auto *code = integer(layout.alternatives.at(constructor.alternative).tag);
          b_.CreateCondBr(b_.CreateICmpEQ(tag, code), selected_arm, next);
        } else {
          auto *payload = borrowed ? load_place(selected, subject) : selected;
          auto *bits = unpack(payload, subject_type);
          auto *code = llvm::ConstantInt::get(llvm::cast<llvm::IntegerType>(bits->getType()), pattern.code);
          b_.CreateCondBr(b_.CreateICmpEQ(bits, code), selected_arm, next);
        }
        break;
      }

      b_.SetInsertPoint(selected_arm);
      for (size_t i = 0; i < checked.case_payload_fields.size(); ++i) {
        const auto &receiver = *item.case_bindings[i];
        const auto &binding = contexts_.front().at(&receiver);
        auto *field = sum_field_address(selected, *pattern.constructor, checked.case_payload_fields[i]);
        if (inline_aggregate_type(binding.operand_type)) {
          b_.CreateCall(bits_transfer(binding.operand_type), {storage_.at(*binding.place), field});
          continue;
        }
        auto *payload = load_place(b_, field, StorageFormat{binding.operand_type});
        const auto conversions = choices(receiver, {});
        auto *value = choose(receiver, {}, conversions, scalar_, [&](const auto &choice) {
          return convert_value(payload, choice.operation->operand_type, choice.operation->type);
        });
        assign_place(storage_.at(*binding.place), value, receiver);
      }
      if (item.children.size() == 2) {
        const auto &guard = *item.children.front();
        emit_case_item(breaking, nullptr, [&] {
          auto *condition = temporary_scope(guard.at, scalar_, [&] { return emit_expression(guard); });
          if (reachable()) {
            auto *body = block("case.accepted");
            b_.CreateCondBr(unpack(condition, Type::boolean), body, next);
            b_.SetInsertPoint(body);
          }
        });
      }
      if (reachable()) {
        const auto &body = *item.children.back();
        if (item.case_resumes && body.kind == K::value_call) {
          auto *result = emit_case_scope(body);
          if (reachable()) {
            const bool resumes = std::ranges::any_of(contexts_, [&](const auto &context) {
              return context.at(&item).case_scope_resumes;
            });
            const bool returns = std::ranges::any_of(contexts_, [&](const auto &context) {
              return context.at(&item).case_arm_completes;
            });
            if (resumes && returns) {
              auto *returned = block("case.returned");
              b_.CreateCondBr(b_.CreateExtractValue(result, case_scan_field()), next, returned);
              b_.SetInsertPoint(returned);
            }
            if (returns) {
              auto *value = b_.CreateExtractValue(result, 0);
              if (projects && contexts_.front().at(&body).borrowed_result && !discard)
                value = load_place(checked_address(view_address(value), body, body.at), body);
              incoming.emplace_back(value, b_.GetInsertBlock());
              b_.CreateBr(join);
            } else if (resumes) b_.CreateBr(next);
            else b_.CreateUnreachable();
          }
        } else {
          emit_case_item(breaking, item.case_resumes ? next : nullptr, [&] {
            std::map<bool, std::vector<size_t>> paths;
            for (size_t i = 0; i < contexts_.size(); ++i)
              paths[contexts_[i].at(&item).case_arm_completes].push_back(i);
            auto *value = contextual_value(body, paths, "case.arm", [&](bool completes) -> llvm::Value * {
              if (!completes) {
                emit_expression(body, Projection::value, true);
                if (reachable()) b_.CreateUnreachable();
                return nullptr;
              }
              if (!result_view) return emit_joined_bits(expr, body, discard);
              auto *address = emit_expression(body, Projection::target, discard);
              return reachable() ? case_view(address) : nullptr;
            });
            if (reachable()) {
              incoming.emplace_back(value, b_.GetInsertBlock());
              b_.CreateBr(join);
            }
          });
        }
      }
      b_.SetInsertPoint(next);
      if (!next->hasNPredecessorsOrMore(1)) b_.CreateUnreachable();
    }
    if (reachable()) {
      if (reaches_end) {
        incoming.emplace_back(empty_scalar(), b_.GetInsertBlock());
        b_.CreateBr(join);
      } else b_.CreateUnreachable();
    }
    if (breaking) {
      b_.SetInsertPoint(breaking);
      if (discard) clear_value(completion_.value);
      auto *value = discard ? empty_scalar() : load(completion_.value);
      b_.CreateStore(empty_scalar(), completion_.value);
      b_.CreateStore(b_.getInt8(0), completion_.kind);
      incoming.emplace_back(value, b_.GetInsertBlock());
      b_.CreateBr(join);
    }
    b_.SetInsertPoint(join);
    if (incoming.empty()) { b_.CreateUnreachable(); return nullptr; }
    if (discard) return carries_value(projection) ? empty_scalar() : null_pointer();
    auto *result = b_.CreatePHI(scalar_, incoming.size(), "case.value");
    for (const auto &[value, predecessor] : incoming) result->addIncoming(value, predecessor);
    if (!result_view) return result;
    auto *address = view_address(result);
    return projection == Projection::target ? address : load_place(checked_address(address, expr, expr.at), expr);
  }
  void check_index_bounds(llvm::Value *index, llvm::Value *length, bool signed_index, const Location &at) {
    if (!bounds_checks_) return;
    auto *in_range = b_.CreateICmpULT(index, length);
    if (signed_index) in_range = b_.CreateAnd(in_range, b_.CreateICmpSGE(index, integer(0)));
    auto *valid = block("bounds.valid"), *invalid = block("bounds.failure");
    b_.CreateCondBr(in_range, valid, invalid);
    b_.SetInsertPoint(invalid);
    save_abrupt(outcome(empty_scalar(), integer(exception_id(ExceptionCategory::index_out_of_bounds)),
                        integer(site(at)), null_pointer(), index, length));
    b_.CreateBr(control_->exit);
    b_.SetInsertPoint(valid);
  }
  llvm::Value *pack_total(llvm::Value *descriptor) {
    return b_.CreateLoad(b_.getInt64Ty(), b_.CreateStructGEP(value_pack_, descriptor, 2));
  }
  llvm::Value *pack_segment(llvm::Value *segments, llvm::Value *number) {
    return b_.CreateGEP(value_pack_segment_, segments, number);
  }
  void copy_pack_values(llvm::Value *source, llvm::Value *destination, llvm::Value *length) {
    auto *start = b_.GetInsertBlock();
    auto *loop = block("pack.copy.next"), *element = block("pack.copy.element"), *done = block("pack.copy.done");
    b_.CreateBr(loop);
    b_.SetInsertPoint(loop);
    auto *index = b_.CreatePHI(b_.getInt64Ty(), 2);
    index->addIncoming(integer(0), start);
    b_.CreateCondBr(b_.CreateICmpULT(index, length), element, done);
    b_.SetInsertPoint(element);
    auto *type = b_.getInt64Ty();
    auto *value = b_.CreateLoad(type, b_.CreateGEP(type, source, index));
    b_.CreateStore(value, b_.CreateGEP(type, destination, index));
    index->addIncoming(b_.CreateAdd(index, integer(1)), element);
    b_.CreateBr(loop);
    b_.SetInsertPoint(done);
  }
  void snapshot_forwarded_pack(llvm::Value *source, llvm::Value *destination) {
    auto *segments = b_.CreateLoad(b_.getPtrTy(), b_.CreateStructGEP(value_pack_, source, 0));
    auto *count = b_.CreateLoad(b_.getInt64Ty(), b_.CreateStructGEP(value_pack_, source, 1));
    auto *start = b_.GetInsertBlock();
    auto *loop = block("pack.forward.next"), *element = block("pack.forward.segment"), *done = block("pack.forward.done");
    b_.CreateBr(loop);
    b_.SetInsertPoint(loop);
    auto *number = b_.CreatePHI(b_.getInt64Ty(), 2);
    auto *offset = b_.CreatePHI(b_.getInt64Ty(), 2);
    number->addIncoming(integer(0), start);
    offset->addIncoming(integer(0), start);
    b_.CreateCondBr(b_.CreateICmpULT(number, count), element, done);
    b_.SetInsertPoint(element);
    auto *segment = pack_segment(segments, number);
    auto *data = b_.CreateLoad(b_.getPtrTy(), b_.CreateStructGEP(value_pack_segment_, segment, 0));
    auto *length = b_.CreateLoad(b_.getInt64Ty(), b_.CreateStructGEP(value_pack_segment_, segment, 1));
    copy_pack_values(data, b_.CreateGEP(b_.getInt64Ty(), destination, offset), length);
    auto *next = b_.GetInsertBlock();
    number->addIncoming(b_.CreateAdd(number, integer(1)), next);
    offset->addIncoming(b_.CreateAdd(offset, length), next);
    b_.CreateBr(loop);
    b_.SetInsertPoint(done);
  }
  void check_pack_capacity(llvm::Value *valid, const Location &at) {
    auto *ready = block("pack.capacity.valid"), *invalid = block("pack.capacity.invalid");
    b_.CreateCondBr(valid, ready, invalid);
    b_.SetInsertPoint(invalid);
    fail_path(ExceptionCategory::invalid_capacity, at);
    b_.SetInsertPoint(ready);
  }
  llvm::Value *pack_count_after(llvm::Value *total, llvm::Value *length, const Location &at) {
    auto *next = b_.CreateAdd(total, length);
    check_pack_capacity(b_.CreateICmpUGE(next, total), at);
    return next;
  }
  llvm::Value *snapshot_pack_operand(const Expression &operand, llvm::Value *source, bool forwarded,
      llvm::Value *length, llvm::AllocaInst *head) {
    const auto &layout = module_.getDataLayout();
    auto *element = b_.getInt64Ty();
    const auto stride = layout.getTypeAllocSize(element).getFixedValue();
    const auto address_bits = std::min(64u, layout.getPointerSizeInBits());
    const auto byte_limit = UINT64_MAX >> (64 - address_bits);
    check_pack_capacity(b_.CreateICmpULE(length, integer(byte_limit / stride)), operand.at);
    auto *start = b_.GetInsertBlock();
    auto *copy = block("pack.snapshot"), *ready = block("pack.snapshot.ready");
    b_.CreateCondBr(b_.CreateICmpNE(length, integer(0)), copy, ready);
    b_.SetInsertPoint(copy);
    auto allocate = runtime("cod_lang_lexical_extent_allocate", b_.getPtrTy(),
        {b_.getPtrTy(), b_.getInt64Ty(), b_.getInt64Ty()});
    auto *storage = b_.CreateCall(allocate, {head, b_.CreateMul(length, integer(stride)),
                                            integer(layout.getABITypeAlign(element).value())});
    auto *allocated = block("pack.extent.allocated"), *oom = block("pack.extent.oom");
    b_.CreateCondBr(b_.CreateIsNotNull(storage), allocated, oom);
    b_.SetInsertPoint(oom);
    fail_path(ExceptionCategory::out_of_memory, operand.at);
    b_.SetInsertPoint(allocated);
    if (forwarded) snapshot_forwarded_pack(source, storage);
    else copy_pack_values(array_data(source, element), storage, length);
    auto *copied = b_.GetInsertBlock();
    b_.CreateBr(ready);
    b_.SetInsertPoint(ready);
    auto *result = b_.CreatePHI(b_.getPtrTy(), 2);
    result->addIncoming(null_pointer(), start);
    result->addIncoming(storage, copied);
    return result;
  }
  llvm::Value *emit_value_pack_call(const Expression &expr, size_t callee, const CheckedOperation::ValuePack &plan) {
    const auto &instance = program_->instances[callee];
    auto *head = slot(b_.getPtrTy(), "pack.extents");
    auto *descriptor = slot(value_pack_, "pack.descriptor");
    const auto segment_count = plan.operands.size() - plan.fixed_count;
    auto *segments = slot(llvm::ArrayType::get(value_pack_segment_, std::max(size_t{1}, segment_count)), "pack.segments");
    auto *result = slot(scalar_, "pack.result");
    Control preparation{control_, block("pack.release")};
    control_ = &preparation;
    b_.CreateStore(null_pointer(), head);
    llvm::Value *total = integer(0);
    std::vector<llvm::Value *> arguments;
    using Operand = CheckedOperation::ValuePack::Operand;
    for (size_t i = 0; i < plan.operands.size(); ++i) {
      const auto mode = plan.operands[i];
      const auto &operand = *expr.children[i];
      if (mode == Operand::fixed) {
        arguments.push_back(prepare_argument(expr, i, false));
        if (!reachable()) break;
        continue;
      }
      llvm::Value *length = integer(0), *storage = null_pointer();
      if (mode == Operand::value) {
        auto *value = emit_expression(operand);
        if (!reachable()) break;
        value = choose(operand, {value}, choices(operand, {&operand}), b_.getInt64Ty(), [&](const Choice &choice) {
          return convert(value, choice.inputs.front(), Type::uint64);
        });
        length = integer(1);
        total = pack_count_after(total, length, operand.at);
        storage = slot(b_.getInt64Ty(), "pack.scalar");
        b_.CreateStore(value, storage);
      } else if (mode == Operand::spread) {
        auto *source = emit_expression(operand, Projection::target);
        if (!reachable()) break;
        source = checked_address(source, operand, operand.at);
        std::map<bool, std::vector<size_t>> shapes;
        for (size_t context = 0; context < contexts_.size(); ++context) {
          const auto type = contexts_[context].at(&operand).type.kind;
          if (type != Type::never) shapes[type == Type::arguments].push_back(context);
        }
        auto *extent = contextual_value(operand, shapes, "pack.spread", [&](bool forwarded) -> llvm::Value * {
          auto *selected = forwarded ? view_address(b_.CreateLoad(scalar_, source)) : source;
          auto *count = forwarded ? pack_total(selected) : array_length(selected);
          auto *next = pack_count_after(total, count, operand.at);
          auto *backing = snapshot_pack_operand(operand, selected, forwarded, count, head);
          auto *segment = b_.CreateStructGEP(value_pack_, descriptor, 2);
          b_.CreateStore(next, segment);
          auto *selection = b_.CreateInsertValue(llvm::PoisonValue::get(value_pack_segment_), backing, 0);
          return b_.CreateInsertValue(selection, count, 1);
        }, value_pack_segment_);
        total = pack_total(descriptor);
        storage = b_.CreateExtractValue(extent, 0);
        length = b_.CreateExtractValue(extent, 1);
      }
      auto *entry = pack_segment(segments, integer(i - plan.fixed_count));
      b_.CreateStore(storage, b_.CreateStructGEP(value_pack_segment_, entry, 0));
      b_.CreateStore(length, b_.CreateStructGEP(value_pack_segment_, entry, 1));
    }
    if (reachable()) {
      b_.CreateStore(segments, b_.CreateStructGEP(value_pack_, descriptor, 0));
      b_.CreateStore(integer(segment_count), b_.CreateStructGEP(value_pack_, descriptor, 1));
      b_.CreateStore(total, b_.CreateStructGEP(value_pack_, descriptor, 2));
      arguments.push_back(pack(b_.CreatePtrToInt(descriptor, b_.getInt64Ty()), Type::uint64));
      auto *target = methods_[instance.method];
      auto *value = propagate(native_call(expr, target, instance.method, std::move(arguments), {}, callee));
      if (reachable()) b_.CreateStore(value, result);
    }
    const bool completed = reachable();
    if (completed) b_.CreateBr(preparation.exit);
    b_.SetInsertPoint(preparation.exit);
    auto release = runtime("cod_lang_lexical_allocations_release", b_.getVoidTy(), {b_.getPtrTy()});
    b_.CreateCall(release, {load(head)});
    control_ = preparation.parent;
    if (!completed) {
      b_.CreateBr(control_->exit);
      return nullptr;
    }
    auto *done = block("pack.completed");
    b_.CreateCondBr(b_.CreateICmpEQ(load(completion_.kind), b_.getInt8(0)), done, control_->exit);
    b_.SetInsertPoint(done);
    return load(result);
  }
  llvm::Value *pack_element_address(llvm::Value *descriptor, llvm::Value *index, const Location &at) {
    if (bounds_checks_) check_index_bounds(index, pack_total(descriptor), false, at);
    auto *segments = b_.CreateLoad(b_.getPtrTy(), b_.CreateStructGEP(value_pack_, descriptor, 0));
    auto *count = b_.CreateLoad(b_.getInt64Ty(), b_.CreateStructGEP(value_pack_, descriptor, 1));
    auto *start = b_.GetInsertBlock();
    auto *search = block("pack.index.search"), *candidate = block("pack.index.segment");
    auto *next = block("pack.index.next"), *found = block("pack.index.found"), *invalid = block("pack.index.invalid");
    b_.CreateBr(search);
    b_.SetInsertPoint(search);
    auto *number = b_.CreatePHI(b_.getInt64Ty(), 2);
    auto *remaining = b_.CreatePHI(b_.getInt64Ty(), 2);
    number->addIncoming(integer(0), start);
    remaining->addIncoming(index, start);
    b_.CreateCondBr(b_.CreateICmpULT(number, count), candidate, invalid);
    b_.SetInsertPoint(candidate);
    auto *segment = pack_segment(segments, number);
    auto *length = b_.CreateLoad(b_.getInt64Ty(), b_.CreateStructGEP(value_pack_segment_, segment, 1));
    b_.CreateCondBr(b_.CreateICmpULT(remaining, length), found, next);
    b_.SetInsertPoint(next);
    number->addIncoming(b_.CreateAdd(number, integer(1)), next);
    remaining->addIncoming(b_.CreateSub(remaining, length), next);
    b_.CreateBr(search);
    b_.SetInsertPoint(invalid);
    b_.CreateUnreachable();
    b_.SetInsertPoint(found);
    auto *data = b_.CreateLoad(b_.getPtrTy(), b_.CreateStructGEP(value_pack_segment_, segment, 0));
    return b_.CreateGEP(b_.getInt64Ty(), data, remaining);
  }
  llvm::Value *emit_indexed_address(const Expression &expr) {
    const auto &array = *expr.children[0];
    auto *view = emit_expression(array, Projection::target);
    if (!reachable()) return nullptr;
    view = checked_address(view, array, expr.at);
    const auto &index_expr = *expr.children[1];
    auto *index_value = emit_expression(index_expr);
    if (!reachable()) return nullptr;
    std::map<std::pair<std::optional<size_t>, Type::Kind>, std::vector<size_t>> alternatives;
    for (size_t i = 0; i < contexts_.size(); ++i) {
      const auto &type = contexts_[i].at(&array).type;
      const auto index = contexts_[i].at(&index_expr).type.kind;
      if (index == Type::never) continue;
      if (type.kind == Type::array) alternatives[{type.identity, index}].push_back(i);
      else if (type.kind == Type::arguments) alternatives[{std::nullopt, index}].push_back(i);
    }
    return contextual_value(expr, alternatives, "array.element", [&](const auto &selection) -> llvm::Value * {
      const auto &[schema, index_kind] = selection;
      if (!schema) {
        auto *descriptor = view_address(b_.CreateLoad(scalar_, view));
        return pack_element_address(descriptor, convert(index_value, Type{index_kind}, Type::uint64), expr.at);
      }
      const auto &shape = program_->arrays.at(*schema);
      auto *index = b_.CreateZExtOrTrunc(convert(index_value, Type{index_kind}, shape.index), b_.getInt64Ty());
      if (bounds_checks_) check_index_bounds(index, array_length(view), shape.index == Type::integer, expr.at);
      auto *element = inline_layout(shape.element);
      return b_.CreateGEP(element, array_data(view, element), index);
    }, b_.getPtrTy());
  }
  llvm::Value *emit_regional_initializer(const Expression &expr, llvm::Value *destination, size_t written) {
    using Plan = std::tuple<size_t, size_t, size_t, std::optional<size_t>>;
    std::map<Plan, std::vector<size_t>> plans;
    for (size_t i = 0; i < contexts_.size(); ++i) {
      const auto &operation = contexts_[i].at(&expr);
      const auto constructor = *operation.sum_constructor;
      plans[{constructor.sum, constructor.alternative, *operation.regional_sum_holder, operation.callee}].push_back(i);
    }
    return contextual_value(expr, plans, "region.sum.initializer", [&](const Plan &plan) -> llvm::Value * {
      const auto [sum, alternative, holder, callee] = plan;
      const SumConstructor constructor{sum, alternative};
      const auto &description = program_->sums.at(sum).alternatives.at(alternative);
      std::vector<llvm::Value *> arguments;
      for (size_t i = 0; i < written; ++i) {
        auto *value = prepare_argument(expr, i, false);
        if (!reachable()) return nullptr;
        arguments.push_back(value);
      }
      if (!callee) throw std::logic_error("reachable regional initializer without a checked body");

      // The frame uses ordinary reference slots. Their selections become
      // relative locators only after the initializer and its cleanup return.
      std::vector<std::pair<llvm::AllocaInst *, llvm::Value *>> edges;
      for (size_t i = 0; i < description.fields.size(); ++i) {
        const auto &field = description.fields[i];
        auto *address = sum_field_address(destination, constructor, i);
        llvm::Value *supplied = address;
        if (field.stores_target()) {
          auto *selection = slot(b_.getPtrTy(), "initializer.reference");
          b_.CreateStore(null_pointer(), selection);
          edges.emplace_back(selection, address);
          supplied = selection;
        }
        auto *view = b_.CreateInsertValue(empty_scalar(), b_.CreatePtrToInt(supplied, b_.getInt64Ty()), 0);
        arguments.push_back(b_.CreateInsertValue(view, b_.getInt32(tag(field.type)), 1));
      }
      const auto method = program_->instances.at(*callee).method;
      auto *entry = methods_.at(method);
      propagate(native_call(expr, entry, method, std::move(arguments), {}, callee));
      if (contexts_.front().at(&expr).type == Type::never) {
        b_.CreateUnreachable();
        return nullptr;
      }
      auto *owner = load(storage_.at(holder));
      for (const auto &[selection, field] : edges)
        b_.CreateStore(regional_scalar_locator(owner, load(selection)), field);
      return destination;
    }, b_.getPtrTy());
  }
  llvm::Value *emit_checked_expression(const Expression &expr, Projection projection, bool discard) {
    const bool read = carries_value(projection);
    using K = Expression::Kind;
    const auto &fact = contexts_.front().at(&expr);
    if (expr.kind == K::call && std::ranges::any_of(contexts_, [&](const auto &context) {
          return context.at(&expr).builtin == BuiltinOperation::array_transfer;
        })) {
      std::map<std::tuple<bool, Type::Kind, size_t>, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < contexts_.size(); ++i) {
        const auto &operation = contexts_[i].at(&expr);
        const bool builtin = operation.builtin == BuiltinOperation::array_transfer;
        auto element = Type{Type::void_type};
        if (builtin && operation.operand_type.kind == Type::array)
          element = program_->arrays.at(operation.operand_type.identity).element;
        if (!inline_aggregate_type(element)) element = Type{Type::void_type};
        alternatives[{builtin, element.kind, element.identity}].push_back(i);
      }
      return contextual_value(expr, alternatives, "array.transfer", [&](const auto &selected) -> llvm::Value * {
        const auto [builtin, kind, identity] = selected;
        if (!builtin) return emit_checked_expression(expr, projection, discard);
        auto *address = emit_indexed_address(expr);
        if (!reachable()) return nullptr;
        const auto &source = *expr.children[2];
        const bool aggregate = kind != Type::void_type;
        auto *value = emit_expression(source, aggregate ? Projection::target : Projection::value);
        if (!reachable()) return nullptr;
        if (aggregate) {
          value = checked_address(value, source, source.at);
          b_.CreateCall(bits_transfer(Type{kind, {}, identity}), {address, value});
        } else assign_place(address, convert_result(expr, source, value, &CheckedOperation::type), expr);
        return !read ? address : discard ? empty_scalar() : load_place(address, expr);
      }, read ? static_cast<llvm::Type *>(scalar_) : b_.getPtrTy());
    }
    if (fact.enum_literal) {
      std::map<std::pair<size_t, std::uint64_t>, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < contexts_.size(); ++i) {
        const auto &member = contexts_[i].at(&expr);
        alternatives[{member.type.identity, member.integer}].push_back(i);
      }
      return contextual_value(expr, alternatives, "enum.member", [&](const auto &member) {
        const auto &[id, code] = member;
        const Type type{Type::enumeration, program_->enums[id].source->name, id};
        return pack(llvm::ConstantInt::get(llvm::cast<llvm::IntegerType>(native_type(type)), code), type);
      });
    }
    if (expr.kind == K::typed_expression &&
        std::ranges::any_of(contexts_, [&](const auto &context) { return context.at(&expr).enum_decode; })) {
      std::map<std::optional<size_t>, std::vector<size_t>> domains;
      for (size_t i = 0; i < contexts_.size(); ++i) {
        const auto &checked = contexts_[i].at(&expr);
        domains[checked.enum_decode ? std::optional{checked.type.identity} : std::nullopt].push_back(i);
      }
      return contextual_value(expr, domains, "enum.decode", [&](auto id) -> llvm::Value * {
        const auto &operand = *expr.children.front();
        if (!id) return emit_expression(operand, projection, discard);
        auto *value = emit_expression(operand);
        if (!reachable()) return nullptr;
        const auto &domain = program_->enums.at(*id);
        auto *code = unpack(value, domain.backing);
        auto *valid = block("enum.code.valid"), *invalid = block("enum.code.invalid");
        auto *match = b_.CreateSwitch(code, invalid);
        for (size_t i = 0; i < domain.members.size(); ++i) {
          const auto &member = domain.members[i];
          if (member.canonical == i)
            match->addCase(llvm::ConstantInt::get(llvm::cast<llvm::IntegerType>(code->getType()), member.code), valid);
        }
        b_.SetInsertPoint(invalid);
        fail_path(ExceptionCategory::type_assertion_failed, expr.at);
        b_.SetInsertPoint(valid);
        return discard ? empty_scalar() : pack(code, contexts_.front().at(&expr).type);
      });
    }
    if (fact.enum_assertion) return emit_expression(*expr.children.front(), projection, discard);
    if (fact.native_call)
      return temporary_scope(expr.at, scalar_, [&] {
        return program_->native_operations.at(*fact.native_call).has_here() ? emit_native_here(expr) : emit_native_call(expr);
      });
    if (fact.argument_sources) {
      using Layout = std::tuple<std::vector<const Expression *>, std::vector<CheckedOperation::OperandPreparation>,
                                std::optional<CheckedOperation::PipelinePreparation>>;
      std::map<Layout, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < contexts_.size(); ++i) {
        const auto &checked = contexts_[i].at(&expr);
        alternatives[{checked.argument_sources.value(), checked.operand_preparation, checked.pipeline_preparation}].push_back(i);
      }
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
    if (fact.scoped_effect) return install_scoped_effect(expr, fact);
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
    case K::here_binding: return save_pipeline_operands(expr, fact.pipeline_preparation.value());
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
      if (fact.type == Type::c_string) {
        auto *view = emit_expression(operand, Projection::target);
        if (!reachable()) return nullptr;
        std::map<bool, std::vector<size_t>> checks;
        for (size_t i = 0; i < contexts_.size(); ++i) checks[contexts_[i].at(&expr).check_c_string].push_back(i);
        return contextual_value(expr, checks, "cstring.assertion", [&](bool check) -> llvm::Value * {
          if (check) {
            auto *bytes = b_.CreateLoad(b_.getPtrTy(), b_.CreateStructGEP(text_view_, view, 0));
            auto *size = b_.CreateLoad(b_.getInt64Ty(), b_.CreateStructGEP(text_view_, view, 1));
            auto search = runtime("memchr", b_.getPtrTy(), {b_.getPtrTy(), b_.getInt32Ty(), b_.getInt64Ty()});
            auto *zero = b_.CreateCall(search, {bytes, b_.getInt32(0), size});
            auto *ready = block("cstring.valid"), *failed = block("cstring.invalid");
            b_.CreateCondBr(b_.CreateIsNull(zero), ready, failed);
            b_.SetInsertPoint(failed);
            fail_path(ExceptionCategory::type_assertion_failed, expr.at);
            b_.SetInsertPoint(ready);
          }
          return discard ? empty_scalar() : view;
        }, discard ? static_cast<llvm::Type *>(scalar_) : b_.getPtrTy());
      }
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
    case K::symbol: {
      auto *binding = symbols_.at(*fact.symbol);
      auto *value = b_.CreateLoad(binding->getValueType(), binding, "effect.current");
      if (program_->symbols.at(*fact.symbol).reference) {
        auto *address = view_address(b_.CreateExtractValue(value, 0, "effect.selection"));
        if (read && discard) return empty_scalar();
        return read ? load_place(address, expr) : address;
      }
      return fact.type.kind == Type::native_handle ? view_address(value) : value;
    }
    case K::touch: return empty_scalar();
    case K::effect: {
      if (fact.native_acquisition) return install_native_effect(expr);
      auto *binding = symbols_.at(*fact.symbol);
      if (program_->symbols.at(*fact.symbol).reference) {
        const auto &source = *expr.children.front();
        auto *target = emit_expression(source, Projection::target);
        if (!reachable()) return nullptr;
        auto *view = b_.CreateInsertValue(llvm::PoisonValue::get(binding->getValueType()), pack_view(target, source), 0);
        view = b_.CreateInsertValue(view, selected_owner(source), 1);
        b_.CreateStore(view, binding);
        return empty_scalar();
      }
      auto *value = emit_expression(*expr.children.front());
      if (!reachable()) return nullptr;
      return finish_effect(fact, convert_result(expr, *expr.children.front(), value), expr.at);
    }
    case K::integer: case K::boolean:
      return pack(llvm::ConstantInt::get(llvm::cast<llvm::IntegerType>(native_type(fact.type)), fact.integer), fact.type);
    case K::string_literal: {
      if (discard) return empty_scalar();
      auto *bytes = b_.CreateGlobalString(expr.text, "text.bytes");
      auto *view = llvm::ConstantStruct::get(text_view_, {bytes, integer(expr.text.size()), integer(expr.text.size() + 1)});
      return new llvm::GlobalVariable(module_, text_view_, true, llvm::GlobalValue::PrivateLinkage, view, "text.literal");
    }
    case K::predicate_assertion: {
      auto *value = emit_expression(*expr.children.front());
      if (!reachable()) return nullptr;
      std::map<bool, std::vector<size_t>> checks;
      for (size_t i = 0; i < contexts_.size(); ++i)
        checks[contexts_[i].at(&expr).check_predicate_assertion].push_back(i);
      return contextual_value(expr, checks, "predicate.assertion", [&](bool check) {
        if (check) {
          auto *ready = block("assertion.satisfied"), *failed = block("assertion.failed");
          b_.CreateCondBr(unpack(value, Type::boolean), ready, failed);
          b_.SetInsertPoint(failed);
          fail_path(ExceptionCategory::assertion_failed, expr.at);
          b_.SetInsertPoint(ready);
        }
        return empty_scalar();
      });
    }
    case K::return_test: {
      std::map<CheckedOperation::ReturnFilter, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < contexts_.size(); ++i) alternatives[contexts_[i].at(&expr).return_filter].push_back(i);
      return contextual_value(expr, alternatives, "return.matches", [&](const auto &filter) {
        if (!filter.can_match) return pack(b_.getFalse(), Type::boolean);
        auto *matches = observed_return();
        if (filter.owner_comparison) {
          const auto [result_owner, expected_owner] = *filter.owner_comparison;
          matches = b_.CreateAnd(matches, b_.CreateICmpEQ(owner_identity(result_owner, expr.at), owner_identity(expected_owner, expr.at)));
        }
        return pack(matches, Type::boolean);
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
      using AddressPlan = std::pair<std::optional<size_t>, bool>;
      const auto emit_address = [&](const AddressPlan &plan) -> llvm::Value * {
        const auto &[captured, reborrow] = plan;
        const auto &selected = contexts_.front().at(&expr);
        auto *address = binding_address(*selected.binding, selected.place);
        if (captured && !bindings_.at(*selected.binding).value_binding)
          address = b_.CreateSelect(b_.CreateICmpEQ(address, null_pointer()), null_pointer(), storage_.at(*captured));
        return reborrow ? native_reborrow(b_.CreateLoad(b_.getInt64Ty(), address)) : address;
      };
      auto *address = [&]() {
        if (contexts_.size() == 1) return emit_address({fact.captured_place, fact.reborrows_native_owner});
        std::map<AddressPlan, std::vector<size_t>> alternatives;
        for (size_t i = 0; i < contexts_.size(); ++i) {
          const auto &selected = contexts_[i].at(&expr);
          alternatives[{selected.captured_place, selected.reborrows_native_owner}].push_back(i);
        }
        return contextual_value(expr, alternatives, "storage.address", emit_address, b_.getPtrTy());
      }();
      return read ? load_place(checked_address(address, expr, expr.at), expr) : address;
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
    case K::value_call: case K::call: case K::behave: case K::perform: case K::pipeline: case K::processor_composition: {
      if (fact.symbol && program_->symbols[*fact.symbol].live_scope)
        return invoke_scoped_effect(expr, *fact.symbol);
      if (fact.builtin == BuiltinOperation::lexical_result)
        return emit_expression(*expr.children.front(), projection, discard);
      if (fact.builtin == BuiltinOperation::text_size || fact.builtin == BuiltinOperation::text_byte ||
          fact.builtin == BuiltinOperation::text_slice)
        return emit_text_access(expr, fact.builtin, discard);
      if (fact.builtin == BuiltinOperation::builder_append) {
        const auto &source = *expr.children[1];
        auto *builder = emit_expression(*expr.children[0], Projection::target);
        if (!reachable()) return nullptr;
        auto *piece = emit_expression(source, Projection::target);
        if (!reachable()) return nullptr;
        std::map<bool, std::vector<size_t>> modes;
        for (size_t i = 0; i < contexts_.size(); ++i) {
          const auto &type = contexts_[i].at(&source).type;
          if (type != Type::never) modes[program_->builder_type(type)].push_back(i);
        }
        auto *status = contextual_value(expr, modes, "builder.append", [&](bool from_builder) -> llvm::Value * {
          const auto name = from_builder ? "cod_lang_builder_append_builder" : "cod_lang_builder_append_text";
          auto append = runtime(name, b_.getInt32Ty(), {b_.getPtrTy(), b_.getPtrTy()});
          return b_.CreateCall(append, {builder, piece});
        }, b_.getInt32Ty());
        auto *ready = block("builder.ready"), *failed = block("builder.failed");
        b_.CreateCondBr(b_.CreateIsNotNull(status), ready, failed);
        b_.SetInsertPoint(failed);
        fail_path(ExceptionCategory::out_of_memory, expr.at);
        b_.SetInsertPoint(ready);
        return empty_scalar();
      }
      if (fact.builtin == BuiltinOperation::array_length) {
        const auto &operand = *expr.children.front();
        auto *address = emit_expression(operand, Projection::target);
        if (!reachable()) return nullptr;
        address = checked_address(address, operand, expr.at);
        if (discard) return empty_scalar();
        auto *length = array_length(address);
        return pack(length, Type::uint64);
      }
      if (fact.value_pack) {
        using Invocation = std::pair<size_t, CheckedOperation::ValuePack>;
        std::map<Invocation, std::vector<size_t>> calls;
        for (size_t i = 0; i < contexts_.size(); ++i) {
          const auto &call = contexts_[i].at(&expr);
          if (call.value_pack && call.callee) calls[{*call.callee, *call.value_pack}].push_back(i);
        }
        return contextual_value(expr, calls, "pack.call", [&](const Invocation &call) {
          return emit_value_pack_call(expr, call.first, call.second);
        });
      }
      return temporary_scope(expr.at, !fact.borrowed_result || read ? scalar_ : static_cast<llvm::Type *>(b_.getPtrTy()), [&]() -> llvm::Value * {
        // An entry-time supplied body leaves its slot empty during preparation.
        std::vector<llvm::Value *> here_values(fact.here_arguments.size());
        std::vector<llvm::Value *> current_owners;
        const auto arguments = fact.pipeline_preparation ? prepare_pipeline(expr, *fact.pipeline_preparation, here_values)
                                                        : prepare_arguments(expr, false, here_values, nullptr, &current_owners);
        if (!reachable()) return nullptr;
        llvm::AllocaInst *here_site = nullptr;
        if (fact.supplied_here && program_->methods[program_->instances[fact.supplied_here->instance].method].here_group) {
          here_site = slot(b_.getPtrTy(), "here.call.destination");
          b_.CreateStore(null_pointer(), here_site);
        }
        auto *result = invoke(expr, arguments, here_site, here_values, current_owners);
        if (here_site) result = consume_here_transfer(result, load(here_site), expr.at);
        auto *value = propagate(result);
        if (!fact.borrowed_result) return own_value(value);
        auto *address = view_address(value);
        return !read ? address : discard ? empty_scalar() : load_place(checked_address(address, expr, expr.at), expr);
      });
    }
    case K::case_expression: {
      const auto *subject = expr.children.front().get();
      std::map<std::tuple<Type::Kind, size_t, bool, bool, bool, bool, bool>, std::vector<size_t>> layouts;
      for (size_t i = 0; i < contexts_.size(); ++i) {
        const auto &input = contexts_[i].at(subject), &result = contexts_[i].at(&expr);
        layouts[{input.type.kind, input.type.identity, input.view, result.case_subject_may_be_null,
                 result.case_reaches_end, result.view, result.scalar_value_join}].push_back(i);
      }
      return contextual_value(expr, layouts, "case.shape", [&](const auto &layout) {
        const auto &[kind, identity, borrowed, nullable, reaches_end, result_view, projects] = layout;
        return emit_case(expr, Type{kind, {}, identity}, borrowed, nullable, reaches_end, result_view, projects, projection, discard);
      }, carries_value(projection) ? scalar_ : static_cast<llvm::Type *>(b_.getPtrTy()));
    }
    case K::case_break: case K::case_fallthrough: {
      llvm::Value *value = empty_scalar();
      if (!expr.children.empty()) {
        const auto &operand = *expr.children.front();
        std::map<std::pair<bool, Projection>, std::vector<size_t>> preparations;
        for (size_t i = 0; i < contexts_.size(); ++i) {
          const auto &exit = contexts_[i].at(&expr);
          const auto source = exit.scalar_value_join ? result_source(contexts_[i].at(&operand)) : Projection::value;
          preparations[{exit.borrowed_result, source}].push_back(i);
        }
        value = contextual_value(expr, preparations, "case.break.value", [&](const auto &preparation) -> llvm::Value * {
          const auto [borrow, source] = preparation;
          if (!borrow) return emit_projected_value(operand, source, false);
          auto *address = emit_expression(operand, Projection::target);
          return reachable() ? case_view(address) : nullptr;
        });
      }
      if (!reachable()) return nullptr;
      const auto kind = expr.kind == K::case_break ? Completion::case_break : Completion::case_scan;
      save_local_completion(value, expr.at, kind, false);
      b_.CreateBr(control_->exit);
      return nullptr;
    }
    case K::case_failure:
      fail_path(ExceptionCategory::unexpected_case, expr.at);
      return nullptr;
    case K::conditional: {
      const auto &test = *expr.children[0];
      auto *condition = emit_expression(test);
      if (!reachable()) return nullptr;
      auto *yes = block("if.then"), *no = block("if.else"), *join = block("if.join");
      b_.CreateCondBr(unpack(condition, Type::boolean), yes, no);
      std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> incoming;
      for (size_t i = 1; i < 3; ++i) {
        b_.SetInsertPoint(i == 1 ? yes : no);
        const auto &arm = *expr.children[i];
        const bool binds_return = i == 1 && test.kind == K::return_test && !test.binding.empty();
        const auto emit = [&]() -> llvm::Value * {
          if (binds_return) {
            std::map<std::tuple<std::optional<size_t>, std::optional<size_t>, bool>, std::vector<size_t>> destinations;
            for (size_t context = 0; context < contexts_.size(); ++context) {
              const auto &pattern = contexts_[context].at(&test);
              const bool matches = pattern.return_filter.can_match;
              destinations[{matches ? pattern.place : std::nullopt, matches ? pattern.binding : std::nullopt,
                            pattern.place && pattern.operand_type.kind == Type::procedure}].push_back(context);
            }
            contextual_value(test, destinations, "return.binding", [&](const auto &destination) {
              const auto [place, binding, callable] = destination;
              if (place) {
                auto *storage = storage_.at(*place);
                store_incoming(*place, observed_return_value());
                if (callable && environments_) temporaries_->values.push_back(storage);
              }
              if (binding) b_.CreateStore(view_address(observed_return_value()), binding_slot(*binding));
              return empty_scalar();
            });
          }
          return projection == Projection::value ? emit_joined_bits(expr, arm, discard)
                                                 : emit_expression(arm, projection, discard);
        };
        const bool callable_binding = binds_return && std::ranges::any_of(contexts_, [&](const auto &context) {
          const auto &pattern = context.at(&test);
          return pattern.return_filter.can_match && pattern.place && pattern.operand_type.kind == Type::procedure;
        });
        auto *value = callable_binding ? temporary_scope(arm.at, read ? scalar_ : static_cast<llvm::Type *>(b_.getPtrTy()), emit) : emit();
        if (reachable()) { incoming.emplace_back(value, b_.GetInsertBlock()); b_.CreateBr(join); }
      }
      b_.SetInsertPoint(join);
      if (incoming.empty()) { b_.CreateUnreachable(); return nullptr; }
      if (discard) return read ? empty_scalar() : null_pointer();
      auto *result = b_.CreatePHI(read ? scalar_ : static_cast<llvm::Type *>(b_.getPtrTy()), incoming.size(), "if.value");
      for (const auto &[value, predecessor] : incoming) result->addIncoming(value, predecessor);
      return result;
    }
    case K::block: {
      std::vector<const Expression *> expressions;
      for (const auto &child : expr.children) expressions.push_back(child.get());
      return sequence(expressions, projection, discard);
    }
    case K::range_for: {
      auto *first = emit_expression(*expr.children[0]);
      if (!reachable()) return nullptr;
      auto *last = emit_expression(*expr.children[1]);
      if (!reachable()) return nullptr;
      auto *index = storage_.at(*fact.place);
      store_incoming(*fact.place, first);
      const auto cases = choices(expr, {expr.children[0].get(), expr.children[1].get()});
      const auto test = [&] {
        auto *current = load_place(index, expr, &CheckedOperation::operand_type);
        return choose(expr, {current, last}, cases, b_.getInt1Ty(), [&](const Choice &choice) {
          const auto type = choice.operation->operand_type;
          auto *left = unpack(current, type), *right = unpack(last, type);
          return type == Type::integer ? b_.CreateICmpSLT(left, right) : b_.CreateICmpULT(left, right);
        });
      };
      auto *body = block("range.body"), *empty = block("range.empty"), *done = block("range.done");
      b_.CreateCondBr(test(), body, empty);
      b_.SetInsertPoint(body);
      iteration(*expr.children[2], true);
      if (reachable()) {
        auto *current = load_place(index, expr, &CheckedOperation::operand_type);
        auto *advanced = choose(expr, {current, last}, cases, scalar_, [&](const Choice &choice) {
          const auto type = choice.operation->operand_type;
          auto *value = unpack(current, type);
          // The half-open test proves current < last, so increment cannot wrap.
          return pack(b_.CreateAdd(value, llvm::ConstantInt::get(value->getType(), 1)), type);
        });
        store_incoming(*fact.place, advanced);
        b_.CreateCondBr(test(), body, done);
      }
      b_.SetInsertPoint(empty);
      if (expr.children.size() == 4) iteration(*expr.children[3], true);
      if (reachable()) b_.CreateBr(done);
      b_.SetInsertPoint(done);
      if (fact.type == Type::never) { b_.CreateUnreachable(); return nullptr; }
      return empty_scalar();
    }
    case K::alias: {
      const bool selected = fact.binding.has_value();
      const bool value = !selected || bindings_.at(*fact.binding).value_binding;
      auto *address = emit_expression(*expr.children.front(), value ? Projection::target : Projection::slot, !selected);
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
        if (declaration->kind == K::bare_ref_slot) {
          const auto id = contexts_.front().at(declaration.get()).binding.value();
          if (auto *slot = bindings_.at(id).reference_slot) b_.CreateStore(null_pointer(), slot);
          continue;
        }
        declare_local(*declaration);
        if (!reachable()) return nullptr;
      }
      return empty_scalar();
    }
    case K::init_reserved: {
      auto *destination = emit_expression(*expr.children.front(), Projection::target);
      if (!reachable()) return nullptr;
      emit_regional_initializer(expr, destination, expr.children.size() - 1);
      return reachable() ? empty_scalar() : nullptr;
    }
    case K::transfer: case K::rebind: {
      if (fact.rebind_native_handle) {
        std::map<size_t, std::vector<size_t>> destinations;
        for (size_t i = 0; i < contexts_.size(); ++i)
          destinations[*contexts_[i].at(&expr).place].push_back(i);
        auto *destination = contextual_value(expr, destinations, "native.rebind.place",
            [&](size_t place) { return storage_.at(place); }, b_.getPtrTy());
        auto *source = emit_expression(*expr.children[1], Projection::target);
        if (!reachable()) return nullptr;
        auto *selection = b_.CreateLoad(native_handle_pair(), source);
        b_.CreateStore(selection, destination);
        return empty_scalar();
      }
      if (projection == Projection::slot) throw std::logic_error("assignment result has no binding slot");
      const auto assignment = [&](bool policy) -> llvm::Value * {
        if (policy) {
          auto *value = emit_text_transfer(expr);
          return !reachable() || read ? value : null_pointer();
        }
        const auto arguments = prepare_arguments(expr);
        if (!reachable()) return nullptr;
        auto *address = assign_prepared(expr, arguments);
        if (expr.kind == K::transfer) return read ? load_place(address, expr) : address;
        return !read ? address : discard ? empty_scalar() : load_place(checked_address(address, expr, expr.at), expr);
      };
      if (expr.kind == K::rebind) return assignment(false);
      const auto selected_policy = [&](const auto &context) {
        const auto &operation = context.at(&expr);
        return operation.callee || operation.arguments.front().binding_slot;
      };
      if (!std::ranges::any_of(contexts_, selected_policy)) return assignment(false);
      std::map<bool, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < contexts_.size(); ++i) alternatives[selected_policy(contexts_[i])].push_back(i);
      return contextual_value(expr, alternatives, "transfer", assignment,
          read ? static_cast<llvm::Type *>(scalar_) : b_.getPtrTy());
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
      if (fact.exception_owner) {
        if (!pending_exception_result_) throw std::logic_error("exception return without a pending owner slot");
        std::map<size_t, std::vector<size_t>> holders;
        for (size_t i = 0; i < contexts_.size(); ++i)
          holders[*contexts_[i].at(&expr).exception_owner].push_back(i);
        value = contextual_value(expr, holders, "exception.return", [&](size_t holder) {
          auto *owner = take_failure_owner(b_, exception_holder_slots_.at(holder));
          b_.CreateStore(owner, pending_exception_result_);
          return load(storage_.at(holder));
        });
      } else if (fact.operand_type.kind == Type::region && fact.operand_type.owned) {
        const auto *make = expr.children.front().get();
        while (make->kind == K::grouped) make = make->children.front().get();
        auto *owner = construct_region(*make);
        if (!reachable()) return nullptr;
        value = b_.CreateInsertValue(value, b_.CreatePtrToInt(owner, b_.getInt64Ty()), 0);
      } else if (!expr.children.empty()) {
        value = emit_expression(*expr.children.front(), fact.borrowed_result ? Projection::target : Projection::value);
        if (!reachable()) return nullptr;
        if (fact.borrowed_result) {
          if (fact.operand_type.kind == Type::native_handle) {
            // Cleanup can replace a pending return. Its selected descriptor
            // must survive until the callable commits that completion.
            auto *pair = native_handle_pair();
            auto *pending = slot(pair, "native.pending.return");
            b_.CreateStore(b_.CreateLoad(pair, value), pending);
            value = pending;
          }
          value = pack_view(value, expr, true);
        } else value = convert_result(expr, *expr.children.front(), value);
      }
      if (fact.here_exit) {
        if (!here_destination_) throw std::logic_error("here group has no protected destination");
        clear_failure_owner();
        if (owned_failures_) {
          discard_transfer_failure(load(completion_.transfer), here_destination_);
          release_failure_owner(b_, take_failure_owner(b_, transfer_failure_slot(b_, here_destination_)));
        }
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
    case K::index: {
      auto *address = emit_indexed_address(expr);
      if (!reachable()) return nullptr;
      return !read ? address : discard ? empty_scalar() : load_place(address, expr);
    }
    case K::field: {
      if (fact.region_holder) {
        std::map<std::tuple<size_t, size_t, size_t>, std::vector<size_t>> projections;
        for (size_t i = 0; i < contexts_.size(); ++i) {
          const auto &field = contexts_[i].at(&expr);
          if (field.region_holder)
            projections[{field.operand_type.identity, *field.region_holder, *field.product_field}].push_back(i);
        }
        auto *member = contextual_value(expr, projections, "region.root.field", [&](const auto &selection) {
          const auto [schema, holder, index] = selection;
          auto *layout = product_layout(schema);
          auto *owner = load(storage_.at(holder));
          auto *address = region_address(owner, integer(0), llvm::ConstantExpr::getSizeOf(layout));
          return b_.CreateStructGEP(layout, address, index);
        }, b_.getPtrTy());
        if (!read) return member;
        return discard ? empty_scalar() : load_place(member, expr);
      }
      if (fact.exception_owner) {
        std::map<std::tuple<size_t, size_t, size_t>, std::vector<size_t>> projections;
        for (size_t i = 0; i < contexts_.size(); ++i) {
          const auto &field = contexts_[i].at(&expr);
          if (field.exception_owner) projections[{field.operand_type.identity, *field.exception_owner, *field.product_field}].push_back(i);
        }
        auto *member = contextual_value(expr, projections, "exception.field", [&](const auto &selection) -> llvm::Value * {
          const auto &[category, holder, index] = selection;
          auto *layout = product_layout(*program_->exception_types.at(category).schema);
          return b_.CreateStructGEP(layout, exception_payload(load(exception_holder_slots_.at(holder))), index);
        }, b_.getPtrTy());
        if (!read) return member;
        return discard ? empty_scalar() : load_place(member, expr);
      }
      if (fact.regional_sum_holder) {
        const auto &parent = *expr.children.front();
        auto *address = emit_expression(parent, Projection::target);
        if (!reachable()) return nullptr;
        using Field = std::tuple<size_t, size_t, size_t, size_t>;
        std::map<Field, std::vector<size_t>> projections;
        for (size_t i = 0; i < contexts_.size(); ++i) {
          const auto &field = contexts_[i].at(&expr);
          if (field.regional_sum_holder)
            projections[{field.sum_constructor->sum, field.sum_constructor->alternative,
                         *field.product_field, *field.regional_sum_holder}].push_back(i);
        }
        auto *target = contextual_value(expr, projections, "region.sum.field", [&](const Field &member) {
          const auto [sum, alternative, field, holder] = member;
          auto *slot = sum_field_address(address, {sum, alternative}, field);
          const auto &description = program_->sums.at(sum).alternatives.at(alternative).fields.at(field);
          if (!description.stores_target()) return static_cast<llvm::Value *>(slot);
          auto *owner = load(storage_.at(holder));
          auto *offset = b_.CreateLoad(b_.getInt64Ty(), slot);
          return regional_scalar_target(owner, offset, sum_layout(description.type.identity).storage);
        }, b_.getPtrTy());
        return !read ? target : discard ? empty_scalar() : load_place(target, expr);
      }
      if (fact.product_field) {
        const auto &parent = *expr.children.front();
        auto *address = emit_expression(parent, Projection::target);
        if (!reachable()) return nullptr;
        address = checked_address(address, parent, expr.at);
        using Field = std::tuple<size_t, size_t, std::optional<size_t>>;
        std::map<Field, std::vector<size_t>> projections;
        for (size_t i = 0; i < contexts_.size(); ++i) {
          const auto &field = contexts_[i].at(&expr);
          if (field.product_field)
            projections[{field.operand_type.identity, *field.product_field, field.regional_product_holder}].push_back(i);
        }
        auto *field = contextual_value(expr, projections, "product.field", [&](const Field &member) -> llvm::Value * {
          const auto [schema, number, holder] = member;
          auto *layout = holder ? regional_product_layout(schema) : product_layout(schema);
          auto *slot = b_.CreateStructGEP(layout, address, number);
          const auto &description = program_->schemas[schema].fields[number];
          if (projection != Projection::slot && description.stores_target()) {
            if (holder) {
              auto *owner = load(storage_.at(*holder));
              auto *offset = b_.CreateLoad(b_.getInt64Ty(), slot);
              return regional_scalar_target(owner, offset, native_type(description.type));
            }
            return b_.CreateLoad(b_.getPtrTy(), slot);
          }
          return slot;
        }, b_.getPtrTy());
        if (!read) return field;
        field = checked_address(field, expr, expr.at);
        return discard ? empty_scalar() : load_place(field, expr);
      }
      auto *value = emit_expression(*expr.children[0]);
      if (!reachable()) return nullptr;
      const auto &type = contexts_.front().at(expr.children[0].get()).type;
      auto *payload = unpack(value, type);
      const unsigned index = expr.text == "category" ? 0 : expr.text == "site" ? 1 : expr.text == "requested" ? 2 : expr.text == "available" ? 3 : 4;
      auto *field = b_.CreateExtractValue(payload, index);
      if (expr.text == "cause_count") field = b_.CreateCall(runtime("cod_lang_exception_count", b_.getInt64Ty(), {b_.getInt64Ty()}), {field});
      return pack(field, Type::uint64);
    }
    case K::region_binding:
      return temporary_scope(expr.at, scalar_, [&] { return acquire_region(expr); });
    case K::exception_binding:
      return receive_exception_holder(expr);
    case K::native_handle_binding: {
      if (expr.binding == "=") {
        auto *source = emit_expression(*expr.children.front(), Projection::target);
        if (!reachable()) return nullptr;
        std::map<size_t, std::vector<size_t>> alternatives;
        for (size_t i = 0; i < contexts_.size(); ++i)
          if (const auto place = contexts_[i].at(&expr).place) alternatives[*place].push_back(i);
        auto *pair = native_handle_pair();
        auto *value = b_.CreateLoad(pair, source);
        return contextual_value(expr, alternatives, "native.borrow", [&](size_t place) {
          b_.CreateStore(value, storage_.at(place));
          return empty_scalar();
        });
      }
      std::map<std::pair<size_t, CheckedOperation::NativeAcquisition>, std::vector<size_t>> plans;
      for (size_t i = 0; i < contexts_.size(); ++i) {
        const auto &acquisition = contexts_[i].at(&expr);
        plans[{*acquisition.place, *acquisition.native_acquisition}].push_back(i);
      }
      return contextual_value(expr, plans, "native.acquire", [&](const auto &selected) {
        return temporary_scope(expr.at, scalar_, [&] { return acquire_native_handle(expr, selected.first, selected.second); });
      });
    }
    case K::make_borrowed_handle:
      return emit_attached_native_handle(expr);
    case K::make_object: {
      if (fact.array_allocator) return emit_array_construction(expr, discard);
      if (fact.regional_sum_holder) {
        using Destination = std::tuple<size_t, size_t, size_t>;
        std::map<Destination, std::vector<size_t>> placements;
        for (size_t i = 0; i < contexts_.size(); ++i) {
          const auto &item = contexts_[i].at(&expr);
          if (item.regional_sum_holder)
            placements[{item.sum_constructor->sum, item.sum_constructor->alternative, *item.regional_sum_holder}].push_back(i);
        }
        auto *address = contextual_value(expr, placements, "region.sum.reservation", [&](const Destination &destination) {
          const auto &[sum, alternative, holder] = destination;
          auto *owner = load(storage_.at(holder));
          const auto &layout = sum_layout(sum);
          auto *target = reserve_regional_row(owner, layout.storage, expr.at);
          const auto tag = layout.alternatives.at(alternative).tag;
          b_.CreateStore(integer(tag), b_.CreateStructGEP(layout.storage, target, 0));
          return target;
        }, b_.getPtrTy());
        if (fact.invokes_regional_initializer && reachable())
          address = emit_regional_initializer(expr, address, expr.children.size());
        if (!reachable()) return nullptr;
        return discard ? empty_scalar() : address;
      }
      if (fact.sum_constructor) {
        const auto constructor = *fact.sum_constructor;
        auto *address = storage_.at(*fact.place);
        const auto items = expr.initializer_items();
        for (size_t i = 0; i < fact.sum_initializers.size(); ++i) {
          const auto &item = *items[i];
          const auto &input = *item.children.back();
          const auto index = fact.sum_initializers[i];
          const auto &type = program_->sums.at(constructor.sum).alternatives.at(constructor.alternative).fields.at(index).type;
          const bool aggregate = inline_aggregate_type(type);
          auto *value = emit_expression(input, aggregate ? Projection::target : Projection::value);
          if (!reachable()) return nullptr;
          auto *field = sum_field_address(address, constructor, index);
          if (aggregate) b_.CreateCall(bits_transfer(type), {field, checked_address(value, input, input.at)});
          else assign_place(field, convert_result(item, input, value), item, &CheckedOperation::operand_type);
        }
        const auto &layout = sum_layout(constructor.sum);
        const auto tag = layout.alternatives.at(constructor.alternative).tag;
        b_.CreateStore(integer(tag), b_.CreateStructGEP(layout.storage, address, 0));
        return discard ? empty_scalar() : address;
      }
      if (fact.lexical_allocation) {
        using Allocation = CheckedOperation::LexicalAllocation;
        std::vector<llvm::Value *> operands;
        for (const auto &piece : expr.children) {
          operands.push_back(emit_expression(*piece, Projection::target));
          if (!reachable()) return nullptr;
        }
        std::map<Allocation, std::vector<size_t>> destinations;
        for (size_t i = 0; i < contexts_.size(); ++i) {
          const auto &construction = contexts_[i].at(&expr);
          if (construction.type != Type::never) destinations[construction.lexical_allocation.value()].push_back(i);
        }
        auto *value = contextual_value(expr, destinations, "lexical.allocate", [&](const Allocation &plan) -> llvm::Value * {
          auto *owner = owner_identity(plan.owner, expr.at);
          if (plan.kind == Allocation::builder) {
            auto create = runtime("cod_lang_lexical_builder_create", b_.getPtrTy(),
                {b_.getPtrTy(), b_.getPtrTy(), b_.getPtrTy(), b_.getInt32Ty()});
            auto *left = operands.empty() ? null_pointer() : operands[0];
            auto *right = operands.empty() ? null_pointer() : operands[1];
            return b_.CreateCall(create, {owner, left, right, b_.getInt32(plan.builder_inputs)});
          }
          const auto name = plan.kind == Allocation::builder_snapshot ? "cod_lang_lexical_builder_finish" : "cod_lang_lexical_text_create";
          auto create = runtime(name, b_.getPtrTy(), {b_.getPtrTy(), b_.getPtrTy()});
          return b_.CreateCall(create, {owner, operands.front()});
        }, b_.getPtrTy());
        auto *ready = block("allocation.ready"), *failed = block("allocation.failed");
        b_.CreateCondBr(b_.CreateIsNotNull(value), ready, failed);
        b_.SetInsertPoint(failed);
        fail_path(ExceptionCategory::out_of_memory, expr.at);
        b_.SetInsertPoint(ready);
        return discard ? empty_scalar() : value;
      }
      if (fact.regional_product_holder) {
        using Destination = std::pair<size_t, size_t>;
        std::map<Destination, std::vector<size_t>> placements;
        for (size_t i = 0; i < contexts_.size(); ++i) {
          const auto &item = contexts_[i].at(&expr);
          if (item.regional_product_holder)
            placements[{item.operand_type.identity, *item.regional_product_holder}].push_back(i);
        }
        auto *address = contextual_value(expr, placements, "region.product", [&](const Destination &destination) {
          auto *owner = load(storage_.at(destination.second));
          auto *row = reserve_regional_row(owner, regional_product_layout(destination.first), expr.at);
          emit_product_initializers(expr, row, owner);
          return row;
        }, b_.getPtrTy());
        return discard ? empty_scalar() : address;
      }
      auto *address = storage_.at(*fact.place);
      if (fact.operand_type.kind == Type::product) emit_product_initializers(expr, address);
      else if (const auto *initializer = expr.construction_initializer()) {
        auto *value = emit_expression(*initializer);
        if (!reachable()) return nullptr;
        assign_place(address, convert_result(expr, *initializer, value), expr, &CheckedOperation::operand_type);
      }
      if (!reachable()) return nullptr;
      if (!read) return address;
      return discard || fact.operand_type.kind == Type::product ? empty_scalar() : load_place(address, expr);
    }
    case K::raise: {
      if (expr.annotation) {
        std::map<size_t, std::vector<size_t>> alternatives;
        for (size_t i = 0; i < contexts_.size(); ++i) alternatives[contexts_[i].at(&expr).operand_type.identity].push_back(i);
        return contextual_value(expr, alternatives, "exception.construct", [&](size_t category) {
          return construct_exception(expr, category);
        });
      }
      llvm::Value *value = fact.place ? load(storage_.at(*fact.place)) : emit_expression(*expr.children[0]);
      if (!reachable()) return nullptr;
      value = unpack(value, program_->exception_type(0));
      if (expr.children.size() == 2) {
        auto *cause = emit_expression(*expr.children[1]);
        if (!reachable()) return nullptr;
        cause = unpack(cause, program_->exception_type(0));
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
      auto *owner = owned_failures_ && fact.exception_owner
          ? take_failure_owner(b_, exception_holder_slots_.at(*fact.exception_owner)) : nullptr;
      raise_exception(value, owner);
      return nullptr;
    }
    case K::try_scope: return emit_finally(expr, projection, discard);
    case K::deferred: {
      const auto &body = *expr.children.front();
      using Layout = std::pair<std::set<size_t>, std::optional<DeferredInputs>>;
      std::map<Layout, std::vector<size_t>> alternatives;
      for (size_t i = 0; i < contexts_.size(); ++i)
        alternatives[{contexts_[i].at(&expr).captures, deferred_inputs(body, contexts_[i].at(&body))}].push_back(i);
      if (alternatives.size() > 1)
        return contextual_value(expr, alternatives, "defer", [&](const auto &) { return emit_expression(expr, projection, discard); });
      return temporary_scope(expr.at, scalar_, [&]() -> llvm::Value * {
      std::vector<llvm::Value *> arguments;
      if (body.kind == K::block) for (const auto id : fact.captures) arguments.push_back(load(storage_.at(id)));
      else arguments = prepare_arguments(body, true);
      if (!reachable()) return nullptr;
      auto record = std::find_if(scope_->actions.begin(), scope_->actions.end(), [&](const CleanupAction &entry) {
        return entry.action == &expr && entry.captures == fact.captures && entry.inputs == alternatives.begin()->first.second;
      });
      if (record == scope_->actions.end()) throw std::logic_error("uncollected deferred action");
      llvm::Value *prepared = record->activation_record;
      if (!prepared) {
        auto *status = slot(b_.getInt64Ty(), "defer.status");
        b_.CreateStore(integer(0), status);
        const auto bytes = module_.getDataLayout().getTypeAllocSize(record->layout).getFixedValue();
        const auto allocate = runtime("cod_lang_defer_allocate", b_.getPtrTy(), {b_.getInt64Ty(), b_.getPtrTy()});
        prepared = b_.CreateCall(allocate, {integer(bytes), status});
        auto *ready = block("defer.prepared"), *failed = block("defer.allocation_failed");
        b_.CreateCondBr(b_.CreateICmpEQ(prepared, null_pointer()), failed, ready);
        b_.SetInsertPoint(failed); fail_path(ExceptionCategory::out_of_memory, expr.at);
        b_.SetInsertPoint(ready);
      }
      prepare_cleanup_record(*scope_, prepared, std::distance(scope_->actions.begin(), record));
      for (size_t i = 0; i < arguments.size(); ++i) {
        auto *field = record->layout->getElementType(i + first_deferred_operand);
        auto *destination = b_.CreateStructGEP(record->layout, prepared, i + first_deferred_operand);
        if (field == native_handle_pair()) b_.CreateStore(b_.CreateLoad(field, view_address(arguments[i])), destination);
        else {
          b_.CreateStore(arguments[i], destination);
          take_value(arguments[i]);
        }
      }
      b_.CreateStore(prepared, scope_->head);
      return empty_scalar();
      });
    }
    default: throw std::logic_error("unchecked expression reached LLVM emission");
    }
  }
  void emit_product_initializers(const Expression &expr, llvm::Value *address, llvm::Value *regional_owner = nullptr) {
    const auto schema_id = contexts_.front().at(&expr).operand_type.identity;
    auto *layout = regional_owner ? regional_product_layout(schema_id) : product_layout(schema_id);
    const auto &schema = program_->schemas.at(schema_id);
    for (const auto &initializer : expr.initializer_items()) {
      const auto &input = *initializer->children.back();
      const auto index = *contexts_.front().at(initializer.get()).product_field;
      const auto &member = schema.fields[index];
      const bool direct = member.storage == PlaceKind::value;
      if (regional_owner && !direct) {
        auto *slot = b_.CreateStructGEP(layout, address, index);
        std::map<bool, std::vector<size_t>> preparations;
        for (size_t i = 0; i < contexts_.size(); ++i)
          preparations[contexts_[i].at(initializer.get()).allocates_scalar_field].push_back(i);
        contextual_value(*initializer, preparations, "region.field.init", [&](bool fresh) -> llvm::Value * {
          llvm::Value *offset;
          if (fresh) {
            auto *scalar = native_type(member.type);
            const auto &data = module_.getDataLayout();
            auto *bytes = integer(data.getTypeAllocSize(scalar).getFixedValue());
            offset = reserve_region_extent(regional_owner, bytes, data.getABITypeAlign(scalar).value(), initializer->at);
            auto *value = emit_expression(input);
            if (!reachable()) return nullptr;
            auto *target = region_address(regional_owner, offset, bytes);
            assign_place(target, convert_result(*initializer, input, value), *initializer, &CheckedOperation::operand_type);
          } else {
            auto *target = emit_expression(input, Projection::target);
            if (!reachable()) return nullptr;
            offset = regional_scalar_locator(regional_owner, target);
          }
          b_.CreateStore(offset, slot);
          return b_.getTrue();
        }, b_.getInt1Ty());
      } else if (member.direct_text_place()) {
        emit_text_transfer(*initializer, b_.CreateStructGEP(layout, address, index));
      } else if (direct && member.type.kind == Type::product) {
        emit_product_initializers(input, b_.CreateStructGEP(layout, address, index));
      } else if (direct && member.type.kind == Type::sum) {
        auto *value = emit_expression(input, Projection::target);
        if (!reachable()) return;
        auto *field = b_.CreateStructGEP(layout, address, index);
        b_.CreateCall(bits_transfer(member.type), {field, checked_address(value, input, input.at)});
      } else {
        auto *value = emit_expression(input, direct ? Projection::value : Projection::target);
        if (!reachable()) return;
        auto *field = b_.CreateStructGEP(layout, address, index);
        if (direct) assign_place(field, convert_result(*initializer, input, value), *initializer, &CheckedOperation::operand_type);
        else b_.CreateStore(value, field);
      }
      if (!reachable()) return;
    }
  }
  llvm::Function *declare(const std::string &name, const CallLayout &layout) {
    std::vector<llvm::Type *> parameters(layout.operands, scalar_);
    parameters.insert(parameters.end(), layout.owners, b_.getPtrTy());
    parameters.insert(parameters.end(), layout.array_owners.size(), array_capability_);
    if (layout.captures) parameters.push_back(b_.getPtrTy());
    parameters.insert(parameters.end(), layout.retained + layout.declaration_context, b_.getInt32Ty());
    if (layout.callable_destination) parameters.push_back(b_.getPtrTy());
    if (layout.native_borrow_destination) parameters.push_back(b_.getPtrTy());
    if (layout.exception_result) parameters.push_back(b_.getPtrTy());
    parameters.insert(parameters.end(), layout.here_count(), here_);
    if (layout.here_group) parameters.push_back(b_.getPtrTy());
    return llvm::Function::Create(llvm::FunctionType::get(outcome_, parameters, false),
        llvm::GlobalValue::InternalLinkage, name, module_);
  }
  void declare_prepared_source(size_t id) {
    const auto &instance = program_->instances.at(id);
    const auto &layout = call_layouts_.at(instance.method);
    if (layout.captures || layout.retained || layout.declaration_context || layout.has_result_destination() ||
        !layout.here_formals || layout.here_formals > 2 || layout.lexical_here || layout.here_group)
      throw std::logic_error("unexpected parameter role in prepared producer");
    auto *entry = llvm::Function::Create(llvm::FunctionType::get(outcome_, {here_, b_.getPtrTy()}, false),
        llvm::GlobalValue::InternalLinkage, "cod.here.source." + std::to_string(id), module_);
    prepared_source_entries_.emplace(id, entry);
    llvm::IRBuilder<> body(llvm::BasicBlock::Create(context_, "entry", entry));
    auto *storage = prepared_producer_layout(layout.operands, layout.owners);
    llvm::Value *backing = entry->getArg(1);
    llvm::Value *upstream = nullptr;
    if (layout.here_formals == 2) {
      auto *connection = prepared_connection_layout();
      upstream = body.CreateLoad(here_, body.CreateStructGEP(connection, backing, 1));
      backing = body.CreateLoad(b_.getPtrTy(), body.CreateStructGEP(connection, backing, 0));
    }
    std::vector<llvm::Value *> arguments;
    for (size_t i = 0; i < layout.operands; ++i)
      arguments.push_back(body.CreateLoad(scalar_, body.CreateStructGEP(storage, backing, i)));
    for (size_t i = 0; i < layout.owners; ++i)
      arguments.push_back(body.CreateLoad(b_.getPtrTy(), body.CreateStructGEP(storage, backing, layout.operands + i)));
    arguments.push_back(entry->getArg(0));
    if (upstream) arguments.push_back(upstream);
    auto *target = methods_.at(instance.method);
    body.CreateRet(body.CreateCall(target, arguments));
  }
  void declare_here_entry(size_t id) {
    const auto &instance = program_->instances.at(id);
    const auto &method = program_->methods.at(instance.method);
    const auto &layout = call_layouts_.at(instance.method);
    const auto &inputs = *method.scope_inputs;
    const auto &reception = inputs.here_reception.value();
    const auto offered = reception.offered;
    const auto places = layout.operands - reception.selected.size();
    const auto handed_owners = inputs.here_input_owners;
    if (layout.captures || layout.retained || layout.declaration_context || layout.has_result_destination() || layout.here_formals)
      throw std::logic_error("unexpected parameter role in supplied here body");
    // The activation starts with the transfer record. Its address identifies
    // the originating call across every forwarding receiver.
    std::vector<llvm::Type *> fields;
    if (method.here_group) fields.push_back(method.here_group->binds_return_payload ? here_return_header() : here_transfer_);
    fields.insert(fields.end(), places, scalar_);
    fields.insert(fields.end(), layout.owners - handed_owners, b_.getPtrTy());
    fields.insert(fields.end(), layout.lexical_here, here_);
    auto *activation = llvm::StructType::get(context_, fields);
    std::vector<llvm::Type *> parameters(offered, scalar_);
    parameters.insert(parameters.end(), handed_owners, b_.getPtrTy());
    parameters.push_back(b_.getPtrTy());
    auto *entry = llvm::Function::Create(llvm::FunctionType::get(outcome_, parameters, false),
        llvm::GlobalValue::InternalLinkage, "cod.here.entry." + std::to_string(id), module_);
    here_entries_.emplace(id, HereEntry{entry, activation, layout.here_group});
    llvm::IRBuilder<> body(llvm::BasicBlock::Create(context_, "entry", entry));
    auto *borrowed = entry->getArg(offered + handed_owners);
    std::vector<llvm::Value *> prepared;
    for (size_t i = 0; i < reception.selected.size(); ++i) {
      llvm::Value *value = entry->getArg(reception.selected[i]);
      if (inputs.here_snapshots.contains(i)) {
        auto *address = body.CreateIntToPtr(body.CreateExtractValue(value, 0), body.getPtrTy());
        value = load_place(body, address, StorageFormat{instance.arguments.at(i)});
      }
      prepared.push_back(value);
    }
    for (size_t i = 0; i < places; ++i)
      prepared.push_back(body.CreateLoad(scalar_, body.CreateStructGEP(activation, borrowed, layout.here_group + i)));
    for (const auto &[owner, input] : instance.frame.owners) {
      const auto role = null_target - 1 - owner;
      if (role < handed_owners) prepared.push_back(entry->getArg(offered + role));
      else {
        const auto field = layout.here_group + places + role - handed_owners;
        prepared.push_back(body.CreateLoad(b_.getPtrTy(), body.CreateStructGEP(activation, borrowed, field)));
      }
    }
    const auto here_begin = layout.here_group + places + layout.owners - handed_owners;
    for (size_t i = 0; i < layout.lexical_here; ++i)
      prepared.push_back(body.CreateLoad(here_, body.CreateStructGEP(activation, borrowed, here_begin + i)));
    if (layout.here_group) prepared.push_back(borrowed);
    auto *result = body.CreateCall(methods_.at(instance.method), prepared);
    if (!layout.here_group) { body.CreateRet(result); return; }
    auto *forward = llvm::BasicBlock::Create(context_, "here.failure", entry);
    auto *completed = llvm::BasicBlock::Create(context_, "here.completed", entry);
    auto *failure = body.CreateOr(body.CreateICmpNE(body.CreateExtractValue(result, 1), integer(0)),
                                  body.CreateIsNotNull(body.CreateExtractValue(result, 3)));
    auto *ordinary_failure = body.CreateAnd(failure, body.CreateIsNull(body.CreateExtractValue(result, 7)));
    body.CreateCondBr(ordinary_failure, forward, completed);
    body.SetInsertPoint(completed);
    body.CreateRet(result);
    body.SetInsertPoint(forward);
    if (owned_failures_) release_failure_owner(body, take_failure_owner(body, transfer_failure_slot(body, borrowed)));
    body.CreateStore(body.getInt8(static_cast<unsigned>(HereExit::failure)), body.CreateStructGEP(here_transfer_, borrowed, 0));
    auto *plain_failure = body.CreateInsertValue(result, null_pointer(), 7);
    body.CreateStore(plain_failure, body.CreateStructGEP(here_transfer_, borrowed, 1));
    body.CreateRet(body.CreateInsertValue(llvm::Constant::getNullValue(outcome_), borrowed, 7));
  }
  void begin(llvm::Function *function, const std::vector<CheckedStorage> &storage, const std::vector<CheckedBinding> &bindings,
             const CheckedFrame &frame = {}, std::span<const std::set<StorageFormat>> storage_types = {}) {
    function_ = function; arguments_.clear(); retained_types_.clear(); parameter_values_.clear(); value_owners_.clear(); query_environment_ = nullptr; here_arguments_.clear(); pipeline_operands_.clear(); here_destination_ = nullptr; temporaries_ = nullptr;
    suspended_return_ = {};
    for (size_t i = 0; i < frame.inputs.size(); ++i) arguments_.push_back(function->getArg(i));
    b_.SetInsertPoint(block("entry")); finish_ = block("completion"); scope_ = nullptr; control_ = nullptr;
    owner_identities_.clear();
    exception_holder_slots_.clear();
    exception_parameter_slots_.clear();
    exception_result_destination_ = pending_exception_result_ = nullptr;
    array_capabilities_.clear();
    lexical_allocations_ = nullptr;
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
    if (owned_failures_) {
      completion_.failure_owner = slot(b_.getPtrTy(), "exception.owner");
      b_.CreateStore(null_pointer(), completion_.failure_owner);
    }
    b_.CreateStore(b_.getFalse(), completion_.has_return);
    b_.CreateStore(empty_scalar(), completion_.value); b_.CreateStore(integer(0), completion_.category);
    b_.CreateStore(integer(0), completion_.site); b_.CreateStore(null_pointer(), completion_.fatal); b_.CreateStore(b_.getInt8(0), completion_.kind);
    for (auto *place : {completion_.requested, completion_.available, completion_.cause}) b_.CreateStore(integer(0), place);
    storage_.clear();
    storage_formats_.clear();
    for (size_t i = 0; i < storage.size(); ++i) {
      const auto &binding = storage[i];
      auto &formats = storage_formats_.emplace_back();
      if (storage_types.empty()) formats.emplace(binding.type);
      else formats = storage_types[i];
      llvm::Type *layout = scalar_;
      if (binding.type.kind == Type::product) layout = product_layout(binding.type.identity);
      else if (binding.type.kind == Type::sum) layout = sum_layout(binding.type.identity).storage;
      else if (binding.type.kind == Type::native_handle)
        layout = binding.type.owned ? static_cast<llvm::Type *>(b_.getInt64Ty()) : native_handle_pair();
      else if (binding.type.kind == Type::region) layout = b_.getPtrTy();
      else if (!formats.contains(StorageFormat{}))
        layout = formats.size() == 1 ? native_type(formats.begin()->type()) : b_.getInt64Ty();
      auto *place = binding.borrowed || binding.subobject ? nullptr : slot(layout, binding.name + ".storage");
      storage_.push_back(place);
      if (place && environments_ && layout == scalar_) initialize_value(place);
    }
    bindings_.clear();
    for (const auto &binding : bindings) bindings_.push_back({binding.value_binding});
    for (size_t i = 0; i < frame.inputs.size(); ++i) if (frame.inputs[i].binding_slot)
      bindings_.at(storage.at(frame.inputs[i].place).binding).reference_slot = view_address(arguments_[i]);
    for (size_t i = 0; i < bindings.size(); ++i) {
      auto &binding = bindings_[i];
      const auto &declaration = bindings[i];
      if (!binding.reference_slot && !declaration.value_binding && !declaration.embedded_slot && !declaration.alias && declaration.selection.empty())
        binding.reference_slot = slot(b_.getPtrTy(), "reference.target");
      if (!declaration.selection.empty() || declaration.indexed_element_owner)
        binding.selected_place = slot(b_.getPtrTy(), "alias.selection");
    }
    for (size_t i = 0; i < arguments_.size(); ++i) {
      const auto &input = frame.inputs[i];
      const auto place = input.place, binding = storage.at(place).binding;
      if (input.binding_slot) continue;
      if (input.mode == ArgumentMode::owning_exception) {
        auto *layout = exception_input_layout();
        auto *carrier = view_address(arguments_[i]);
        auto *source = b_.CreateLoad(b_.getPtrTy(), b_.CreateStructGEP(layout, carrier, 0));
        auto *owner_slot = b_.CreateLoad(b_.getPtrTy(), b_.CreateStructGEP(layout, carrier, 1));
        auto *holder = slot(b_.getPtrTy(), "exception.parameter");
        b_.CreateStore(take_failure_owner(b_, owner_slot), holder);
        store_incoming(place, b_.CreateLoad(scalar_, source));
        exception_holder_slots_.emplace(place, holder);
        exception_parameter_slots_.push_back(holder);
        continue;
      }
      if (input.mode != ArgumentMode::value) {
        auto *address = view_address(arguments_[i]);
        auto &bound = bindings_.at(binding);
        if (bound.value_binding) bound.direct_place = address;
        else b_.CreateStore(address, bound.reference_slot);
        continue;
      }
      store_incoming(place, arguments_[i]);
      if (environments_ && storage_.at(place)->getAllocatedType() == scalar_) parameter_values_.push_back(storage_.at(place));
      if (auto *slot = bindings_.at(binding).reference_slot) b_.CreateStore(storage_.at(place), slot);
    }
  }
  void body(llvm::Function *function, const CheckedMethod &method, const CheckedInstance &layout) {
    const auto prepared = checked_body(layout);
    const auto &frame = prepared.frame;
    const auto &call = call_layouts_.at(layout.method);
    if (function->arg_size() != call.parameter_count()) throw std::logic_error("native body disagrees with its parameter roles");
    begin(function, prepared.storage, layout.bindings, frame, prepared.storage_types);
    if (call.exception_result) {
      pending_exception_result_ = slot(b_.getPtrTy(), "exception.pending.result");
      b_.CreateStore(null_pointer(), pending_exception_result_);
    }
    for (size_t i = 0; i < call.array_owners.size(); ++i)
      array_capabilities_[call.array_owners[i]] = function->getArg(call.array_owners_begin() + i);
    case_scan_request_ = method.source->case_scan_scope ? slot(b_.getInt1Ty(), "case.scan.requested") : nullptr;
    if (case_scan_request_) b_.CreateStore(b_.getFalse(), case_scan_request_);
    for (size_t i = 0; i < call.here_count(); ++i) here_arguments_.push_back(function->getArg(call.here_index() + i));
    if (call.here_group) here_destination_ = function->getArg(call.here_index() + call.here_count());
    if (method.here_group && method.here_group->supplier_return.has_value())
      suspended_return_.present = b_.getInt1(*method.here_group->supplier_return);
    if (method.here_group && method.here_group->binds_return_payload)
      suspended_return_.value = b_.CreateLoad(scalar_, b_.CreateStructGEP(here_return_header(), here_destination_, 1));
    contexts_ = {{&prepared.context.operations(), &layout}};
    if (call.captures) {
      auto *values = function->getArg(call.captures_index());
      for (size_t c = 0; c < call.captures; ++c) {
        auto *captured = b_.CreateLoad(scalar_, b_.CreateGEP(scalar_, values, integer(c)));
        // Writable locals acquire complete values below, after the incoming
        // context is validated. Read-only observation borrows immutable data.
        if (!prepared.writable_captures.contains(c)) store_incoming(frame.captures.at(c), captured);
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
    auto *requires_return = !method.regional_initializer && method.source->completion_requires_return(*layout.result.type)
        ? b_.getTrue() : nullptr;
    std::optional<Control> initialization;
    if (!prepared.writable_captures.empty()) {
      initialization = Control{nullptr, block("inputs.failed")};
      control_ = &*initialization;
    }
    for (const auto c : prepared.writable_captures) {
      const auto place = frame.captures.at(c);
      if (storage_.at(place)->getAllocatedType() == scalar_) parameter_values_.push_back(storage_.at(place));
      auto *value = arguments_.at(call.operands + c);
      store_incoming(place, copy_value(value, method.captures.at(c).at));
    }
    llvm::Value *returns_void_at_end = nullptr;
    if (!method.regional_initializer && method.source->returns_void_at_end(Type::void_type))
      returns_void_at_end = requires_return ? b_.CreateNot(requires_return) : b_.getTrue();
    call_scope({method.source->body.get()}, method.source->at, requires_return,
        call.here_group ? &*layout.result.type : nullptr, returns_void_at_end);
    if (initialization) {
      b_.SetInsertPoint(initialization->exit);
      for (auto i = parameter_values_.rbegin(); i != parameter_values_.rend(); ++i) clear_value(*i);
      b_.CreateBr(finish_);
    }
    control_ = nullptr;
    b_.SetInsertPoint(finish_);
    for (auto i = exception_parameter_slots_.rbegin(); i != exception_parameter_slots_.rend(); ++i)
      release_failure_owner(b_, take_failure_owner(b_, *i));
    if (environments_) {
      auto *failed = block("return.discard"), *ready = block("return.ready");
      auto *kind = load(completion_.kind);
      auto *discard = protected_outcomes_ ? b_.CreateICmpUGE(kind, b_.getInt8(2)) : b_.CreateICmpEQ(kind, b_.getInt8(2));
      b_.CreateCondBr(discard, failed, ready);
      b_.SetInsertPoint(failed); clear_value(completion_.value); b_.CreateBr(ready); b_.SetInsertPoint(ready);
    }
    auto *value = load(completion_.value);
    if (call.native_borrow_destination) {
      auto *native = b_.getTrue();
      auto *ready = b_.CreateAnd(native, b_.CreateICmpEQ(load(completion_.kind), b_.getInt8(1)));
      auto *unpublished_value = b_.CreateSelect(native, empty_scalar(), value);
      auto *publish = block("native.return.publish"), *done = block("native.return.complete");
      auto *unpublished = b_.GetInsertBlock();
      b_.CreateCondBr(ready, publish, done);
      b_.SetInsertPoint(publish);
      auto *destination = function->getArg(call.native_result_index());
      b_.CreateStore(b_.CreateLoad(native_handle_pair(), view_address(value)), destination);
      auto *result = b_.CreateInsertValue(value, b_.CreatePtrToInt(destination, b_.getInt64Ty()), 0);
      b_.CreateBr(done);
      b_.SetInsertPoint(done);
      auto *delivered = b_.CreatePHI(scalar_, 2);
      delivered->addIncoming(unpublished_value, unpublished);
      delivered->addIncoming(result, publish);
      value = delivered;
    }
    if (call.callable_destination) b_.CreateStore(value, function->getArg(call.result_index()));
    if (call.exception_result) {
      auto *owner = take_failure_owner(b_, pending_exception_result_);
      auto *normal = b_.CreateICmpEQ(load(completion_.kind), b_.getInt8(1));
      b_.CreateStore(b_.CreateSelect(normal, owner, null_pointer()), function->getArg(call.exception_result_index()));
      release_failure_owner(b_, b_.CreateSelect(normal, null_pointer(), owner));
    }
    b_.CreateRet(outcome(value, load(completion_.category), load(completion_.site), load(completion_.fatal),
        load(completion_.requested), load(completion_.available), load(completion_.cause),
        protected_outcomes_ ? load(completion_.transfer) : nullptr, owned_failures_ ? load(completion_.failure_owner) : nullptr,
        case_scan_request_ ? b_.CreateAnd(load(case_scan_request_), b_.CreateICmpEQ(load(completion_.kind), b_.getInt8(0))) : nullptr));
    case_scan_request_ = nullptr;
  }
  void emit_sites() {
    function_ = llvm::Function::Create(llvm::FunctionType::get(b_.getPtrTy(), {b_.getInt64Ty()}, false),
        llvm::GlobalValue::ExternalLinkage, "cod_lang_exception_site", module_);
    b_.SetInsertPoint(block("entry"));
    auto *unknown = block("unknown");
    auto *select = b_.CreateSwitch(function_->getArg(0), unknown, sites_.size());
    for (size_t i = 0; i < sites_.size(); ++i) {
      auto *known = block("site"); select->addCase(integer(i + 1), known); b_.SetInsertPoint(known);
      b_.CreateRet(b_.CreateGlobalString(location_text(sites_[i].location)));
    }
    b_.SetInsertPoint(unknown); b_.CreateRet(null_pointer());

    if (!named_exception_site_) return;
    function_ = named_exception_site_;
    b_.SetInsertPoint(block("entry"));
    auto *unnamed = block("unnamed");
    auto *selected_site = b_.CreateSwitch(function_->getArg(1), unnamed);
    for (size_t i = 0; i < sites_.size(); ++i) {
      const auto &nominal = sites_[i].nominal;
      if (!nominal) continue;
      auto *named = block("named");
      selected_site->addCase(integer(i + 1), named);
      b_.SetInsertPoint(named);
      auto *matches = b_.CreateICmpEQ(function_->getArg(0), integer(nominal->category));
      auto *display = b_.CreateGlobalString(program_->exception_display_name(nominal->category, nominal->spelling));
      b_.CreateRet(b_.CreateSelect(matches, display, null_pointer()));
    }
    b_.SetInsertPoint(unnamed); b_.CreateRet(null_pointer());
  }
  void emit_uncaught_failure() {
    auto *category = load(completion_.category);
    auto *where = load(completion_.site);
    if (owned_failures_) {
      clear_failure_owner();
      auto *intrinsic = block("uncaught.intrinsic");
      const auto first_nominal = exception_categories.size();
      auto *select = b_.CreateSwitch(category, intrinsic, program_->exception_types.size() - first_nominal);
      auto report = runtime("cod_lang_uncaught_nominal", b_.getVoidTy(), {b_.getPtrTy(), b_.getInt64Ty()});
      if (!named_exception_site_ && first_nominal < program_->exception_types.size()) {
        auto *type = llvm::FunctionType::get(b_.getPtrTy(), {b_.getInt64Ty(), b_.getInt64Ty()}, false);
        named_exception_site_ = llvm::Function::Create(type, llvm::GlobalValue::InternalLinkage,
                                                       "cod.exception.named_site", module_);
      }
      for (size_t i = first_nominal; i < program_->exception_types.size(); ++i) {
        auto *nominal = block("uncaught.nominal");
        select->addCase(integer(i), nominal);
        b_.SetInsertPoint(nominal);
        auto *used_name = b_.CreateCall(named_exception_site_, {category, where});
        auto *canonical = b_.CreateGlobalString(program_->exception_display_name(i));
        auto *name = b_.CreateSelect(b_.CreateIsNotNull(used_name), used_name, canonical);
        b_.CreateCall(report, {name, where});
        b_.CreateUnreachable();
      }
      b_.SetInsertPoint(intrinsic);
    }
    b_.CreateCall(runtime("cod_lang_uncaught", b_.getVoidTy(), std::vector<llvm::Type *>(5, b_.getInt64Ty())),
        {category, where, load(completion_.requested), load(completion_.available), load(completion_.cause)});
    b_.CreateUnreachable();
  }
  llvm::BasicBlock *emit_exit_diagnostics(const std::string &recovered) {
    auto *fatal = block("fatal"), *exception = block("exception"), *normal = block(recovered);
    b_.CreateCondBr(b_.CreateICmpNE(load(completion_.fatal), null_pointer()), fatal, exception);
    b_.SetInsertPoint(fatal);
    clear_failure_owner();
    b_.CreateCall(runtime("cod_lang_fail", b_.getVoidTy(), {b_.getPtrTy()}), {load(completion_.fatal)});
    b_.CreateUnreachable();
    b_.SetInsertPoint(exception);
    auto *uncaught = block("uncaught");
    b_.CreateCondBr(b_.CreateICmpNE(load(completion_.category), integer(0)), uncaught, normal);
    b_.SetInsertPoint(uncaught);
    emit_uncaught_failure();
    return normal;
  }
  llvm::Type *c_scalar_type(const Type &type) {
    return type == Type::boolean ? b_.getInt8Ty() : native_type(type);
  }
  void emit_native_entry(const CheckedNativeEntry &entry, llvm::GlobalVariable *active) {
    const auto &method = program_->methods.at(entry.method);
    const auto &form = *method.source;
    const bool returns_value = entry.result != Type::void_type;
    std::vector<llvm::Type *> types;
    for (const auto &input : entry.inputs)
      types.push_back(input.owner ? b_.getPtrTy() : c_scalar_type(input.type));
    types.insert(types.end(), form.owner_parameters.size() + returns_value, b_.getPtrTy());
    auto *bridge = llvm::Function::Create(llvm::FunctionType::get(b_.getVoidTy(), types, false),
        llvm::GlobalValue::ExternalLinkage, native_entry_body_name(form), module_);
    for (size_t i = 0; i < entry.inputs.size(); ++i)
      if (types[i]->isIntegerTy(8))
        bridge->addParamAttr(i, llvm::Attribute::ZExt);
    begin(bridge, {}, {});
    auto *nested = b_.CreateLoad(b_.getInt1Ty(), active);
    b_.CreateStore(b_.getTrue(), active);
    std::vector<llvm::Value *> arguments;
    for (size_t i = 0; i < entry.inputs.size(); ++i) {
      const auto &input = entry.inputs[i];
      auto *supplied = bridge->getArg(i);
      if (input.owner) {
        auto *view = b_.CreateInsertValue(empty_scalar(), b_.CreatePtrToInt(supplied, b_.getInt64Ty()), 0);
        arguments.push_back(b_.CreateInsertValue(view, b_.getInt32(tag(method.parameters[i])), 1));
      }
      else arguments.push_back(pack(supplied, input.type));
    }
    for (const auto &[owner, input] : input_frame(entry.method).owners) {
      const auto role = null_target - 1 - owner;
      arguments.push_back(bridge->getArg(entry.inputs.size() + role));
    }
    auto *outcome = b_.CreateCall(methods_.at(entry.method), arguments);
    auto *completed = block("entry.completed");
    b_.CreateCondBr(abrupt_outcome(outcome), finish_, completed);
    b_.SetInsertPoint(finish_);
    save_abrupt(outcome);
    if (protected_outcomes_) discard_transfer_failure(load(completion_.transfer), null_pointer());
    b_.SetInsertPoint(emit_exit_diagnostics("entry.escaped.transfer"));
    b_.CreateCall(runtime("cod_lang_fail", b_.getVoidTy(), {b_.getPtrTy()}),
        {b_.CreateGlobalString("protected completion escaped its supplying call")});
    b_.CreateUnreachable();
    b_.SetInsertPoint(completed);
    if (returns_value) {
      auto *result = unpack(b_.CreateExtractValue(outcome, 0), entry.result);
      if (entry.result == Type::boolean) result = b_.CreateZExt(result, b_.getInt8Ty());
      b_.CreateStore(result, bridge->getArg(types.size() - 1));
    }
    auto *release = block("entry.release.causes"), *returned = block("entry.return");
    b_.CreateStore(nested, active);
    b_.CreateCondBr(nested, returned, release);
    b_.SetInsertPoint(release);
    b_.CreateCall(runtime("cod_lang_exception_finish", b_.getVoidTy(), {}));
    b_.CreateBr(returned);
    b_.SetInsertPoint(returned);
    b_.CreateRetVoid();
  }
public:
  std::pair<std::string, std::string> emit(const CheckedProgram &program, bool emit_text, bool emit_bitcode, bool bounds_checks = true) {
    program_ = &program; bounds_checks_ = bounds_checks;
    const auto allocates_lexically = [](const auto &operations) {
      return std::ranges::any_of(operations, [](const auto &operation) { return operation.second.lexical_allocation.has_value(); });
    };
    owns_lexical_allocations_ = std::ranges::any_of(program.instances, [&](const auto &instance) {
      return allocates_lexically(instance.operations);
    });
    protected_outcomes_ = std::ranges::any_of(program.methods, [](const auto &method) { return method.here_group && !method.instances.empty(); });
    owned_failures_ = program.deferred_aggregates || program.exception_types.size() > exception_categories.size();
    case_scope_outcomes_ = std::ranges::any_of(program.methods, [](const auto &method) {
      return method.source->case_scan_scope && !method.instances.empty();
    });
    if (protected_outcomes_ || owned_failures_ || case_scope_outcomes_) {
      std::vector<llvm::Type *> fields(outcome_->element_begin(), outcome_->element_end());
      fields.resize(fields.size() + protected_outcomes_ + owned_failures_, b_.getPtrTy());
      if (case_scope_outcomes_) fields.push_back(b_.getInt1Ty());
      outcome_->setBody(fields);
    }
    if (protected_outcomes_) here_transfer_->setBody({b_.getInt8Ty(), outcome_});
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
      if (!program.methods[i].scope_inputs && program.methods[i].here_targets.empty() &&
          !program.methods[i].has_higher_order_here() && !program.methods[i].instances.empty())
        retain_method(i);
    for (const auto &entry : program.native_entries) retain_method(entry.method);
    std::set<size_t> used_symbols;
    std::set<size_t> here_instances;
    std::set<size_t> prepared_sources;
    const auto collect_operations = [&](const auto &operations) {
      for (const auto &[expr, operation] : operations) {
        if (operation.callee) retain_method(program.instances[*operation.callee].method);
        if (operation.scoped_effect) {
          const auto &target = program.instances[*operation.scoped_effect];
          retain_method(target.method);
          if (operation.publishes_effect)
            used_symbols.insert(target.scoped_replay_bindings.begin(), target.scoped_replay_bindings.end());
        }
        for (const auto &call : operation.nominal_calls) retain_method(program.instances[call.callee].method);
        if (operation.opened_source) {
          prepared_sources.insert(*operation.opened_source);
          retain_method(program.instances[*operation.opened_source].method);
        }
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
        if (operation.symbol && (expr->kind == Expression::Kind::symbol || (installation && operation.publishes_effect) ||
            operation.callable_call || program.symbols[*operation.symbol].live_scope))
          used_symbols.insert(*operation.symbol);
      }
    };
    for (const auto &value : program.callables) {
      for (const auto instance : value.implementations()) retain_method(program.instances[instance].method);
      const auto &symbols = program.captured_symbols(value);
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
        llvm::Type *layout = scalar_;
        if (symbol.live_scope) layout = b_.getPtrTy();
        else if (symbol.reference) layout = llvm::StructType::get(context_, {scalar_, b_.getPtrTy()});
        slot = new llvm::GlobalVariable(module_, layout, false, llvm::GlobalValue::InternalLinkage,
            llvm::Constant::getNullValue(layout), "cod.symbol." + std::to_string(i));
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
    std::map<size_t, std::set<size_t>> here_array_owners;
    for (size_t i = 0; i < program.methods.size(); ++i) {
      if (!emitted_methods[i]) continue;
      const auto &method = program.methods[i];
      const auto &frame = input_frame(i);
      call_layouts_[i] = {frame.inputs.size(), frame.owners.size(), method.captures.size(),
          method.retained_types.size(), method.declaration_context(), false, false, method.here.size(), method.here_group.has_value(),
          method.here_inputs() - method.here.size()};
      auto &call = call_layouts_[i];
      std::set<size_t> array_owners;
      for (const auto id : method.instances) {
        const auto &instance = program.instances[id];
        const auto &result = instance.result;
        if (!instance.array_allocator_roles)
          implementation_gap(method.source->at, "emission without completed array allocator requirements");
        const auto &required = *instance.array_allocator_roles;
        array_owners.insert(required.begin(), required.end());
        call.callable_destination |= !result.borrowed() && result.type->kind == Type::procedure;
        call.native_borrow_destination |= result.native_borrow();
        call.exception_result |= result.owns_exception();
      }
      if (!method.here.empty() && !method.scope_inputs) {
        auto &owners = here_array_owners[method.definition];
        owners.insert(array_owners.begin(), array_owners.end());
      } else call.array_owners.assign(array_owners.begin(), array_owners.end());
    }
    methods_.resize(program.methods.size());
    for (size_t i = 0; i < program.methods.size(); ++i) {
      if (!emitted_methods[i]) continue;
      const auto &method = program.methods[i];
      auto &layout = call_layouts_[i];
      // An observed supplier return can select either profile at the same call site.
      if (!method.here.empty() && !method.scope_inputs) {
        const auto &owners = here_array_owners.at(method.definition);
        layout.array_owners.assign(owners.begin(), owners.end());
      }
      const auto prefix = "cod.method." + std::to_string(method.definition) + "." + method.source->name +
          (i == method.definition ? std::string{} : ".interpretation." + std::to_string(i));
      if (method.instances.size() != 1 || !method.source->type_parameters.empty() || !method.captures.empty() ||
          !method.retained_types.empty() || method.declaration_context())
        throw std::logic_error("seed emission requires one concrete native frame per method");
      methods_[i] = declare(prefix + ".native", layout);
    }
    for (const auto instance : here_instances) declare_here_entry(instance);
    for (const auto instance : prepared_sources) declare_prepared_source(instance);
    for (size_t i = 0; i < program.methods.size(); ++i) {
      if (!emitted_methods[i]) continue;
      const auto &method = program.methods[i];
      body(methods_[i], method, program.instances[method.instances.front()]);
    }
    // Nested crossings share the supplying execution's exception history.
    auto *active = new llvm::GlobalVariable(module_, b_.getInt1Ty(), false, llvm::GlobalValue::InternalLinkage,
        b_.getFalse(), "cod.native.entry.active");
    active->setThreadLocalMode(llvm::GlobalVariable::GeneralDynamicTLSModel);
    for (const auto &entry : program.native_entries) emit_native_entry(entry, active);
    emit_sites();
    emit_bits_transfers();
    std::string errors;
    llvm::raw_string_ostream diagnostics_out(errors);
    if (llvm::verifyModule(module_, &diagnostics_out)) throw std::logic_error("invalid emitted CoD module: " + errors);
    std::string text, bitcode;
    if (emit_text) { llvm::raw_string_ostream output(text); module_.print(output, nullptr); }
    if (emit_bitcode) { llvm::raw_string_ostream output(bitcode); llvm::WriteBitcodeToFile(module_, output); }
    return {std::move(text), std::move(bitcode)};
  }
  NativeRowLayouts provider_row_layouts() {
    NativeRowLayouts result;
    const auto &data = module_.getDataLayout();
    const auto measure = [&](const Type &type) {
      if (type.kind != Type::array) return;
      const auto &element = program_->arrays.at(type.identity).element;
      if (element.kind != Type::product || result.contains(element.identity)) return;
      auto *row = product_layout(element.identity);
      const auto *layout = data.getStructLayout(row);
      NativeRowLayout measured{data.getTypeAllocSize(row).getFixedValue(), data.getABITypeAlign(row).value(), {}};
      for (size_t i = 0; i < row->getNumElements(); ++i) measured.offsets.push_back(layout->getElementOffset(i));
      result.emplace(element.identity, std::move(measured));
    };
    for (const auto &native : program_->native_operations) {
      if (native.has_here()) for (const auto &input : native.arrays()) measure(input.type);
      else for (const auto &input : native.inputs) measure(input.type);
    }
    return result;
  }
};

} // namespace cod::language
