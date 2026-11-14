#!/usr/bin/env python3
"""Lossless fold updates preserve the accumulator's established type and cleanup."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
TYPES = ('u8', 'u32', 'u64', 'i64', 'f32', 'f64')
ZERO = dict(zip(TYPES, ('0u8', '0u32', '0u64', '0', '0f32', '0f64')))
VALUE = dict(zip(TYPES, ('255u8', '4294967295u32', '18446744073709551615u64', '-7', '0.5f32', '0.5f64')))
OUTPUT = dict(zip(TYPES, ('255', '4294967295', '18446744073709551615', '-7', '0.5', '0.5')))
WIDEN = {'u8': {'u32', 'u64', 'i64', 'f32', 'f64'}, 'u32': {'u64', 'i64', 'f64'}, 'f32': {'f64'}}
TAGS = ''.join(f'func tag(x:{type})->u64{{return {index}u64;}}' for index, type in enumerate(TYPES))

runner.CASES = {}
for source in TYPES:
    for index, target in enumerate(TYPES):
        accepted = source == target or target in WIDEN.get(source, set())
        program = IO + TAGS + f'val x<-fold(i in 0..2,a={ZERO[target]}){{{VALUE[source]}}};behave @print(tag(x));behave @print(x);'
        runner.CASES[f'{source}_to_{target}'] = (
            program, 0 if accepted else 2, f'{index}\n{OUTPUT[source]}\n' if accepted else '',
            '' if accepted else 'fold body does not satisfy the accumulator type')

runner.CASES.update({
    'shared_accumulator_types': (IO + TAGS + 'func f(seed:T)->const T where T {return fold(i in 0..2,a=seed){7u8};}'
        'behave @print(tag(f(0u32)));behave @print(tag(f(0f64)));behave @print(f(0u32));behave @print(f(0f64));',
        0, '1\n5\n7\n7\n', ''),
    'shared_body_types': (IO + 'func f(x:T)->f64 where T {return fold(i in 0..2,a=0f64){x};}'
        'behave @print(f(255u8));behave @print(f(4294967295u32));behave @print(f(0.5f32));behave @print(f(1f64));',
        0, '255\n4294967295\n0.5\n1\n', ''),
    'shared_exact_and_widened': (IO + 'func f(seed:T,x:U)->const T where {T,U} {return fold(i in 0..2,a=seed){x};}'
        'behave @print(f(0u8,7u8));behave @print(f(0f64,7u8));behave @print(f(0f64,0.5f32));',
        0, '7\n7\n0.5\n', ''),
    'captured_accumulator_types': (IO + TAGS + 'func small()->u32{return 0u32;}func large()->f64{return 0f64;}'
        'func factory(seed:func()->const T)->func()->const T where T {'
        'return func[seed]()->const T {return fold(i in 0..2,a=seed()){7u8};};}'
        'func a=factory(small);func b=factory(large);behave @print(tag(a()));behave @print(tag(b()));'
        'behave @print(a());behave @print(b());', 0, '1\n5\n7\n7\n', ''),
    'readonly_updated_accumulator': (IO + 'func f(x:f64)->u8 {return if(x==0f64){2u8}else{5u8};}'
        'behave @print(fold(i in 0..3,a=0f64){f(a)});', 0, '5\n', ''),
    'empty_range': (IO + 'behave @print(fold(i in 3..3,a=9f64){behave @print(99);7u8});', 0, '9\n', ''),
    'body_scope_cleanup': (IO + 'proc observe(x:f64)->void {behave @print(x);}'
        'behave @print(fold(i in 0..2,a=0f64){{defer observe(a);7u8}});', 0, '0\n7\n7\n', ''),
    'deferred_snapshots': (IO + 'proc observe(x:f64)->void {behave @print(x);}'
        '{{behave @print(fold(i in 0..2,a=0f64){defer observe(a);7u8});}};', 0, '7\n7\n0\n', ''),
    'body_failure': (IO + 'proc observe(x:f64)->void {behave @print(x);}'
        'func bad()->u8 {} {{defer observe(9f64);fold(i in 0..2,a=0f64){defer observe(a);bad()};}};',
        70, '0\n9\n', 'uncaught MissingReturnException'),
    'body_return': (IO + 'func f()->f64 {return fold(i in 0..2,a=0f64){if(i==0){7u8}else{return a;}};}'
        'behave @print(f());', 0, '7\n', ''),
    'literal_does_not_narrow': ('fold(i in 0..2,a=0u8){1u32};', 2, '', 'fold body does not satisfy'),
    'empty_body_still_checked': ('fold(i in 0..0,a=0u8){1u32};', 2, '', 'fold body does not satisfy'),
    'bool_stays_distinct': ('fold(i in 0..2,a=0u8){true};', 2, '', 'fold body does not satisfy'),
})
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
