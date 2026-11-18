#!/usr/bin/env python3
"""Fixed scalar family coverage preserves preparation and complete values."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
COUNT = 32
PARAMETERS = ','.join(f'n{i}:u64' for i in range(COUNT))
INPUTS = ','.join(['u64'] * COUNT)
NAMES = ','.join(f'n{i}' for i in range(COUNT))
ARGUMENTS = ','.join(f'{i + 1}{("u8", "u32", "u64")[i % 3]}' for i in range(COUNT))
VIEW = f'func({INPUTS})->u64'
BASE = f'func base({PARAMETERS})->u64{{return n0+n31;}}'
HELD = f'func held:{VIEW}=base;'
CALL = f'behave @print(held({ARGUMENTS}));'
runner.CASES = {
    'contextual_value_result': (IO + f'func held:{VIEW}=func({PARAMETERS}){{return 7u32;}};' + CALL, 0, '7\n', ''),
    'contextual_returned_result': (IO + f'func factory()->{VIEW}{{return func({PARAMETERS}){{return 7u32;}};}}'
        'func held=factory();' + CALL, 0, '7\n', ''),
    'query_context_retains_fixed_view': (IO +
        'func classify(x:u8)->u64{return 1u64;}func classify(x:u32)->u64{return 7u64;}' +
        f'func factory(sample:T)->{VIEW} where T{{return func({PARAMETERS})->u64{{'
        'val value<-make local typeof(sample){7u8};return classify(value);};}' +
        'func first=factory(1u8);func second=factory(1u32);' +
        f'behave @print(first({ARGUMENTS}));behave @print(second({ARGUMENTS}));', 0, '1\n7\n', ''),
    'mixed_inputs': (IO + BASE + HELD + CALL, 0, '33\n', ''),
    'inferred_signature': (IO + BASE.replace('->u64', '') + 'func held=base;' + CALL, 0, '33\n', ''),
    'fixed_declaration': (IO + f'func base({PARAMETERS})->u64;' + BASE + 'func held=base;' + CALL, 0, '33\n', ''),
    'wider_formals': (IO + BASE.replace(':u64', ':f64').replace('->u64', '->f64') +
        f'func held:func({INPUTS.replace("u64", "u32")})->f64=base;' +
        CALL.replace('u64', 'u32'), 0, '33\n', ''),
    'independent_result_adaptation': (IO + f'func base({PARAMETERS})->u8{{return 255u8;}}' +
        f'func held:func({INPUTS})->f32=base;symbol @kind:proc(f32)->u64;'
        'proc @kind(x:f32)->u64{return 7u64;}proc @kind(x:T)->u64 where T{return 1u64;}effect @kind;'
        f'behave @print(behave @kind(held({ARGUMENTS})));', 0, '7\n', ''),
    'declaration_inspection_without_body': (IO + f'func base({PARAMETERS})->u64;func inspect(f:{VIEW})->u64;'
        'val value<-make local typeof(inspect(base)){7u64};behave @print(value);', 0, '7\n', ''),
    'inspection_does_not_execute_body': (IO + BASE.replace('func base', 'proc base').replace('return n0+n31;', 'behave @print(99u64);return n0+n31;') +
        f'func inspect(f:proc({INPUTS})->u64)->u64;val value<-make local typeof(inspect(base)){{7u64}};behave @print(value);', 0, '7\n', ''),
    'returned_complete_capture': (IO + BASE + f'func wrap(f:{VIEW})->{VIEW}{{return func[f]({PARAMETERS})->u64{{return f({NAMES})+1u64;}};}}'
        'func held=wrap(wrap(base));' + CALL, 0, '35\n', ''),
    'prepared_callee_before_operand_rebind': (IO + BASE + BASE.replace('base(', 'other(').replace('n0+n31', 'n0+n31+10u64') +
        HELD + CALL.replace('1u8,', '{held<-other;1u8},', 1) + CALL, 0, '33\n43\n', ''),
    'deferred_prepared_call': (IO + f'proc show({PARAMETERS})->void{{behave @print(n0+n31);}}'
        f'proc held:proc({INPUTS})->void=show;defer held({ARGUMENTS});behave @print(1u64);', 0, '1\n33\n', ''),
    'complete_dynamic_replay': (IO + 'symbol @bias:u64;' +
        f'proc @source({PARAMETERS})->u64{{return n0+n31+@bias;}}'
        'effect @bias<-1u64;effect @source;effect @bias<-7u64;'
        f'behave @print(perform @source({ARGUMENTS}));behave @print(behave @source({ARGUMENTS}));behave @print(@bias);', 0, '34\n40\n7\n', ''),
    'declared_checked_obligation': ('using "std:exceptions" StaticallyUnknownTypeException;' +
        BASE.replace('->u64{', '->u64 throws StaticallyUnknownTypeException{') + HELD,
        2, '', 'required checked exception'),
    'checked_deferred_call': (IO + 'using "std:exceptions" StaticallyUnknownTypeException;' +
        f'proc show({PARAMETERS})->void throws StaticallyUnknownTypeException{{behave @print(n0+n31);}}'
        f'proc held:proc({INPUTS})->void throws StaticallyUnknownTypeException=show;'
        f'{{{{defer catch(e:const StaticallyUnknownTypeException){{}}finally{{behave @print(7u64);}};defer held({ARGUMENTS});}}}};',
        0, '33\n7\n', ''),
    'failure_runs_remaining_cleanup': (IO + 'using "std:exceptions" MissingReturnError;func missing()->u64{}' +
        f'proc broken({PARAMETERS})->u64{{defer{{behave @print(42u64);}};return missing();}}'
        f'proc held:proc({INPUTS})->u64=broken;try{{held({ARGUMENTS});{{}}}}catch(e:const MissingReturnError){{behave @print(7u64);}};',
        0, '42\n7\n', '[-Wmissing-return]'),
    'invalid_unused_body': (BASE.replace('n0+n31', 'true') + HELD, 2, '', 'return type'),
    'narrow_inputs_do_not_cover_view': (BASE.replace(':u64', ':u8') + HELD, 2, '', 'no applicable method'),
    'repeated_generic_identity': ('func base(a:T,b:T)->u64 where T{return 1u64;}func held:func(u64,u64)->u64=base;',
        2, '', 'no applicable method'),
    'partial_declaration_is_not_skipped': ('using "std:exceptions" StaticallyUnknownTypeException;'
        'func base(a:u8,b:u64)->u64 throws StaticallyUnknownTypeException;'
        'func base(a:u64,b:u64)->u64{return a+b;}func held:func(u64,u64)->u64=base;',
        2, '', 'required checked exception'),
    'selected_module_interpretation': ({
        'base.cod': BASE.replace('n0+n31', 'helper(n0)') + 'func helper(x:T)->u64 where T{return 1u64;}export base;export helper;',
        'extension.cod': 'import "base.cod" base;import "base.cod" helper;func helper(x:u64)->u64{return 7u64;}export base;',
        'main.cod': IO + 'using "base.cod" base as original;using "extension.cod" base as extended;' +
            f'func a:{VIEW}=original;func b:{VIEW}=extended;' +
            f'behave @print(a({ARGUMENTS}));behave @print(b({ARGUMENTS}));behave @print(a({ARGUMENTS}));',
    }, 0, '1\n7\n1\n', ''),
}
for count in (8, 64):
    parameters = ','.join(f'n{i}:u64' for i in range(count))
    inputs = ','.join(['u64'] * count)
    arguments = ','.join(f'{i + 1}u8' for i in range(count))
    runner.CASES[f'arity_{count}'] = (IO + f'func base({parameters})->u64{{return n0+n{count-1};}}'
        f'func held:func({inputs})->u64=base;behave @print(held({arguments}));', 0, f'{count+1}\n', '')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
