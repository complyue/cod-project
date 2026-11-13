// Isolated storage/execution experiment. None of these records is a CoD ABI.
#include <shilos/dbmr.hh>
#include <shilos/vector.hh>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <vector>

using namespace shilos;
using I = std::int64_t;

// Observe region blocks only, including a root constructor that throws.
static std::array<void *, 4096> watched{};
static std::size_t region_entries = 0, region_frees = 0, region_live = 0, region_peak = 0;
static void watch_region(void *p) {
  auto slot = std::find(watched.begin(), watched.end(), nullptr);
  assert(slot != watched.end()); *slot = p;
  ++region_entries; region_peak = std::max(region_peak, ++region_live);
}
void *operator new(std::size_t n) {
  if (auto *p = std::malloc(n ? n : 1)) return p;
  throw std::bad_alloc();
}
void operator delete(void *p) noexcept {
  if (p) {
    auto slot = std::find(watched.begin(), watched.end(), p);
    if (slot != watched.end()) { *slot = nullptr; ++region_frees; --region_live; }
  }
  std::free(p);
}
void operator delete(void *p, std::size_t) noexcept { ::operator delete(p); }

enum class Op : std::uint32_t {
  nop, constant, reg, add, sub, mod, eq, select, current_view, policy,
  let, set, branch, ret, raise, event, install, lease, release, call, invoke, defer
};
struct NodeSeed { Op op; I value; std::uint32_t slot, type, a, b, c, next; };
struct ProcSeed { std::uint32_t body, parameters, error_slot, result_type; std::array<std::uint32_t, 8> slots; };
#include "regional_fixture.inc"

struct Node {
  Op op{};
  I value = 0;
  std::uint32_t id = 0, slot = 0, type = 0;
  regional_ptr<Node> a, b, c, next;
  Node() = default;
  template <typename RT> Node(memory_region<RT> &, const NodeSeed &seed, std::uint32_t index)
      : op(seed.op), value(seed.value), id(index), slot(seed.slot), type(seed.type) {}
  Node(const Node &) = delete; Node(Node &&) = delete;
};
struct Procedure {
  regional_ptr<Node> body;
  std::uint32_t parameters = 0, error_slot = 0, result_type = 0;
  std::array<std::uint32_t, 8> slots{};
  Procedure() = default;
  template <typename RT> Procedure(memory_region<RT> &, const ProcSeed &seed)
      : parameters(seed.parameters), error_slot(seed.error_slot), result_type(seed.result_type), slots(seed.slots) {}
  Procedure(const Procedure &) = delete; Procedure(Procedure &&) = delete;
};
struct Program {
  static constexpr UUID TYPE_UUID{"dba46f37-f568-41b3-a563-31515cb927c1"};
  regional_vector<Node> nodes;
  regional_vector<Procedure> procedures;
  std::array<std::uint32_t, 3> stock_views = fixture_views;
  explicit Program(memory_region<Program> &mr) : nodes(mr), procedures(mr) {}
  Program(const Program &) = delete; Program(Program &&) = delete;
};
using ProgramView = global_ptr<Program, Program>;
static void construct_program(memory_region<Program> &mr) {
  auto root = mr.root();
  for (std::uint32_t id = 0; id < fixture_nodes.size(); ++id) root->nodes.emplace_back(mr, fixture_nodes[id], id);
  for (const auto &seed : fixture_procedures) root->procedures.emplace_back(mr, seed);
  const auto node = [&](std::uint32_t id) { return mr.cast_ptr(&root->nodes[id]); };
  for (std::uint32_t id = 0; id < fixture_nodes.size(); ++id) {
    auto target = node(id); const auto &seed = fixture_nodes[id];
    if (seed.a) target.set(&Node::a, node(seed.a));
    if (seed.b) target.set(&Node::b, node(seed.b));
    if (seed.c) target.set(&Node::c, node(seed.c));
    if (seed.next) target.set(&Node::next, node(seed.next));
  }
  for (std::uint32_t id = 0; id < fixture_procedures.size(); ++id) {
    auto procedure = mr.cast_ptr(&root->procedures[id]);
    if (fixture_procedures[id].body) procedure.set(&Procedure::body, node(fixture_procedures[id].body));
  }
}

struct Task {
  std::uint32_t node = 0;
  regional_ptr<Task> next;
  Task() = default;
  template <typename RT> explicit Task(memory_region<RT> &, std::uint32_t id = 0) : node(id) {}
  Task(const Task &) = delete; Task(Task &&) = delete;
};
struct Frame {
  std::array<I, 32> values{};
  std::uint32_t procedure = 0, receiver = 0, action = 0, stage = 0;
  I saved_view = 0, pending = 0, saved_return = 0;
  bool thrown = false, has_return = false, cleanup = false;
  regional_ptr<Frame> parent;
  regional_ptr<Task> tasks, defers;
  Frame() = default;
  template <typename RT> explicit Frame(memory_region<RT> &, std::uint32_t id = 0) : procedure(id) {}
  Frame(const Frame &) = delete; Frame(Frame &&) = delete;
};
struct State {
  static constexpr UUID TYPE_UUID{"b3cc83bc-a106-4393-921d-acd45fa25196"};
  regional_vector<Frame> frames;
  regional_ptr<Frame> top;
  I binding = 0, result = 0;
  bool thrown = false, done = false;
  explicit State(memory_region<State> &mr) : frames(mr) { watch_region(&mr); }
  State(const State &) = delete; State(State &&) = delete;
};
struct Lease {
  static constexpr UUID TYPE_UUID{"be85f80d-bc36-44d8-94d4-23a0de2d8b7c"};
  I token = 0;
  explicit Lease(memory_region<Lease> &mr, I value = 0, bool fail = false) : token(value) {
    watch_region(&mr);
    if (fail) throw std::runtime_error("injected root construction failure");
  }
  Lease(const Lease &) = delete; Lease(Lease &&) = delete;
};
struct Policy {
  static constexpr UUID TYPE_UUID{"64be46ed-eac2-459f-b68b-f48391655e9e"};
  regional_ptr<I> divisor;
  explicit Policy(memory_region<Policy> &mr, I value = 2) : divisor(mr.create_bits<I>(value)) { watch_region(&mr); }
  Policy(const Policy &) = delete; Policy(Policy &&) = delete;
};
// Host-only lifetime witnesses do not own or extend the borrowed region.
struct Life { bool alive = true; std::size_t owner = 0; };
struct PolicyOwner {
  std::shared_ptr<Life> life;
  std::unique_ptr<auto_region<Policy>> region;
  PolicyOwner(I value, std::size_t id)
      : life(std::make_shared<Life>(Life{true, id})), region(std::make_unique<auto_region<Policy>>(std::size_t{256}, value)) {}
  void expire() { life->alive = false; region.reset(); }
  ~PolicyOwner() { expire(); }
};
struct Borrow {
  global_ptr<I, Policy> pointer;
  std::shared_ptr<Life> life;
  const I *address() const { return life->alive ? pointer.get() : nullptr; }
};
// A host callable-image model: copied bits are independent; references keep
// actual owners. This is not the complete CoD captured-environment ABI.
struct Image { I selected_view; std::array<Borrow, 3> policies; };
struct BorrowExpired {};
struct Runtime {
  Image image;
  I initial_view = 0, fail_acquire = -1;
  I attempts = 0, acquired = 0, released = 0, policy_reads = 0;
  std::map<I, std::unique_ptr<auto_region<Lease>>> leases;
  std::vector<std::array<I, 3>> events;
  explicit Runtime(const Image &value, I initial, I failure) : image(value), initial_view(initial), fail_acquire(failure) {
    events.reserve(8192); // Bounded fixture; callbacks never allocate or unwind.
  }
  void event(I tag, I a, I b) noexcept {
    assert(events.size() < events.capacity()); events.push_back({tag, a, b});
  }
  I acquire() noexcept {
    ++attempts;
    try {
      const I id = acquired + 1;
      auto owner = std::make_unique<auto_region<Lease>>(std::size_t{128}, id, attempts == fail_acquire);
      assert(leases.emplace(id, std::move(owner)).second); ++acquired; return id;
    } catch (...) { return 0; }
  }
  bool release(I id) noexcept {
    const auto found = leases.find(id);
    if (found == leases.end()) return false;
    leases.erase(found); ++released; return true;
  }
  const I *policy(I view) noexcept {
    assert(view >= 0 && view < 3); ++policy_reads;
    return image.policies[static_cast<std::size_t>(view)].address();
  }
};
static thread_local Runtime *active = nullptr;
extern "C" I exp_acquire() noexcept { return active->acquire(); }
extern "C" I exp_release(I id) noexcept { return active->release(id) ? 0 : 1; }
extern "C" const I *exp_policy_address(I view) noexcept { return active->policy(view); }
extern "C" I exp_policy_value(I view, I *out) noexcept {
  if (const auto *value = active->policy(view)) { *out = *value; return 0; }
  return 905;
}
extern "C" void exp_event(I tag, I a, I b) noexcept { active->event(tag, a, b); }
extern "C" void exp_native(const I *inputs, I initial_view, I *result);

class Machine {
  ProgramView program_;
  auto_region<State> state_;
  Runtime &runtime_;
  using FrameView = global_ptr<Frame, State>;
  const Node &node(std::uint32_t id) const { return program_->nodes[id]; }
  const Procedure &procedure(std::uint32_t id) const { return program_->procedures[id]; }
  static std::uint32_t id(const regional_ptr<Node> &p) { return p ? p->id : 0; }
  void tasks(FrameView frame, std::uint32_t head) {
    if (!head) return;
    auto task = state_->create<Task>(head);
    task.set(&Task::next, frame.get(&Frame::tasks)); frame.set(&Frame::tasks, task);
  }
  void push(std::uint32_t method, const std::vector<I> &arguments, std::uint32_t receiver = 0) {
    auto root = state_->root(); const auto &signature = procedure(method);
    assert(arguments.size() == signature.parameters);
    root->frames.emplace_back(*state_, method);
    auto frame = state_->cast_ptr(&root->frames[root->frames.size() - 1]);
    frame->receiver = receiver; frame->saved_view = root->binding;
    frame.set(&Frame::parent, root.get(&State::top)); root.set(&State::top, frame);
    for (std::size_t i = 0; i < arguments.size(); ++i) frame->values[signature.slots[i]] = arguments[i];
    tasks(frame, id(signature.body));
  }
  void stage(FrameView frame, std::uint32_t kind) {
    frame->stage = kind; frame.clear(&Frame::tasks);
    const auto &action = node(frame->action);
    if (kind == 1) tasks(frame, id(action.a));
    if (kind == 2) {
      frame->values[procedure(frame->procedure).error_slot] = frame->pending;
      tasks(frame, id(action.b));
    }
    if (kind == 3) tasks(frame, id(action.c));
  }
  void next_action(FrameView frame) {
    frame->cleanup = true; frame.clear(&Frame::tasks);
    auto next = frame.get(&Frame::defers);
    if (!next) { frame->stage = 4; frame->action = 0; return; }
    frame->action = next->node; frame.set(&Frame::defers, next.get(&Task::next));
    const auto &action = node(frame->action);
    stage(frame, frame->thrown ? (action.value ? 2 : 3) : 1);
  }
  void abrupt(FrameView frame, bool thrown, I value) {
    frame->thrown = thrown; frame->pending = value;
    if (!thrown) { frame->has_return = true; frame->saved_return = value; }
    frame.clear(&Frame::tasks);
    if (!frame->cleanup) next_action(frame);
    else if (frame->stage == 1 && thrown && node(frame->action).value) stage(frame, 2);
    else if (frame->stage != 3) stage(frame, 3);
    else next_action(frame);
  }
  I eval(const Node &expression, FrameView frame) {
    switch (expression.op) {
    case Op::constant: return expression.value;
    case Op::reg: return frame->values[expression.slot];
    case Op::current_view: return state_->root()->binding;
    case Op::policy: {
      const auto *value = runtime_.policy(expression.value);
      if (!value) throw BorrowExpired{};
      return *value;
    }
    case Op::add: return static_cast<I>(static_cast<std::uint64_t>(eval(*expression.a, frame)) + static_cast<std::uint64_t>(eval(*expression.b, frame)));
    case Op::sub: return static_cast<I>(static_cast<std::uint64_t>(eval(*expression.a, frame)) - static_cast<std::uint64_t>(eval(*expression.b, frame)));
    case Op::mod: return eval(*expression.a, frame) % eval(*expression.b, frame);
    case Op::eq: return eval(*expression.a, frame) == eval(*expression.b, frame);
    case Op::select: return eval(*expression.a, frame) ? eval(*expression.b, frame) : eval(*expression.c, frame);
    default: throw std::logic_error("not an expression");
    }
  }
  std::vector<I> arguments(std::uint32_t head, FrameView frame) {
    std::vector<I> values;
    for (auto current = head; current; current = id(node(current).next)) values.push_back(eval(node(current), frame));
    return values;
  }
  void step() {
    auto root = state_->root(); auto frame = root.get(&State::top); auto task = frame.get(&Frame::tasks);
    if (!task) {
      if (!frame->cleanup) { next_action(frame); return; }
      if (frame->stage == 1) { stage(frame, 3); return; }
      if (frame->stage == 2) { frame->thrown = false; frame->pending = frame->saved_return; stage(frame, 3); return; }
      if (frame->stage == 3) { next_action(frame); return; }
      assert(frame->stage == 4);
      const bool thrown = frame->thrown || !frame->has_return;
      const I value = frame->thrown || frame->has_return ? frame->pending : 903;
      root->binding = frame->saved_view; auto parent = frame.get(&Frame::parent); root.set(&State::top, parent);
      if (!parent) { root->done = true; root->thrown = thrown; root->result = value; return; }
      if (thrown) abrupt(parent, true, value); else parent->values[frame->receiver] = value;
      return;
    }
    const auto &instruction = node(task->node);
    task->node = id(instruction.next);
    if (!task->node) frame.set(&Frame::tasks, task.get(&Task::next));
    try {
      switch (instruction.op) {
      case Op::branch: tasks(frame, eval(*instruction.a, frame) ? id(instruction.b) : id(instruction.c)); break;
      case Op::let: case Op::set: frame->values[instruction.slot] = eval(*instruction.a, frame); break;
      case Op::ret: abrupt(frame, false, eval(*instruction.a, frame)); break;
      case Op::raise: abrupt(frame, true, eval(*instruction.a, frame)); break;
      case Op::event: {
        const auto tag = eval(*instruction.a, frame), a = eval(*instruction.b, frame), b = eval(*instruction.c, frame);
        runtime_.event(tag, a, b); break;
      }
      case Op::install: root->binding = eval(*instruction.a, frame); break;
      case Op::defer: {
        auto registration = state_->create<Task>(instruction.id);
        registration.set(&Task::next, frame.get(&Frame::defers)); frame.set(&Frame::defers, registration); break;
      }
      case Op::lease: {
        const auto token = runtime_.acquire();
        if (token) frame->values[instruction.slot] = token; else abrupt(frame, true, 904);
        break;
      }
      case Op::release: assert(runtime_.release(eval(*instruction.a, frame))); break;
      case Op::call: push(static_cast<std::uint32_t>(instruction.value), arguments(id(instruction.a), frame), instruction.slot); break;
      case Op::invoke: {
        const auto view = eval(*instruction.a, frame); assert(view >= 0 && view < 3);
        push(program_->stock_views[view], arguments(id(instruction.b), frame), instruction.slot); break;
      }
      default: throw std::logic_error("not a statement");
      }
    } catch (const BorrowExpired &) { abrupt(frame, true, 905); }
  }
public:
  Machine(ProgramView program, Runtime &runtime, std::size_t capacity)
      : program_(program), state_(capacity), runtime_(runtime) { state_->root()->binding = runtime.initial_view; }
  std::array<I, 5> run(const std::array<I, 5> &inputs) {
    push(fixture_entry, std::vector<I>(inputs.begin(), inputs.end()));
    while (!state_->root()->done) step();
    const auto root = state_->root();
    assert(runtime_.leases.empty());
    return {root->thrown, root->result, root->binding, runtime_.acquired, runtime_.released};
  }
  std::size_t occupied() const { return state_->occupation(); }
};

static std::uint64_t fingerprint(const void *memory, std::size_t bytes) {
  auto p = static_cast<const unsigned char *>(memory); std::uint64_t h = 1469598103934665603ULL;
  while (bytes--) h = (h ^ *p++) * 1099511628211ULL;
  return h;
}
static Borrow borrow(PolicyOwner &owner) { return {owner.region->operator->()->root().get(&Policy::divisor), owner.life}; }

int main(int argc, char **argv) {
  assert(argc == 12);
  const std::filesystem::path path = argv[1];
  std::array<I, 5> inputs{};
  for (int i = 0; i < 5; ++i) inputs[i] = std::stoll(argv[i + 2]);
  const I initial = std::stoll(argv[7]), failure = std::stoll(argv[8]);
  const bool alias = std::stoll(argv[9]) != 0, expired = std::stoll(argv[10]) != 0;
  const I policy_zero = std::stoll(argv[11]);
  assert(inputs[0] > 0 && inputs[0] <= 1000 && initial >= 0 && initial < 3 && policy_zero > 0);
  std::uint64_t original_hash = 0; std::size_t program_bytes = 0;
  {
    auto writer = DBMR<Program>::create(path.string(), 1 << 20);
    construct_program(writer.region());
    program_bytes = writer.region().occupation();
    original_hash = fingerprint(&writer.region(), program_bytes);
  }
  std::array<I, 5> interpreted{}, native{}; std::size_t state_bytes = 0, trace_size = 0;
  std::uint64_t trace_hash = 0; I reads = 0;
  {
    // Existing mapper, known fixture only: two addresses, identical physical
    // graph bytes. No external definition admission or file protocol is added.
    auto first = DBMR<Program>::readonly(path.string());
    auto second = DBMR<Program>::readonly(path.string());
    assert(&first.region() != &second.region());
    assert(fingerprint(&first.region(), program_bytes) == original_hash);
    assert(fingerprint(&second.region(), program_bytes) == original_hash);
    PolicyOwner a(policy_zero, 1), b(3, 2), c(5, 3);
    Image original{1, {borrow(a), alias ? borrow(a) : borrow(b), borrow(c)}};
    Image copy = original; copy.selected_view = initial; original.selected_view = (initial + 1) % 3;
    assert(copy.selected_view == initial);
    if (expired) a.expire();
    Runtime reference(copy, copy.selected_view, failure);
    {
      Machine machine(first.region().root(), reference, static_cast<std::size_t>(inputs[0]) * 4096 + 262144);
      interpreted = machine.run(inputs); state_bytes = machine.occupied();
    }
    Runtime remapped(copy, copy.selected_view, failure);
    {
      Machine machine(second.region().root(), remapped, static_cast<std::size_t>(inputs[0]) * 4096 + 262144);
      assert(machine.run(inputs) == interpreted); assert(remapped.events == reference.events);
    }
    Runtime compiled(copy, copy.selected_view, failure); active = &compiled;
    exp_native(inputs.data(), copy.selected_view, native.data()); active = nullptr;
    assert(native == interpreted); assert(compiled.events == reference.events);
    assert(compiled.policy_reads == reference.policy_reads);
    assert(compiled.leases.empty() && reference.leases.empty() && remapped.leases.empty());
    assert(fingerprint(&first.region(), program_bytes) == original_hash && fingerprint(&second.region(), program_bytes) == original_hash);
    trace_size = reference.events.size(); trace_hash = fingerprint(reference.events.data(), trace_size * sizeof(reference.events[0])); reads = reference.policy_reads;
    for (const auto &e : reference.events) std::printf("E %lld %lld %lld\n", static_cast<long long>(e[0]), static_cast<long long>(e[1]), static_cast<long long>(e[2]));
  }
  assert(region_live == 0 && region_entries == region_frees);
  std::printf("RESULT %lld %lld %lld %lld %lld\n", static_cast<long long>(native[0]), static_cast<long long>(native[1]), static_cast<long long>(native[2]), static_cast<long long>(native[3]), static_cast<long long>(native[4]));
  std::printf("METRICS %zu %zu %zu %zu %zu %zu %llu %lld\n", program_bytes, state_bytes, region_entries, region_frees, region_peak, trace_size, static_cast<unsigned long long>(trace_hash), static_cast<long long>(reads));
}
