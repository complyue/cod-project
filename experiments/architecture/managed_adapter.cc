// Experimental C bridge for the installed JDK's foreign-memory API.
// Its offsets and operation numbers describe this fixture, not a CoD ABI.
#include "regional_fixture.hh"
#include <cstddef>
#include <type_traits>

static_assert(std::is_standard_layout_v<Node> && std::is_standard_layout_v<Procedure>);
static_assert(sizeof(I) == 8 && sizeof(std::intptr_t) == 8);

struct Store {
  DBMR<Program> first, second;
  std::size_t bytes;
  std::uint64_t hash;
  Store(const char *path, std::size_t size, std::uint64_t checksum)
      : first(DBMR<Program>::readonly(path)), second(DBMR<Program>::readonly(path)), bytes(size), hash(checksum) {
    assert(&first.region() != &second.region());
  }
  memory_region<Program> &mapping(I index) { assert(index == 0 || index == 1); return index ? second.region() : first.region(); }
  ~Store() {
    assert(fingerprint(&first.region(), bytes) == hash && fingerprint(&second.region(), bytes) == hash);
  }
};
struct Environment {
  const std::size_t entries_before = region_entries, frees_before = region_frees;
  PolicyOwner a, b, c;
  Image image;
  Runtime runtime;
  Environment(I initial, I failure, I alias, I divisor)
      : a(divisor, 1), b(3, 2), c(5, 3), image{initial, {borrow(a), alias ? borrow(a) : borrow(b), borrow(c)}},
        runtime(image, initial, failure) {}
};

extern "C" Store *bridge_open(const char *path) noexcept {
  try {
    std::size_t bytes; std::uint64_t hash;
    {
      auto writer = DBMR<Program>::create(path, 1 << 20); construct_program(writer.region());
      bytes = writer.region().occupation(); hash = fingerprint(&writer.region(), bytes);
    }
    return new Store(path, bytes, hash);
  } catch (...) { return nullptr; }
}
extern "C" void bridge_close(Store *store) noexcept { delete store; }
extern "C" const void *bridge_base(Store *store, I mapping) noexcept { return &store->mapping(mapping); }
extern "C" I bridge_size(Store *store) noexcept { return static_cast<I>(store->bytes); }
extern "C" void bridge_layout(I *out) noexcept {
  const I values[] = {offsetof(Node, op), offsetof(Node, value), offsetof(Node, slot), offsetof(Node, a),
      offsetof(Node, b), offsetof(Node, c), offsetof(Node, next), offsetof(Node, id),
      offsetof(Procedure, body), offsetof(Procedure, parameters), offsetof(Procedure, error_slot), offsetof(Procedure, slots),
      static_cast<I>(fixture_nodes.size()), static_cast<I>(fixture_procedures.size()), fixture_entry,
      fixture_views[0], fixture_views[1], fixture_views[2]};
  std::copy(std::begin(values), std::end(values), out);
}
extern "C" void bridge_directory(Store *store, I procedures, I *out) noexcept {
  auto &mr = store->first.region(); auto program = mr.root();
  const auto *base = reinterpret_cast<const std::byte *>(&mr);
  if (procedures) {
    for (std::size_t i = 0; i < program->procedures.size(); ++i) out[i] = reinterpret_cast<const std::byte *>(&program->procedures[i]) - base;
  } else {
    for (std::size_t i = 0; i < program->nodes.size(); ++i) out[i] = reinterpret_cast<const std::byte *>(&program->nodes[i]) - base;
  }
}
static I node_id(const regional_ptr<Node> &node) { return node ? node->id : 0; }
extern "C" I bridge_field(Store *store, I mapping, I procedures, I id, I field) noexcept {
  auto program = store->mapping(mapping).root();
  if (procedures) {
    assert(id >= 0 && static_cast<std::size_t>(id) < program->procedures.size());
    const auto &p = program->procedures[id];
    if (field == 0) return node_id(p.body);
    if (field == 1) return p.parameters;
    if (field == 2) return p.error_slot;
    assert(field >= 3 && field < 11); return p.slots[field - 3];
  }
  assert(id > 0 && static_cast<std::size_t>(id) < program->nodes.size());
  const auto &n = program->nodes[id];
  switch (field) {
  case 0: return static_cast<I>(n.op);
  case 1: return n.value;
  case 2: return n.slot;
  case 3: return node_id(n.a);
  case 4: return node_id(n.b);
  case 5: return node_id(n.c);
  case 6: return node_id(n.next);
  case 7: return n.id;
  default: std::abort();
  }
}
extern "C" Environment *bridge_environment(I initial, I failure, I alias, I divisor) noexcept {
  assert(region_live == 0 && divisor > 0);
  try { return new Environment(initial, failure, alias, divisor); }
  catch (...) { return nullptr; }
}
extern "C" const I *bridge_policy_address(Environment *environment, I view) noexcept {
  assert(view >= 0 && view < 3);
  return environment->image.policies[view].address();
}
extern "C" I bridge_command(Environment *environment, I command, I argument) noexcept {
  auto &runtime = environment->runtime;
  switch (command) {
  case 0: return runtime.acquire();
  case 1: return runtime.release(argument) ? 0 : 97;
  case 2: { const auto *value = runtime.policy(argument); return value ? *value : 0; }
  case 3:
    assert(argument >= 0 && argument < 3);
    if (argument == 0) environment->a.expire();
    else if (argument == 1) environment->b.expire();
    else environment->c.expire();
    return 0;
  case 4: return runtime.acquired;
  case 5: return runtime.released;
  case 6: return runtime.policy_reads;
  default: std::abort();
  }
}
extern "C" void bridge_native(Environment *environment, const I *inputs, I *out) noexcept {
  assert(!active); active = &environment->runtime;
  exp_native(inputs, environment->image.selected_view, out); active = nullptr;
  assert(environment->runtime.leases.empty());
}
extern "C" I bridge_events(Environment *environment, I *out, I capacity) noexcept {
  const auto &events = environment->runtime.events;
  assert(capacity >= static_cast<I>(events.size() * 3));
  for (const auto &event : events) for (const auto value : event) *out++ = value;
  return static_cast<I>(events.size());
}
extern "C" void bridge_finish(Environment *environment, I *out) noexcept {
  assert(environment->runtime.leases.empty());
  const auto entries = environment->entries_before, frees = environment->frees_before;
  delete environment;
  out[0] = static_cast<I>(region_entries - entries); out[1] = static_cast<I>(region_frees - frees);
  assert(out[0] == out[1] && region_live == 0);
}
