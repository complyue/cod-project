"""AOT source lowerer: one body per method/interpretation, no specialization pass."""
import json
from pathlib import Path
from model import Universe, Type, TYPES, STABLE_IDS, METHODS, Method, coverage, dominates, callsite_targets


def match(u, variable, constraint):
    return '(' + ' || '.join(f'{variable}.evidence == T_{name}' for name in sorted(u.members(constraint))) + ')'


def select(u, methods, parameters, name, result, arguments, invocation):
    proof = coverage(u, parameters, methods)
    lines = [f'{result} {name}({arguments}) {{']
    for i, m in enumerate(methods):
        conditions = ' && '.join(match(u, f'a{j}', t) for j, t in enumerate(m.signature))
        lines.append(f'  const bool applicable_{i} = {conditions};')
    for i, m in enumerate(methods):
        exclusions = ''.join(f' && !applicable_{j}' for j, n in enumerate(methods) if dominates(u, n, m))
        lines.append(f'  if (applicable_{i}{exclusions}) return {invocation(m)};')
    lines += ['  throw Failure{930}; // Unreachable after coverage and admission.', '}']
    return '\n'.join(lines), proof


def generate(directory: Path, reverse=False, install_entry=True, simplify_callsites=False, profile="base"):
    if not install_entry:
        raise ValueError('entry extension requires explicit installation')
    directory.mkdir(parents=True, exist_ok=True)
    types = TYPES if profile == "base" else [Type("EarlierUnrelated", parents=("Datum",), size=8, align=8), *reversed(TYPES)]
    codes = {t.name: STABLE_IDS[t.name] for t in types}
    if profile == "wrong":
        codes["Example"],codes["Other"] = codes["Other"],codes["Example"]
    u = Universe(types,codes=codes)
    header = ['// Generated from the finite selected semantic artifact.',
              'struct Descriptor { TypeId identity; const char *name; const char *semantic_key; bool abstract; std::size_t size, align; };']
    for name, fp in u.type_ids.items():
        header.append(f'constexpr TypeId T_{name} = {fp}ULL;')
    header.append('static constexpr Descriptor descriptors[] = {')
    for name, t in u.types.items():
        header.append(f'  {{T_{name}, "{name}", "org.cod.fixture.micro.{name}", {str(t.abstract).lower()}, {t.size}, {t.align}}},')
    header.append('};')
    header.append('static constexpr TypeId root_evidence[] = {T_Example,T_Other,T_IntegerLiteral,T_BooleanLiteral,T_LocalSlot,T_RegionalSlot,T_ValueSummary,T_AddressSummary};')
    (directory/'type_ids.inc').write_text('\n'.join(header)+'\n')
    layout = []
    cpp_types = {'IntegerLiteral': 'Word', 'BooleanLiteral': 'Word', 'Meters':'Word', 'Seconds':'Word',
                 'LocalSlot':'Word', 'RegionalSlot':'RegionalSlot', 'Example':'Example', 'Other':'Other',
                 'ValueSummary':'Word', 'AddressSummary':'Word'}
    for name, cpp in cpp_types.items():
        t = u.types[name]
        layout += [f'static_assert(sizeof({cpp}) == {t.size} && alignof({cpp}) == {t.align});']
        for field, offset, _, _ in t.fields:
            layout += [f'static_assert(offsetof({cpp}, {field}) == {offset});']
    layout += ['static_assert(sizeof(TypeId) == 8 && sizeof(Choice) == 16);',
               'static_assert(std::endian::native == std::endian::little);']
    code = layout[:]
    access_paths = {}
    # Generate local field access from declared storage, not a nested-choice product.
    for field_name in ('x', 'y'):
        lines = [f'View read_{field_name}(View plan) {{', '  switch (plan.evidence) {']
        for name in sorted(u.members('Plan')):
            fields = {f: (o,k,a) for f,o,k,a in u.types[name].fields}
            if field_name not in fields or fields[field_name][1] != 'choice':
                raise ValueError('missing field contract')
            lines.append(f'  case T_{name}: return field<{name}>(plan, &{name}::{field_name});')
        lines += ['  default: throw Failure{931};', '  }', '}']
        code.append('\n'.join(lines)); access_paths[field_name] = len(u.members('Plan'))
    # All admitted Scalars have the same value field offset/storage. No tag test.
    offsets = {(dict((f,(o,k)) for f,o,k,_ in u.types[n].fields)['value']) for n in u.members('Scalar')}
    assert offsets == {(0,'i64')}
    code.append('I scalar_value(View value) { return word_at(value, 0); }')
    code.append('''I destination_value(View value) {
  if (value.evidence == T_LocalSlot) return word_at(value, 0);
  if (value.evidence == T_RegionalSlot) return word_at(value, 8);
  throw Failure{932};
}
I encode_operand(View scalar, View destination) {
  const I value = scalar_value(scalar), offset = destination_value(destination);
  if (destination.evidence == T_RegionalSlot && scalar.evidence == T_BooleanLiteral)
    return (value ? 255 : 0) + offset;
  return value + offset;
}''')
    # This local update is generated from the same field contract. A prepared
    # replacement can fail before commit; admitted single-thread writes cannot.
    code.append('''void replace_x(View plan, View replacement, bool fail) {
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
}''')
    methods = list(reversed(METHODS)) if reverse else METHODS
    proofs, body_names = [], []
    ids = {m.name: i for i,m in enumerate(METHODS)}
    for view in (0,1):
        for m in methods:
            body_name = f'method_{view}_{m.name}'
            body_names.append(body_name)
            # Authored operation template: field reads, scalar operation, context and result.
            code.append(f'''View {body_name}(View a0, View a1, memory_region<Root> &out, const FamilyValue &context) {{
  trace({1000+view*10+ids[m.name]});
  const View x = read_x(a0);
  const I scalar = scalar_value(x);
  trace(1100, scalar);
  const View y = read_y(a0);
  const I location = destination_value(y);
  trace(1101, location);
  const I encoded = encode_operand(x, y);
  const I value = encoded + destination_value(a1) + {m.bias+view*100} + captured_adjustment(context);
  const TypeId kind = a1.evidence == T_RegionalSlot ? T_AddressSummary : T_ValueSummary;
  trace(1200, value);
  return summary(out, kind, value);
}}''')
        text, proof = select(u, methods, ('Plan','Destination'), f'select_{view}', 'View',
                             'View a0, View a1, memory_region<Root> &out, const FamilyValue &context',
                             lambda m: f'method_{view}_{m.name}(a0, a1, out, context)')
        code.append(text); proofs.append(proof)
    code.append('''View invoke_lower(View plan, View dest, memory_region<Root> &out, const FamilyValue &context) {
  if (context.interpretation == 0) return select_0(plan, dest, out, context);
  if (context.interpretation == 1) return select_1(plan, dest, out, context);
  throw Failure{936};
}''')
    constants = [Method('integer',('IntegerLiteral',),31), Method('boolean',('BooleanLiteral',),32),
                 Method('meters',('Meters',),71), Method('seconds',('Seconds',),72)]
    for m in constants:
        code.append(f'I kind_{m.name}(View) {{ return {m.bias}; }}')
        body_names.append('kind_'+m.name)
    text, proof = select(u, constants, ('Scalar',), 'select_kind', 'I', 'View a0', lambda m: f'kind_{m.name}(a0)')
    code.append(text); proofs.append(proof)
    common = [Method('common',('Scalar',),17)]
    code.append('I common_scalar(View) { return 17; }')
    body_names.append('common_scalar')
    text,proof=select(u,common,('Scalar',),'select_common','I','View a0',lambda m:'common_scalar(a0)')
    code.append(text);proofs.append(proof)
    exact=callsite_targets(u,constants,[{'Meters'}])
    x_alternatives=set().union(*(set(a) for n in u.members('Plan') for f,o,k,a in u.types[n].fields if f=='x'))
    several=callsite_targets(u,common,[x_alternatives])
    assert exact == ['meters'] and several == ['common']
    code.append('I callsite_meter(View value) { return '+('kind_meters' if simplify_callsites else 'select_kind')+'(value); }')
    code.append('I callsite_common(View value) { return '+('common_scalar' if simplify_callsites else 'select_common')+'(value); }')
    code.append('''I consume_summary(View result) {
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
}''')
    (directory/'generated.inc').write_text('\n\n'.join(code)+'\n')
    info = {'specialization': 'disabled; no BBV, versioning, tuple cloning or runtime compiler',
            'call_site_selection_elision':simplify_callsites,
            'call_site_proofs':{'exact_Meters':exact,'IntegerLiteral_or_BooleanLiteral':several},
            'world_profile':profile, 'method_bodies': body_names, 'body_count':len(body_names), 'field_access_paths':access_paths,
            'coverage':proofs, 'type_ids':u.type_ids,
            'canonical_descriptors':{name:u.canonical(name) for name in u.types}, 'source_positions':body_names+['read_x','read_y','encode_operand','run_scope']}
    (directory/'generation.json').write_text(json.dumps(info,indent=2)+'\n')
    return info
