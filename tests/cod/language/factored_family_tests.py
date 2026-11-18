#!/usr/bin/env python3
"""Shared scalar formals preserve original-input selection and preparation."""
from itertools import product
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
COUNT = 16


def parameters(count=COUNT, first='u64', common='u64', last=None):
    return ','.join(f'n{i}:' + (first if i == 0 else last if i == count - 1 and last else common)
                    for i in range(count))


def view(count=COUNT, kind='func', common='u64', result='u64', failure=''):
    return kind + '(' + ','.join([common] * count) + ')->' + result + failure


def arguments(count=COUNT, first='u8', common='u64'):
    kinds = ('u8', 'u32', 'u64') if common == 'u64' else ('u8', 'u32', 'f32', 'f64')
    return ','.join(str(i + 1) + (first if i == 0 else kinds[i % len(kinds)]) for i in range(count))


def family(count=COUNT, name='choose', kind='func', common='u64', extra=0):
    result = ''
    for first, tag in (('u32', 32000), (common, 64000)):
        expression = str(tag + extra) + common + '+' + '+'.join(f'n{i}' for i in range(count))
        result += kind + ' ' + name + '(' + parameters(count, first, common) + ')->' + common + '{return ' + expression + ';}'
    return result


VIEW = view()
PARAMETERS = parameters()
NAMES = ','.join(f'n{i}' for i in range(COUNT))
ARGUMENTS = arguments()
TOTAL = COUNT * (COUNT + 1) // 2
BASE = family()
HELD = 'func held:' + VIEW + '=choose;'
CALL = 'behave @print(held(' + ARGUMENTS + '));'

runner.CASES = {}
for count in (3, 16, 32):
    total = count * (count + 1) // 2
    runner.CASES[f'arity_{count}'] = (IO + family(count) + 'func held:' + view(count) + '=choose;' +
        'behave @print(held(' + arguments(count) + '));behave @print(held(' + arguments(count, 'u64') + '));',
        0, f'{32000 + total}\n{64000 + total}\n', '')

runner.CASES['floating_inputs'] = (IO + family(common='f64') + 'func held:' + view(common='f64', result='f64') + '=choose;' +
    'behave @print(held(' + arguments(common='f64') + '));behave @print(held(' + arguments(first='f32', common='f64') + '));',
    0, f'{32000 + TOTAL}\n{64000 + TOTAL}\n', '')
runner.CASES['ordered_operands'] = (IO + BASE + HELD +
    'proc operand(n:u8)->u8{behave @print(n);return n;}' +
    'behave @print(held(' + ','.join(f'operand({i + 1}u8)' for i in range(COUNT)) + '));',
    0, ''.join(f'{i + 1}\n' for i in range(COUNT)) + f'{32000 + TOTAL}\n', '')
runner.CASES['prepared_callee'] = (IO + BASE + family(name='other', extra=1000) + HELD +
    CALL.replace('1u8,', '{held<-other;1u8},', 1) + CALL, 0, f'{32000 + TOTAL}\n{33000 + TOTAL}\n', '')


def phase_family(count, common='u64', repeated=False):
    formals = parameters(count, first='T', common=common)
    if repeated:
        formals = formals.replace('n1:' + common, 'n1:T')
    fixed = parameters(count, first='u64', common=common)
    return ('func choose(' + formals + ')->u64 where T{return 101u64;}' +
            'func choose(' + fixed + ')->u64{return 202u64;}')


def compare_phase_selection(name, definitions, formals, calls):
    source = IO + definitions + 'func held:func(' + ','.join(formals) + ')->u64=choose;'
    expected = []
    for types, value in calls:
        operands = ','.join('1' + type_name for type_name in types)
        source += 'behave @print(choose(' + operands + '));behave @print(held(' + operands + '));'
        expected.extend((value, value))
    runner.CASES[name] = (source, 0, ''.join(f'{value}\n' for value in expected), '')


compare_phase_selection('generic_phase_exhaustive', phase_family(3), ['u64'] * 3,
    [(types, 101 if types[0] != 'u64' and all(t == 'u64' for t in types[1:]) else 202)
     for types in product(('u8', 'u32', 'u64'), repeat=3)])
for count in (16, 32):
    compare_phase_selection(f'generic_phase_width_{count}', phase_family(count), ['u64'] * count, [
        (['u8'] + ['u64'] * (count - 1), 101),
        (['u8', 'u8'] + ['u64'] * (count - 2), 202),
        (['u32'] + ['u64'] * (count - 2) + ['u32'], 202),
        (['u64'] * count, 202),
    ])
compare_phase_selection('generic_phase_same_type', phase_family(COUNT, repeated=True), ['u64'] * COUNT, [
    (['u8', 'u8'] + ['u64'] * (COUNT - 2), 101),
    (['u8', 'u32'] + ['u64'] * (COUNT - 2), 202),
    (['u8', 'u8', 'u8'] + ['u64'] * (COUNT - 3), 202),
])
compare_phase_selection('generic_phase_always_widens', phase_family(COUNT, common='f64'),
    ['u64'] + ['u32'] * (COUNT - 1), [
        (['u8'] + ['u32'] * (COUNT - 1), 202),
        (['u8'] * COUNT, 202),
    ])
CLASSIFY = 'func classify(x:u8)->u64{return 8u64;}func classify(x:u32)->u64{return 32u64;}'
compare_phase_selection('generic_phase_preserves_type',
    CLASSIFY + phase_family(COUNT).replace('return 101u64;', 'return classify(n0);'), ['u64'] * COUNT, [
        (['u8'] + ['u64'] * (COUNT - 1), 8),
        (['u32'] + ['u64'] * (COUNT - 1), 32),
        (['u32', 'u8'] + ['u64'] * (COUNT - 2), 202),
    ])
runner.CASES['generic_phase_signature_query'] = (
    IO + phase_family(COUNT) + 'func inspect(value:' + VIEW + ')->u64;'
    'val mark<-make local typeof(inspect(choose)){7u64};behave @print(mark);', 0, '7\n', '')

OTHER_ARITY = 'func choose(flag:bool)->u64{return 303u64;}'
DISJOINT_PARAMETERS = parameters(first='bool', common='u32')
DISJOINT_METHOD = 'func choose(' + DISJOINT_PARAMETERS + ')->bool{return true;}'
DISJOINT_DECLARATIONS = 'func choose()->bool;func choose(' + DISJOINT_PARAMETERS + ')->bool;'
runner.CASES['irrelevant_arity_preserves_wide_view'] = (
    IO + BASE + OTHER_ARITY + HELD + CALL + 'behave @print(choose(false));',
    0, f'{32000 + TOTAL}\n303\n', '')
runner.CASES['disjoint_scalar_keeps_ordinary_direct_call'] = (
    IO + DISJOINT_METHOD + BASE + HELD + CALL +
    'behave @print(choose(false,' + ','.join(['1u32'] * (COUNT - 1)) + '));',
    0, f'{32000 + TOTAL}\ntrue\n', '')
compare_phase_selection('disjoint_method_keeps_widening_phase', OTHER_ARITY + DISJOINT_METHOD + phase_family(COUNT),
    ['u64'] * COUNT, [
        (['u8'] + ['u64'] * (COUNT - 1), 101),
        (['u8', 'u8'] + ['u64'] * (COUNT - 2), 202),
        (['u64'] * COUNT, 202),
    ])
runner.CASES['disjoint_declarations_preserve_inspection'] = (
    IO + DISJOINT_DECLARATIONS + 'func choose(' + PARAMETERS + ')->u64;func inspect(value:' + VIEW + ')->u64;'
    'val mark<-make local typeof(inspect(choose)){7u64};behave @print(mark);', 0, '7\n', '')
runner.CASES['disjoint_contracts_preserve_symbolic_view'] = (
    IO + 'symbol @choose:' + view(kind='proc') + ';' + family(kind='proc', name='@choose') +
    (OTHER_ARITY + DISJOINT_METHOD).replace('func choose', 'proc @choose') +
    'effect @choose;behave @print(behave @choose(' + ARGUMENTS + '));',
    0, f'{32000 + TOTAL}\n', '')


def qualified_phase_family(count):
    readonly = parameters(count, first='const u64')
    return phase_family(count) + f'func choose({readonly})->u64{{return 303u64;}}'


for count in (16, 32):
    compare_phase_selection(f'qualified_phase_width_{count}', qualified_phase_family(count), ['u64'] * count, [
        (['u8'] + ['u64'] * (count - 1), 101),
        (['u8', 'u8'] + ['u64'] * (count - 2), 202),
        (['u32'] + ['u64'] * (count - 2) + ['u32'], 202),
        (['u64'] * count, 202),
    ])
runner.CASES['qualified_phase_signature_query'] = (
    IO + qualified_phase_family(COUNT) + 'func inspect(value:' + VIEW + ')->u64;'
    'val mark<-make local typeof(inspect(choose)){7u64};behave @print(mark);', 0, '7\n', '')
QUALIFIED_SYMBOL = qualified_phase_family(COUNT).replace('func choose(', 'proc @choose(').replace('u64;}', 'u64+@bias;}')
EXACT_TAIL = ','.join(['1u8'] + ['1u64'] * (COUNT - 1))
runner.CASES['qualified_symbol_uses_both_phases'] = (
    IO + 'symbol @bias:u64;symbol @choose:' + view(kind='proc') + ';' + QUALIFIED_SYMBOL +
    'effect @bias<-1u64;effect @choose;effect @bias<-7u64;'
    f'behave @print(perform @choose({EXACT_TAIL}));'
    f'behave @print(behave @choose({ARGUMENTS}));', 0, '102\n209\n', '')

QUALIFIED = 'func choose(' + parameters(first='const u64') + ')->u64{return 202u64;}'
READONLY_VIEW = VIEW.replace('func(u64', 'func(const u64', 1)
TAIL = ','.join(['1u64'] * (COUNT - 1))
runner.CASES['view_keeps_declared_access'] = (
    IO + 'func choose(' + PARAMETERS + ')->u64{return 101u64;}' + QUALIFIED +
    'func open:' + VIEW + '=choose;func fixed:' + READONLY_VIEW + '=choose;const val arg<-1u8;'
    f'behave @print(choose(1u8,{TAIL}));behave @print(open(arg,{TAIL}));'
    f'behave @print(choose(arg,{TAIL}));behave @print(fixed(1u8,{TAIL}));', 0, '101\n101\n202\n202\n', '')
runner.CASES['qualified_unused_case_must_conform'] = (
    'func choose(' + parameters(first='u8') + ')->u64{return 101u64;}' + QUALIFIED + HELD,
    2, '', 'callable view contract')

CALLBACK = 'func(u64)->u64'
CALLBACK_VIEW = VIEW.replace('func(', f'func({CALLBACK},', 1)
OFFSET = ('func with_offset(val delta:u64)->func(u64)->u64{'
          'return func[delta](value:u64)->u64{return value+delta;};}'
          'func callback=with_offset(10u64);')
FIXED_CALLBACK = f'func choose(cb:{CALLBACK},{PARAMETERS})->u64{{return cb(n0)+n15;}}'
GENERIC_CALLBACK = (f'func choose(cb:{CALLBACK},{parameters(first="T")})->u64 where T'
                    '{return cb(n0)+n15+100u64;}')
for name, methods, outputs in (
    ('exact_callback_with_scalar_inputs', FIXED_CALLBACK, '14\n14\n14\n14\n14\n14\n'),
    ('exact_callback_preserves_scalar_phases', GENERIC_CALLBACK+FIXED_CALLBACK, '114\n114\n14\n14\n14\n14\n'),
):
    source = IO + OFFSET + methods + f'func held:{CALLBACK_VIEW}=choose;'
    for first, second in (('1u8', '2u64'), ('1u8', '2u8'), ('1u64', '2u64')):
        inputs = ','.join(['callback', first, second] + ['3u64'] * (COUNT - 2))
        source += f'behave @print(choose({inputs}));behave @print(held({inputs}));'
    runner.CASES[name] = (source, 0, outputs, '')

CALLBACK_BINDS_TYPE = (f'func choose(cb:func(T)->u64,{parameters(first="T")})->u64 where T'
                       '{return cb(n0)+n15;}')
MATCHED_CALLBACK_VIEW = CALLBACK_VIEW.replace('func(u64)->u64,u64', 'func(u8)->u64,u8', 1)
MATCHED_INPUTS = ','.join(['increment', '1u8'] + ['3u64'] * (COUNT - 1))
runner.CASES['exact_callback_supplies_nested_type'] = (
    IO + 'func increment(value:u8)->u64{return value+10u64;}' + CALLBACK_BINDS_TYPE +
    f'func held:{MATCHED_CALLBACK_VIEW}=choose;behave @print(held({MATCHED_INPUTS}));', 0, '14\n', '')
runner.CASES['exact_callback_preserves_same_type_constraint'] = (
    CALLBACK_BINDS_TYPE + 'func held:' + MATCHED_CALLBACK_VIEW.replace('->u64,u8,', '->u64,u64,', 1) + '=choose;',
    2, '', 'no applicable method')
runner.CASES['exact_callback_declaration_query'] = (
    IO + f'func choose(cb:{CALLBACK},{PARAMETERS})->u64;func inspect(value:{CALLBACK_VIEW})->u64;'
    'val mark<-make local typeof(inspect(choose)){7u64};behave @print(mark);', 0, '7\n', '')

BORROWED_RESULT = (f'func choose(ref x:u64 in O,ref y:u64 in O,{PARAMETERS})->ref u64 in O'
                   ' where {owner O}{return y;}')
BORROWED_HOLDER = 'ref left:u64=1u64,right:u64=2u64;func held=choose;'
WRITE_SELECTED = f'held(left,right,{ARGUMENTS})<-9u64;'
runner.CASES['borrowed_result_with_scalar_inputs'] = (
    IO + BORROWED_RESULT + BORROWED_HOLDER + WRITE_SELECTED +
    'behave @print(left);behave @print(right);', 0, '1\n9\n', '')
runner.CASES['borrowed_inputs_keep_actual_owner_check'] = (
    BORROWED_RESULT + BORROWED_HOLDER +
    '{{ref local:u64=3u64;held(left,local,' + ARGUMENTS + ');}};', 2, '', 'different actual owners')
runner.CASES['borrowed_input_keeps_exact_target_type'] = (
    BORROWED_RESULT + BORROWED_HOLDER + 'ref narrower:u32=2u32;held(left,narrower,' + ARGUMENTS + ');',
    2, '', 'argument does not satisfy the callable view type')

BORROWED_DECLARATION = (f'func choose(ref x:u64 in O,ref y:u64 in O,{parameters(first="u32")})->ref u64 in O'
                        ' throws NullPointerException where {owner O};')
runner.CASES['borrowed_declaration_keeps_checked_obligation'] = (
    NPE + BORROWED_DECLARATION + BORROWED_RESULT + BORROWED_HOLDER + WRITE_SELECTED,
    2, '', 'requires a covering catch or throws declaration')
runner.CASES['borrowed_declaration_handled'] = (
    IO + NPE + BORROWED_DECLARATION + BORROWED_RESULT + BORROWED_HOLDER +
    'try{' + WRITE_SELECTED + '{}}catch(e:const NullPointerException){};behave @print(right);', 0, '9\n', '')
runner.CASES['nullable_borrowed_input_with_scalar_inputs'] = (
    IO + f'func choose(ptr x:u64 in O,{PARAMETERS})->bool where {{owner O}}{{return x is null;}}'
    'ref left:u64=1u64;ptr absent:u64=null in here;func held=choose;'
    f'behave @print(held(left,{ARGUMENTS}));behave @print(held(absent,{ARGUMENTS}));', 0, 'false\ntrue\n', '')

runner.CASES['returned_complete_capture'] = (IO + BASE +
    'func wrap(callback:' + VIEW + ')->' + VIEW + '{return func[callback](' + PARAMETERS +
    ')->u64{return callback(' + NAMES + ')+1u64;};}func held=wrap(choose);' + CALL,
    0, f'{64001 + TOTAL}\n', '')
runner.CASES['recursive_consumer'] = (IO + BASE +
    'func relay(callback:' + VIEW + ',depth:u64,small:bool)->u64{if(depth==0u64){'
    'if(small){return callback(' + ARGUMENTS + ');}else{return callback(' + arguments(first='u64') + ');};'
    '}else{return relay(callback,depth-1u64,small);};}' + HELD +
    'behave @print(relay(held,40u64,true));behave @print(relay(held,400u64,false));',
    0, f'{32000 + TOTAL}\n{64000 + TOTAL}\n', '')

SHOW = ''.join('proc show(' + parameters(first=first) + ')->void{behave @print(' + str(tag) +
    'u64+' + '+'.join(f'n{i}' for i in range(COUNT)) + ');}' for first, tag in (('u32', 32000), ('u64', 64000)))
OTHER_SHOW = SHOW.replace('show(', 'other(').replace('32000u64', '33000u64').replace('64000u64', '65000u64')
runner.CASES['deferred_prepared_callee'] = (IO + SHOW + OTHER_SHOW +
    'proc held:' + view(kind='proc', result='void') + '=show;defer held(' + ARGUMENTS + ');held<-other;behave @print(7u64);',
    0, f'7\n{32000 + TOTAL}\n', '')
runner.CASES['dynamic_replay'] = (IO + 'symbol @bias:u64;symbol @choose:' + view(kind='proc') + ';' +
    family(name='@choose', kind='proc').replace(';}', '+@bias;}') +
    'effect @bias<-1u64;effect @choose;effect @bias<-7u64;'
    'behave @print(perform @choose(' + ARGUMENTS + '));behave @print(behave @choose(' + ARGUMENTS + '));behave @print(@bias);',
    0, f'{32001 + TOTAL}\n{32007 + TOTAL}\n7\n', '')

PARTIAL = 'func choose(' + parameters(last='u32') + ')->u64 throws NullPointerException;'
IMPLEMENTATION = 'func choose(' + PARAMETERS + ')->u64{return n0+n15;}'
runner.CASES['partial_declaration_required'] = (NPE + PARTIAL + IMPLEMENTATION + HELD,
    2, '', 'required checked exception')
runner.CASES['disjoint_contracts_keep_overlapping_checked_obligation'] = (
    NPE + DISJOINT_DECLARATIONS + PARTIAL + IMPLEMENTATION + HELD, 2, '', 'required checked exception')
runner.CASES['partial_declaration_inferred'] = (IO + NPE + PARTIAL + IMPLEMENTATION + 'func held=choose;'
    'try{' + CALL + '{};}catch(e:const NullPointerException){};', 0, '17\n', '')
runner.CASES['declaration_query_without_body'] = (IO + NPE + PARTIAL +
    'func choose(' + PARAMETERS + ')->u64;func inspect(callback:' + view(failure=' throws NullPointerException') + ')->u64;'
    'val result<-make local typeof(inspect(choose)){7u64};behave @print(result);', 0, '7\n', '')
runner.CASES['qualifier_phase_ambiguity'] = (
    'func choose(' + parameters(first='const u8') + ')->u64{return 8u64;}'
    'func choose(' + parameters(first='u32') + ')->u64{return 32u64;}'
    'func held:func(u8,' + ','.join(['u64'] * (COUNT - 1)) + ')->u64=choose;', 2, '', 'ambiguous methods')
runner.CASES['cross_coordinate_ambiguity'] = (BASE +
    'func choose(' + PARAMETERS.replace('n1:u64', 'n1:u32') + ')->u64{return 96u64;}' + HELD,
    2, '', 'ambiguous methods')
NARROW = 'func choose(' + parameters(first='u32') + ')->bool{return true;}'
runner.CASES['narrow_result_mismatch'] = (NARROW +
    'func choose(' + PARAMETERS + ')->u64{return 64u64;}' + HELD, 2, '', 'callable view contract')
runner.CASES['selected_module_interpretation'] = ({
    'base.cod': ('func helper(x:T)->u64 where T{return 1u64;}' +
        'func choose(' + parameters(first='u32') + ')->u64{return helper(n0);}' +
        'func choose(' + PARAMETERS + ')->u64{return helper(n0);}export choose;export helper;'),
    'extension.cod': 'import "base.cod" choose;import "base.cod" helper;func helper(x:u32)->u64{return 7u64;}export choose;',
    'main.cod': IO + 'using "base.cod" choose as original;using "extension.cod" choose as extended;' +
        'func first:' + VIEW + '=original;func second:' + VIEW + '=extended;'
        'behave @print(first(' + ARGUMENTS + '));behave @print(second(' + ARGUMENTS + '));'
        'behave @print(second(' + arguments(first='u64') + '));behave @print(first(' + ARGUMENTS + '));',
}, 0, '1\n7\n1\n1\n', '')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
