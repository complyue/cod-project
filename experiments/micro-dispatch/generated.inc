static_assert(sizeof(Word) == 16 && alignof(Word) == 8);

static_assert(offsetof(Word, value) == 0);

static_assert(offsetof(Word, site) == 8);

static_assert(sizeof(Word) == 16 && alignof(Word) == 8);

static_assert(offsetof(Word, value) == 0);

static_assert(offsetof(Word, site) == 8);

static_assert(sizeof(Word) == 16 && alignof(Word) == 8);

static_assert(offsetof(Word, value) == 0);

static_assert(offsetof(Word, site) == 8);

static_assert(sizeof(Word) == 16 && alignof(Word) == 8);

static_assert(offsetof(Word, value) == 0);

static_assert(offsetof(Word, site) == 8);

static_assert(sizeof(Word) == 16 && alignof(Word) == 8);

static_assert(offsetof(Word, value) == 0);

static_assert(offsetof(Word, site) == 8);

static_assert(sizeof(RegionalSlot) == 32 && alignof(RegionalSlot) == 16);

static_assert(offsetof(RegionalSlot, region_symbol) == 0);

static_assert(offsetof(RegionalSlot, value) == 8);

static_assert(offsetof(RegionalSlot, site) == 16);

static_assert(sizeof(Example) == 40 && alignof(Example) == 8);

static_assert(offsetof(Example, x) == 0);

static_assert(offsetof(Example, y) == 16);

static_assert(offsetof(Example, site) == 32);

static_assert(sizeof(Other) == 40 && alignof(Other) == 8);

static_assert(offsetof(Other, site) == 0);

static_assert(offsetof(Other, y) == 8);

static_assert(offsetof(Other, x) == 24);

static_assert(sizeof(Word) == 16 && alignof(Word) == 8);

static_assert(offsetof(Word, value) == 0);

static_assert(offsetof(Word, site) == 8);

static_assert(sizeof(Word) == 16 && alignof(Word) == 8);

static_assert(offsetof(Word, value) == 0);

static_assert(offsetof(Word, site) == 8);

static_assert(sizeof(TypeId) == 8 && sizeof(Choice) == 16);

static_assert(std::endian::native == std::endian::little);

View read_x(View plan) {
  switch (plan.evidence) {
  case T_Example: return field<Example>(plan, &Example::x);
  case T_Other: return field<Other>(plan, &Other::x);
  default: throw Failure{931};
  }
}

View read_y(View plan) {
  switch (plan.evidence) {
  case T_Example: return field<Example>(plan, &Example::y);
  case T_Other: return field<Other>(plan, &Other::y);
  default: throw Failure{931};
  }
}

I scalar_value(View value) { return word_at(value, 0); }

I destination_value(View value) {
  if (value.evidence == T_LocalSlot) return word_at(value, 0);
  if (value.evidence == T_RegionalSlot) return word_at(value, 8);
  throw Failure{932};
}
I encode_operand(View scalar, View destination) {
  const I value = scalar_value(scalar), offset = destination_value(destination);
  if (destination.evidence == T_RegionalSlot && scalar.evidence == T_BooleanLiteral)
    return (value ? 255 : 0) + offset;
  return value + offset;
}

void replace_x(View plan, View replacement, bool fail) {
  if (replacement.evidence != T_IntegerLiteral && replacement.evidence != T_BooleanLiteral) throw Failure{933};
  if (fail) throw Failure{934};
  switch (plan.evidence) {
  case T_Example: {
    auto p = object<Example>(plan); store_choice(p.region().cast_ptr(&p->x), replacement); return;
  }
  case T_Other: {
    auto p = object<Other>(plan); store_choice(p.region().cast_ptr(&p->x), replacement); return;
  }
  default: throw Failure{935};
  }
}

View method_0_general(View a0, View a1, memory_region<Root> &out, const FamilyValue &context) {
  trace(1000);
  const View x = read_x(a0);
  const I scalar = scalar_value(x);
  trace(1100, scalar);
  const View y = read_y(a0);
  const I location = destination_value(y);
  trace(1101, location);
  const I encoded = encode_operand(x, y);
  const I value = encoded + destination_value(a1) + 10 + captured_adjustment(context);
  const TypeId kind = a1.evidence == T_RegionalSlot ? T_AddressSummary : T_ValueSummary;
  trace(1200, value);
  return summary(out, kind, value);
}

View method_0_literal(View a0, View a1, memory_region<Root> &out, const FamilyValue &context) {
  trace(1001);
  const View x = read_x(a0);
  const I scalar = scalar_value(x);
  trace(1100, scalar);
  const View y = read_y(a0);
  const I location = destination_value(y);
  trace(1101, location);
  const I encoded = encode_operand(x, y);
  const I value = encoded + destination_value(a1) + 20 + captured_adjustment(context);
  const TypeId kind = a1.evidence == T_RegionalSlot ? T_AddressSummary : T_ValueSummary;
  trace(1200, value);
  return summary(out, kind, value);
}

View method_0_address(View a0, View a1, memory_region<Root> &out, const FamilyValue &context) {
  trace(1002);
  const View x = read_x(a0);
  const I scalar = scalar_value(x);
  trace(1100, scalar);
  const View y = read_y(a0);
  const I location = destination_value(y);
  trace(1101, location);
  const I encoded = encode_operand(x, y);
  const I value = encoded + destination_value(a1) + 30 + captured_adjustment(context);
  const TypeId kind = a1.evidence == T_RegionalSlot ? T_AddressSummary : T_ValueSummary;
  trace(1200, value);
  return summary(out, kind, value);
}

View method_0_intersection(View a0, View a1, memory_region<Root> &out, const FamilyValue &context) {
  trace(1003);
  const View x = read_x(a0);
  const I scalar = scalar_value(x);
  trace(1100, scalar);
  const View y = read_y(a0);
  const I location = destination_value(y);
  trace(1101, location);
  const I encoded = encode_operand(x, y);
  const I value = encoded + destination_value(a1) + 40 + captured_adjustment(context);
  const TypeId kind = a1.evidence == T_RegionalSlot ? T_AddressSummary : T_ValueSummary;
  trace(1200, value);
  return summary(out, kind, value);
}

View select_0(View a0, View a1, memory_region<Root> &out, const FamilyValue &context) {
  const bool applicable_0 = (a0.evidence == T_Example || a0.evidence == T_Other) && (a1.evidence == T_LocalSlot || a1.evidence == T_RegionalSlot);
  const bool applicable_1 = (a0.evidence == T_Example) && (a1.evidence == T_LocalSlot || a1.evidence == T_RegionalSlot);
  const bool applicable_2 = (a0.evidence == T_Example || a0.evidence == T_Other) && (a1.evidence == T_RegionalSlot);
  const bool applicable_3 = (a0.evidence == T_Example) && (a1.evidence == T_RegionalSlot);
  if (applicable_0 && !applicable_1 && !applicable_2 && !applicable_3) return method_0_general(a0, a1, out, context);
  if (applicable_1 && !applicable_3) return method_0_literal(a0, a1, out, context);
  if (applicable_2 && !applicable_3) return method_0_address(a0, a1, out, context);
  if (applicable_3) return method_0_intersection(a0, a1, out, context);
  throw Failure{930}; // Unreachable after coverage and admission.
}

View method_1_general(View a0, View a1, memory_region<Root> &out, const FamilyValue &context) {
  trace(1010);
  const View x = read_x(a0);
  const I scalar = scalar_value(x);
  trace(1100, scalar);
  const View y = read_y(a0);
  const I location = destination_value(y);
  trace(1101, location);
  const I encoded = encode_operand(x, y);
  const I value = encoded + destination_value(a1) + 110 + captured_adjustment(context);
  const TypeId kind = a1.evidence == T_RegionalSlot ? T_AddressSummary : T_ValueSummary;
  trace(1200, value);
  return summary(out, kind, value);
}

View method_1_literal(View a0, View a1, memory_region<Root> &out, const FamilyValue &context) {
  trace(1011);
  const View x = read_x(a0);
  const I scalar = scalar_value(x);
  trace(1100, scalar);
  const View y = read_y(a0);
  const I location = destination_value(y);
  trace(1101, location);
  const I encoded = encode_operand(x, y);
  const I value = encoded + destination_value(a1) + 120 + captured_adjustment(context);
  const TypeId kind = a1.evidence == T_RegionalSlot ? T_AddressSummary : T_ValueSummary;
  trace(1200, value);
  return summary(out, kind, value);
}

View method_1_address(View a0, View a1, memory_region<Root> &out, const FamilyValue &context) {
  trace(1012);
  const View x = read_x(a0);
  const I scalar = scalar_value(x);
  trace(1100, scalar);
  const View y = read_y(a0);
  const I location = destination_value(y);
  trace(1101, location);
  const I encoded = encode_operand(x, y);
  const I value = encoded + destination_value(a1) + 130 + captured_adjustment(context);
  const TypeId kind = a1.evidence == T_RegionalSlot ? T_AddressSummary : T_ValueSummary;
  trace(1200, value);
  return summary(out, kind, value);
}

View method_1_intersection(View a0, View a1, memory_region<Root> &out, const FamilyValue &context) {
  trace(1013);
  const View x = read_x(a0);
  const I scalar = scalar_value(x);
  trace(1100, scalar);
  const View y = read_y(a0);
  const I location = destination_value(y);
  trace(1101, location);
  const I encoded = encode_operand(x, y);
  const I value = encoded + destination_value(a1) + 140 + captured_adjustment(context);
  const TypeId kind = a1.evidence == T_RegionalSlot ? T_AddressSummary : T_ValueSummary;
  trace(1200, value);
  return summary(out, kind, value);
}

View select_1(View a0, View a1, memory_region<Root> &out, const FamilyValue &context) {
  const bool applicable_0 = (a0.evidence == T_Example || a0.evidence == T_Other) && (a1.evidence == T_LocalSlot || a1.evidence == T_RegionalSlot);
  const bool applicable_1 = (a0.evidence == T_Example) && (a1.evidence == T_LocalSlot || a1.evidence == T_RegionalSlot);
  const bool applicable_2 = (a0.evidence == T_Example || a0.evidence == T_Other) && (a1.evidence == T_RegionalSlot);
  const bool applicable_3 = (a0.evidence == T_Example) && (a1.evidence == T_RegionalSlot);
  if (applicable_0 && !applicable_1 && !applicable_2 && !applicable_3) return method_1_general(a0, a1, out, context);
  if (applicable_1 && !applicable_3) return method_1_literal(a0, a1, out, context);
  if (applicable_2 && !applicable_3) return method_1_address(a0, a1, out, context);
  if (applicable_3) return method_1_intersection(a0, a1, out, context);
  throw Failure{930}; // Unreachable after coverage and admission.
}

View invoke_lower(View plan, View dest, memory_region<Root> &out, const FamilyValue &context) {
  if (context.interpretation == 0) return select_0(plan, dest, out, context);
  if (context.interpretation == 1) return select_1(plan, dest, out, context);
  throw Failure{936};
}

I kind_integer(View) { return 31; }

I kind_boolean(View) { return 32; }

I kind_meters(View) { return 71; }

I kind_seconds(View) { return 72; }

I select_kind(View a0) {
  const bool applicable_0 = (a0.evidence == T_IntegerLiteral);
  const bool applicable_1 = (a0.evidence == T_BooleanLiteral);
  const bool applicable_2 = (a0.evidence == T_Meters);
  const bool applicable_3 = (a0.evidence == T_Seconds);
  if (applicable_0) return kind_integer(a0);
  if (applicable_1) return kind_boolean(a0);
  if (applicable_2) return kind_meters(a0);
  if (applicable_3) return kind_seconds(a0);
  throw Failure{930}; // Unreachable after coverage and admission.
}

I common_scalar(View) { return 17; }

I select_common(View a0) {
  const bool applicable_0 = (a0.evidence == T_BooleanLiteral || a0.evidence == T_IntegerLiteral || a0.evidence == T_Meters || a0.evidence == T_Seconds);
  if (applicable_0) return common_scalar(a0);
  throw Failure{930}; // Unreachable after coverage and admission.
}

I callsite_meter(View value) { return select_kind(value); }

I callsite_common(View value) { return select_common(value); }

I consume_summary(View result) {
  if (result.evidence != T_ValueSummary && result.evidence != T_AddressSummary) throw Failure{937};
  return word_at(result, 0);
}
struct Completion { I value = 0, error = 0; bool has_return = false; };
Completion run_scope(View plan, View destination, memory_region<Root> &out,
                     FamilyValue entry, FamilyValue dependency, I mode, I cleanup, Native &native) {
  const FamilyValue saved = *current_family;
  trace(80); // Evaluate suppressed dependency initialization operand.
  const FamilyValue suppressed = dependency;
  (void)suppressed; trace(81); // Constructed value; no TLS write/restoration event.
  install(entry);
  const FamilyValue captured = entry;
  entry = dependency; // Replacing this holder cannot alter captured.
  if (mode) install(dependency); // Procedure-local installation remains active.
  const FamilyValue selected = mode == 2 ? captured : *current_family;
  Completion pending;
  std::vector<I> actions;
  try {
    native.acquire(1); actions.push_back(1); actions.push_back(4);
    native.acquire(2); actions.push_back(2);
    native.acquire(3); actions.push_back(3); actions.push_back(5);
    const View result = invoke_lower(plan, destination, out, selected);
    assert(&result.payload.pointer->region() == &out);
    const I value = consume_summary(result); // Abstract stored result and return.
    native.emit(value);
    pending = {value, 0, true};
  } catch (Failure error) { pending.error = error.code; }
  while (!actions.empty()) {
    const I action = actions.back(); actions.pop_back();
    if (action <= 3) { native.release(action); continue; }
    if (action == 5) {
      trace(85);
      if (cleanup) { pending.error = 902; trace(86); }
      continue;
    }
    assert(action == 4);
    if (pending.error) {
      trace(90, pending.error); pending.error = 0;
      if (cleanup == 2) pending = {777, 0, true};
    }
    trace(95);
    if (cleanup == 3) pending = {888, 0, true};
  }
  restore(saved);
  if (!pending.error && !pending.has_return) pending.error = 903;
  trace(800, pending.value, pending.error);
  return pending;
}
