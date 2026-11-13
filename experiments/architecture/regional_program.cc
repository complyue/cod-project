#include "regional_fixture.hh"

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
