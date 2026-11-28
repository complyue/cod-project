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
  enum CompletionKind : std::uint8_t { case_break = 4, case_scan = 5 };
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
  enum class ArrayBacking { inline_elements, foreign_loan };
  llvm::StructType *array_header_ =
      llvm::StructType::create(context_, {b_.getInt64Ty(), b_.getInt8Ty()}, "cod.array.header");
  llvm::StructType *borrowed_array_ =
      llvm::StructType::create(context_, {array_header_, b_.getPtrTy()}, "cod.array.borrowed");
  llvm::StructType *native_array_view_ =
      llvm::StructType::create(context_, {b_.getPtrTy(), b_.getInt64Ty()}, "cod.native.array.view");
  llvm::StructType *allocation_capability_ =
      llvm::StructType::create(context_, {b_.getInt1Ty(), b_.getPtrTy()}, "cod.owner.allocation");
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
    size_t operands = 0, owners = 0;
    bool callable_destination = false, native_borrow_destination = false;
    size_t here_formals = 0;
    bool here_group = false;
    size_t lexical_here = 0;
    std::vector<size_t> allocation_owners{};
    bool exception_result = false;
    bool aggregate_destination = false;
    size_t allocation_owners_begin() const { return operands + owners; }
    size_t result_index() const { return allocation_owners_begin() + allocation_owners.size(); }
    bool value_destination() const { return callable_destination || aggregate_destination; }
    size_t native_result_index() const { return result_index() + value_destination(); }
    size_t exception_result_index() const { return native_result_index() + native_borrow_destination; }
    bool has_result_destination() const { return value_destination() || native_borrow_destination || exception_result; }
    size_t here_index() const { return exception_result_index() + exception_result; }
    size_t here_count() const { return here_formals + lexical_here; }
    size_t parameter_count() const { return here_index() + here_count() + here_group; }
  };
  std::vector<CallLayout> call_layouts_;
  bool bounds_checks_ = true;
  std::vector<Type> callable_types_;
  std::map<size_t, llvm::StructType *> product_layouts_;
  std::map<size_t, llvm::StructType *> regional_product_layouts_;
  std::vector<llvm::Function *> methods_;
  struct HereEntry { llvm::Function *entry; llvm::StructType *activation; bool group; };
  std::map<size_t, HereEntry> here_entries_;
  std::vector<llvm::GlobalVariable *> symbols_;
  // Operation facts belong to the selected body's frozen frame.
  struct BoundOperations {
    const std::map<const Expression *, CheckedOperation> *operations;
    const CheckedOperation &at(const Expression *expression) const { return operations->at(expression); }
  };
  BoundOperations checked_{};
  std::vector<llvm::Value *> arguments_;
  std::vector<llvm::AllocaInst *> storage_;
  std::vector<StorageFormat> storage_formats_;
  // A checked place role and its native slots describe one lexical binding.
  // Each payload place has its own index, independently of bindings.
  struct EmissionBinding {
    bool value_binding;
    llvm::Value *reference_slot = nullptr;
    llvm::Value *direct_place = nullptr;
  };
  std::vector<EmissionBinding> bindings_;
  std::map<size_t, llvm::Value *> owner_identities_;
  const std::map<size_t, size_t> *region_controllers_ = nullptr;
  std::map<size_t, llvm::Value *> allocation_capabilities_;
  llvm::AllocaInst *lexical_allocations_ = nullptr;
  bool owns_lexical_allocations_ = false;
  std::map<size_t, llvm::AllocaInst *> exception_holder_slots_;
  std::vector<llvm::AllocaInst *> exception_parameter_slots_;
  ResultMode result_mode_ = ResultMode::value;
  std::vector<llvm::Value *> here_arguments_;
  llvm::Value *here_destination_ = nullptr;
  llvm::Function *function_ = nullptr;
  llvm::BasicBlock *finish_ = nullptr;
  // Live slots and suspended values share the complete pending state. Native
  // call outcomes carry a protected destination independently of local return and failure.
  template <typename T> struct CompletionState {
    T value{}, category{}, site{}, fatal{}, requested{}, available{}, cause{}, kind{}, has_return{}, transfer{};
    T failure_owner{}, result_owner{};
    auto fields() const {
      return std::array{value, category, site, fatal, requested, available, cause, kind, has_return,
                        transfer, failure_owner, result_owner};
    }
    template <typename Transform> auto transform(const Transform &apply) const {
      return CompletionState<decltype(apply(value))>{apply(value), apply(category), apply(site), apply(fatal),
          apply(requested), apply(available), apply(cause), apply(kind), apply(has_return), transfer ? apply(transfer) : nullptr,
          failure_owner ? apply(failure_owner) : nullptr, result_owner ? apply(result_owner) : nullptr};
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
  struct CleanupAction {
    const Expression *action;
    llvm::StructType *layout;
    std::optional<size_t> native_constructor = {};
    llvm::AllocaInst *activation_record = nullptr;
    std::optional<size_t> native_symbol = {};
    bool region_release = false;
    bool exception_release = false;
  };
  struct Control {
    Control *parent;
    llvm::BasicBlock *exit;
  };
  Control *control_ = nullptr;
  struct Scope {
    Scope *parent;
    llvm::BasicBlock *cleanup;
    llvm::AllocaInst *head;
    std::vector<CleanupAction> actions;
  };
  llvm::StructType *cleanup_header_ = llvm::StructType::get(context_, {b_.getPtrTy(), b_.getInt64Ty()});
  Scope *scope_ = nullptr;
  Scope *native_effect_scope_ = nullptr;
  struct EagerOperation {
    std::vector<Type> inputs;
    const CheckedOperation *operation;
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
    return bindings_.at(id).reference_slot;
  }
  llvm::Value *binding_address(size_t id, std::optional<size_t> place) {
    const auto &binding = bindings_.at(id);
    if (binding.reference_slot) return b_.CreateLoad(b_.getPtrTy(), binding.reference_slot);
    if (binding.direct_place) return binding.direct_place;
    return storage_.at(*place);
  }
  llvm::Value *convert_value(llvm::Value *value, const Type &from, const Type &to) {
    return to.kind == Type::procedure ? value : pack(convert(value, from, to), to);
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
        fields.push_back(field.stores_target() ? b_.getInt64Ty() : inline_layout(field.type));
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
  llvm::Value *allocation_capability(const CheckedOperation::OwnerAllocator &allocator) {
    using Allocation = CheckedOperation::OwnerAllocator;
    if (allocator.kind == Allocation::input) return allocation_capabilities_.at(allocator.place);
    const bool region = allocator.kind == Allocation::region;
    auto *backing = region ? load(storage_.at(allocator.place)) : lexical_allocation_owner();
    auto *capability = b_.CreateInsertValue(llvm::PoisonValue::get(allocation_capability_), b_.getInt1(region), 0);
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
  llvm::Value *allocate_lexical_extent(llvm::Value *owner, llvm::Value *bytes, std::uint64_t alignment,
                                      const Location &at) {
    auto allocate = runtime("cod_lang_lexical_extent_allocate", b_.getPtrTy(),
        {b_.getPtrTy(), b_.getInt64Ty(), b_.getInt64Ty()});
    auto *storage = b_.CreateCall(allocate, {owner, bytes, integer(alignment)});
    auto *ready = block("lexical.extent.ready"), *unavailable = block("lexical.extent.unavailable");
    b_.CreateCondBr(b_.CreateIsNotNull(storage), ready, unavailable);
    b_.SetInsertPoint(unavailable);
    fail_path(ExceptionCategory::out_of_memory, at);
    b_.SetInsertPoint(ready);
    return storage;
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
    auto *lexical_storage = allocate_lexical_extent(backing, bytes, alignment, at);
    auto *lexical_predecessor = b_.GetInsertBlock();
    b_.CreateBr(ready);
    b_.SetInsertPoint(ready);
    auto *array = b_.CreatePHI(b_.getPtrTy(), 2);
    array->addIncoming(regional_storage, regional_predecessor);
    array->addIncoming(lexical_storage, lexical_predecessor);
    return array;
  }
  llvm::Value *emit_array_construction(const Expression &expr, bool discard) {
    const auto &length = *expr.children.front();
    const auto &fill = *expr.children.back();
    auto *count_value = emit_expression(length);
    if (!reachable()) return nullptr;

    const auto &fact = checked_.at(&expr);
    const auto &allocator = fact.array_allocator.value();
    const auto &array = program_->arrays.at(fact.operand_type.identity);
    const StorageFormat length_format{checked_.at(&length).type};
    const StorageFormat fill_format{checked_.at(&fill).type};
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
    auto *capability = allocation_capability(allocator);
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
    return discard ? empty_scalar() : storage;
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
  // transport envelope in storage.
  llvm::Value *load_place(llvm::IRBuilder<> &builder, llvm::Value *address, StorageFormat format) {
    if (format.kind == Type::void_type) return builder.CreateLoad(scalar_, address);
    const auto type = format.type();
    return pack(builder, builder.CreateLoad(native_type(type), address), type);
  }
  void store_place(llvm::Value *address, llvm::Value *value, StorageFormat format) {
    b_.CreateStore(format.kind == Type::void_type ? value : unpack(value, format.type()), address);
  }
  void store_incoming(size_t place, llvm::Value *value) {
    store_place(storage_.at(place), value, storage_formats_.at(place));
  }
  llvm::Value *view_address(llvm::Value *value) { return b_.CreateIntToPtr(b_.CreateExtractValue(value, 0), b_.getPtrTy()); }
  llvm::Value *pack_view(llvm::Value *address, const Expression &expr, bool returned = false) {
    const auto &fact = checked_.at(&expr);
    const auto &type = returned ? fact.operand_type : fact.type;
    // An untyped null has no target identity.
    auto *identity = b_.getInt32(type.kind == Type::null_ref ? 0u : tag(type));
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
  llvm::Value *take_pointer(llvm::IRBuilder<> &builder, llvm::Value *place) {
    auto *owner = builder.CreateLoad(builder.getPtrTy(), place);
    builder.CreateStore(null_pointer(), place);
    return owner;
  }
  void release_failure_owner(llvm::IRBuilder<> &builder, llvm::Value *owner) {
    if (owner) builder.CreateCall(runtime("cod_lang_release_exception_region", builder.getVoidTy(), {builder.getPtrTy()}), {owner});
  }
  void release_result_owner(llvm::Value *owner) {
    if (!owner) return;
    if (result_mode_ == ResultMode::owned_region)
      b_.CreateCall(runtime("cod_lang_region_release", b_.getVoidTy(), {b_.getPtrTy()}), {owner});
    else release_failure_owner(b_, owner);
  }
  llvm::Value *exception_payload(llvm::Value *owner) {
    return b_.CreateCall(runtime("cod_lang_exception_payload", b_.getPtrTy(), {b_.getPtrTy()}), {owner});
  }
  llvm::StructType *exception_input_layout() {
    return llvm::StructType::get(context_, {b_.getPtrTy(), b_.getPtrTy()});
  }
  void clear_failure_owner() {
    if (owned_failures_) release_failure_owner(b_, take_pointer(b_, completion_.failure_owner));
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
    release_failure_owner(b_, take_pointer(b_, transfer_failure_slot(b_, transfer)));
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
    if (completion_.result_owner) {
      auto *owner = load(completion_.result_owner);
      b_.CreateStore(b_.CreateSelect(leaves, null_pointer(), owner), completion_.result_owner);
      release_result_owner(b_.CreateSelect(leaves, owner, null_pointer()));
    }
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
  void save_local_completion(llvm::Value *value, unsigned kind, bool returned) {
    b_.CreateStore(value, completion_.value);
    clear_failure_owner();
    b_.CreateStore(integer(0), completion_.category);
    b_.CreateStore(null_pointer(), completion_.fatal);
    b_.CreateStore(b_.getInt1(returned), completion_.has_return);
    b_.CreateStore(b_.getInt8(kind), completion_.kind);
  }
  void save_return(llvm::Value *value) {
    save_local_completion(value, 1, !here_destination_);
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
  llvm::Value *initialize_exception_region(const Location &at, size_t category, const Initialize &emit_fields,
                                           llvm::Value *capacity) {
    auto *layout = product_layout(*program_->exception_types.at(category).schema);
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
    auto *completed = reachable() ? block("exception.initialized") : nullptr;
    if (completed) b_.CreateBr(completed);
    b_.SetInsertPoint(construction.exit);
    release_failure_owner(b_, owner);
    b_.CreateBr(control_->exit);
    if (!completed) return nullptr;
    b_.SetInsertPoint(completed);
    return owner;
  }
  template <typename Initialize>
  void raise_constructed_exception(const Location &at, size_t category, const Initialize &emit_fields) {
    auto *owner = initialize_exception_region(at, category, emit_fields, integer(COD_LANG_EXCEPTION_REGION_DEFAULT));
    if (!reachable()) return;
    save_abrupt(outcome(empty_scalar(), integer(category), integer(site(at)), null_pointer(),
                        nullptr, nullptr, nullptr, nullptr, owner));
    b_.CreateBr(control_->exit);
  }
  struct OwnedException {
    llvm::Value *value{}, *owner{};
  };
  OwnedException construct_braced_exception(const Expression &expr, size_t category) {
    llvm::Value *capacity = integer(COD_LANG_EXCEPTION_REGION_DEFAULT);
    if (expr.exception_placement == Expression::ExceptionPlacement::capacity) {
      const auto &input = *expr.children.back();
      auto *prepared = emit_expression(input);
      if (!reachable()) return {};
      capacity = convert(prepared, checked_.at(&input).type, Type::uint64);
      constexpr auto limit = std::uint64_t{std::numeric_limits<std::ptrdiff_t>::max()};
      auto *valid = block("exception.capacity.valid"), *invalid = block("exception.capacity.invalid");
      b_.CreateCondBr(b_.CreateICmpULE(capacity, integer(limit)), valid, invalid);
      b_.SetInsertPoint(invalid);
      fail_path(ExceptionCategory::invalid_capacity, expr.at);
      b_.SetInsertPoint(valid);
    }
    auto *owner = initialize_exception_region(expr.at, category, [&](llvm::StructType *layout, llvm::Value *payload) {
      for (const auto &initializer : expr.initializer_items()) {
        const auto &input = *initializer->children.front();
        auto *value = emit_expression(input);
        if (!reachable()) break;
        const auto index = *checked_.at(initializer.get()).product_field;
        assign_place(b_.CreateStructGEP(layout, payload, index), convert_result(*initializer, input, value),
                     *initializer, &CheckedOperation::operand_type);
      }
    }, capacity);
    if (!reachable()) return {};
    const auto origin = site(expr.at, NamedExceptionSite{category, expr.annotation->name});
    auto *value = b_.CreateInsertValue(llvm::Constant::getNullValue(exception_), integer(category), 0);
    return {b_.CreateInsertValue(value, integer(origin), 1), owner};
  }
  void require(llvm::Value *condition, const Location &at, const std::string &message) {
    auto *good = block("numeric.valid"), *bad = block("numeric.failure");
    b_.CreateCondBr(condition, good, bad);
    b_.SetInsertPoint(bad);
    fail_path(ExceptionCategory::system, at, b_.CreateGlobalString(location_text(at) + ": " + message));
    b_.SetInsertPoint(good);
  }
  using PlaceType = Type CheckedOperation::*;
  StorageFormat place_format(const Expression &expr, PlaceType type) const {
    const auto &payload = checked_.at(&expr).*type;
    if (payload == Type::never) throw std::logic_error("no checked payload type for reachable storage access");
    return StorageFormat{payload};
  }
  llvm::Value *load_place(llvm::Value *address, const Expression &expr, PlaceType type = &CheckedOperation::type) {
    return load_place(b_, address, place_format(expr, type));
  }
  void assign_place(llvm::Value *address, llvm::Value *value, const Expression &expr,
                    PlaceType type = &CheckedOperation::type) {
    store_place(address, value, place_format(expr, type));
  }
  llvm::Value *convert_result(const Expression &expr, const Expression &input, llvm::Value *value,
      Type CheckedOperation::*destination = &CheckedOperation::operand_type) {
    const auto &context = checked_;
    const auto &from = context.at(&input).type;
    const auto &to = context.at(&expr).*destination;
    if (from == to || from == Type::never || to == Type::never) return value;
    if (from == Type::void_type) return empty_scalar();
    return convert_value(value, from, to);
  }
  llvm::Value *binary(const Expression &expr, llvm::Value *left, llvm::Value *right, const EagerOperation &choice) {
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
          require(b_.CreateNot(overflow), expr.at, "Int64 division overflow"); result = b_.CreateSDiv(left, right);
        } else result = b_.CreateSRem(left, b_.CreateSelect(overflow, llvm::ConstantInt::get(native, 1), right));
      } else result = op == "/" ? b_.CreateUDiv(left, right) : b_.CreateURem(left, right);
    }
    if (!result) throw std::logic_error("unchecked scalar operator");
    return pack(result, choice.operation->type);
  }
  const CheckedFrame &input_frame(size_t method) const {
    return program_->instances.at(program_->methods.at(method).instance.value()).frame;
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
    if (region_controllers_) {
      const auto controller = region_controllers_->find(owner);
      if (controller != region_controllers_->end()) return load(storage_.at(controller->second));
    }
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
  llvm::Value *selected_owner(const Expression &expr) {
    const auto &fact = checked_.at(&expr);
    if (fact.owners.size() != 1) throw std::logic_error("unchecked reference owner reached native transport");
    return owner_identity(*fact.owners.begin(), expr.at);
  }
  llvm::Value *prepare_here(const CheckedOperation::SuppliedHere &body, const Location &at) {
    const auto &entry = here_entries_.at(body.instance);
    auto *value = b_.CreateInsertValue(llvm::UndefValue::get(here_), entry.entry, 0);
    if (!entry.group && body.places.empty() && body.owners.empty() && body.lexical_here.empty())
      return b_.CreateInsertValue(value, null_pointer(), 1);
    auto *activation = slot(entry.activation, "here.activation");
    const auto &method = program_->methods[program_->instances[body.instance].method];
    if (entry.group && owned_failures_) b_.CreateStore(llvm::Constant::getNullValue(here_transfer_), activation);
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
    auto *converted = convert(value, checked_.at(&operand).type, type);
    return type == Type::boolean ? b_.CreateZExt(converted, c_scalar_type(type)) : converted;
  }
  llvm::Value *emit_native_call(const Expression &expr) {
    {
      const size_t id = *checked_.at(&expr).native_call;
      const auto &native = program_->native_operations.at(id);
      std::vector<llvm::Value *> arguments;
      std::vector<llvm::Type *> types;
      for (size_t i = 0; i < native.inputs.size(); ++i) {
        const auto &input = native.inputs[i];
        const auto &operand = *expr.children[i];
        llvm::Value *argument;
        if (input.owner) {
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
    }
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
    const auto &fact = checked_.at(&expr);
    {
      const auto &body = std::get<CheckedOperation::SuppliedHere>(fact.here_arguments.at(0));
      const auto &operand = *expr.children.front();
      auto *borrow = emit_expression(operand, Projection::target);
      if (!reachable()) return nullptr;
      const auto &native = program_->native_operations.at(*fact.native_call);
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
        completed = consume_here_transfer(result, std::array{b_.CreateExtractValue(callback, 1)});
      return propagate(completed);
    }
  }
  llvm::Value *native_call(const Expression &expr, llvm::Function *target, size_t method, std::vector<llvm::Value *> prepared,
                           std::vector<llvm::Value *> *here_sites = nullptr,
                           std::span<llvm::Value *const> current_owners = {}, llvm::Value *exception_result = nullptr) {
    const auto &layout = call_layouts_.at(method);
    if (prepared.size() != layout.operands) throw std::logic_error("unchecked native call arity");
    llvm::AllocaInst *destination = nullptr;
    const auto &invocation = checked_.at(&expr);
    const auto &instance = program_->instances.at(invocation.callee.value());
    if (instance.method != method) throw std::logic_error("native call disagrees with its selected method");
    const auto &definition = program_->methods[method];
    for (const auto &[owner, input] : instance.frame.owners) {
      if (definition.scope_inputs) {
        prepared.push_back(owner_identity(invocation.scope_owners.at(null_target - 1 - owner), expr.at));
      } else {
        const auto operand = input.value();
        prepared.push_back(!current_owners.empty() && current_owners[operand]
            ? current_owners[operand] : selected_owner(invocation.argument(expr, operand)));
      }
    }
    for (const auto role : layout.allocation_owners) {
      const auto &arguments = invocation.allocator_arguments;
      if (arguments) {
        if (const auto selected = arguments->find(role); selected != arguments->end()) {
          prepared.push_back(allocation_capability(selected->second));
          continue;
        }
      }
      if (instance.allocator_roles->contains(role))
        implementation_gap(expr.at, "call without a prepared array allocator capability");
      prepared.push_back(llvm::Constant::getNullValue(allocation_capability_));
    }
    if (layout.value_destination()) {
      if (layout.aggregate_destination) {
        const auto &type = *instance.result.type;
        destination = slot(inline_layout(type), "aggregate.result");
      } else destination = slot(scalar_, "callable.result");
      prepared.push_back(destination);
    }
    if (layout.native_borrow_destination) prepared.push_back(slot(native_handle_pair(), "native.call.result"));
    if (layout.exception_result) {
      if (!exception_result) throw std::logic_error("owning call without its checked exception result destination");
      prepared.push_back(exception_result);
    }
    if (invocation.here_arguments.size() != layout.here_formals)
      throw std::logic_error("call disagrees with its here parameter roles");
    for (const auto &input : invocation.here_arguments) {
      if (const auto *literal = std::get_if<CheckedOperation::SuppliedHere>(&input)) {
        auto *argument = prepare_here(*literal, expr.at);
        if (here_sites && here_entries_.at(literal->instance).group)
          here_sites->push_back(b_.CreateExtractValue(argument, 1));
        prepared.push_back(argument);
      } else {
        prepared.push_back(here_arguments_.at(std::get<CheckedOperation::ForwardedHere>(input).input));
      }
    }
    for (size_t slot = 0; slot < layout.lexical_here; ++slot)
      prepared.push_back(here_arguments_.at(invocation.scope_here.at(slot)));
    if (!target || prepared.size() != layout.parameter_count() || target->arg_size() != layout.parameter_count())
      throw std::logic_error("native call disagrees with its parameter roles");
    llvm::Value *result = b_.CreateCall(target, prepared);
    if (destination) {
      auto *value = layout.aggregate_destination ? pack_address(destination) : load(destination);
      result = b_.CreateInsertValue(result, value, 0);
    }
    return result;
  }
  llvm::Value *eager_builtin(const Expression &expr, const std::vector<llvm::Value *> &arguments, const EagerOperation &choice) {
    if (choice.operation->builtin == BuiltinOperation::binary) return binary(expr, arguments[0], arguments[1], choice);
    const auto type = choice.operation->operand_type;
    auto *argument = unpack(arguments.front(), type);
    if (choice.operation->builtin_name == "!") return pack(b_.CreateNot(argument), type);
    if (choice.operation->builtin_name == "+") return arguments.front();
    if (floating(type)) return pack(b_.CreateFNeg(argument), type);
    auto *zero = pack(llvm::ConstantInt::get(llvm::cast<llvm::IntegerType>(native_type(type)), 0), type);
    return binary(expr, zero, arguments.front(), EagerOperation{{type, type}, choice.operation});
  }
  llvm::Value *swap_builtin(const std::vector<llvm::Value *> &arguments, BuiltinOperation operation, Type bits = {}) {
    auto *left = view_address(arguments[0]), *right = view_address(arguments[1]);
    auto *type = operation == BuiltinOperation::reference_swap ? static_cast<llvm::Type *>(b_.getPtrTy()) :
        scalar_type(bits) ? native_type(bits) : scalar_;
    auto *left_value = b_.CreateLoad(type, left), *right_value = b_.CreateLoad(type, right);
    b_.CreateStore(right_value, left); b_.CreateStore(left_value, right);
    return outcome(empty_scalar(), integer(0), integer(0), null_pointer());
  }
  llvm::Value *invoke(const Expression &expr, const std::vector<llvm::Value *> &arguments,
                      std::vector<llvm::Value *> *here_sites = nullptr, std::span<llvm::Value *const> current_owners = {}) {
    const auto &fact = checked_.at(&expr);
    if (fact.invokes_here) {
      auto *argument = here_arguments_.at(*fact.invokes_here);
      auto prepared = arguments;
      std::vector<llvm::Type *> parameters(arguments.size(), scalar_);
      for (const auto input : here_owner_inputs(fact.operand_type)) {
        prepared.push_back(selected_owner(fact.argument(expr, input)));
        parameters.push_back(b_.getPtrTy());
      }
      prepared.push_back(b_.CreateExtractValue(argument, 1));
      auto *entry = b_.CreateExtractValue(argument, 0);
      parameters.push_back(b_.getPtrTy());
      return b_.CreateCall(llvm::FunctionType::get(outcome_, parameters, false), entry, prepared);
    }
    if (fact.builtin == BuiltinOperation::reference_swap) return swap_builtin(arguments, BuiltinOperation::reference_swap);
    if (fact.builtin == BuiltinOperation::identity) {
      auto *left_owner = selected_owner(*expr.children[0]);
      auto *right_owner = selected_owner(*expr.children[1]);
      auto *left = view_address(arguments[0]);
      auto *right = view_address(arguments[1]);
      auto *equal = b_.CreateAnd(b_.CreateICmpEQ(left_owner, right_owner),
          b_.CreateICmpEQ(left, right));
      const auto &a = checked_.at(expr.children[0].get()).type;
      const auto &b = checked_.at(expr.children[1].get()).type;
      if (a != b && (a.kind == Type::product || b.kind == Type::product))
        equal = b_.CreateAnd(equal, b_.CreateIsNull(left));
      if (expr.text == "is not") equal = b_.CreateNot(equal);
      return outcome(pack(equal, Type::boolean), integer(0), integer(0), null_pointer());
    }
    if (fact.callee) {
      const auto method = program_->instances[*fact.callee].method;
      return native_call(expr, methods_[method], method, arguments, here_sites, current_owners);
    }
    if (fact.builtin == BuiltinOperation::unary || fact.builtin == BuiltinOperation::binary) {
      EagerOperation operation{{}, &fact};
      for (const auto &argument : fact.arguments) operation.inputs.push_back(argument.type);
      return outcome(eager_builtin(expr, arguments, operation), integer(0), integer(0), null_pointer());
    }
    if (fact.builtin == BuiltinOperation::bits_swap || fact.builtin == BuiltinOperation::reference_swap)
      return swap_builtin(arguments, fact.builtin, fact.arguments.front().type);
    if (fact.builtin != BuiltinOperation::print) throw std::logic_error("unchecked call reached native invocation");
    const auto &type = fact.arguments.front().type;
    auto *argument = unpack(arguments.front(), type);
    std::string suffix;
    if (type == Type::boolean) { suffix = "bool"; argument = b_.CreateZExt(argument, b_.getInt32Ty()); }
    else if (type == Type::integer) suffix = "i64";
    else if (type == Type::float32) suffix = "f32";
    else if (type == Type::float64) suffix = "f64";
    else { suffix = "u64"; argument = b_.CreateZExtOrTrunc(argument, b_.getInt64Ty()); }
    b_.CreateCall(runtime("cod_lang_print_" + suffix, b_.getVoidTy(), {argument->getType()}), {argument});
    return outcome(empty_scalar(), integer(0), integer(0), null_pointer());
  }
  llvm::Value *consume_here_transfer(llvm::Value *result, std::span<llvm::Value *const> here_sites) {
    auto *target = b_.CreateExtractValue(result, 7);
    llvm::Value *matches = b_.getFalse();
    for (auto *here_site : here_sites)
      matches = b_.CreateOr(matches, b_.CreateICmpEQ(target, here_site));
    auto *matched = block("here.leaves"), *ordinary = block("here.call.outcome"), *join = block("here.outcome");
    b_.CreateCondBr(b_.CreateAnd(b_.CreateIsNotNull(target), matches), matched, ordinary);
    b_.SetInsertPoint(ordinary);
    b_.CreateBr(join);
    b_.SetInsertPoint(matched);
    auto *cancel = block("here.cleanup.failed"), *deliver = block("here.deliver");
    b_.CreateCondBr(failed(result), cancel, deliver);
    b_.SetInsertPoint(cancel);
    if (owned_failures_) release_failure_owner(b_, take_pointer(b_, transfer_failure_slot(b_, target)));
    auto *cleanup_failure = b_.CreateInsertValue(result, null_pointer(), 7);
    b_.CreateBr(join);
    b_.SetInsertPoint(deliver);
    auto *kind = b_.CreateLoad(b_.getInt8Ty(), b_.CreateStructGEP(here_transfer_, target, 0));
    auto *payload = b_.CreateLoad(outcome_, here_payload_slot(b_, target));
    if (owned_failures_) b_.CreateStore(null_pointer(), transfer_failure_slot(b_, target));
    auto *returning = block("here.supplier.return");
    b_.CreateCondBr(b_.CreateICmpEQ(kind, b_.getInt8(static_cast<unsigned>(HereExit::return_supplier))), returning, join);
    b_.SetInsertPoint(returning);
    save_return(b_.CreateExtractValue(payload, 0));
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
    for (size_t i = 0; i < expr.children.size(); ++i)
      collect_cleanup(*expr.children[i], scope, only_native_effects,
                      nested_iteration || (expr.kind == K::range_for && i >= 2));
    if (expr.kind == K::effect && !nested_iteration) {
      const auto effect = checked_.operations->find(&expr);
      if (effect != checked_.operations->end() && effect->second.native_acquisition) {
        const auto &op = effect->second;
        auto *layout = llvm::StructType::get(context_, {cleanup_header_, native_handle_pair(), scalar_});
        scope.actions.push_back({&expr, layout, op.native_acquisition->constructor,
                                 slot(layout, "native.effect.record"), op.symbol});
      }
    }
    if (only_native_effects) return;
    if (expr.kind == K::deferred && checked_.operations->contains(&expr)) {
      scope.actions.push_back({&expr, cleanup_header_, {}, slot(cleanup_header_, "defer.block.record")});
      return;
    }
    if (expr.kind == K::region_binding) {
      auto *layout = llvm::StructType::get(context_, {cleanup_header_, b_.getPtrTy()});
      scope.actions.push_back({&expr, layout, {}, slot(layout, "region.cleanup"), {}, true});
      return;
    }
    if (expr.kind == K::exception_binding) {
      auto *layout = llvm::StructType::get(context_, {cleanup_header_, b_.getPtrTy()});
      scope.actions.push_back({&expr, layout, {}, slot(layout, "exception.cleanup"), {}, false, true});
      return;
    }
    if (expr.kind == Expression::Kind::native_handle_binding && expr.binding == "<-") {
      const auto operation = checked_.operations->find(&expr);
      if (operation != checked_.operations->end() && operation->second.native_acquisition)
        scope.actions.push_back({&expr, llvm::StructType::get(context_, {cleanup_header_, b_.getInt64Ty()}),
                                 operation->second.native_acquisition->constructor});
    }
  }
  void prepare_cleanup_record(Scope &scope, llvm::Value *record, size_t action) {
    b_.CreateStore(load(scope.head), b_.CreateStructGEP(cleanup_header_, record, 0));
    b_.CreateStore(integer(action), b_.CreateStructGEP(cleanup_header_, record, 1));
  }
  struct DeferredOutcomes {
    llvm::AllocaInst *failures, *count, *fatal;
  };
  llvm::Value *current_failure() {
    return outcome(empty_scalar(), load(completion_.category), load(completion_.site), load(completion_.fatal),
                   load(completion_.requested), load(completion_.available), load(completion_.cause), nullptr,
                   load(completion_.failure_owner));
  }
  void record_deferred_failure(const DeferredOutcomes &queue) {
    auto *failure = current_failure();
    b_.CreateStore(null_pointer(), completion_.failure_owner);
    auto *append = block("defer.failure.append"), *terminal = block("defer.failure.fatal");
    auto *done = block("defer.failure.saved");
    b_.CreateCondBr(b_.CreateIsNull(b_.CreateExtractValue(failure, 3)), append, terminal);
    b_.SetInsertPoint(append);
    auto *count = load(queue.count);
    b_.CreateStore(failure, b_.CreateInBoundsGEP(outcome_, queue.failures, count));
    b_.CreateStore(b_.CreateAdd(count, integer(1)), queue.count);
    b_.CreateBr(done);
    b_.SetInsertPoint(terminal);
    release_failure_owner(b_, b_.CreateExtractValue(load(queue.fatal), failure_owner_field()));
    b_.CreateStore(failure, queue.fatal);
    b_.CreateBr(done);
    b_.SetInsertPoint(done);
  }
  void complete_cleanup(const std::function<void()> &emit, const DeferredOutcomes *deferred = nullptr) {
    // Suspend the pending completion while cleanup executes its own control flow.
    auto pending = completion_.transform([&](auto *place) { return load(place); });
    b_.CreateStore(b_.getInt8(0), completion_.kind);
    b_.CreateStore(integer(0), completion_.category);
    b_.CreateStore(null_pointer(), completion_.fatal);
    if (owned_failures_) b_.CreateStore(null_pointer(), completion_.failure_owner);
    if (completion_.result_owner) b_.CreateStore(null_pointer(), completion_.result_owner);
    b_.CreateStore(b_.getFalse(), completion_.has_return);
    if (protected_outcomes_) b_.CreateStore(null_pointer(), completion_.transfer);
    Control action{control_, block("cleanup.completed")};
    control_ = &action;
    emit();
    if (reachable()) b_.CreateBr(action.exit);
    b_.SetInsertPoint(action.exit);
    control_ = action.parent;
    auto *restore = block("cleanup.preserve"), *replace = block("cleanup.replace"), *done = block("cleanup.done");
    auto *kind = load(completion_.kind);
    llvm::Value *resume = b_.CreateICmpEQ(kind, b_.getInt8(0));
    if (deferred) {
      auto *record = block("defer.failure"), *nonfailure = block("defer.nonfailure");
      b_.CreateCondBr(b_.CreateICmpEQ(kind, b_.getInt8(2)), record, nonfailure);
      b_.SetInsertPoint(record);
      record_deferred_failure(*deferred);
      if (protected_outcomes_) discard_transfer_failure(load(completion_.transfer), pending.transfer);
      b_.CreateBr(restore);
      b_.SetInsertPoint(nonfailure);
      // A return in the receiver's cleanup cannot consume its supplier's
      // pending transfer. A new transfer still has its own destination.
      llvm::Value *must_resume = b_.CreateIsNotNull(pending.fatal);
      if (protected_outcomes_) must_resume = b_.CreateOr(must_resume, b_.CreateIsNotNull(pending.transfer));
      resume = b_.CreateOr(resume, b_.CreateAnd(b_.CreateICmpEQ(kind, b_.getInt8(1)), must_resume));
    }
    b_.CreateCondBr(resume, restore, replace);
    b_.SetInsertPoint(restore);
    // A receiver-local cleanup return can itself be abandoned when the
    // suspended supplier transfer resumes.
    if (completion_.result_owner) release_result_owner(load(completion_.result_owner));
    const auto places = completion_.fields();
    const auto values = pending.fields();
    for (size_t i = 0; i < places.size(); ++i) if (places[i]) b_.CreateStore(values[i], places[i]);
    b_.CreateBr(done);
    b_.SetInsertPoint(replace);
    release_failure_owner(b_, pending.failure_owner);
    release_result_owner(pending.result_owner);
    b_.CreateBr(done);
    b_.SetInsertPoint(done);
    auto *transfer = pending.transfer;
    if (here_destination_) {
      // A failure in the supplying group's own finally cancels its pending
      // outward transfer. Receiver cleanup keeps the destination for delivery.
      auto *replaced = b_.CreateAnd(b_.CreateICmpEQ(pending.kind, b_.getInt8(3)),
                                   b_.CreateICmpEQ(load(completion_.kind), b_.getInt8(2)));
      auto *local = b_.CreateICmpEQ(transfer, here_destination_);
      transfer = b_.CreateSelect(b_.CreateAnd(replaced, local), null_pointer(), transfer);
      discard_transfer_failure(pending.transfer, transfer);
    }
    preserve_transfer(transfer);
  }
  void retain_deferred_failure(llvm::Value *aggregate, bool member, llvm::Value *failure, llvm::Value *owner) {
    std::vector<llvm::Type *> types{b_.getPtrTy()};
    types.insert(types.end(), 6, b_.getInt64Ty());
    types.push_back(b_.getPtrTy());
    b_.CreateCall(runtime("cod_lang_retain_deferred_failure", b_.getVoidTy(), types),
                  {aggregate, integer(member), b_.CreateExtractValue(failure, 1), b_.CreateExtractValue(failure, 2),
                   b_.CreateExtractValue(failure, 4), b_.CreateExtractValue(failure, 5),
                   b_.CreateExtractValue(failure, 6), owner});
  }
  void clear_completion_transfer() {
    if (!protected_outcomes_) return;
    discard_transfer_failure(take_pointer(b_, completion_.transfer), null_pointer());
  }
  struct DeferredBodyFailure {
    llvm::Value *value, *owner;
  };
  DeferredBodyFailure deferred_body_failure() {
    auto *body = current_failure();
    auto *body_owner = completion_.failure_owner;
    if (!protected_outcomes_) return {body, body_owner};
    // An incoming group failure belongs to the body, even when the
    // receiver's own exception slot is empty.
    auto *entry = b_.GetInsertBlock();
    auto *transfer = load(completion_.transfer);
    auto *inspect = block("defer.inspect.supplier"), *join = block("defer.body.failure");
    b_.CreateCondBr(b_.CreateAnd(b_.CreateNot(failed(body)), b_.CreateIsNotNull(transfer)), inspect, join);
    b_.SetInsertPoint(inspect);
    auto *kind = b_.CreateLoad(b_.getInt8Ty(), b_.CreateStructGEP(here_transfer_, transfer, 0));
    auto *incoming = b_.CreateICmpEQ(kind, b_.getInt8(static_cast<unsigned>(HereExit::failure)));
    auto *value = b_.CreateLoad(outcome_, here_payload_slot(b_, transfer));
    auto *from_transfer = b_.CreateSelect(incoming, value, body);
    auto *owner = b_.CreateSelect(incoming, transfer_failure_slot(b_, transfer), body_owner);
    b_.CreateBr(join);
    b_.SetInsertPoint(join);
    auto *selected = b_.CreatePHI(outcome_, 2);
    selected->addIncoming(body, entry);
    selected->addIncoming(from_transfer, inspect);
    auto *selected_owner = b_.CreatePHI(b_.getPtrTy(), 2);
    selected_owner->addIncoming(body_owner, entry);
    selected_owner->addIncoming(owner, inspect);
    return {selected, selected_owner};
  }
  void finish_deferred_failures(const DeferredOutcomes &queue, const Location &at) {
    auto *count = load(queue.count);
    auto *fatal = load(queue.fatal);
    auto *has_fatal = b_.CreateIsNotNull(b_.CreateExtractValue(fatal, 3));
    auto *decide = block("defer.finish"), *done = block("defer.completed");
    b_.CreateCondBr(b_.CreateOr(has_fatal, b_.CreateICmpNE(count, integer(0))), decide, done);
    b_.SetInsertPoint(decide);
    auto *aggregate = slot(b_.getPtrTy(), "defer.aggregate");
    b_.CreateStore(null_pointer(), aggregate);
    auto *terminal = block("defer.deliver.fatal"), *test_body = block("defer.test.body");
    auto *retire = block("defer.retire.failures");
    b_.CreateCondBr(has_fatal, terminal, test_body);
    b_.SetInsertPoint(terminal);
    clear_completion_transfer();
    save_abrupt(fatal);
    b_.CreateBr(retire);

    b_.SetInsertPoint(test_body);
    const auto [body, body_owner] = deferred_body_failure();
    auto *publish = block("defer.publish");
    b_.CreateCondBr(b_.CreateIsNotNull(b_.CreateExtractValue(body, 3)), retire, publish);
    b_.SetInsertPoint(publish);
    auto *status = slot(b_.getInt64Ty(), "defer.aggregate.status");
    auto acquire = runtime("cod_lang_acquire_deferred_aggregate", b_.getPtrTy(), {b_.getInt64Ty(), b_.getPtrTy()});
    auto *owner = b_.CreateCall(acquire, {count, status});
    auto *ready = block("defer.aggregate.ready"), *unavailable = block("defer.aggregate.unavailable");
    auto *where = integer(site(at));
    b_.CreateCondBr(b_.CreateIsNotNull(owner), ready, unavailable);
    b_.SetInsertPoint(unavailable);
    clear_completion_transfer();
    save_abrupt(outcome(empty_scalar(), load(status), where, null_pointer()));
    b_.CreateBr(retire);
    b_.SetInsertPoint(ready);
    b_.CreateStore(owner, aggregate);
    auto *attach = block("defer.attach.body"), *deliver = block("defer.deliver.aggregate");
    b_.CreateCondBr(failed(body), attach, deliver);
    b_.SetInsertPoint(attach);
    retain_deferred_failure(owner, false, body, take_pointer(b_, body_owner));
    b_.CreateBr(deliver);
    b_.SetInsertPoint(deliver);
    clear_completion_transfer();
    save_abrupt(outcome(empty_scalar(), integer(deferred_aggregate_id), where, null_pointer(),
                        nullptr, nullptr, nullptr, nullptr, owner));
    b_.CreateBr(retire);

    b_.SetInsertPoint(retire);
    auto *check = block("defer.member.next"), *test = block("defer.member.test");
    auto *keep = block("defer.member.attach"), *release = block("defer.member.release");
    auto *step = block("defer.member.complete");
    auto *retained = load(aggregate);
    b_.CreateBr(check);
    b_.SetInsertPoint(check);
    auto *index = b_.CreatePHI(b_.getInt64Ty(), 2);
    index->addIncoming(integer(0), retire);
    b_.CreateCondBr(b_.CreateICmpEQ(index, count), done, test);
    b_.SetInsertPoint(test);
    auto *failure = b_.CreateLoad(outcome_, b_.CreateInBoundsGEP(outcome_, queue.failures, index));
    auto *child = b_.CreateExtractValue(failure, failure_owner_field());
    b_.CreateCondBr(b_.CreateIsNotNull(retained), keep, release);
    b_.SetInsertPoint(keep);
    retain_deferred_failure(retained, true, failure, child);
    b_.CreateBr(step);
    b_.SetInsertPoint(release);
    release_failure_owner(b_, child);
    b_.CreateBr(step);
    b_.SetInsertPoint(step);
    index->addIncoming(b_.CreateAdd(index, integer(1)), step);
    b_.CreateBr(check);
    b_.SetInsertPoint(done);
  }
  void drain(Scope &scope, const Location &at, size_t deferred_blocks = 0) {
    b_.SetInsertPoint(scope.cleanup);
    if (scope.actions.empty()) return;
    std::optional<DeferredOutcomes> failures;
    auto *next = scope.cleanup;
    if (deferred_blocks) {
      // A non-repeated registration contributes at most one escaping failure.
      // Reserve the slots up front so appending after OOM cannot allocate.
      failures.emplace(slot(llvm::ArrayType::get(outcome_, deferred_blocks), "defer.failures"),
                       slot(b_.getInt64Ty(), "defer.failure.count"), slot(outcome_, "defer.fatal"));
      b_.CreateStore(integer(0), failures->count);
      b_.CreateStore(llvm::Constant::getNullValue(outcome_), failures->fatal);
      next = block("defer.next");
      b_.CreateBr(next);
      b_.SetInsertPoint(next);
    }
    auto *pop = block("defer.pop"), *done = block("defer.done"), *invalid = block("defer.invalid");
    auto *record = load(scope.head);
    b_.CreateCondBr(b_.CreateICmpEQ(record, null_pointer()), done, pop);
    b_.SetInsertPoint(pop);
    auto *previous = b_.CreateLoad(b_.getPtrTy(), b_.CreateStructGEP(cleanup_header_, record, 0));
    auto *action = b_.CreateLoad(b_.getInt64Ty(), b_.CreateStructGEP(cleanup_header_, record, 1));
    b_.CreateStore(previous, scope.head);
    auto *dispatch = b_.CreateSwitch(action, invalid, scope.actions.size());
    for (size_t i = 0; i < scope.actions.size(); ++i) {
      const auto &descriptor = scope.actions[i];
      if (descriptor.action->kind == Expression::Kind::deferred) {
        auto *run = block("defer.block");
        dispatch->addCase(integer(i), run);
        b_.SetInsertPoint(run);
        complete_cleanup([&] { emit_expression(*descriptor.action->children.front(), Projection::value, true); },
                         &failures.value());
        b_.CreateBr(next);
        continue;
      }
      if (descriptor.region_release) {
        auto *destroy = block("region.release");
        dispatch->addCase(integer(i), destroy);
        b_.SetInsertPoint(destroy);
        auto *owner = b_.CreateLoad(b_.getPtrTy(), b_.CreateStructGEP(descriptor.layout, record, 1));
        b_.CreateCall(runtime("cod_lang_region_release", b_.getVoidTy(), {b_.getPtrTy()}), {owner});
        b_.CreateBr(next);
        continue;
      }
      if (descriptor.exception_release) {
        auto *destroy = block("exception.release");
        dispatch->addCase(integer(i), destroy);
        b_.SetInsertPoint(destroy);
        auto *holder = b_.CreateLoad(b_.getPtrTy(), b_.CreateStructGEP(descriptor.layout, record, 1));
        release_failure_owner(b_, take_pointer(b_, holder));
        b_.CreateBr(next);
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
        b_.CreateBr(next);
        continue;
      }
      throw std::logic_error("missing compiled cleanup operation");
    }
    b_.SetInsertPoint(invalid);
    save_abrupt(outcome(empty_scalar(), integer(exception_id(ExceptionCategory::unsound_runtime_typing)), integer(0), null_pointer()));
    b_.CreateBr(next);
    b_.SetInsertPoint(done);
    if (failures) finish_deferred_failures(*failures, at);
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
      const auto effect = checked_.operations->find(&expr);
      if (effect != checked_.operations->end() && effect->second.publishes_effect && !effect->second.native_acquisition)
        symbols.insert(*effect->second.symbol);
    }
    const size_t children = expr.kind == K::range_for ? 2 : expr.children.size();
    for (size_t i = 0; i < children; ++i) collect_effects(*expr.children[i], symbols);
  }
  struct SavedEffect {
    size_t symbol;
    llvm::Value *value;
  };
  using SavedEffects = std::vector<SavedEffect>;
  SavedEffects save_effects(const std::vector<const Expression *> &expressions) {
    std::set<size_t> symbols;
    for (const auto *expr : expressions) collect_effects(*expr, symbols);
    SavedEffects saved;
    for (const auto id : symbols) {
      auto *binding = symbols_.at(id);
      saved.push_back({id, b_.CreateLoad(binding->getValueType(), binding, "effect.previous")});
    }
    return saved;
  }
  void restore_effects(const SavedEffects &saved) {
    for (const auto &effect : saved) b_.CreateStore(effect.value, symbols_.at(effect.symbol));
  }
  llvm::Value *finish_effect(const CheckedOperation &effect, llvm::Value *value) {
    if (effect.publishes_effect) b_.CreateStore(value, symbols_.at(*effect.symbol));
    return empty_scalar();
  }
  llvm::Value *iteration(const Expression &expr, bool discard) {
    Scope native{nullptr, nullptr, nullptr, {}};
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
    if ((checked_.at(&expr).type == Type::never)) {
      b_.CreateUnreachable(); return nullptr;
    }
    return load(result);
  }
  void call_scope(const std::vector<const Expression *> &expressions, const Location &at,
                  size_t deferred_blocks,
                  llvm::Value *requires_return = nullptr, const Type *feedback = nullptr,
                  llvm::Value *returns_void_at_end = nullptr) {
    auto *outer_native = native_effect_scope_;
    Scope scope{scope_, block("scope.cleanup"), slot(b_.getPtrTy(), "defer.head"), {}};
    Control control{control_, scope.cleanup};
    control_ = &control;
    scope_ = &scope;
    native_effect_scope_ = &scope;
    b_.CreateStore(null_pointer(), scope.head);
    for (const auto *expr : expressions) collect_cleanup(*expr, scope);
    const auto effects = save_effects(expressions);
    const bool supplies_value = feedback && *feedback != Type::void_type;
    auto *value = sequence(expressions, Projection::value, !supplies_value);
    if (supplies_value && reachable()) {
      const auto &body = *expressions.back();
      value = convert_value(value, checked_.at(&body).type, *feedback);
      b_.CreateStore(value, completion_.value);
    }
    if (reachable()) {
      if (const auto *known = llvm::dyn_cast_or_null<llvm::ConstantInt>(returns_void_at_end)) {
        if (known->isOne()) save_return(empty_scalar());
      } else if (returns_void_at_end) {
        auto *returned = block("return.void");
        b_.CreateCondBr(returns_void_at_end, returned, scope.cleanup);
        b_.SetInsertPoint(returned);
        save_return(empty_scalar());
      }
      b_.CreateBr(scope.cleanup);
    }
    if (case_scan_request_) {
      b_.SetInsertPoint(scope.cleanup);
      auto *kind = load(completion_.kind);
      auto *scan = b_.CreateICmpEQ(kind, b_.getInt8(CompletionKind::case_scan));
      b_.CreateStore(scan, case_scan_request_);
      b_.CreateStore(b_.CreateSelect(scan, b_.getInt8(0), kind), completion_.kind);
      // The drain loops through its own header for every action. Only
      // arrival from the body can set the pending scan request.
      auto *draining = block("scope.drain");
      b_.CreateBr(draining);
      scope.cleanup = draining;
    }
    drain(scope, at, deferred_blocks); restore_effects(effects);
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
    scope_ = scope.parent; native_effect_scope_ = outer_native; control_ = control.parent;
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
    fields.insert(fields.end(), call.allocation_owners.size(), allocation_capability_);
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
    for (const auto role : call_layouts_.at(method).allocation_owners) {
      auto *capability = allocation_capability(operation.allocator_arguments->at(role));
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
    std::vector<llvm::Value *> arguments;
    for (size_t i = 0; i < expr.children.size(); ++i) {
      arguments.push_back(prepare_argument(expr, i));
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
  }
  llvm::Value *region_address(llvm::Value *owner, llvm::Value *offset, llvm::Value *size) {
    auto address = runtime("cod_lang_region_address", b_.getPtrTy(),
        {b_.getPtrTy(), b_.getInt64Ty(), b_.getInt64Ty()});
    return b_.CreateCall(address, {owner, offset, size});
  }
  llvm::Value *regional_locator(llvm::Value *owner, llvm::Value *address) {
    auto *base = region_address(owner, integer(0), integer(0));
    auto *offset = b_.CreateSub(b_.CreatePtrToInt(address, b_.getInt64Ty()),
                               b_.CreatePtrToInt(base, b_.getInt64Ty()));
    return b_.CreateSelect(b_.CreateIsNull(address), integer(UINT64_MAX), offset);
  }
  llvm::Value *regional_target(llvm::Value *owner, llvm::Value *offset, llvm::Type *layout) {
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
  llvm::Value *allocate_aggregate(const Expression &expr, llvm::Type *layout, llvm::Value *region = nullptr) {
    if (region) return reserve_regional_row(region, layout, expr.at);
    const auto &fact = checked_.at(&expr);
    if (!fact.lexical_row_allocator) return storage_.at(fact.place.value());
    const auto &data = module_.getDataLayout();
    auto *capability = allocation_capability(*fact.lexical_row_allocator);
    auto *owner = b_.CreateExtractValue(capability, 1);
    auto *bytes = integer(data.getTypeAllocSize(layout).getFixedValue());
    return allocate_lexical_extent(owner, bytes, data.getABITypeAlign(layout).value(), expr.at);
  }
  llvm::Value *construct_region(const Expression &make) {
    const auto &input = *make.children.front();
    auto *capacity_value = emit_expression(input);
    if (!reachable()) return nullptr;
    auto *capacity = convert(capacity_value, checked_.at(&input).type, Type::uint64);
    const auto &fact = checked_.at(&make);
    const auto schema_id = fact.operand_type.identity;
    const auto &order = fact.region_initializers;
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
          auto *prepared = convert_value(operand, checked_.at(&value).type, type);
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
    const auto holder = checked_.at(&binding).region_holder.value();
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
  }
  OwnedException emit_exception_source(const Expression &written) {
    const auto &expr = ungrouped_expression(written);
    const auto &fact = checked_.at(&expr);
    if (fact.exception_owner) {
      const auto source = *fact.exception_owner;
      return {load(storage_.at(source)), take_pointer(b_, exception_holder_slots_.at(source))};
    }
    if (!fact.owns_exception_result) throw std::logic_error("unchecked owning exception source");
    if (expr.kind == Expression::Kind::make_exception) {
      auto prepared = construct_braced_exception(expr, fact.type.identity);
      if (reachable()) prepared.value = pack(prepared.value, fact.type);
      return prepared;
    }
    const auto arguments = prepare_arguments(expr);
    if (!reachable()) return {};
    const auto method = program_->instances.at(*fact.callee).method;
    auto *owner = slot(b_.getPtrTy(), "exception.call.result");
    const auto result = native_call(expr, methods_.at(method), method, arguments, nullptr, {}, owner);
    auto *value = propagate(result);
    return {value, load(owner)};
  }
  llvm::Value *discard_exception(const Expression &expr) {
    const auto prepared = emit_exception_source(expr);
    if (!reachable()) return nullptr;
    release_failure_owner(b_, prepared.owner);
    return empty_scalar();
  }
  llvm::Value *receive_exception_holder(const Expression &binding) {
    const auto prepared = emit_exception_source(*binding.children.front());
    if (!reachable()) return nullptr;
    const auto place = checked_.at(&binding).place.value();
    auto *holder = slot(b_.getPtrTy(), "exception.owner");
    exception_holder_slots_.emplace(place, holder);
    b_.CreateStore(prepared.value, storage_.at(place));
    b_.CreateStore(prepared.owner, holder);
    const auto action = std::ranges::find_if(scope_->actions, [&](const auto &entry) {
      return entry.exception_release && entry.action == &binding;
    });
    if (action == scope_->actions.end()) throw std::logic_error("uncollected exception holder release");
    auto *record = action->activation_record;
    prepare_cleanup_record(*scope_, record, std::distance(scope_->actions.begin(), action));
    b_.CreateStore(holder, b_.CreateStructGEP(action->layout, record, 1));
    b_.CreateStore(record, scope_->head);
    return empty_scalar();
  }
  llvm::Value *install_native_effect(const Expression &effect) {
    using Plan = std::pair<size_t, CheckedOperation::NativeAcquisition>;
    const auto &operation = checked_.at(&effect);
    {
      const Plan plan = Plan{*operation.symbol, *operation.native_acquisition};
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
    }
  }
  llvm::Value *emit_attached_native_handle(const Expression &expr) {
    {
      const CheckedOperation::NativeAcquisition plan = *checked_.at(&expr).native_acquisition;
      auto *owner = plan.parent_borrow ? binding_address(*plan.parent_borrow, plan.parent_place)
                                      : native_reborrow(load(storage_.at(*plan.parent_place)));
      auto *pair = native_handle_pair();
      auto *owner_word = b_.CreateLoad(b_.getInt64Ty(), b_.CreateStructGEP(pair, owner, 1));
      auto *child = slot(pair, "native.child.borrow");
      if (!call_native_constructor(plan, b_.CreateStructGEP(pair, child, 0), owner)) return nullptr;
      b_.CreateStore(owner_word, b_.CreateStructGEP(pair, child, 1));
      return child;
    }
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
    auto *saved = slot(read ? scalar_ : static_cast<llvm::Type *>(b_.getPtrTy()), "try.value");
    auto *value = emit_expression(*expr.children[0], projection, discard);
    if (reachable()) { b_.CreateStore(read && discard ? empty_scalar() : value, saved); b_.CreateBr(guarded.exit); }
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
        finalizer = &handler;
        break;
      }
      const auto &fact = checked_.at(&handler);
      auto *selected = block("catch.selected"), *next = block("catch.next");
      auto *caught_value = pack(active_exception(), fact.type);
      auto *caught = b_.CreateSelect(catch_matches(load(completion_.category), fact.type.identity), caught_value, empty_scalar());
      b_.CreateCondBr(b_.CreateICmpNE(b_.CreateExtractValue(caught, 1), b_.getInt32(0)), selected, next);
      b_.SetInsertPoint(selected);
      b_.CreateStore(caught, storage_.at(*fact.place));
      auto *pending_transfer = protected_outcomes_ ? load(completion_.transfer) : nullptr;
      auto *caught_owner = owned_failures_ ? take_pointer(b_, completion_.failure_owner) : nullptr;
      llvm::AllocaInst *holder = nullptr;
      if (owned_failures_) {
        holder = slot(b_.getPtrTy(), "catch.owner");
        b_.CreateStore(caught_owner, holder);
        exception_holder_slots_.emplace(*fact.place, holder);
      }
      if (protected_outcomes_) b_.CreateStore(null_pointer(), completion_.transfer);
      b_.CreateStore(b_.getInt8(0), completion_.kind); b_.CreateStore(integer(0), completion_.category); b_.CreateStore(null_pointer(), completion_.fatal);
      b_.CreateStore(b_.getFalse(), completion_.has_return);
      b_.CreateStore(empty_scalar(), completion_.value);
      Control catching{control_, protected_outcomes_ || owned_failures_ ? block("catch.completed") : handled};
      control_ = &catching;
      auto *result = emit_expression(*handler.children.front(), projection, discard);
      if (reachable()) { b_.CreateStore(read && discard ? empty_scalar() : result, saved); b_.CreateBr(catching.exit); }
      if (protected_outcomes_ || owned_failures_) {
        b_.SetInsertPoint(catching.exit);
        if (holder) release_failure_owner(b_, take_pointer(b_, holder));
        preserve_transfer(pending_transfer);
        b_.CreateBr(handled);
      }
      if (holder) exception_holder_slots_.erase(*fact.place);
      control_ = catching.parent;
      b_.SetInsertPoint(next);
    }
    b_.CreateBr(handled);
    b_.SetInsertPoint(handled);
    if (finalizer) complete_cleanup([&] { emit_expression(*finalizer); });
    auto *done = block("try.continue");
    b_.CreateCondBr(b_.CreateICmpEQ(load(completion_.kind), b_.getInt8(0)), done, control_->exit);
    b_.SetInsertPoint(done);
    if (read && discard) return empty_scalar();
    if (checked_.at(&expr).type == Type::never) { b_.CreateUnreachable(); return nullptr; }
    auto *result = load(saved);
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
  llvm::Value *checked_address(llvm::Value *address, const Expression &expr, const Location &at) {
    return checked_address(address, (checked_.at(&expr).targets.contains(null_target)), at);
  }
  llvm::Value *prepare_value(const Expression &expr, size_t index, llvm::Value *value) {
    const auto &context = checked_;
    const auto &call = context.at(&expr);
    const auto &from = context.at(&call.argument(expr, index)).type;
    const auto &to = call.arguments.at(index).type;
    return from == to || from == Type::never ? value : convert_value(value, from, to);
  }
  llvm::Value *prepare_argument(const Expression &expr, size_t index,
                                std::vector<llvm::Value *> *current_owners = nullptr) {
    const auto &call = checked_.at(&expr);
    const auto &child = call.argument(expr, index);
    if (const auto symbol = call.arguments.at(index).current_reference_symbol) {
      if (!current_owners) throw std::logic_error("current reference argument lacks owner preparation");
      // Both projections must survive preparation of all later operands.
      auto *binding = symbols_.at(*symbol);
      auto *selected = b_.CreateLoad(binding->getValueType(), binding, "effect.current.argument");
      if (current_owners->empty()) current_owners->resize(call.argument_count(expr));
      (*current_owners)[index] = b_.CreateExtractValue(selected, 1, "effect.argument.owner");
      return b_.CreateExtractValue(selected, 0, "effect.argument.selection");
    }
    const auto &argument = call.arguments.at(index);
    const auto &operand = checked_.at(&child);
    if (argument.mode == ArgumentMode::owning_exception) {
      if (!operand.exception_owner)
        throw std::logic_error("an owning exception argument lacks its checked holder");
      const auto holder = *operand.exception_owner;
      auto *layout = exception_input_layout();
      auto *input = slot(layout, "exception.input");
      b_.CreateStore(storage_.at(holder), b_.CreateStructGEP(layout, input, 0));
      b_.CreateStore(exception_holder_slots_.at(holder), b_.CreateStructGEP(layout, input, 1));
      auto *carrier = b_.CreateInsertValue(empty_scalar(), b_.CreatePtrToInt(input, b_.getInt64Ty()), 0);
      return b_.CreateInsertValue(carrier, b_.getInt32(tag(argument.type)), 1);
    }
    if (operand.type.kind == Type::abstract_type) {
      const auto *source = &child;
      while (source->kind == Expression::Kind::grouped) source = source->children.front().get();
      auto *binding = symbols_.at(*checked_.at(source).symbol);
      auto *selected = b_.CreateLoad(binding->getValueType(), binding, "effect.current");
      return b_.CreateExtractValue(selected, 0, "effect.selection");
    }
    const auto projection = argument.binding_slot ? Projection::slot
        : argument.mode == ArgumentMode::value ? Projection::value : Projection::target;
    auto *value = emit_expression(child, projection);
    if (!reachable()) return nullptr;
    if (argument.mode == ArgumentMode::value) return prepare_value(expr, index, value);
    if ((argument.mode == ArgumentMode::borrowed_read && !call.destination_arguments) ||
        (expr.kind == Expression::Kind::transfer && index == 0))
      value = checked_address(value, child, child.at);
    return pack_view(value, child);
  }
  std::vector<llvm::Value *> prepare_arguments(const Expression &expr, llvm::Value *first_place = nullptr,
                                               std::vector<llvm::Value *> *current_owners = nullptr) {
    const auto &fact = checked_.at(&expr);
    std::vector<llvm::Value *> arguments(fact.argument_count(expr));
    for (size_t written = 0; written < arguments.size(); ++written) {
      const auto i = fact.argument_order.empty() ? written : fact.argument_order.at(written);
      if (first_place && i == 0) arguments[i] = pack_view(first_place, fact.argument(expr, i));
      else arguments[i] = prepare_argument(expr, i, current_owners);
      if (!reachable()) return {};
    }
    check_destination_arguments(expr, arguments);
    return arguments;
  }
  llvm::Value *emit_text_transfer(const Expression &expr, llvm::Value *destination = nullptr) {
    const auto arguments = prepare_arguments(expr, destination);
    return reachable() ? propagate(invoke(expr, arguments)) : nullptr;
  }
  void check_destination_arguments(const Expression &expr, const std::vector<llvm::Value *> &arguments) {
    const auto &call = checked_.at(&expr);
    if (!call.destination_arguments) return;
    for (size_t i = 0; i < expr.children.size(); ++i) {
      const auto &argument = call.arguments.at(i);
      if (argument.mode != ArgumentMode::borrowed_read) continue;
      auto *address = view_address(arguments[i]);
      if (argument.binding_slot) {
        address = b_.CreateLoad(b_.getPtrTy(), address);
        checked_address(address, call.nullable_arguments.contains(i), expr.at);
      } else checked_address(address, *expr.children[i], expr.at);
    }
  }
  llvm::Value *assign_prepared(const Expression &expr, const std::vector<llvm::Value *> &arguments) {
    auto *destination = view_address(arguments[0]);
    if (expr.kind == Expression::Kind::transfer) {
      const auto &type = checked_.at(&expr).operand_type;
      if (inline_aggregate_type(type))
        b_.CreateCall(bits_transfer(type), {destination, view_address(arguments[1])});
      else assign_place(destination, arguments[1], expr);
      return destination;
    }
    auto *target = view_address(arguments[1]);
    if (checked_.at(&expr).regional_storage)
      b_.CreateStore(regional_locator(selected_owner(expr), target), destination);
    else b_.CreateStore(target, destination);
    return target;
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
    const auto &binding = checked_.at(&declaration);
    const auto &source = checked_.at(declaration.children.front().get());
    const bool same_place = binding.place && binding.place == source.place;
    {
      const std::tuple<bool, bool, bool> mode = std::tuple<bool, bool, bool>{binding.view, binding.mixed_view, same_place};
      const auto [borrowed, mixed, same_place] = mode;
      const auto &binding = checked_.at(&declaration);
      llvm::Value *address = binding.place ? static_cast<llvm::Value *>(storage_.at(*binding.place)) : null_pointer();
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
    }
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
    auto *owner = owner_identity(checked_.at(&expr).lexical_allocation.value().owner, expr.at);
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
        (checked_.at(&expr).discarded_view)) {
      {
        const bool view_only = checked_.at(&expr).discarded_view;
        emit_expression(expr, view_only ? Projection::target : Projection::value, true);
        return reachable() ? empty_scalar() : nullptr;
      }
    }
    const bool abrupt = (checked_.at(&expr).type == Type::never);
    // An abrupt mixed branch has no target/slot result. Retain its intermediate
    // value or view through cleanup without forcing the parent's result shape.
    if (!carries_value(projection) && abrupt &&
        (checked_.at(&expr).mixed_view))
      projection = Projection::receiver;
    llvm::Value *value;
    if (projection == Projection::receiver) {
      const auto &fact = checked_.at(&expr);
      const unsigned alternatives = fact.mixed_view ? 2u : fact.view ? 1u : 0u;
      value = [&](unsigned kind) -> llvm::Value * {
        if (!kind) return emit_expression(expr, Projection::value, discard);
        if (kind == 2) return emit_checked_expression(expr, projection, discard);
        auto *target = emit_expression(expr, Projection::target, discard);
        if (!reachable()) return nullptr;
        // Receiver-only target encoding: no value type or owned payload. Valid
        // materialized values have nonzero tags and keep normal payload cleanup.
        return b_.CreateInsertValue(empty_scalar(), b_.CreatePtrToInt(target, b_.getInt64Ty()), 0);
      }(alternatives);
    } else value = emit_checked_expression(expr, projection, discard);
    if (abrupt) {
      if (reachable()) b_.CreateUnreachable();
      return nullptr;
    }
    if (expr.checked_ref_argument && projection == Projection::target && reachable()) {
      using Presence = CheckedOperation::CheckedRefPresence;
      {
        const Presence presence = checked_.at(&expr).checked_ref_presence;
        if (presence == Presence::definite_absence) {
          fail_path(ExceptionCategory::null_pointer, *expr.checked_ref_argument);
          return nullptr;
        }
        return checked_address(value, presence == Presence::possible_absence, *expr.checked_ref_argument);
      }
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
    auto *direct = block("projection.direct"), *borrowed = block("projection.target"), *done = block("projection.ready");
    b_.CreateCondBr(b_.CreateICmpEQ(b_.CreateExtractValue(result, 1), b_.getInt32(0)), borrowed, direct);
    b_.SetInsertPoint(direct);
    auto *value_path = b_.GetInsertBlock(); b_.CreateBr(done);
    b_.SetInsertPoint(borrowed);
    auto *value = load_place(checked_address(view_address(result), expr, expr.at), expr);
    auto *target_path = b_.GetInsertBlock(); b_.CreateBr(done);
    b_.SetInsertPoint(done);
    auto *selected = b_.CreatePHI(scalar_, 2, "projection.value");
    selected->addIncoming(result, value_path); selected->addIncoming(value, target_path);
    return selected;
  }
  llvm::Value *emit_joined_bits(const Expression &join, const Expression &arm, bool discard) {
    const auto &fact = checked_.at(&arm);
    const auto source = checked_.at(&join).scalar_value_join ? result_source(fact) : Projection::value;
    return emit_projected_value(arm, source, discard);
  }
  llvm::Value *emit_case_scope(const Expression &body) {
    const auto arguments = prepare_arguments(body);
    if (!reachable()) return nullptr;
    auto *result = invoke(body, arguments);
    propagate(result);
    return result;
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
    if (breaking) dispatch->addCase(b_.getInt8(CompletionKind::case_break), breaking);
    if (scanning) {
      auto *resume = block("case.resume");
      dispatch->addCase(b_.getInt8(CompletionKind::case_scan), resume);
      b_.SetInsertPoint(resume);
      b_.CreateStore(empty_scalar(), completion_.value);
      b_.CreateStore(b_.getInt8(0), completion_.kind);
      b_.CreateBr(scanning);
    }
    b_.SetInsertPoint(normal);
  }
  llvm::Value *pack_address(llvm::Value *address) {
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
          emit_expression(item, Projection::value, true);
        });
        continue;
      }

      auto *selected_arm = block("case.matched"), *next = block("case.next");
      const auto &checked = checked_.at(&item);
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
      case P::present:
        if (may_be_null) b_.CreateCondBr(b_.CreateIsNotNull(selected), selected_arm, next);
        else b_.CreateBr(selected_arm);
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
      if (pattern.kind == P::present) {
        const auto &binding = checked_.at(item.case_bindings.front().get());
        b_.CreateStore(selected, bindings_.at(*binding.binding).reference_slot);
      }
      for (size_t i = 0; i < checked.case_payload_fields.size(); ++i) {
        const auto &receiver = *item.case_bindings[i];
        const auto &binding = checked_.at(&receiver);
        auto *field = sum_field_address(selected, *pattern.constructor, checked.case_payload_fields[i]);
        if (inline_aggregate_type(binding.operand_type)) {
          b_.CreateCall(bits_transfer(binding.operand_type), {storage_.at(*binding.place), field});
          continue;
        }
        auto *payload = load_place(b_, field, StorageFormat{binding.operand_type});
        auto *value = convert_value(payload, binding.operand_type, binding.type);
        assign_place(storage_.at(*binding.place), value, receiver);
      }
      if (item.children.size() == 2) {
        const auto &guard = *item.children.front();
        emit_case_item(breaking, nullptr, [&] {
          auto *condition = emit_expression(guard);
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
            const bool resumes = (checked_.at(&item).case_scope_resumes);
            const bool returns = (checked_.at(&item).case_arm_completes);
            if (resumes && returns) {
              auto *returned = block("case.returned");
              b_.CreateCondBr(b_.CreateExtractValue(result, case_scan_field()), next, returned);
              b_.SetInsertPoint(returned);
            }
            if (returns) {
              auto *value = b_.CreateExtractValue(result, 0);
              if (projects && checked_.at(&body).borrowed_result && !discard)
                value = load_place(checked_address(view_address(value), body, body.at), body);
              incoming.emplace_back(value, b_.GetInsertBlock());
              b_.CreateBr(join);
            } else if (resumes) b_.CreateBr(next);
            else b_.CreateUnreachable();
          }
        } else {
          emit_case_item(breaking, item.case_resumes ? next : nullptr, [&] {
            const bool paths = checked_.at(&item).case_arm_completes;
            auto *value = [&](bool completes) -> llvm::Value * {
              if (!completes) {
                emit_expression(body, Projection::value, true);
                if (reachable()) b_.CreateUnreachable();
                return nullptr;
              }
              if (!result_view) return emit_joined_bits(expr, body, discard);
              auto *address = emit_expression(body, Projection::target, discard);
              return reachable() ? pack_address(address) : nullptr;
            }(paths);
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
      if (discard) b_.CreateStore(empty_scalar(), completion_.value);
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
  llvm::Value *emit_indexed_address(const Expression &expr) {
    const auto &array = *expr.children[0];
    auto *view = emit_expression(array, Projection::target);
    if (!reachable()) return nullptr;
    view = checked_address(view, array, expr.at);
    const auto &index_expr = *expr.children[1];
    auto *index_value = emit_expression(index_expr);
    if (!reachable()) return nullptr;
    const auto &array_type = checked_.at(&array).type;
    if (array_type.kind != Type::array) throw std::logic_error("indexed place without a checked array type");
    const auto &shape = program_->arrays.at(array_type.identity);
    const auto &index_type = checked_.at(&index_expr).type;
    auto *index = b_.CreateZExtOrTrunc(convert(index_value, index_type, shape.index), b_.getInt64Ty());
    if (bounds_checks_) check_index_bounds(index, array_length(view), shape.index == Type::integer, expr.at);
    auto *element = inline_layout(shape.element);
    return b_.CreateGEP(element, array_data(view, element), index);
  }
  llvm::Value *emit_regional_initializer(const Expression &expr, llvm::Value *destination, size_t written) {
    using Plan = std::tuple<size_t, size_t, size_t, std::optional<size_t>>;
    const auto &operation = checked_.at(&expr);
    const auto constructor = *operation.sum_constructor;
    {
      const Plan plan = Plan{constructor.sum, constructor.alternative, *operation.regional_sum_holder, operation.callee};
      const auto [sum, alternative, holder, callee] = plan;
      const SumConstructor constructor{sum, alternative};
      const auto &description = program_->sums.at(sum).alternatives.at(alternative);
      std::vector<llvm::Value *> arguments;
      for (size_t i = 0; i < written; ++i) {
        auto *value = prepare_argument(expr, i);
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
      propagate(native_call(expr, entry, method, std::move(arguments)));
      if (checked_.at(&expr).type == Type::never) {
        b_.CreateUnreachable();
        return nullptr;
      }
      auto *owner = load(storage_.at(holder));
      for (const auto &[selection, field] : edges)
        b_.CreateStore(regional_locator(owner, load(selection)), field);
      return destination;
    }
  }
  void emit_product_initializer(const Expression &expr, llvm::Value *destination,
                                llvm::StructType *layout, llvm::Value *region) {
    const auto &fact = checked_.at(&expr);
    const auto &fields = program_->schemas.at(fact.type.identity).fields;
    std::vector<llvm::AllocaInst *> slots;
    std::vector<llvm::Value *> arguments;
    for (const auto &field : fields) {
      auto *selection = slot(b_.getPtrTy(), "initializer.reference");
      slots.push_back(selection);
      auto *carrier = b_.CreateInsertValue(empty_scalar(), b_.CreatePtrToInt(selection, b_.getInt64Ty()), 0);
      arguments.push_back(b_.CreateInsertValue(carrier, b_.getInt32(tag(field.type)), 1));
    }
    const auto method = program_->instances.at(*fact.callee).method;
    propagate(native_call(expr, methods_.at(method), method, std::move(arguments)));
    for (size_t i = 0; i < slots.size(); ++i) {
      auto *target = load(slots[i]);
      auto *value = region ? regional_locator(region, target) : target;
      b_.CreateStore(value, b_.CreateStructGEP(layout, destination, i));
    }
  }

  llvm::Value *emit_checked_expression(const Expression &expr, Projection projection, bool discard) {
    const bool read = carries_value(projection);
    using K = Expression::Kind;
    const auto &fact = checked_.at(&expr);
    if (expr.kind == K::call && (checked_.at(&expr).builtin == BuiltinOperation::array_transfer)) {
      const auto &operation = checked_.at(&expr);
      const bool builtin = operation.builtin == BuiltinOperation::array_transfer;
      auto element = Type{Type::void_type};
      if (builtin && operation.operand_type.kind == Type::array)
        element = program_->arrays.at(operation.operand_type.identity).element;
      if (!inline_aggregate_type(element)) element = Type{Type::void_type};
      {
        const std::tuple<bool, Type::Kind, size_t> selected = std::tuple<bool, Type::Kind, size_t>{builtin, element.kind, element.identity};
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
      }
    }
    if (fact.enum_literal)
      return pack(llvm::ConstantInt::get(llvm::cast<llvm::IntegerType>(native_type(fact.type)), fact.integer), fact.type);
    if (expr.kind == K::typed_expression && fact.enum_decode) {
      auto *value = emit_expression(*expr.children.front());
      if (!reachable()) return nullptr;
      const auto &domain = program_->enums.at(fact.type.identity);
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
      return discard ? empty_scalar() : pack(code, fact.type);
    }
    if (fact.enum_assertion) return emit_expression(*expr.children.front(), projection, discard);
    if (fact.native_call)
      return program_->native_operations.at(*fact.native_call).has_here() ? emit_native_here(expr) : emit_native_call(expr);
    if (fact.builtin == BuiltinOperation::unary || fact.builtin == BuiltinOperation::binary) {
      std::vector<llvm::Value *> arguments;
      EagerOperation operation{{}, &fact};
      for (const auto &child : expr.children) {
        arguments.push_back(emit_expression(*child));
        if (!reachable()) return nullptr;
        operation.inputs.push_back(checked_.at(child.get()).type);
      }
      return eager_builtin(expr, arguments, operation);
    }
    if (fact.scoped_effect) return install_scoped_effect(expr, fact);
    switch (expr.kind) {
    case K::grouped: {
      const auto &operand = *expr.children.front();
      if (projection != Projection::value) return emit_expression(operand, projection, discard);
      const auto source = result_source(checked_.at(&operand));
      {
        const Projection mode = source;
        const auto source = mode;
        auto *value = emit_projected_value(operand, source, discard);
        return value;
      }
    }
    case K::typed_expression: {
      const auto &operand = *expr.children.front();
      if (fact.type == Type::c_string) {
        auto *view = emit_expression(operand, Projection::target);
        if (!reachable()) return nullptr;
        {
          const bool check = checked_.at(&expr).check_c_string;
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
        }
      }
      {
        const auto &input = ungrouped_expression(operand);
        const auto &result = checked_.at(&input);
        if (result.owns_exception_result) return discard_exception(input);
        emit_expression(input, result_source(result));
        return reachable() ? empty_scalar() : nullptr;
      }
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
      return finish_effect(fact, convert_result(expr, *expr.children.front(), value));
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
      {
        const bool check = checked_.at(&expr).check_predicate_assertion;
        if (check) {
          auto *ready = block("assertion.satisfied"), *failed = block("assertion.failed");
          b_.CreateCondBr(unpack(value, Type::boolean), ready, failed);
          b_.SetInsertPoint(failed);
          fail_path(ExceptionCategory::assertion_failed, expr.at);
          b_.SetInsertPoint(ready);
        }
        return empty_scalar();
      }
    }
    case K::real: {
      auto text = expr.text;
      if (text.ends_with("f32") || text.ends_with("f64")) text.resize(text.size() - 3);
      if (text.find_first_of(".eE") == std::string::npos) text += ".0";
      return pack(llvm::ConstantFP::get(native_type(fact.type), text), fact.type);
    }
    case K::name: {
      if (projection == Projection::slot) return binding_slot(*fact.binding);
      auto *address = binding_address(*fact.binding, fact.place);
      if (fact.reborrows_native_owner) address = native_reborrow(b_.CreateLoad(b_.getInt64Ty(), address));
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
      if (fact.owns_exception_result) return discard_exception(expr);
      if (fact.symbol && program_->symbols[*fact.symbol].live_scope)
        return invoke_scoped_effect(expr, *fact.symbol);
      if (fact.builtin == BuiltinOperation::text_size || fact.builtin == BuiltinOperation::text_byte ||
          fact.builtin == BuiltinOperation::text_slice)
        return emit_text_access(expr, fact.builtin, discard);
      if (fact.builtin == BuiltinOperation::builder_append) {
        const auto &source = *expr.children[1];
        auto *builder = emit_expression(*expr.children[0], Projection::target);
        if (!reachable()) return nullptr;
        auto *piece = emit_expression(source, Projection::target);
        if (!reachable()) return nullptr;
        const auto name = program_->builder_type(checked_.at(&source).type)
            ? "cod_lang_builder_append_builder" : "cod_lang_builder_append_text";
        auto append = runtime(name, b_.getInt32Ty(), {b_.getPtrTy(), b_.getPtrTy()});
        auto *status = b_.CreateCall(append, {builder, piece});
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
      std::vector<llvm::Value *> current_owners;
      const auto arguments = prepare_arguments(expr, nullptr, &current_owners);
      if (!reachable()) return nullptr;
      std::vector<llvm::Value *> here_sites;
      auto *result = invoke(expr, arguments, &here_sites, current_owners);
      if (!here_sites.empty()) result = consume_here_transfer(result, here_sites);
      auto *value = propagate(result);
      if (!fact.view && inline_aggregate_type(fact.type)) return view_address(value);
      if (!fact.borrowed_result) return value;
      auto *address = view_address(value);
      return !read ? address : discard ? empty_scalar() : load_place(checked_address(address, expr, expr.at), expr);
    }
    case K::case_expression: {
      const auto *subject = expr.children.front().get();
      const auto &input = checked_.at(subject), &result = checked_.at(&expr);
      {
        const std::tuple<Type::Kind, size_t, bool, bool, bool, bool, bool> layout = std::tuple<Type::Kind, size_t, bool, bool, bool, bool, bool>{input.type.kind, input.type.identity, input.view, result.case_subject_may_be_null,
               result.case_reaches_end, result.view, result.scalar_value_join};
        const auto &[kind, identity, borrowed, nullable, reaches_end, result_view, projects] = layout;
        return emit_case(expr, Type{kind, {}, identity}, borrowed, nullable, reaches_end, result_view, projects, projection, discard);
      }
    }
    case K::case_break: case K::case_fallthrough: {
      llvm::Value *value = empty_scalar();
      if (!expr.children.empty()) {
        const auto &operand = *expr.children.front();
        const auto &exit = checked_.at(&expr);
        const auto source = exit.scalar_value_join ? result_source(checked_.at(&operand)) : Projection::value;
        const std::pair<bool, Projection> preparations = std::pair<bool, Projection>{exit.borrowed_result, source};
        value = [&](const auto &preparation) -> llvm::Value * {
          const auto [borrow, source] = preparation;
          if (!borrow) return emit_projected_value(operand, source, false);
          auto *address = emit_expression(operand, Projection::target);
          return reachable() ? pack_address(address) : nullptr;
        }(preparations);
      }
      if (!reachable()) return nullptr;
      const auto kind = expr.kind == K::case_break ? CompletionKind::case_break : CompletionKind::case_scan;
      save_local_completion(value, kind, false);
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
        auto *value = projection == Projection::value ? emit_joined_bits(expr, arm, discard)
                                                      : emit_expression(arm, projection, discard);
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
      const auto &type = fact.operand_type;
      const auto test = [&] {
        auto *current = load_place(index, expr, &CheckedOperation::operand_type);
        auto *left = unpack(current, type), *right = unpack(last, type);
        return type == Type::integer ? b_.CreateICmpSLT(left, right) : b_.CreateICmpULT(left, right);
      };
      auto *body = block("range.body"), *empty = block("range.empty"), *done = block("range.done");
      b_.CreateCondBr(test(), body, empty);
      b_.SetInsertPoint(body);
      iteration(*expr.children[2], true);
      if (reachable()) {
        auto *current = load_place(index, expr, &CheckedOperation::operand_type);
        auto *value = unpack(current, type);
        // The half-open test proves current < last, so increment cannot wrap.
        auto *advanced = pack(b_.CreateAdd(value, llvm::ConstantInt::get(value->getType(), 1)), type);
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
      emit_expression(*expr.children.front(), Projection::target, true);
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
          const auto id = checked_.at(declaration.get()).binding.value();
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
        auto *destination = storage_.at(*fact.place);
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
        if (expr.kind == K::transfer) {
          if (!read) return address;
          return discard && inline_aggregate_type(fact.type) ? empty_scalar() : load_place(address, expr);
        }
        return !read ? address : discard ? empty_scalar() : load_place(checked_address(address, expr, expr.at), expr);
      };
      return assignment(expr.kind == K::transfer && (fact.callee || fact.arguments.front().binding_slot));
    }
    case K::return_tail: {
      if (fact.type == Type::void_type) return emit_expression(*expr.children.front(), Projection::value, true);
      [[fallthrough]];
    }
    case K::return_value: case K::break_value: case K::continue_value: {
      llvm::Value *value = empty_scalar();
      if (fact.owns_exception_result) {
        if (!completion_.result_owner) throw std::logic_error("exception return without a pending owner slot");
        const auto prepared = emit_exception_source(*expr.children.front());
        if (!reachable()) return nullptr;
        b_.CreateStore(prepared.owner, completion_.result_owner);
        value = prepared.value;
      } else if (fact.operand_type.kind == Type::region && fact.operand_type.owned) {
        const auto *make = expr.children.front().get();
        while (make->kind == K::grouped) make = make->children.front().get();
        auto *owner = construct_region(*make);
        if (!reachable()) return nullptr;
        b_.CreateStore(owner, completion_.result_owner);
      } else if (!expr.children.empty() && inline_aggregate_type(fact.operand_type) && !fact.borrowed_result) {
        const auto &source = *expr.children.front();
        auto *input = emit_expression(source, checked_.at(&source).view ? Projection::target : Projection::value);
        if (!reachable()) return nullptr;
        // A pending return may be suspended while cleanup runs. Give each
        // prepared return its own snapshot until the final outcome is known.
        auto *pending = b_.CreateAlloca(inline_layout(fact.operand_type), nullptr, "aggregate.pending.return");
        b_.CreateCall(bits_transfer(fact.operand_type), {pending, input});
        value = pack_address(pending);
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
          release_failure_owner(b_, take_pointer(b_, transfer_failure_slot(b_, here_destination_)));
        }
        auto *payload = outcome(value, integer(0), integer(0), null_pointer());
        b_.CreateStore(b_.getInt8(static_cast<unsigned>(*fact.here_exit)), b_.CreateStructGEP(here_transfer_, here_destination_, 0));
        b_.CreateStore(payload, b_.CreateStructGEP(here_transfer_, here_destination_, 1));
        b_.CreateStore(here_destination_, completion_.transfer);
        b_.CreateStore(b_.getInt1(*fact.here_exit == HereExit::return_supplier), completion_.has_return);
        b_.CreateStore(b_.getInt8(3), completion_.kind);
      } else save_return(value);
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
        auto *layout = product_layout(fact.operand_type.identity);
        auto *owner = load(storage_.at(*fact.region_holder));
        auto *address = region_address(owner, integer(0), llvm::ConstantExpr::getSizeOf(layout));
        auto *member = b_.CreateStructGEP(layout, address, *fact.product_field);
        if (!read) return member;
        return discard ? empty_scalar() : load_place(member, expr);
      }
      if (fact.exception_owner) {
        auto *layout = product_layout(*program_->exception_types.at(fact.operand_type.identity).schema);
        auto *payload = exception_payload(load(exception_holder_slots_.at(*fact.exception_owner)));
        auto *member = b_.CreateStructGEP(layout, payload, *fact.product_field);
        if (!read) return member;
        return discard ? empty_scalar() : load_place(member, expr);
      }
      if (fact.regional_sum_holder) {
        const auto &parent = *expr.children.front();
        auto *address = emit_expression(parent, Projection::target);
        if (!reachable()) return nullptr;
        const auto constructor = fact.sum_constructor.value();
        const auto field = fact.product_field.value();
        auto *slot = sum_field_address(address, constructor, field);
        const auto &description = program_->sums.at(constructor.sum).alternatives.at(constructor.alternative).fields.at(field);
        llvm::Value *target = slot;
        if (description.stores_target()) {
          auto *owner = load(storage_.at(*fact.regional_sum_holder));
          auto *offset = b_.CreateLoad(b_.getInt64Ty(), slot);
          target = regional_target(owner, offset, sum_layout(description.type.identity).storage);
        }
        return !read ? target : discard ? empty_scalar() : load_place(target, expr);
      }
      if (fact.product_field) {
        const auto &parent = *expr.children.front();
        auto *address = emit_expression(parent, Projection::target);
        if (!reachable()) return nullptr;
        address = checked_address(address, parent, expr.at);
        const auto schema = fact.operand_type.identity;
        const auto number = *fact.product_field;
        auto *layout = fact.regional_storage ? regional_product_layout(schema) : product_layout(schema);
        auto *slot = b_.CreateStructGEP(layout, address, number);
        const auto &description = program_->schemas[schema].fields[number];
        llvm::Value *field = slot;
        if (projection != Projection::slot && description.stores_target()) {
          if (fact.regional_storage) {
            auto *owner = selected_owner(expr);
            auto *offset = b_.CreateLoad(b_.getInt64Ty(), slot);
            // A named array edge selects its header. Indexing uses that
            // header's existing backing and length.
            llvm::Type *target_layout = array_header_;
            if (description.type.kind == Type::product)
              target_layout = regional_product_layout(description.type.identity);
            else if (description.type.kind != Type::array)
              target_layout = inline_layout(description.type);
            field = regional_target(owner, offset, target_layout);
          } else field = b_.CreateLoad(b_.getPtrTy(), slot);
        }
        if (!read) return field;
        field = checked_address(field, expr, expr.at);
        return discard ? empty_scalar() : load_place(field, expr);
      }
      auto *value = emit_expression(*expr.children[0]);
      if (!reachable()) return nullptr;
      const auto &type = checked_.at(expr.children[0].get()).type;
      auto *payload = unpack(value, type);
      const unsigned index = expr.text == "category" ? 0 : expr.text == "site" ? 1 : expr.text == "requested" ? 2 : expr.text == "available" ? 3 : 4;
      auto *field = b_.CreateExtractValue(payload, index);
      if (expr.text == "cause_count") field = b_.CreateCall(runtime("cod_lang_exception_count", b_.getInt64Ty(), {b_.getInt64Ty()}), {field});
      return pack(field, Type::uint64);
    }
    case K::region_binding:
      return acquire_region(expr);
    case K::exception_binding:
      return receive_exception_holder(expr);
    case K::make_exception:
      return discard_exception(expr);
    case K::native_handle_binding: {
      if (expr.binding == "=") {
        auto *source = emit_expression(*expr.children.front(), Projection::target);
        if (!reachable()) return nullptr;
        auto *value = b_.CreateLoad(native_handle_pair(), source);
        b_.CreateStore(value, storage_.at(fact.place.value()));
        return empty_scalar();
      }
      return acquire_native_handle(expr, fact.place.value(), fact.native_acquisition.value());
    }
    case K::make_borrowed_handle:
      return emit_attached_native_handle(expr);
    case K::make_object: {
      if (fact.array_allocator) return emit_array_construction(expr, discard);
      if (fact.regional_sum_holder) {
        const auto constructor = fact.sum_constructor.value();
        auto *owner = load(storage_.at(*fact.regional_sum_holder));
        const auto &layout = sum_layout(constructor.sum);
        auto *address = reserve_regional_row(owner, layout.storage, expr.at);
        const auto tag = layout.alternatives.at(constructor.alternative).tag;
        b_.CreateStore(integer(tag), b_.CreateStructGEP(layout.storage, address, 0));
        if (fact.invokes_regional_initializer && reachable())
          address = emit_regional_initializer(expr, address, expr.children.size());
        if (!reachable()) return nullptr;
        return discard ? empty_scalar() : address;
      }
      if (fact.sum_constructor) {
        const auto constructor = *fact.sum_constructor;
        const auto &layout = sum_layout(constructor.sum);
        auto *owner = fact.regional_storage ? selected_owner(expr) : nullptr;
        auto *address = allocate_aggregate(expr, layout.storage, owner);
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
        const auto &plan = *fact.lexical_allocation;
        auto *owner = owner_identity(plan.owner, expr.at);
        llvm::Value *value;
        if (plan.kind == Allocation::builder) {
          auto create = runtime("cod_lang_lexical_builder_create", b_.getPtrTy(),
              {b_.getPtrTy(), b_.getPtrTy(), b_.getPtrTy(), b_.getInt32Ty()});
          auto *left = operands.empty() ? null_pointer() : operands[0];
          auto *right = operands.empty() ? null_pointer() : operands[1];
          value = b_.CreateCall(create, {owner, left, right, b_.getInt32(plan.builder_inputs)});
        } else {
          const auto name = plan.kind == Allocation::builder_snapshot ? "cod_lang_lexical_builder_finish" : "cod_lang_lexical_text_create";
          auto create = runtime(name, b_.getPtrTy(), {b_.getPtrTy(), b_.getPtrTy()});
          value = b_.CreateCall(create, {owner, operands.front()});
        }
        auto *ready = block("allocation.ready"), *failed = block("allocation.failed");
        b_.CreateCondBr(b_.CreateIsNotNull(value), ready, failed);
        b_.SetInsertPoint(failed);
        fail_path(ExceptionCategory::out_of_memory, expr.at);
        b_.SetInsertPoint(ready);
        return discard ? empty_scalar() : value;
      }
      if (fact.operand_type.kind == Type::product) {
        const auto schema = fact.operand_type.identity;
        auto *owner = fact.regional_storage ? selected_owner(expr) : nullptr;
        auto *layout = owner ? regional_product_layout(schema) : product_layout(schema);
        auto *address = allocate_aggregate(expr, layout, owner);
        if (fact.callee) emit_product_initializer(expr, address, layout, owner);
        else emit_product_initializers(expr, address, owner);
        if (!reachable()) return nullptr;
        return discard ? empty_scalar() : address;
      }
      auto *address = storage_.at(*fact.place);
      if (const auto *initializer = expr.construction_initializer()) {
        auto *value = emit_expression(*initializer);
        if (!reachable()) return nullptr;
        assign_place(address, convert_result(expr, *initializer, value), expr, &CheckedOperation::operand_type);
      }
      if (!reachable()) return nullptr;
      if (!read) return address;
      return discard ? empty_scalar() : load_place(address, expr);
    }
    case K::raise: {
      if (expr.annotation) {
        const auto prepared = construct_braced_exception(expr, fact.operand_type.identity);
        if (reachable()) raise_exception(prepared.value, prepared.owner);
        return nullptr;
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
          ? take_pointer(b_, exception_holder_slots_.at(*fact.exception_owner)) : nullptr;
      raise_exception(value, owner);
      return nullptr;
    }
    case K::try_scope: return emit_finally(expr, projection, discard);
    case K::deferred: {
      auto &scope = *scope_;
      const auto action = std::ranges::find(scope.actions, &expr, &CleanupAction::action);
      if (action == scope.actions.end()) throw std::logic_error("uncollected deferred block");
      auto *record = action->activation_record;
      prepare_cleanup_record(scope, record, std::distance(scope.actions.begin(), action));
      b_.CreateStore(record, scope.head);
      return empty_scalar();
    }
    default: throw std::logic_error("unchecked expression reached LLVM emission");
    }
  }
  void emit_product_initializers(const Expression &expr, llvm::Value *address, llvm::Value *regional_owner = nullptr) {
    const auto schema_id = checked_.at(&expr).operand_type.identity;
    auto *layout = regional_owner ? regional_product_layout(schema_id) : product_layout(schema_id);
    const auto &schema = program_->schemas.at(schema_id);
    for (const auto &initializer : expr.initializer_items()) {
      const auto &input = *initializer->children.back();
      const auto &write = checked_.at(initializer.get());
      const auto index = *write.product_field;
      const auto &member = schema.fields[index];
      const bool direct = member.storage == PlaceKind::value;
      if (regional_owner && !direct) {
        auto *slot = b_.CreateStructGEP(layout, address, index);
        const bool preparations = write.allocates_scalar_field;
        [&](bool fresh) -> llvm::Value * {
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
            offset = regional_locator(regional_owner, target);
          }
          b_.CreateStore(offset, slot);
          return b_.getTrue();
        }(preparations);
      } else if (member.direct_text_place()) {
        emit_text_transfer(*initializer, b_.CreateStructGEP(layout, address, index));
      } else if (write.initialize_nested_product) {
        emit_product_initializers(input, b_.CreateStructGEP(layout, address, index));
      } else if (direct && inline_aggregate_type(member.type)) {
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
    parameters.insert(parameters.end(), layout.allocation_owners.size(), allocation_capability_);
    if (layout.value_destination()) parameters.push_back(b_.getPtrTy());
    if (layout.native_borrow_destination) parameters.push_back(b_.getPtrTy());
    if (layout.exception_result) parameters.push_back(b_.getPtrTy());
    parameters.insert(parameters.end(), layout.here_count(), here_);
    if (layout.here_group) parameters.push_back(b_.getPtrTy());
    return llvm::Function::Create(llvm::FunctionType::get(outcome_, parameters, false),
        llvm::GlobalValue::InternalLinkage, name, module_);
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
    if (layout.has_result_destination() || layout.here_formals)
      throw std::logic_error("unexpected parameter role in supplied here body");
    // The activation starts with the transfer record. Its address identifies
    // the originating call across every forwarding receiver.
    std::vector<llvm::Type *> fields;
    if (method.here_group) fields.push_back(here_transfer_);
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
    for (const auto position : reception.selected) prepared.push_back(entry->getArg(position));
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
    if (owned_failures_) release_failure_owner(body, take_pointer(body, transfer_failure_slot(body, borrowed)));
    body.CreateStore(body.getInt8(static_cast<unsigned>(HereExit::failure)), body.CreateStructGEP(here_transfer_, borrowed, 0));
    auto *plain_failure = body.CreateInsertValue(result, null_pointer(), 7);
    body.CreateStore(plain_failure, body.CreateStructGEP(here_transfer_, borrowed, 1));
    body.CreateRet(body.CreateInsertValue(llvm::Constant::getNullValue(outcome_), borrowed, 7));
  }
  void begin(llvm::Function *function, const std::vector<CheckedStorage> &storage, const std::vector<CheckedBinding> &bindings,
             const CheckedFrame &frame) {
    function_ = function; arguments_.clear(); here_arguments_.clear(); here_destination_ = nullptr;
    for (size_t i = 0; i < frame.inputs.size(); ++i) arguments_.push_back(function->getArg(i));
    b_.SetInsertPoint(block("entry")); finish_ = block("completion"); scope_ = nullptr; control_ = nullptr;
    owner_identities_.clear();
    region_controllers_ = frame.region_controllers.empty() ? nullptr : &frame.region_controllers;
    exception_holder_slots_.clear();
    exception_parameter_slots_.clear();
    result_mode_ = ResultMode::value;
    allocation_capabilities_.clear();
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
      const auto &format = storage_formats_.emplace_back(binding.type);
      llvm::Type *layout = scalar_;
      if (binding.type.kind == Type::product) layout = product_layout(binding.type.identity);
      else if (binding.type.kind == Type::sum) layout = sum_layout(binding.type.identity).storage;
      else if (binding.type.kind == Type::native_handle)
        layout = binding.type.owned ? static_cast<llvm::Type *>(b_.getInt64Ty()) : native_handle_pair();
      else if (binding.type.kind == Type::region) layout = b_.getPtrTy();
      else if (format.kind != Type::void_type) layout = native_type(format.type());
      auto *place = binding.borrowed ? nullptr : slot(layout, binding.name + ".storage");
      storage_.push_back(place);
    }
    bindings_.clear();
    for (const auto &binding : bindings) bindings_.push_back({binding.value_binding});
    for (size_t i = 0; i < frame.inputs.size(); ++i) if (frame.inputs[i].binding_slot)
      bindings_.at(storage.at(frame.inputs[i].place).binding).reference_slot = view_address(arguments_[i]);
    for (size_t i = 0; i < bindings.size(); ++i) {
      auto &binding = bindings_[i];
      const auto &declaration = bindings[i];
      if (!binding.reference_slot && !declaration.value_binding)
        binding.reference_slot = slot(b_.getPtrTy(), "reference.target");
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
        b_.CreateStore(take_pointer(b_, owner_slot), holder);
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
      if (auto *reference = bindings_.at(binding).reference_slot) b_.CreateStore(storage_.at(place), reference);
    }
  }
  void body(llvm::Function *function, const CheckedMethod &method, const CheckedInstance &layout) {
    const auto &call = call_layouts_.at(layout.method);
    if (function->arg_size() != call.parameter_count()) throw std::logic_error("native body disagrees with its parameter roles");
    begin(function, layout.storage, layout.bindings, layout.frame);
    result_mode_ = layout.result.mode;
    if (layout.result.owns_region() || call.exception_result) {
      completion_.result_owner = slot(b_.getPtrTy(), "return.owner");
      b_.CreateStore(null_pointer(), completion_.result_owner);
    }
    for (size_t i = 0; i < call.allocation_owners.size(); ++i)
      allocation_capabilities_[call.allocation_owners[i]] = function->getArg(call.allocation_owners_begin() + i);
    case_scan_request_ = method.source->case_scan_scope ? slot(b_.getInt1Ty(), "case.scan.requested") : nullptr;
    if (case_scan_request_) b_.CreateStore(b_.getFalse(), case_scan_request_);
    for (size_t i = 0; i < call.here_count(); ++i) here_arguments_.push_back(function->getArg(call.here_index() + i));
    if (call.here_group) here_destination_ = function->getArg(call.here_index() + call.here_count());
    checked_ = {&layout.operations};
    auto *requires_return = !method.regional_initializer && method.source->completion_requires_return(*layout.result.type)
        ? b_.getTrue() : nullptr;
    llvm::Value *returns_void_at_end = nullptr;
    if (!method.regional_initializer && method.source->returns_void_at_end(Type::void_type))
      returns_void_at_end = requires_return ? b_.CreateNot(requires_return) : b_.getTrue();
    call_scope({method.source->body.get()}, method.source->at, layout.deferred_blocks, requires_return,
        call.here_group ? &*layout.result.type : nullptr, returns_void_at_end);
    control_ = nullptr;
    b_.SetInsertPoint(finish_);
    for (auto i = exception_parameter_slots_.rbegin(); i != exception_parameter_slots_.rend(); ++i)
      release_failure_owner(b_, take_pointer(b_, *i));
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
    if (call.aggregate_destination) {
      auto *ready = block("aggregate.return.publish"), *done = block("aggregate.return.complete");
      b_.CreateCondBr(b_.CreateICmpEQ(load(completion_.kind), b_.getInt8(1)), ready, done);
      b_.SetInsertPoint(ready);
      b_.CreateCall(bits_transfer(*layout.result.type), {function->getArg(call.result_index()), view_address(value)});
      b_.CreateBr(done);
      b_.SetInsertPoint(done);
      value = empty_scalar();
    }
    if (completion_.result_owner) {
      auto *owner = take_pointer(b_, completion_.result_owner);
      auto *normal = b_.CreateICmpEQ(load(completion_.kind), b_.getInt8(1));
      auto *selected = b_.CreateSelect(normal, owner, null_pointer());
      if (call.exception_result) b_.CreateStore(selected, function->getArg(call.exception_result_index()));
      else value = b_.CreateInsertValue(value, b_.CreatePtrToInt(selected, b_.getInt64Ty()), 0);
      release_result_owner(b_.CreateSelect(normal, null_pointer(), owner));
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
    begin(bridge, {}, {}, {});
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
      return std::ranges::any_of(operations, [](const auto &operation) {
        const auto &fact = operation.second;
        return fact.lexical_allocation ||
            (fact.lexical_row_allocator &&
             fact.lexical_row_allocator->kind == CheckedOperation::OwnerAllocator::lexical);
      });
    };
    owns_lexical_allocations_ = std::ranges::any_of(program.instances, [&](const auto &instance) {
      return allocates_lexically(instance.operations);
    });
    protected_outcomes_ = std::ranges::any_of(program.methods, [](const auto &method) { return method.here_group && method.instance.has_value(); });
    owned_failures_ = program.exception_types.size() > exception_categories.size() ||
        std::ranges::any_of(program.instances, [](const auto &instance) { return instance.deferred_blocks != 0; });
    case_scope_outcomes_ = std::ranges::any_of(program.methods, [](const auto &method) {
      return method.source->case_scan_scope && method.instance.has_value();
    });
    if (protected_outcomes_ || owned_failures_ || case_scope_outcomes_) {
      std::vector<llvm::Type *> fields(outcome_->element_begin(), outcome_->element_end());
      fields.resize(fields.size() + protected_outcomes_ + owned_failures_, b_.getPtrTy());
      if (case_scope_outcomes_) fields.push_back(b_.getInt1Ty());
      outcome_->setBody(fields);
    }
    if (protected_outcomes_) here_transfer_->setBody({b_.getInt8Ty(), outcome_});
    std::vector<llvm::Type *> fields{b_.getInt64Ty(), b_.getInt32Ty()};
    if (program.exception_values) fields.push_back(exception_);
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
      if (!program.methods[i].scope_inputs &&
          !program.methods[i].has_higher_order_here() && program.methods[i].instance)
        retain_method(i);
    for (const auto &entry : program.native_entries) retain_method(entry.method);
    std::set<size_t> used_symbols;
    std::set<size_t> here_instances;
    const auto collect_operations = [&](const auto &operations) {
      for (const auto &[expr, operation] : operations) {
        if (operation.callee) retain_method(program.instances[*operation.callee].method);
        if (operation.scoped_effect) {
          const auto &target = program.instances[*operation.scoped_effect];
          retain_method(target.method);
          if (operation.publishes_effect)
            used_symbols.insert(target.scoped_replay_bindings.begin(), target.scoped_replay_bindings.end());
        }
        for (const auto &input : operation.here_arguments)
          if (const auto *literal = std::get_if<CheckedOperation::SuppliedHere>(&input)) {
            here_instances.insert(literal->instance);
            retain_method(program.instances[literal->instance].method);
          }
        const bool installation = expr->kind == Expression::Kind::effect || expr->kind == Expression::Kind::procedure_effect;
        if (operation.symbol && (expr->kind == Expression::Kind::symbol || (installation && operation.publishes_effect) ||
            program.symbols[*operation.symbol].live_scope))
          used_symbols.insert(*operation.symbol);
      }
    };
    for (size_t i = 0; i < pending_methods.size(); ++i) {
      const auto &method = program.methods[pending_methods[i]];
      collect_operations(program.instances.at(method.instance.value()).operations);
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
    std::map<size_t, std::set<size_t>> here_allocation_owners;
    for (size_t i = 0; i < program.methods.size(); ++i) {
      if (!emitted_methods[i]) continue;
      const auto &method = program.methods[i];
      const auto &frame = input_frame(i);
      call_layouts_[i] = {frame.inputs.size(), frame.owners.size(),
          false, false, method.here.size(), method.here_group,
          method.here_inputs() - method.here.size()};
      auto &call = call_layouts_[i];
      std::set<size_t> allocation_owners;
      {
        const auto &instance = program.instances.at(method.instance.value());
        const auto &result = instance.result;
        if (!instance.allocator_roles)
          implementation_gap(method.source->at, "emission without completed allocator requirements");
        const auto &required = *instance.allocator_roles;
        allocation_owners.insert(required.begin(), required.end());
        call.callable_destination |= !result.borrowed() && result.type->kind == Type::procedure;
        call.aggregate_destination = result.mode == ResultMode::value && inline_aggregate_type(*result.type);
        call.native_borrow_destination |= result.native_borrow();
        call.exception_result |= result.owns_exception();
      }
      if (!method.here.empty() && !method.scope_inputs) {
        auto &owners = here_allocation_owners[method.definition];
        owners.insert(allocation_owners.begin(), allocation_owners.end());
      } else call.allocation_owners.assign(allocation_owners.begin(), allocation_owners.end());
    }
    methods_.resize(program.methods.size());
    for (size_t i = 0; i < program.methods.size(); ++i) {
      if (!emitted_methods[i]) continue;
      const auto &method = program.methods[i];
      auto &layout = call_layouts_[i];
      // An observed supplier return can select either profile at the same call site.
      if (!method.here.empty() && !method.scope_inputs) {
        const auto &owners = here_allocation_owners.at(method.definition);
        layout.allocation_owners.assign(owners.begin(), owners.end());
      }
      auto prefix = "cod.method." + std::to_string(method.definition) + "." + method.source->name;
      if (i != method.definition) {
        prefix += method.interpretation ? ".interpretation." : ".instance.";
        prefix += std::to_string(i);
      }
      if (!method.instance || !method.type_parameters.empty())
        throw std::logic_error("seed emission requires one concrete native frame per method");
      methods_[i] = declare(prefix + ".native", layout);
    }
    for (const auto instance : here_instances) declare_here_entry(instance);
    for (size_t i = 0; i < program.methods.size(); ++i) {
      if (!emitted_methods[i]) continue;
      const auto &method = program.methods[i];
      body(methods_[i], method, program.instances.at(method.instance.value()));
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
