//===--- lang_checked.hh - LLVM-independent scalar obligations ------===//
#pragma once

#include "lang_syntax.hh"
#include "lang_diagnostics.hh"
#include "lang_exceptions.hh"
#include "lang_completion.hh"

#include <charconv>
#include <bit>
#include <cmath>
#include <cstdint>
#include <deque>
#include <limits>
#include <map>

namespace cod::language {

[[noreturn]] inline void implementation_gap(const Location &at, const std::string &detail) {
  fail(at, "micro-dispatch implementation gap: " + detail + "; this is not a language restriction");
}

inline bool scalar_type(const Type &type) { return numeric(type) || type == Type::boolean; }
inline bool bits_type(const Type &type) { return scalar_type(type) || type.kind == Type::exception; }
inline Type exception_type(size_t category) { return {Type::exception, exception_categories.at(category).name, category}; }
inline bool accepts_exception(const Type &source, const Type &target) {
  return source.kind == Type::exception && target.kind == Type::exception && (!target.identity || source == target);
}
inline bool transfers_bits(const Type &source, const Type &target) { return widens(source, target) || accepts_exception(source, target); }
inline std::set<size_t> exception_possibilities(const Type &type) {
  if (type.kind != Type::exception) return {};
  if (type.identity) return {type.identity};
  std::set<size_t> result;
  for (size_t i = 1; i < exception_categories.size(); ++i) result.insert(i);
  return result;
}

struct CheckedOperation {
  Type type;
  Type operand_type;
  std::optional<size_t> callee;
  std::optional<size_t> place;
  std::optional<size_t> binding;
  std::optional<size_t> accumulator;
  bool read_only = false;
  bool view = false;
  std::uint64_t integer = 0;
  bool minimum_literal = false;
  bool initializes = false;
  std::set<size_t> exceptions;
  std::set<size_t> captures;
  bool cause_at = false;
};

struct CheckedStorage {
  std::string name;
  Type type;
  size_t scope;
  bool read_only;
};

struct CheckedBinding {
  size_t place;
  bool read_only;
  bool value_binding;
  bool fixed;
};

struct LexicalState {
  struct Scope { size_t id; std::map<std::string, size_t> bindings; bool declarations = true; };
  std::vector<Scope> scopes{{0, {}}};
  std::set<size_t> bindings_available, available, initialized;
  std::map<size_t, std::set<size_t>> input_initialized{{0, {}}};
  std::map<size_t, std::uint64_t> requirements;
  std::map<size_t, std::set<size_t>> exceptions;
  std::map<size_t, size_t> queues{{0, 0}};
  std::uint64_t inputs = 1;
  bool live = true;

  std::optional<size_t> lookup(const std::string &name) const {
    for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope)
      if (const auto found = scope->bindings.find(name); found != scope->bindings.end()) return found->second;
    return {};
  }
};

// Substitutions are checking obligations, not semantic methods or activation records.
struct CheckedInstance {
  size_t method;
  std::vector<Type> arguments;
  Type result;
  std::map<const Expression *, CheckedOperation> operations;
  bool complete = false;
  std::vector<CheckedStorage> storage;
  std::vector<CheckedBinding> bindings;
};
struct CheckedMethod {
  const Form *source;
  std::vector<size_t> instances;
  std::vector<Type> parameters;
  Type result;
  std::set<size_t> throws_categories;
};
struct CheckedProgram {
  std::vector<CheckedMethod> methods;
  std::deque<CheckedInstance> instances;
  std::map<const Expression *, CheckedOperation> entry;
  std::vector<const Expression *> initializers;
  std::vector<WarningDiagnostic> warnings;
  std::vector<CheckedStorage> entry_storage;
  std::vector<CheckedBinding> entry_bindings;
  bool exception_values = false;
};

class ScalarChecker {
  CheckedProgram program_;
  std::map<std::string, std::vector<size_t>> families_;
  std::set<std::string> print_symbols_;
  std::set<std::string> module_values_;
  std::map<std::string, Type> exception_types_;
  std::set<size_t> warned_missing_return_;
  std::set<const Expression *> warned_shadow_;
  LexicalState lexical_;
  std::vector<CheckedStorage> *storage_ = nullptr;
  std::vector<CheckedBinding> *bindings_ = nullptr;
  std::map<size_t, std::vector<size_t>> scope_storage_;
  struct AbruptPath { LexicalState state; std::set<size_t> exceptions; Location at; Completion completion{2, false}; };
  struct AbruptFlow { size_t depth; std::vector<AbruptPath> paths; };
  AbruptFlow *abrupt_ = nullptr;
  std::optional<size_t> caught_;
  std::set<size_t> catch_bindings_;
  std::map<size_t, std::map<size_t, std::uint64_t>> caught_inputs_;
  const CheckedMethod *checking_method_ = nullptr;
  std::set<size_t> loop_controls_, delayed_controls_;
  struct CatchCapture {
    CatchCapture *parent;
    std::set<size_t> bindings, required;
  };
  CatchCapture *captures_ = nullptr;
  CleanupEquations cleanup_;
  size_t cleanup_scope_ = 0;
  const Expression *prepared_call_ = nullptr;
  std::set<size_t> deferred_inputs_;
  std::vector<Location> read_sites_;
  bool delayed_ = false;
  size_t next_scope_ = 1;
  static constexpr size_t instance_limit = 4096;

  LexicalState::Scope &declaration_scope(const std::string &name, const Location &at) {
    auto selected = lexical_.scopes.rbegin();
    while (!selected->declarations) {
      if (selected->bindings.contains(name)) fail(at, "duplicate declaration of catch parameter '" + name + "'");
      ++selected;
    }
    auto &scope = *selected;
    if (scope.bindings.contains(name)) fail(at, "duplicate declaration of '" + name + "' in one lexical scope");
    return scope;
  }
  size_t bind(const std::string &name, size_t place, bool read_only, bool value_binding, bool fixed, const Location &at) {
    auto &scope = declaration_scope(name, at);
    const auto id = bindings_->size();
    bindings_->push_back({place, read_only, value_binding, fixed});
    scope.bindings.emplace(name, id);
    lexical_.bindings_available.insert(id);
    return id;
  }
  size_t declare(const std::string &name, const Type &type, bool read_only, bool value_binding, const Location &at) {
    const auto scope = declaration_scope(name, at).id;
    const auto id = storage_->size();
    storage_->push_back({name, type, scope, read_only});
    scope_storage_[scope].push_back(id);
    bind(name, id, read_only, value_binding, false, at);
    lexical_.available.insert(id);
    return id;
  }
  static std::set<size_t> intersect(const std::set<size_t> &a, const std::set<size_t> &b) {
    std::set<size_t> result;
    for (const auto id : a) if (b.contains(id)) result.insert(id);
    return result;
  }
  static std::vector<size_t> inputs(std::uint64_t mask) {
    std::vector<size_t> result;
    for (size_t i = 0; i <= exception_categories.size(); ++i) if (mask & (std::uint64_t{1} << i)) result.push_back(i);
    return result;
  }
  void initialize(size_t id) {
    lexical_.initialized.insert(id);
    for (const auto input : inputs(lexical_.inputs)) lexical_.input_initialized[input].insert(id);
  }
  static void refresh_initialized(LexicalState &state) {
    bool first = true;
    for (const auto input : inputs(state.inputs)) {
      const auto &known = state.input_initialized.at(input);
      state.initialized = first ? known : intersect(state.initialized, known);
      first = false;
    }
  }
  void join(const LexicalState &a, bool a_exits, const LexicalState &b, bool b_exits, const Location &at) {
    a_exits = a_exits || !a.live;
    b_exits = b_exits || !b.live;
    lexical_ = a;
    for (size_t i = 0; i < b.scopes.size(); ++i)
      for (const auto &[name, id] : b.scopes[i].bindings) {
        auto [found, inserted] = lexical_.scopes[i].bindings.emplace(name, id);
        if (!inserted && found->second != id) fail(at, "duplicate declaration of '" + name + "' in one lexical scope");
      }
    lexical_.bindings_available = a_exits ? b.bindings_available : b_exits ? a.bindings_available : intersect(a.bindings_available, b.bindings_available);
    lexical_.available = a_exits ? b.available : b_exits ? a.available : intersect(a.available, b.available);
    lexical_.initialized = a_exits ? b.initialized : b_exits ? a.initialized : intersect(a.initialized, b.initialized);
    lexical_.input_initialized = a_exits ? b.input_initialized : a.input_initialized;
    lexical_.requirements = a_exits ? b.requirements : a.requirements;
    if (!a_exits && !b_exits) {
      for (const auto input : inputs(b.inputs)) {
        auto &known = lexical_.input_initialized[input];
        known = a.inputs & (std::uint64_t{1} << input) ? intersect(known, b.input_initialized.at(input)) : b.input_initialized.at(input);
      }
      for (const auto &[read, mask] : b.requirements) lexical_.requirements[read] |= mask;
    }
    lexical_.exceptions = a_exits ? b.exceptions : a.exceptions;
    if (!a_exits && !b_exits) for (const auto &[id, categories] : b.exceptions)
      lexical_.exceptions[id].insert(categories.begin(), categories.end());
    lexical_.queues = a_exits ? b.queues : a.queues;
    if (!a_exits && !b_exits) for (const auto &[scope, queue] : b.queues)
      if (lexical_.queues.at(scope) != queue)
        lexical_.queues[scope] = cleanup_.join(cleanup_.known(lexical_.queues.at(scope), a.initialized), cleanup_.known(queue, b.initialized));
    lexical_.inputs = a_exits ? b.inputs : b_exits ? a.inputs : a.inputs | b.inputs;
    lexical_.live = !a_exits || !b_exits;
    refresh_initialized(lexical_);
  }
  void retire_scope(LexicalState &state) {
    const auto &scope = state.scopes.back();
    for (const auto &[name, id] : scope.bindings) state.bindings_available.erase(id);
    if (const auto owned = scope_storage_.find(scope.id); owned != scope_storage_.end()) for (const auto id : owned->second) {
      state.available.erase(id);
      state.initialized.erase(id);
      for (auto &[input, initialized] : state.input_initialized) initialized.erase(id);
      state.exceptions.erase(id);
    }
    state.queues.erase(scope.id);
    state.scopes.pop_back();
  }
  void leave_scope() {
    if (const auto owned = scope_storage_.find(lexical_.scopes.back().id); owned != scope_storage_.end())
      for (const auto id : owned->second) loop_controls_.erase(id);
    retire_scope(lexical_);
  }
  bool capture_catch(size_t id) {
    bool captured = false;
    for (auto *frame = captures_; frame; frame = frame->parent)
      if (frame->bindings.contains(id)) { frame->required.insert(id); captured = true; }
    return captured;
  }
  void note_abrupt(const Location &at, std::set<size_t> exceptions = {}, Completion completion = {2, false}) {
    if (!lexical_.live) return;
    if (!exceptions.empty()) completion.reason = 3 + *exceptions.begin();
    if (!abrupt_) {
      for (const auto category : exceptions)
        if (exception_categories.at(category).checked && (!checking_method_ || !checking_method_->throws_categories.contains(category)))
          fail(at, "checked exception '" + std::string(exception_categories[category].name) + "' requires a covering catch or throws declaration");
      return;
    }
    auto state = lexical_;
    while (state.scopes.size() > abrupt_->depth) retire_scope(state);
    abrupt_->paths.push_back({std::move(state), std::move(exceptions), at, completion});
  }
  CheckedOperation finish_scope(CheckedOperation result, AbruptFlow &flow, AbruptFlow *outer,
                                size_t scope, const Location &at) {
    const auto body = lexical_;
    if (result.type != Type::never && lexical_.live) flow.paths.push_back({lexical_, {}, at, {0, false}});
    std::vector<LexicalState> normals;
    std::vector<AbruptPath> exits;
    std::optional<Type> normal_type;
    for (const auto &path : flow.paths) {
      const auto queue = path.state.queues.at(scope);
      std::vector<Completion> inputs;
      if (path.exceptions.empty()) inputs.push_back(path.completion);
      else for (const auto category : path.exceptions) inputs.push_back({3 + category, path.completion.returned});
      for (const auto input : inputs) for (const auto &[encoded, effects] : cleanup_.apply(queue, input))
          for (const auto origin : ScalarChecker::inputs(path.state.inputs)) {
        const auto completion = Completion::from(encoded);
        auto state = path.state;
        state.inputs = std::uint64_t{1} << origin;
        state.initialized = path.state.input_initialized.at(origin);
        for (auto &[read, mask] : state.requirements) mask &= state.inputs;
        for (const auto read : effects.reads) {
          const auto binding = cleanup_.binding(read);
          if (state.initialized.contains(binding)) continue;
          if (!delayed_ || !deferred_inputs_.contains(binding))
            fail(read_sites_.at(read), "cannot prove initialization of '" + (*storage_)[binding].name + "' for deferred read");
          state.requirements[read] |= state.inputs;
        }
        for (const auto id : effects.initialized) if (state.available.contains(id)) state.initialized.insert(id);
        state.input_initialized = {{origin, state.initialized}};
        if (cleanup_.has_actions(queue)) for (auto &[id, categories] : state.exceptions)
          if (!(*storage_)[id].read_only) categories = exception_possibilities((*storage_)[id].type);
        state.queues[scope] = 0;
        if (!completion.reason) {
          const auto type = path.completion.reason ? Type{Type::void_type} : result.type;
          if (normal_type && *normal_type != type)
            fail(at, "scope recovery has no original value; use an explicit return in the recovering handler");
          normal_type = type;
          normals.push_back(std::move(state));
        } else exits.push_back({std::move(state), completion.reason >= 3 ? std::set<size_t>{completion.reason - 3} : std::set<size_t>{}, path.at, completion});
      }
    }
    abrupt_ = outer;
    for (const auto &path : exits) {
      lexical_ = path.state;
      note_abrupt(path.at, path.exceptions, path.completion);
    }
    lexical_ = body;
    if (normals.empty()) { lexical_.live = false; result.type = Type::never; }
    else {
      join(body, true, normals.front(), false, at);
      for (size_t i = 1; i < normals.size(); ++i) { const auto previous = lexical_; join(previous, false, normals[i], false, at); }
      result.type = *normal_type;
      if (result.type == Type::void_type) { result.place.reset(); result.binding.reset(); result.view = result.read_only = false; result.exceptions.clear(); }
    }
    return result;
  }
  static std::optional<std::uint64_t> constant_integer(const Expression &expr,
      const std::map<const Expression *, CheckedOperation> &operations) {
    const auto &fact = operations.at(&expr);
    if (expr.kind == Expression::Kind::integer) return fact.integer;
    if (fact.minimum_literal) return std::uint64_t{1} << 63;
    if (expr.kind == Expression::Kind::unary && (expr.text == "+" || expr.text == "-"))
      if (const auto value = constant_integer(*expr.children.front(), operations))
        return expr.text == "-" ? std::uint64_t{0} - *value : *value;
    return {};
  }
  Type resolve_local_type(const Type &type, const CheckedInstance *instance, const Location &at) const {
    if (instance) {
      const auto &method = *program_.methods[instance->method].source;
      if (const auto index = binder(method, type)) return (*match(program_.methods[instance->method], instance->arguments))[*index];
    }
    const auto resolved = resolve_type(type);
    if (!bits_type(resolved)) implementation_gap(at, "non-bits local storage type '" + type_name(type) + "'");
    return resolved;
  }
  Type resolve_type(const Type &type) const {
    if (type.kind == Type::named) if (const auto found = exception_types_.find(type.name); found != exception_types_.end()) return found->second;
    return type;
  }

  static std::optional<size_t> binder(const Form &method, const Type &type) {
    if (type.kind != Type::named) return {};
    for (size_t i = 0; i < method.type_parameters.size(); ++i)
      if (method.type_parameters[i].name == type.name) return i;
    return {};
  }
  static void signature_type(const Form &method, const Type &type, const Location &at, bool result = false) {
    if (bits_type(type) || binder(method, type) || (result && type == Type::void_type)) return;
    implementation_gap(at, "non-scalar signature type '" + type_name(type) + "'");
  }
  static std::optional<std::vector<Type>> match(const CheckedMethod &method, const std::vector<Type> &arguments) {
    if (method.parameters.size() != arguments.size()) return {};
    std::vector<Type> substitutions(method.source->type_parameters.size());
    for (size_t i = 0; i < arguments.size(); ++i) {
      const auto &parameter = method.parameters[i];
      if (const auto index = binder(*method.source, parameter)) {
        auto &bound = substitutions[*index];
        if (bound != Type::void_type && bound != arguments[i]) return {};
        bound = arguments[i];
      } else if (parameter != arguments[i] && !accepts_exception(arguments[i], parameter)) return {};
    }
    return substitutions;
  }
  // Signature containment includes repeated binders; layout equality is irrelevant.
  static bool implies(const CheckedMethod &narrow, const CheckedMethod &broad) {
    if (narrow.parameters.size() != broad.parameters.size()) return false;
    const auto same_narrow = [&](size_t a, size_t b) {
      const auto x = binder(*narrow.source, narrow.parameters[a]);
      const auto y = binder(*narrow.source, narrow.parameters[b]);
      return x && y ? x == y : !x && !y && narrow.parameters[a] == narrow.parameters[b] &&
          !(narrow.parameters[a].kind == Type::exception && !narrow.parameters[a].identity);
    };
    for (size_t i = 0; i < broad.parameters.size(); ++i) {
      const auto broad_binder = binder(*broad.source, broad.parameters[i]);
      if (!broad_binder) {
        if (binder(*narrow.source, narrow.parameters[i]) || (narrow.parameters[i] != broad.parameters[i] && !accepts_exception(narrow.parameters[i], broad.parameters[i])))
          return false;
      } else {
        for (size_t j = 0; j < i; ++j)
          if (binder(*broad.source, broad.parameters[j]) == broad_binder && !same_narrow(i, j)) return false;
      }
    }
    return true;
  }
  size_t demand(size_t method, std::vector<Type> arguments, const Location &at) {
    auto &definition = program_.methods[method];
    for (size_t i = 0; i < arguments.size(); ++i)
      if (accepts_exception(arguments[i], definition.parameters[i])) arguments[i] = definition.parameters[i];
    for (const auto id : definition.instances)
      if (program_.instances[id].arguments == arguments) return id;
    if (program_.instances.size() == instance_limit)
      fail(at, "compiler resource limit: 4096 scalar checking obligations while preparing '" + definition.source->name + "'");
    const auto substitutions = match(definition, arguments);
    if (!substitutions) throw std::logic_error("unchecked method demand");
    auto result = definition.result;
    if (const auto index = binder(*definition.source, result)) {
      result = (*substitutions)[*index];
      if (result == Type::void_type)
        implementation_gap(at, "result type parameter is not determined by the scalar arguments");
    }
    const auto id = program_.instances.size();
    program_.instances.push_back({method, arguments, result, {}, false, {}});
    definition.instances.push_back(id);
    return id;
  }
  size_t select(const Expression &expr, const std::vector<Type> &arguments) {
    const auto family = families_.find(expr.text);
    if (family == families_.end()) fail(expr.at, "unknown callable '" + expr.text + "'");
    std::vector<size_t> applicable;
    for (const auto method : family->second)
      if (match(program_.methods[method], arguments)) applicable.push_back(method);
    std::vector<size_t> best;
    for (const auto candidate : applicable) {
      const auto &definition = program_.methods[candidate];
      const bool dominated = std::any_of(applicable.begin(), applicable.end(), [&](size_t other) {
        const auto &alternative = program_.methods[other];
        return implies(alternative, definition) && !implies(definition, alternative);
      });
      if (!dominated) best.push_back(candidate);
    }
    if (best.empty()) fail(expr.at, "no applicable method for '" + expr.text + "' (argument types or same-T relation do not match)");
    if (best.size() != 1) fail(expr.at, "ambiguous methods for '" + expr.text + "'");
    return demand(best.front(), arguments, expr.at);
  }
  CheckedOperation check_guarded(const Expression &expr, CheckedInstance *instance,
      std::map<const Expression *, CheckedOperation> &operations, bool read = true) {
    const auto before = lexical_;
    auto *outer = abrupt_;
    AbruptFlow incoming{lexical_.scopes.size(), {}};
    if (expr.kind == Expression::Kind::deferred) {
      for (size_t category = 1; category < exception_categories.size(); ++category) {
        auto state = lexical_; state.inputs = std::uint64_t{1} << category;
        state.input_initialized = {{category, state.initialized}};
        incoming.paths.push_back({std::move(state), {category}, expr.at, Completion::failure(category)});
      }
      auto state = lexical_; state.inputs = std::uint64_t{1} << exception_categories.size();
      state.input_initialized = {{exception_categories.size(), state.initialized}};
      incoming.paths.push_back({std::move(state), {}, expr.at, {2, false}});
    }
    abrupt_ = &incoming;
    const auto body = check(*expr.children[0], instance, operations, read);
    const auto body_state = lexical_;
    std::vector<AbruptPath> control_inputs;
    for (const auto &path : incoming.paths) if (path.exceptions.empty()) control_inputs.push_back(path);
    std::vector<LexicalState> normals;
    std::vector<const CheckedOperation *> views;
    if (body != Type::never) { normals.push_back(body_state); views.push_back(&operations.at(expr.children[0].get())); }
    Type result_type = body;
    auto result_exceptions = body == Type::never ? std::set<size_t>{} : operations.at(expr.children[0].get()).exceptions;
    std::set<size_t> handled;
    AbruptFlow outgoing{before.scopes.size(), {}};
    const Expression *finalizer = nullptr;
    for (size_t i = 1; i < expr.children.size(); ++i) {
      const auto &handler = *expr.children[i];
      if (handler.kind == Expression::Kind::block) { finalizer = &handler; break; }
      program_.exception_values = true;
      if (handler.annotation && instance && binder(*program_.methods[instance->method].source, *handler.annotation))
        implementation_gap(handler.at, "generic typed-catch selection");
      const auto type = handler.annotation ? resolve_type(*handler.annotation) : exception_type(0);
      if (type.kind != Type::exception) fail(handler.at, "catch requires a selected intrinsic exception type");
      if (handler.annotation && !type.identity) fail(handler.at, "use an untyped catch for SystemException");
      if (!handled.insert(type.identity).second) fail(handler.at, "duplicate exception category in catch clauses");
      std::vector<LexicalState> entries;
      std::set<size_t> categories;
      std::map<size_t, std::uint64_t> category_inputs;
      for (auto &path : incoming.paths) {
        auto selected = type.identity ? intersect(path.exceptions, {type.identity}) : path.exceptions;
        if (selected.empty()) continue;
        entries.push_back(path.state);
        categories.insert(selected.begin(), selected.end());
        for (const auto category : selected) category_inputs[category] |= path.state.inputs;
        for (const auto category : selected) path.exceptions.erase(category);
      }
      lexical_ = body_state;
      if (entries.empty()) join(body_state, true, before, false, handler.at);
      else {
        join(body_state, true, entries.front(), false, handler.at);
        for (size_t p = 1; p < entries.size(); ++p) { const auto joined = lexical_; join(joined, false, entries[p], false, handler.at); }
      }
      lexical_.scopes.push_back({next_scope_++, {}});
      const auto id = declare(handler.text, type, true, true, handler.at);
      lexical_.scopes.back().declarations = false;
      initialize(id);
      lexical_.exceptions[id] = categories.empty() ? exception_possibilities(type) : categories;
      auto active = caught_; caught_ = id; catch_bindings_.insert(id);
      caught_inputs_[id] = std::move(category_inputs);
      AbruptFlow handler_exits{before.scopes.size(), {}};
      abrupt_ = &handler_exits;
      const auto handled_type = check(*handler.children.front(), instance, operations, read);
      caught_ = active; catch_bindings_.erase(id);
      caught_inputs_.erase(id);
      leave_scope();
      CheckedOperation fact; fact.type = type; fact.place = id; fact.exceptions = categories;
      operations[&handler] = fact;
      if (handled_type != Type::never) {
        if (result_type != Type::never && result_type != handled_type && expr.kind != Expression::Kind::deferred)
          fail(handler.at, "try body and catch arms must have the same result type");
        result_type = handled_type;
        if (!entries.empty()) {
          normals.push_back(lexical_);
          views.push_back(&operations.at(handler.children.front().get()));
          const auto &values = operations.at(handler.children.front().get()).exceptions;
          result_exceptions.insert(values.begin(), values.end());
        }
      }
      if (!entries.empty()) outgoing.paths.insert(outgoing.paths.end(), handler_exits.paths.begin(), handler_exits.paths.end());
    }
    for (auto &path : incoming.paths) if (!path.exceptions.empty()) outgoing.paths.push_back(path);
    // Empty-category paths include returns and terminal numeric failures; catches cannot handle them.
    std::vector<LexicalState> final_inputs = normals;
    for (const auto &path : control_inputs) final_inputs.push_back(path.state);
    for (const auto &path : outgoing.paths) final_inputs.push_back(path.state);
    lexical_ = body_state;
    if (!final_inputs.empty()) {
      join(body_state, true, final_inputs.front(), false, expr.at);
      for (size_t i = 1; i < final_inputs.size(); ++i) { const auto joined = lexical_; join(joined, false, final_inputs[i], false, expr.at); }
    }
    AbruptFlow final_exits{before.scopes.size(), {}};
    abrupt_ = &final_exits;
    const auto cleanup = finalizer ? check(*finalizer, instance, operations) : Type{Type::void_type};
    if (cleanup != Type::void_type && cleanup != Type::never) fail(expr.at, "finally must complete with void or an explicit return");
    const auto final_state = lexical_;
    abrupt_ = outer;
    for (const auto &path : final_exits.paths) {
      lexical_ = path.state;
      if (path.completion.reason == 1) note_abrupt(path.at, path.exceptions, path.completion);
      else {
        bool pending_return = false, pending_normal = !normals.empty();
        for (const auto *paths : {&outgoing.paths, &control_inputs}) for (const auto &input : *paths) {
          pending_return = pending_return || input.completion.returned;
          pending_normal = pending_normal || !input.completion.returned;
        }
        if (pending_normal) note_abrupt(path.at, path.exceptions, path.completion);
        if (pending_return) { auto retained = path.completion; retained.returned = true; note_abrupt(path.at, path.exceptions, retained); }
      }
    }
    if (cleanup != Type::never) {
      for (const auto *paths : {&outgoing.paths, &control_inputs}) for (const auto &path : *paths) {
        lexical_ = finalizer ? final_state : path.state;
        lexical_.inputs = finalizer ? path.state.inputs & final_state.inputs : path.state.inputs;
        lexical_.live = lexical_.live && lexical_.inputs;
        refresh_initialized(lexical_);
        note_abrupt(path.at, path.exceptions, path.completion);
      }
    }
    lexical_ = body_state;
    if (!normals.empty()) {
      join(body_state, true, normals.front(), false, expr.at);
      for (size_t i = 1; i < normals.size(); ++i) { const auto joined = lexical_; join(joined, false, normals[i], false, expr.at); }
    }
    if (finalizer) {
      const auto normal = lexical_;
      lexical_ = final_state;
      lexical_.bindings_available.insert(normal.bindings_available.begin(), normal.bindings_available.end());
      lexical_.available.insert(normal.available.begin(), normal.available.end());
      lexical_.initialized.insert(normal.initialized.begin(), normal.initialized.end());
      lexical_.inputs = normal.inputs;
      lexical_.live = normal.live;
      for (const auto input : inputs(normal.inputs)) {
        const auto &known = normal.input_initialized.at(input);
        lexical_.input_initialized[input].insert(known.begin(), known.end());
      }
      refresh_initialized(lexical_);
    }
    if (cleanup == Type::never || normals.empty()) lexical_.live = false;
    CheckedOperation result;
    result.type = cleanup == Type::never || normals.empty() ? Type{Type::never} :
        expr.kind == Expression::Kind::deferred ? Type{Type::void_type} : result_type;
    if (!views.empty()) {
      result.place = views.front()->place;
      result.binding = views.front()->binding;
      for (const auto *view : views) {
        result.view = result.view || view->view;
        result.read_only = result.read_only || view->read_only;
        if (result.place != view->place) result.place.reset();
        if (result.binding != view->binding) result.binding.reset();
      }
    }
    result.exceptions = std::move(result_exceptions);
    return result;
  }
  Type check(const Expression &expr, CheckedInstance *instance,
             std::map<const Expression *, CheckedOperation> &operations, bool read = true) {
    using K = Expression::Kind;
    CheckedOperation operation;
    const auto child = [&](size_t i, bool read_value = true) { return check(*expr.children.at(i), instance, operations, read_value); };
    const auto view = [&](const CheckedOperation &source) {
      operation.place = source.place;
      operation.binding = source.binding;
      operation.view = source.view;
      operation.read_only = source.read_only;
    };
    const auto require_value = [&](const Type &type) {
      if (!bits_type(type)) implementation_gap(expr.at, "non-bits or abruptly completing operand");
    };
    const auto &method = instance ? program_.methods[instance->method].source : nullptr;
    if (instance && !lexical_.lookup(expr.text) && module_values_.contains(expr.text) &&
        (expr.kind == K::name || expr.kind == K::call || expr.kind == K::perform || expr.kind == K::behave))
      implementation_gap(expr.at, "module-local binding lookup and capture in callable bodies");
    if (expr.spread) implementation_gap(expr.at, "spread arguments");
    switch (expr.kind) {
    case K::integer: {
      auto digits = std::string_view(expr.text);
      operation.type = Type::integer;
      if (digits.ends_with("u8")) { operation.type = Type::uint8; digits.remove_suffix(2); }
      else if (digits.ends_with("u32")) { operation.type = Type::uint32; digits.remove_suffix(3); }
      else if (digits.ends_with("u64")) { operation.type = Type::uint64; digits.remove_suffix(3); }
      const auto parsed = std::from_chars(digits.data(), digits.data() + digits.size(), operation.integer);
      const auto limit = operation.type == Type::uint8 ? std::uint64_t{255} :
          operation.type == Type::uint32 ? std::uint64_t{0xffffffff} :
          operation.type == Type::integer ? std::uint64_t{0x7fffffffffffffff} : std::numeric_limits<std::uint64_t>::max();
      if (parsed.ec != std::errc{} || parsed.ptr != digits.data() + digits.size() || operation.integer > limit)
        fail(expr.at, "integer literal is outside " + type_name(operation.type));
      break;
    }
    case K::real: {
      operation.type = expr.text.ends_with("f32") ? Type::float32 : Type::float64;
      auto digits = std::string_view(expr.text);
      if (digits.ends_with("f32") || digits.ends_with("f64")) digits.remove_suffix(3);
      const auto in_range = [&](auto value) {
        const auto parsed = std::from_chars(digits.data(), digits.data() + digits.size(), value);
        return parsed.ec == std::errc{} && parsed.ptr == digits.data() + digits.size() && std::isfinite(value);
      };
      if (!(operation.type == Type::float32 ? in_range(float{}) : in_range(double{})))
        fail(expr.at, "floating literal is outside " + type_name(operation.type) + " range");
      break;
    }
    case K::boolean: operation.type = Type::boolean; operation.integer = expr.text == "true"; break;
    case K::name: {
      const auto binding = lexical_.lookup(expr.text);
      if (!binding) fail(expr.at, "unknown scalar value '" + expr.text + "'");
      if (!lexical_.bindings_available.contains(*binding)) fail(expr.at, "binding '" + expr.text + "' is not available on every reaching path");
      const auto &slot = (*bindings_)[*binding];
      const auto place = slot.place;
      if (!lexical_.available.contains(place)) fail(expr.at, "binding '" + expr.text + "' refers to storage whose lexical owner has ended");
      if (delayed_controls_.contains(place)) fail(expr.at, "a deferred block cannot capture loop-control binding '" + expr.text + "'");
      const bool captured = capture_catch(place);
      if (read && !lexical_.initialized.contains(place)) {
        std::uint64_t missing = 0;
        for (const auto input : inputs(lexical_.inputs))
          if (!lexical_.input_initialized.at(input).contains(place)) missing |= std::uint64_t{1} << input;
        if (missing) {
          if (!delayed_ || !deferred_inputs_.contains(place)) fail(expr.at, "read of '" + expr.text + "' before definite initialization");
          const auto obligation = cleanup_.read(place);
          read_sites_.push_back(expr.at);
          lexical_.requirements[obligation] |= missing;
        }
      }
      operation.binding = binding;
      operation.place = place;
      operation.type = (*storage_)[place].type;
      operation.read_only = slot.read_only;
      operation.view = true;
      if (operation.type.kind == Type::exception)
        operation.exceptions = delayed_ && !captured ? exception_possibilities(operation.type) : lexical_.exceptions.at(place);
      break;
    }
    case K::unary: {
      if (expr.text == "-" && expr.children.front()->kind == K::integer) {
        auto magnitude = std::string_view(expr.children.front()->text);
        while (magnitude.starts_with('0')) magnitude.remove_prefix(1);
        if (magnitude == "9223372036854775808") {
          operation.type = operation.operand_type = Type::integer;
          operation.minimum_literal = true;
          break;
        }
      }
      const auto argument = child(0);
      require_value(argument);
      if (expr.text == "!") {
        if (argument != Type::boolean) fail(expr.at, "logical negation requires bool");
      } else if (argument != Type::integer && !(expr.text == "-" && floating(argument)))
        fail(expr.at, "numeric unary operation requires i64, or floating point for negation");
      operation.type = operation.operand_type = argument;
      if (expr.text == "-" && argument == Type::integer) note_abrupt(expr.at);
      break;
    }
    case K::binary: {
      const bool logical = expr.text == "&&" || expr.text == "||";
      const auto left = child(0);
      const auto before_right = lexical_;
      const auto right = child(1);
      const auto after_right = lexical_;
      if (logical) join(before_right, false, after_right, right == Type::never, expr.at);
      require_value(left); require_value(right);
      const bool equality = expr.text == "==" || expr.text == "!=";
      const bool comparison = equality || expr.text == "<" || expr.text == ">" || expr.text == "<=" || expr.text == ">=";
      if (logical || (equality && left == Type::boolean && right == Type::boolean)) {
        if (left != Type::boolean || right != Type::boolean) fail(expr.at, "logical operation requires bool operands");
        operation.operand_type = Type::boolean;
      } else {
        if (!numeric(left) || !numeric(right)) fail(expr.at, "no applicable numeric operation '" + expr.text + "'");
        if (widens(left, right)) operation.operand_type = right;
        else if (widens(right, left)) operation.operand_type = left;
        else fail(expr.at, "numeric operands have no lossless common type");
        if (expr.text == "%" && floating(operation.operand_type)) fail(expr.at, "remainder requires integer operands");
      }
      operation.type = comparison || logical ? Type{Type::boolean} : operation.operand_type;
      if (!comparison && !logical && integral(operation.operand_type)) note_abrupt(expr.at);
      break;
    }
    case K::call: {
      if (lexical_.lookup(expr.text))
        fail(expr.at, "nearest binding '" + expr.text + "' is not callable");
      std::vector<Type> arguments;
      for (size_t i = 0; i < expr.children.size(); ++i) { arguments.push_back(child(i)); require_value(arguments.back()); }
      if (expr.text == "cause_at" && !families_.contains(expr.text)) {
        if (arguments.size() != 2 || arguments[0].kind != Type::exception || arguments[1] != Type::uint64)
          fail(expr.at, "cause_at requires an exception and a u64 index");
        operation.type = exception_type(0); operation.exceptions = exception_possibilities(operation.type); operation.cause_at = true;
        break;
      }
      operation.callee = select(expr, arguments);
      const auto &target = program_.instances[*operation.callee];
      if (method && method->kind == Form::Kind::function && program_.methods[target.method].source->kind == Form::Kind::procedure)
        fail(expr.at, "a function cannot invoke a procedure");
      operation.type = target.result;
      operation.exceptions = exception_possibilities(operation.type);
      auto failures = program_.methods[target.method].throws_categories;
      for (size_t i = 1; i < exception_categories.size(); ++i) if (!exception_categories[i].checked) failures.insert(i);
      if (&expr != prepared_call_) {
        note_abrupt(expr.at, std::move(failures));
        note_abrupt(expr.at);
      }
      break;
    }
    case K::behave: case K::perform: {
      if (lexical_.lookup(expr.text))
        fail(expr.at, "nearest binding '" + expr.text + "' is not a symbol");
      if (!print_symbols_.contains(expr.text)) implementation_gap(expr.at, "dynamic procedure call other than selected std:io print");
      if (method && method->kind == Form::Kind::function) fail(expr.at, "a function cannot invoke a dynamic procedure");
      if (expr.children.size() != 1) fail(expr.at, "print requires one scalar argument");
      if (!scalar_type(child(0))) fail(expr.at, "print requires one primitive scalar argument");
      operation.type = Type::void_type;
      break;
    }
    case K::conditional: {
      if (child(0) != Type::boolean) fail(expr.at, "if condition requires bool");
      const auto before = lexical_;
      const auto yes = child(1, read);
      const auto yes_state = lexical_;
      lexical_ = before;
      const auto no = child(2, read);
      const auto no_state = lexical_;
      join(yes_state, yes == Type::never, no_state, no == Type::never, expr.at);
      if (yes == Type::never) operation.type = no;
      else if (no == Type::never || yes == no) operation.type = yes;
      else implementation_gap(expr.at, "conditional join with different scalar result types");
      const auto &yes_fact = operations.at(expr.children[1].get()), &no_fact = operations.at(expr.children[2].get());
      operation.exceptions = yes == Type::never ? no_fact.exceptions : yes_fact.exceptions;
      if (yes != Type::never && no != Type::never) operation.exceptions.insert(no_fact.exceptions.begin(), no_fact.exceptions.end());
      if (yes == Type::never) view(no_fact);
      else if (no == Type::never) view(yes_fact);
      else {
        operation.view = yes_fact.view || no_fact.view;
        operation.read_only = yes_fact.read_only || no_fact.read_only;
        if (yes_fact.place == no_fact.place) operation.place = yes_fact.place;
        if (yes_fact.binding == no_fact.binding) operation.binding = yes_fact.binding;
      }
      break;
    }
    case K::block: {
      auto *outer = abrupt_;
      const auto parent_scope = cleanup_scope_;
      if (expr.scoped) {
        cleanup_scope_ = next_scope_++;
        lexical_.scopes.push_back({cleanup_scope_, {}});
        lexical_.queues[cleanup_scope_] = 0;
      }
      AbruptFlow flow{lexical_.scopes.size(), {}};
      if (expr.scoped) abrupt_ = &flow;
      bool exits = false;
      for (size_t i = 0; i < expr.children.size(); ++i) {
        operation.type = child(i, i + 1 == expr.children.size() ? read : true);
        exits = exits || operation.type == Type::never;
        if (exits) lexical_.live = false;
      }
      if (!expr.children.empty()) view(operations.at(expr.children.back().get()));
      if (!expr.children.empty()) operation.exceptions = operations.at(expr.children.back().get()).exceptions;
      if (exits) operation.type = Type::never;
      if (expr.scoped) {
        operation = finish_scope(operation, flow, outer, cleanup_scope_, expr.at);
        leave_scope();
        cleanup_scope_ = parent_scope;
      }
      break;
    }
    case K::range_for: case K::fold: {
      const bool fold = expr.kind == K::fold;
      const auto first = child(0), last = child(1);
      if (!integral(first) || !integral(last)) fail(expr.at, "a range requires integer endpoints");
      if (first != last) implementation_gap(expr.at, "range endpoints with different integer types");
      Type accumulator;
      if (fold) { accumulator = child(2); require_value(accumulator); }
      const auto before = lexical_;
      bool body_possible = true, empty_possible = true;
      const auto start = constant_integer(*expr.children[0], operations), end = constant_integer(*expr.children[1], operations);
      if (start && end) {
        body_possible = first == Type::integer ? std::bit_cast<std::int64_t>(*start) < std::bit_cast<std::int64_t>(*end) : *start < *end;
        empty_possible = !body_possible;
      }
      const auto header = cleanup_.header(lexical_.queues.at(cleanup_scope_));
      lexical_.queues[cleanup_scope_] = header;
      lexical_.live = lexical_.live && body_possible;
      for (auto &[id, categories] : lexical_.exceptions)
        if (!(*storage_)[id].read_only) categories = exception_possibilities((*storage_)[id].type);
      lexical_.scopes.push_back({next_scope_++, {}});
      operation.place = declare(expr.text, first, true, true, expr.at);
      initialize(*operation.place);
      loop_controls_.insert(*operation.place);
      if (fold) {
        operation.accumulator = declare(expr.binding, accumulator, true, true, expr.at);
        initialize(*operation.accumulator);
        lexical_.exceptions[*operation.accumulator] = exception_possibilities(accumulator);
        loop_controls_.insert(*operation.accumulator);
      }
      const auto body = child(fold ? 3 : 2);
      if (fold && body != Type::never && body != accumulator) {
        if (widens(body, accumulator)) implementation_gap(expr.at, "fold accumulator conversion");
        fail(expr.at, "fold body does not satisfy the accumulator type");
      }
      leave_scope();
      const auto body_state = lexical_;
      if (body != Type::never && lexical_.live)
        cleanup_.backedge(header, cleanup_.known(lexical_.queues.at(cleanup_scope_), lexical_.initialized));
      lexical_ = before;
      lexical_.live = lexical_.live && empty_possible;
      const auto otherwise = !fold && expr.children.size() == 4 ? child(3) : Type{Type::void_type};
      const auto empty_state = lexical_;
      const bool body_exits = !body_possible || body == Type::never;
      const bool empty_exits = !empty_possible || otherwise == Type::never;
      join(body_state, body_exits, empty_state, empty_exits, expr.at);
      operation.type = body_exits && empty_exits ? Type{Type::never} : fold ? accumulator : Type{Type::void_type};
      operation.operand_type = first;
      operation.exceptions = exception_possibilities(operation.type);
      break;
    }
    case K::pointer_group: {
      const bool value = expr.declaration_kind == "val";
      for (const auto &declaration : expr.children) {
        if (declaration->kind == K::block) { check(*declaration, instance, operations); continue; }
        if (!declaration->owner_name.empty()) {
          if (value) fail(declaration->at, "val storage belongs to its lexical scope, not an external owner");
          implementation_gap(declaration->at, "explicit owner annotations on local reference bindings");
        }
        const auto &initializer = *declaration->children.front();
        const bool copy = value || declaration->binding == "<-";
        Type initial_type;
        bool initialized = true;
        if (initializer.kind == K::make_local) {
          if (initializer.annotation) initial_type = resolve_local_type(*initializer.annotation, instance, initializer.at);
          else {
            const auto &query = *initializer.children.front();
            if (query.kind != K::name) implementation_gap(query.at, "typeof on a non-name local initializer");
            const auto binding = lexical_.lookup(query.text);
            if (!binding) fail(query.at, "unknown value in typeof: '" + query.text + "'");
            if (!lexical_.bindings_available.contains(*binding)) fail(query.at, "binding '" + query.text + "' is not available on every reaching path");
            initial_type = (*storage_)[(*bindings_)[*binding].place].type;
          }
          const size_t value_index = initializer.annotation ? 0 : 1;
          initialized = initializer.children.size() > value_index;
          if (initialized) {
            const auto value_type = check(*initializer.children[value_index], instance, operations);
            if (!transfers_bits(value_type, initial_type)) fail(initializer.at, "initializer does not satisfy local storage type");
          }
          CheckedOperation allocation;
          allocation.type = initial_type;
          allocation.initializes = initialized;
          if (initialized) allocation.exceptions = operations.at(initializer.children[value_index].get()).exceptions;
          operations[&initializer] = allocation;
        } else initial_type = check(initializer, instance, operations, copy || expr.declaration_kind == "ref");
        require_value(initial_type);
        const auto type = declaration->annotation ? resolve_local_type(*declaration->annotation, instance, declaration->at) : initial_type;
        const auto &source = operations.at(&initializer);
        const bool borrowed = !copy && source.view;
        if (borrowed && !source.place) implementation_gap(initializer.at, "reference binding to runtime-selected storage");
        if (borrowed ? initial_type != type : !transfers_bits(initial_type, type))
          fail(declaration->at, "initializer does not satisfy declared " + expr.declaration_kind + " type");
        if (borrowed && !lexical_.available.contains(*source.place))
          fail(initializer.at, "reference initializer's lexical owner has ended");
        if (borrowed && declaration->annotation && !declaration->read_only && source.read_only)
          fail(declaration->at, "readonly source cannot initialize a writable reference target");
        if (!borrowed && !initialized && expr.declaration_kind == "ref")
          fail(declaration->at, "ref requires definitely initialized target storage");
        if (lexical_.lookup(declaration->text) && !lexical_.scopes.back().bindings.contains(declaration->text) &&
            warned_shadow_.insert(declaration.get()).second)
          program_.warnings.push_back({Warning::shadow, declaration->at, "declaration shadows outer binding '" + declaration->text + "'"});
        const bool read_only = declaration->read_only || (value && expr.fixed_binding) || (borrowed && source.read_only);
        size_t place;
        if (borrowed) {
          place = *source.place;
          bind(declaration->text, place, read_only, false, expr.fixed_binding, declaration->at);
        } else {
          place = declare(declaration->text, type, read_only, value, declaration->at);
          bindings_->back().fixed = expr.fixed_binding;
          if (initialized) initialize(place);
          if (type.kind == Type::exception) lexical_.exceptions[place] = initialized ? source.exceptions : exception_possibilities(type);
        }
        CheckedOperation binding;
        binding.place = place;
        binding.operand_type = type;
        binding.view = borrowed;
        binding.initializes = !borrowed && initialized;
        operations[declaration.get()] = binding;
      }
      break;
    }
    case K::transfer: case K::rebind: {
      const auto destination = child(0, false);
      const auto &target = operations.at(expr.children[0].get());
      if (!target.view) fail(expr.at, "assignment requires a storage destination");
      if (!target.place) implementation_gap(expr.at, "assignment through a runtime-selected storage view");
      const auto id = *target.place;
      const auto &storage = (*storage_)[id];
      if (std::none_of(lexical_.scopes.begin(), lexical_.scopes.end(),
          [&](const LexicalState::Scope &scope) { return scope.id == storage.scope; }))
        fail(expr.at, "assignment destination's lexical owner has ended");
      if (expr.kind == K::rebind) {
        if (!target.binding) fail(expr.at, "rebinding requires a reference binding destination");
        const auto &binding = (*bindings_)[*target.binding];
        if (binding.value_binding) fail(expr.at, "val storage has no reference binding; use '<-' for bits transfer");
        if (binding.fixed) fail(expr.at, "cannot rebind a const reference binding");
        implementation_gap(expr.at, "mutable reference targets and fixed-owner flow");
      }
      if (target.read_only) fail(expr.at, "cannot write through a readonly destination");
      const auto source = child(1);
      if (!transfers_bits(source, destination)) fail(expr.at, "bits transfer source does not satisfy destination type");
      initialize(id);
      operation.type = operation.operand_type = destination;
      operation.place = id;
      operation.read_only = operation.view = true;
      operation.exceptions = operations.at(expr.children[1].get()).exceptions;
      if (destination.kind == Type::exception) lexical_.exceptions[id] = operation.exceptions;
      break;
    }
    case K::return_value: {
      if (!instance) fail(expr.at, "return requires an enclosing callable");
      const auto value = expr.children.empty() ? Type{Type::void_type} : child(0);
      if (value != Type::never && !transfers_bits(value, instance->result))
        fail(expr.at, "return type '" + type_name(value) + "' does not satisfy '" + type_name(instance->result) + "'");
      operation.type = Type::never;
      operation.operand_type = instance->result;
      note_abrupt(expr.at, {}, {1, true});
      break;
    }
    case K::field: {
      if (child(0).kind != Type::exception) implementation_gap(expr.at, "non-exception field selection");
      if (expr.text != "category" && expr.text != "site" && expr.text != "requested" && expr.text != "available" && expr.text != "cause_count")
        fail(expr.at, "unknown intrinsic exception field '" + expr.text + "'");
      operation.type = Type::uint64;
      break;
    }
    case K::raise: {
      Type type;
      if (expr.children.empty()) {
        if (!caught_) fail(expr.at, "bare throw requires an active catch");
        capture_catch(*caught_);
        operation.place = caught_; type = (*storage_)[*caught_].type; operation.exceptions = lexical_.exceptions.at(*caught_);
      } else {
        type = child(0); operation.exceptions = operations.at(expr.children[0].get()).exceptions;
      }
      if (type.kind != Type::exception) fail(expr.at, "throw requires an intrinsic exception value");
      if (expr.children.size() == 2) {
        if (child(1).kind != Type::exception) fail(expr.at, "explicit cause requires an exception value");
        note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)});
      }
      const auto binding = expr.children.empty() ? caught_ : expr.children[0]->kind == K::name ? operations.at(expr.children[0].get()).place : std::optional<size_t>{};
      if (binding && caught_inputs_.contains(*binding)) {
        const auto inputs = lexical_.inputs;
        for (const auto category : operation.exceptions) {
          lexical_.inputs = inputs & caught_inputs_.at(*binding)[category];
          if (lexical_.inputs) note_abrupt(expr.at, {category});
        }
        lexical_.inputs = inputs;
      } else note_abrupt(expr.at, operation.exceptions);
      operation.type = Type::never;
      break;
    }
    case K::try_scope: operation = check_guarded(expr, instance, operations, read); break;
    case K::deferred: {
      const auto action = cleanup_.action();
      CleanupEquations::Action summary;
      if (expr.children.front()->kind == K::block) {
        const auto before = lexical_;
        auto caught_inputs = std::move(caught_inputs_);
        caught_inputs_.clear();
        const auto controls = delayed_controls_;
        CatchCapture captures{captures_, catch_bindings_, {}};
        captures_ = &captures;
        const auto was_delayed = delayed_;
        const auto previous_inputs = deferred_inputs_;
        auto *outer = abrupt_;
        delayed_controls_.insert(loop_controls_.begin(), loop_controls_.end());
        delayed_ = true;
        deferred_inputs_ = before.available;
        lexical_.queues[cleanup_scope_] = 0;
        lexical_.inputs = 1;
        lexical_.input_initialized = {{0, lexical_.initialized}};
        lexical_.requirements.clear();
        lexical_.live = true;
        AbruptFlow flow{lexical_.scopes.size(), {}};
        abrupt_ = &flow;
        summary.guarded = expr.children.size() > 1;
        const auto completed = summary.guarded ? check_guarded(expr, instance, operations).type : child(0);
        if (completed != Type::never && lexical_.live) flow.paths.push_back({lexical_, {}, expr.at, {0, false}});
        for (const auto &path : flow.paths) {
          const auto queue = path.state.queues.at(cleanup_scope_);
          for (const auto input : inputs(path.state.inputs)) {
            const auto selected = std::uint64_t{1} << input;
            CleanupEquations::Effects effects{{}, path.state.input_initialized.at(input)};
            for (const auto &[read, mask] : path.state.requirements) if (mask & selected) effects.reads.insert(read);
            if (path.exceptions.empty()) summary.paths.push_back({selected, path.completion, queue, effects});
            else for (const auto category : path.exceptions)
              summary.paths.push_back({selected, {3 + category, path.completion.returned}, queue, effects});
          }
        }
        lexical_ = before;
        caught_inputs_ = std::move(caught_inputs);
        delayed_controls_ = controls;
        operation.captures = std::move(captures.required);
        captures_ = captures.parent;
        delayed_ = was_delayed;
        deferred_inputs_ = previous_inputs;
        abrupt_ = outer;
      } else {
        const auto *prepared = prepared_call_;
        prepared_call_ = expr.children.front().get();
        child(0);
        prepared_call_ = prepared;
        const auto &call = operations.at(expr.children.front().get());
        summary.paths.push_back({1, {0, false}, 0});
        if (call.callee) {
          auto failures = program_.methods[program_.instances[*call.callee].method].throws_categories;
          for (size_t category = 1; category < exception_categories.size(); ++category)
            if (!exception_categories[category].checked) failures.insert(category);
          for (const auto category : failures) summary.paths.push_back({1, Completion::failure(category), 0});
          summary.paths.push_back({1, {2, false}, 0});
        }
      }
      cleanup_.define(action, std::move(summary));
      note_abrupt(expr.at, {exception_id(ExceptionCategory::out_of_memory)});
      if (lexical_.live) lexical_.queues[cleanup_scope_] = cleanup_.push(lexical_.queues.at(cleanup_scope_), action);
      operation.type = Type::void_type;
      break;
    }
    default: implementation_gap(expr.at, "source expression '" + expr.text + "' is outside the scalar-method foundation");
    }
    operations[&expr] = operation;
    return operation.type;
  }

public:
  CheckedProgram check(const std::vector<std::unique_ptr<Form>> &forms) {
    for (const auto &form : forms) {
      if ((form->kind != Form::Kind::using_resource && form->kind != Form::Kind::import_resource) || form->path != "std:exceptions" || form->using_symbol) continue;
      const auto found = std::find_if(exception_categories.begin(), exception_categories.end(), [&](const ExceptionDescriptor &entry) { return form->name == entry.name; });
      if (found == exception_categories.end()) fail(form->at, "unknown intrinsic exception selection '" + form->name + "'");
      if (!exception_types_.emplace(form->alias, exception_type(exception_id(found->category))).second) fail(form->at, "duplicate exception type selection");
      program_.exception_values = true;
    }
    for (const auto &form : forms) {
      if (form->kind == Form::Kind::expression) { program_.initializers.push_back(form->body.get()); continue; }
      if ((form->kind == Form::Kind::using_resource || form->kind == Form::Kind::import_resource) && form->path == "std:exceptions" && !form->using_symbol) continue;
      if (form->kind == Form::Kind::using_resource && form->using_symbol && form->path == "std:io" && form->name == "print") {
        if (!print_symbols_.insert(form->alias).second) fail(form->at, "duplicate print symbol binding");
        continue;
      }
      if (form->kind != Form::Kind::function && form->kind != Form::Kind::procedure)
        implementation_gap(form->at, "module declaration or selection '" + form->name + "'");
      if (form->symbolic || form->operation || form->conversion || form->explicit_captures || !form->owner_parameters.empty() ||
          !form->body || !form->result_owner.empty())
        implementation_gap(form->at, "callable captures, effects, declarations, operators, owners or throws contracts");
      if (!form->result_type) implementation_gap(form->at, "omitted callable result type");
      if (*form->result_type != Type::void_type && (!form->result_const || !form->result_kind.empty()))
        implementation_gap(form->at, "non-const scalar result contract");
      const auto resolve_signature = [&](const Type &type) { return binder(*form, type) ? type : resolve_type(type); };
      CheckedMethod prepared{form.get(), {}, {}, resolve_signature(*form->result_type), {}};
      signature_type(*form, prepared.result, form->at, true);
      for (const auto &type : form->throws_types) {
        const auto resolved = resolve_type(type);
        if (resolved.kind != Type::exception) implementation_gap(form->at, "throws contracts require selected intrinsic exception types");
        const auto categories = exception_possibilities(resolved);
        prepared.throws_categories.insert(categories.begin(), categories.end());
      }
      std::set<std::string> names;
      for (const auto &parameter : form->parameters) {
        if (!names.insert(parameter.name).second) fail(parameter.at, "duplicate parameter name");
        if (parameter.variadic || parameter.automatic_conversion || !parameter.owner.empty())
          implementation_gap(parameter.at, "variadic, conversion or borrowed parameter contract");
        prepared.parameters.push_back(resolve_signature(parameter.type));
        signature_type(*form, prepared.parameters.back(), parameter.at);
      }
      for (const auto &parameter : form->type_parameters)
        if (parameter.bound) implementation_gap(parameter.at, "bounded type parameters");
      const auto id = program_.methods.size();
      for (const auto other : families_[form->name]) {
        const auto &definition = program_.methods[other];
        if (implies(prepared, definition) && implies(definition, prepared)) {
          for (size_t p = 0; p < form->parameters.size(); ++p)
            if (form->parameters[p].read_only != definition.source->parameters[p].read_only)
              implementation_gap(form->at, "target-access-qualified overload selection");
          fail(form->at, "duplicate method signature for '" + form->name + "'");
        }
        if (definition.source->kind != form->kind) implementation_gap(form->at, "mixed function/procedure family");
      }
      program_.methods.push_back(std::move(prepared));
      families_[form->name].push_back(id);
    }
    for (const auto &symbol : print_symbols_)
      if (families_.contains(symbol)) fail(program_.methods[families_[symbol].front()].source->at, "callable conflicts with symbol '" + symbol + "'");
    for (size_t i = 0; i < program_.methods.size(); ++i) {
      const auto &method = *program_.methods[i].source;
      if (!method.type_parameters.empty()) continue;
      demand(i, program_.methods[i].parameters, method.at);
    }
    storage_ = &program_.entry_storage;
    bindings_ = &program_.entry_bindings;
    lexical_ = {};
    next_scope_ = 1;
    AbruptFlow entry_flow{1, {}};
    abrupt_ = &entry_flow;
    for (const auto *expr : program_.initializers)
      if (check(*expr, nullptr, program_.entry) == Type::never) lexical_.live = false;
    CheckedOperation entry_result; entry_result.type = lexical_.live ? Type{Type::void_type} : Type{Type::never};
    finish_scope(entry_result, entry_flow, nullptr, 0, program_.initializers.empty() ? Location{} : program_.initializers.front()->at);
    for (const auto &[name, id] : lexical_.scopes.front().bindings) module_values_.insert(name);
    for (size_t i = 0; i < program_.instances.size(); ++i) {
      auto &instance = program_.instances[i];
      const auto &method = *program_.methods[instance.method].source;
      checking_method_ = &program_.methods[instance.method];
      storage_ = &instance.storage;
      bindings_ = &instance.bindings;
      scope_storage_.clear();
      lexical_ = {};
      cleanup_ = {};
      read_sites_.clear();
      cleanup_scope_ = 0;
      next_scope_ = 1;
      for (size_t p = 0; p < method.parameters.size(); ++p) {
        const auto &parameter = method.parameters[p];
        const bool value = parameter.declaration_kind == "val";
        const auto id = declare(parameter.name, instance.arguments[p], parameter.read_only || (value && parameter.fixed_binding), value, parameter.at);
        bindings_->back().fixed = parameter.fixed_binding;
        initialize(id);
        lexical_.exceptions[id] = exception_possibilities(instance.arguments[p]);
      }
      AbruptFlow flow{1, {}};
      abrupt_ = &flow;
      CheckedOperation completed;
      completed.type = check(*method.body, &instance, instance.operations) == Type::never ? Type{Type::never} : Type{Type::void_type};
      const auto completion = finish_scope(completed, flow, nullptr, 0, method.at).type;
      if (instance.result != Type::void_type && completion != Type::never && warned_missing_return_.insert(instance.method).second)
        program_.warnings.push_back({Warning::missing_return, method.at, "callable may finish without an explicit return"});
      instance.complete = true;
    }
    return std::move(program_);
  }
};

} // namespace cod::language
