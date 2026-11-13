#pragma once

#include <shilos/dbmr.hh>
#include <array>
#include <algorithm>
#include <string_view>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>
#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>

using namespace shilos;
using I = std::int64_t;
using U = std::uint64_t;
using TypeId = std::uint64_t;
using Byte = std::uint8_t;
#include "type_ids.inc"

struct Failure { I code; };
static thread_local std::vector<I> events;
template <typename... T> static void trace(I code, T... values) {
  events.push_back(code);
  (events.push_back(I(values)), ...);
}

struct Choice {
  TypeId evidence = 0;
  regional_ptr<Byte> payload;
  template <typename R> explicit Choice(memory_region<R> &) {}
  Choice(const Choice &) = delete;
  Choice(Choice &&) = delete;
};
struct Word {
  I value = 0, site = 0;
  template <typename R> Word(memory_region<R> &, I v = 0, I s = 0) : value(v), site(s) {}
  Word(const Word &) = delete;
  Word(Word &&) = delete;
};
struct alignas(16) RegionalSlot {
  I region_symbol = 1, value = 0, site = 0;
  template <typename R> RegionalSlot(memory_region<R> &, I v = 0, I s = 0) : value(v), site(s) {}
  RegionalSlot(const RegionalSlot &) = delete;
  RegionalSlot(RegionalSlot &&) = delete;
};
struct Example {
  Choice x, y;
  I site;
  template <typename R> Example(memory_region<R> &mr, I s = 0) : x(mr), y(mr), site(s) {}
  Example(const Example &) = delete;
  Example(Example &&) = delete;
};
struct Other {
  I site;
  Choice y, x;
  template <typename R> Other(memory_region<R> &mr, I s = 0) : site(s), y(mr), x(mr) {}
  Other(const Other &) = delete;
  Other(Other &&) = delete;
};
struct Policy {
  I increment = 1;
  template <typename R> explicit Policy(memory_region<R> &) {}
  Policy(const Policy &) = delete;
  Policy(Policy &&) = delete;
};
// Fixture root metadata, stored once; not a second tag on each value.
struct Binding {
  TypeId identity = 0;
  std::array<char,64> semantic_key{};
  U key_size = 0, size = 0, align = 0;
};
struct Root {
  static constexpr UUID TYPE_UUID{"c6e489da-94b7-466a-b246-33b29ee32faa"};
  U binding_count = 0;
  std::array<Binding,8> bindings{};
  Choice object, destination, result;
  Policy policy;
  explicit Root(memory_region<Root> &mr) : object(mr), destination(mr), result(mr), policy(mr) {}
  Root(const Root &) = delete;
  Root(Root &&) = delete;
};

// Reseatable host borrow, selectively reused from the archived experiment.
// Reconstructing a global_ptr preserves its actual owner; no regional value moves.
template <typename T> struct Borrow {
  std::optional<global_ptr<T, Root>> pointer;
  Borrow() = default;
  Borrow(global_ptr<T, Root> p) : pointer(p) {}
  Borrow(const Borrow &) = default;
  Borrow &operator=(const Borrow &other) {
    if (this != &other) { pointer.reset(); if (other.pointer) pointer.emplace(*other.pointer); }
    return *this;
  }
  const T *operator->() const { return pointer->get(); }
};
struct View {
  TypeId evidence;
  Borrow<Byte> payload;
};
static const Descriptor &descriptor(TypeId evidence) {
  for (const auto &d : descriptors) if (d.identity == evidence) return d;
  throw Failure{910};
}
static const Descriptor &named(const std::string &name) {
  for (const auto &d : descriptors) if (name == d.name) return d;
  throw Failure{911};
}
static void prepare_catalog(memory_region<Root> &mr) {
  mr.root()->binding_count = std::size(root_evidence);
  for (std::size_t i = 0; i < std::size(root_evidence); ++i) {
    const auto &d = descriptor(root_evidence[i]);
    auto &b = mr.root()->bindings[i];
    const std::string_view key(d.semantic_key);
    assert(key.size() <= b.semantic_key.size());
    b.identity=d.identity; b.size=d.size; b.align=d.align; b.key_size=key.size();
    std::memcpy(b.semantic_key.data(),key.data(),key.size());
  }
}
static void validate_catalog(const memory_region<Root> &mr) {
  if (mr.root()->binding_count != std::size(root_evidence)) throw Failure{938};
  std::array<bool,8> seen{};
  for (const auto &b : mr.root()->bindings) {
    const auto &d = descriptor(b.identity);
    if (b.key_size > b.semantic_key.size() ||
        std::string_view(b.semantic_key.data(),b.key_size) != d.semantic_key ||
        b.size != d.size || b.align != d.align || d.abstract) throw Failure{938};
    auto it = std::find(std::begin(root_evidence),std::end(root_evidence),b.identity);
    if (it == std::end(root_evidence) || seen[it-std::begin(root_evidence)]) throw Failure{938};
    seen[it-std::begin(root_evidence)] = true;
  }
}
static View get_choice(global_ptr<Choice, Root> choice) {
  return {choice->evidence, choice.get(&Choice::payload)};
}
static void store_choice(global_ptr<Choice, Root> choice, View value) {
  const auto &d = descriptor(value.evidence);
  if (d.abstract || !value.payload.pointer) throw Failure{912};
  if (&choice.region() != &value.payload.pointer->region()) throw Failure{913};
  // All checks/allocations precede this no-callback, nonthrowing commit.
  // Sequential execution only: this is not a concurrent publication protocol.
  choice.set(&Choice::payload, *value.payload.pointer);
  choice->evidence = value.evidence;
}
template <typename T> static global_ptr<T, Root> object(View value) {
  return value.payload.pointer->region().cast_ptr(reinterpret_cast<T *>(value.payload.pointer->get()));
}
template <typename T> static View field(View value, Choice T::*member) {
  auto parent = object<T>(value);
  return get_choice(parent.region().cast_ptr(&(parent.get()->*member)));
}
static I word_at(View value, std::size_t offset) {
  I result;
  std::memcpy(&result, value.payload.pointer->get() + offset, sizeof result);
  return result;
}
static View make_word(memory_region<Root> &mr, TypeId evidence, I value, I site) {
  const auto &d = descriptor(evidence);
  if (d.abstract || d.size != sizeof(Word) || d.align != alignof(Word)) throw Failure{914};
  auto p = mr.create<Word>(value, site);
  return {evidence, mr.cast_ptr(reinterpret_cast<Byte *>(p.get()))};
}
static View make_destination(memory_region<Root> &mr, TypeId evidence, I value, I site) {
  if (evidence == T_LocalSlot) return make_word(mr, evidence, value, site);
  if (evidence != T_RegionalSlot) throw Failure{915};
  auto p = mr.create<RegionalSlot>(value, site);
  return {evidence, mr.cast_ptr(reinterpret_cast<Byte *>(p.get()))};
}
static View make_plan(memory_region<Root> &mr, TypeId evidence, View x, View y) {
  if (x.evidence != T_IntegerLiteral && x.evidence != T_BooleanLiteral) throw Failure{916};
  if (y.evidence != T_LocalSlot && y.evidence != T_RegionalSlot) throw Failure{917};
  if (evidence == T_Example) {
    auto p = mr.create<Example>(I{101});
    store_choice(mr.cast_ptr(&p->x), x); store_choice(mr.cast_ptr(&p->y), y);
    return {evidence, mr.cast_ptr(reinterpret_cast<Byte *>(p.get()))};
  }
  if (evidence == T_Other) {
    auto p = mr.create<Other>(I{102});
    store_choice(mr.cast_ptr(&p->x), x); store_choice(mr.cast_ptr(&p->y), y);
    return {evidence, mr.cast_ptr(reinterpret_cast<Byte *>(p.get()))};
  }
  throw Failure{918};
}
static View summary(memory_region<Root> &mr, TypeId evidence, I value) {
  auto result = make_word(mr, evidence, value, 200);
  store_choice(mr.cast_ptr(&mr.root()->result), result);
  return get_choice(mr.cast_ptr(&mr.root()->result));
}

struct FamilyValue { I interpretation, bias; Borrow<Policy> policy; };
static thread_local std::optional<FamilyValue> current_family;
static thread_local std::optional<View> current_input;
static thread_local I unrelated_binding = 77;
static void install(FamilyValue value) { current_family.emplace(value); trace(200, value.interpretation); }
static void restore(FamilyValue value) { current_family.emplace(value); trace(201, value.interpretation); }
static I captured_adjustment(const FamilyValue &f) {
  trace(220, f.bias, f.policy->increment);
  return f.bias + f.policy->increment;
}

// Scoped native resources. Primitive lifecycle operations are adapted from
// the archived LLVM fixture, without its IR, versioner or TypeTerm workload.
class Native {
  LLVMContextRef context_ = nullptr;
  LLVMModuleRef module_ = nullptr;
  LLVMBuilderRef builder_ = nullptr;
  I failure_;
  std::filesystem::path output_;
  std::array<bool, 4> live_{};
public:
  I acquired = 0, released = 0;
  Native(I failure, const std::filesystem::path &path) : failure_(failure), output_(path) {}
  Native(const Native &) = delete;
  Native &operator=(const Native &) = delete;
  void acquire(I code) {
    assert(code >= 1 && code <= 3 && !live_[code]); trace(500 + code);
    if (failure_ == code) throw Failure{904};
    if (code == 1) context_ = LLVMContextCreate();
    if (code == 2) { assert(live_[1]); module_ = LLVMModuleCreateWithNameInContext("micro_dispatch", context_); }
    if (code == 3) { assert(live_[2]); builder_ = LLVMCreateBuilderInContext(context_); }
    live_[code] = true; ++acquired;
  }
  void release(I code) {
    assert(live_[code]);
    if (code == 3) { LLVMDisposeBuilder(builder_); builder_ = nullptr; }
    if (code == 2) { assert(!live_[3]); LLVMDisposeModule(module_); module_ = nullptr; }
    if (code == 1) { assert(!live_[2]); LLVMContextDispose(context_); context_ = nullptr; }
    live_[code] = false; ++released; trace(600 + code);
  }
  void emit(I result) {
    assert(live_[3]); trace(700, result);
    auto ty = LLVMInt64TypeInContext(context_);
    auto fn = LLVMAddFunction(module_, "answer", LLVMFunctionType(ty, nullptr, 0, false));
    LLVMPositionBuilderAtEnd(builder_, LLVMAppendBasicBlockInContext(context_, fn, "entry"));
    if (failure_ != 4) LLVMBuildRet(builder_, LLVMConstInt(ty, U(result), true));
    char *message = nullptr;
    const bool bad = LLVMVerifyModule(module_, LLVMReturnStatusAction, &message);
    if (message) LLVMDisposeMessage(message);
    char *text = LLVMPrintModuleToString(module_);
    std::ofstream file(output_); file << text; assert(file.good()); LLVMDisposeMessage(text);
    trace(bad ? 711 : 710);
    if (bad) throw Failure{901};
  }
  ~Native() { assert(acquired == released && !context_ && !module_ && !builder_); }
};

struct Input { TypeId plan, x, y, destination; I xv, yv, dv; };
static Input read_input(const std::filesystem::path &path) {
  std::ifstream f(path);
  std::string p, x, y, d; I xv, yv, dv; TypeId supplied;
  if (!(f >> p >> x >> xv >> y >> yv >> d >> dv >> supplied)) throw Failure{920};
  const auto &pd = named(p); const auto &xd = named(x); const auto &yd = named(y); const auto &dd = named(d);
  if (supplied != xd.identity) throw Failure{921};
  // This numerical fixture admits a bounded value domain; it does not select
  // the language's general overflow policy.
  for (I value : {xv, yv, dv}) if (value < -1000000 || value > 1000000) throw Failure{924};
  if (xd.identity == T_BooleanLiteral && xv != 0 && xv != 1) throw Failure{922};
  if ((pd.identity != T_Example && pd.identity != T_Other) ||
      (xd.identity != T_IntegerLiteral && xd.identity != T_BooleanLiteral) ||
      (yd.identity != T_LocalSlot && yd.identity != T_RegionalSlot) ||
      (dd.identity != T_LocalSlot && dd.identity != T_RegionalSlot)) throw Failure{923};
  return {pd.identity, xd.identity, yd.identity, dd.identity, xv, yv, dv};
}
static void construct_input(memory_region<Root> &mr, const Input &in) {
  auto x = make_word(mr, in.x, in.xv, 11);
  auto y = make_destination(mr, in.y, in.yv, 12);
  auto p = make_plan(mr, in.plan, x, y);
  store_choice(mr.cast_ptr(&mr.root()->object), p);
  store_choice(mr.cast_ptr(&mr.root()->destination), make_destination(mr, in.destination, in.dv, 13));
}
