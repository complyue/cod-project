#include "runtime.hh"
#include "generated.inc"
#include <tuple>

int main(int argc, char **argv) {
  assert(argc == 7 || argc == 8);
  const bool read_saved = argc == 8 && std::string(argv[7]) == "read";
  const bool write_only = argc == 8 && std::string(argv[7]) == "write";
  const I mode = std::stoll(argv[3]), failure = std::stoll(argv[4]), cleanup = std::stoll(argv[5]);
  assert(mode >= 0 && mode <= 5 && failure >= 0 && failure <= 4 && cleanup >= 0 && cleanup <= 3);
  Input input{};
  try { if (!read_saved) input = read_input(argv[1]); }
  catch (Failure e) { std::cerr << "REJECT " << e.code << '\n'; return 2; }
  if (!read_saved) {
    auto writer = DBMR<Root>::create(argv[2], 1 << 20);
    construct_input(writer.region(), input);
    prepare_catalog(writer.region());
  }
  if (write_only) { std::cout << "WRITTEN\n"; return 0; }
  assert(!read_saved || mode < 4);
  auto first = DBMR<Root>::readonly(argv[2]), second = DBMR<Root>::readonly(argv[2]);
  try { validate_catalog(first.region()); validate_catalog(second.region()); }
  catch (Failure e) { std::cerr << "REJECT " << e.code << '\n'; return 2; }
  assert(&first.region() != &second.region());
  const auto bytes = first.region().occupation();
  std::vector<Byte> original_bytes(bytes);
  std::memcpy(original_bytes.data(), &first.region(), bytes);
  std::vector<I> saved_events;
  std::tuple<I,I,I,TypeId> saved;
  for (auto *mr : {&first.region(), &second.region()}) {
    auto_region<Root> policy_a(4096), policy_b(4096), output(1 << 16), scratch(1 << 16);
    auto a = Borrow<Policy>(policy_a->cast_ptr(&policy_a->root()->policy));
    auto b = Borrow<Policy>(policy_b->cast_ptr(&policy_b->root()->policy));
    policy_b->root()->policy.increment = 3;
    if (mode == 2) policy_a->root()->policy.increment = 2;
    FamilyValue entry{0,5,a}, dependency{1,9,mode == 3 ? a : b};
    current_family.emplace(FamilyValue{0,0,a});
    View plan = get_choice(mr->cast_ptr(&mr->root()->object));
    View destination = get_choice(mr->cast_ptr(&mr->root()->destination));
    if (mode >= 4) {
      construct_input(*scratch, input);
      plan = get_choice(scratch->cast_ptr(&scratch->root()->object));
      destination = get_choice(scratch->cast_ptr(&scratch->root()->destination));
      const View alias = plan;
      const View old = read_x(plan);
      auto replacement = make_word(*scratch, T_BooleanLiteral, 1, 17);
      bool failed = false;
      try { replace_x(alias, replacement, mode == 5); }
      catch (Failure e) { assert(e.code == 934); failed = true; }
      const View changed = read_x(plan);
      assert(plan.evidence == alias.evidence && plan.evidence == input.plan);
      assert(&changed.payload.pointer->region() == scratch.get());
      if (mode == 4) assert(!failed && changed.evidence == T_BooleanLiteral && scalar_value(changed) == 1);
      else assert(failed && changed.evidence == old.evidence && scalar_value(changed) == scalar_value(old));
    }
    current_input.emplace(plan); // Abstract view retains concrete identity and actual owner.
    auto meters = make_word(*output, T_Meters, 7, 0), seconds = make_word(*output, T_Seconds, 7, 0);
    assert(scalar_value(meters) == scalar_value(seconds));
    assert(select_kind(meters) == 71 && select_kind(seconds) == 72);
    assert(callsite_meter(meters) == 71 && callsite_common(read_x(plan)) == 17);
    const I kind = select_kind(read_x(plan)); // Selection matters even without payload access.
    events.clear();
    Native native(failure, argv[6]);
    auto completion = run_scope(*current_input, destination, *output, entry, dependency, mode >= 4 ? 0 : mode, cleanup, native);
    assert(native.acquired == native.released);
    assert(current_family->interpretation == 0 && current_family->bias == 0);
    assert(&current_family->policy.pointer->region() == policy_a.get() && unrelated_binding == 77);
    const auto actual = std::tuple{completion.value, completion.error, kind, output->root()->result.evidence};
    if (saved_events.empty()) { saved_events = events; saved = actual; }
    else assert(saved_events == events && saved == actual);
    assert(std::memcmp(mr, original_bytes.data(), bytes) == 0);
    current_input.reset(); current_family.reset();
  }
  std::cout << "RESULT " << std::get<0>(saved) << ' ' << std::get<1>(saved) << ' ' << std::get<2>(saved) << ' ' << std::get<3>(saved) << '\n';
  std::cout << "TRACE"; for (I x : saved_events) std::cout << ' ' << x;
  std::cout << "\nREGIONS input bytes unchanged, distinct mapping bases, actual owners preserved\n";
}
