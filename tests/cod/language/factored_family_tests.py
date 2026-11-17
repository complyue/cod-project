#!/usr/bin/env python3
"""Fixed scalar families preserve original-input selection and preparation."""
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
    CALL.replace('1u8,', '{{held<-other;1u8}},', 1) + CALL, 0, f'{32000 + TOTAL}\n{33000 + TOTAL}\n', '')
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
