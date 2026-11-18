#!/usr/bin/env python3
"""Lexical callable coverage retains actual inputs, captures and enclosing types."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
TAGS = ''.join(f'func tag(x:{kind})->u64{{return {value}u64;}}' for kind, value in
               [('u8', 8), ('u32', 32), ('u64', 64), ('i64', 65), ('f32', 132), ('f64', 164)])
runner.CASES = {
    'own_generic_input_identity': (IO + TAGS + 'func held:func(u64)->u64=func(x:T)->u64 where T{return tag(x);};'
        'behave @print(held(1u8));behave @print(held(1u32));behave @print(held(1u64));', 0, '8\n32\n64\n', ''),
    'concrete_parameter_prepares_input': (IO + TAGS + 'func held:func(u64)->u64=func(x:u64)->u64{return tag(x);};'
        'behave @print(held(1u8));behave @print(held(1u32));', 0, '64\n64\n', ''),
    'generic_result_adaptation': (IO + 'func held:func(u64)->const u64=func(x:T)->const T where T{return x;};'
        'behave @print(held(255u8));behave @print(held(4294967295u32));', 0, '255\n4294967295\n', ''),
    'fixed_result_adaptation': (IO + 'func held:func()->f64=func()->u32{return 4294967295u32;};behave @print(held());', 0, '4294967295\n', ''),
    'contextual_result_retains_actual_input': (IO + TAGS + 'func held:func(u64)->u64=func(x:T) where T{return tag(x);};'
        'behave @print(held(1u8));behave @print(held(1u64));', 0, '8\n64\n', ''),
    'inferred_concrete_view': (IO + TAGS + 'func held=func(x:u64){return tag(x);};behave @print(held(1u8));', 0, '64\n', ''),
    'captured_family_all_instances': (IO + TAGS + 'func classifier:func(u64)->u64=tag;'
        'func held:func(u64)->u64=func[classifier](x:T)->u64 where T{return classifier(x);};'
        'behave @print(held(1u8));behave @print(held(1u32));behave @print(held(1u64));', 0, '8\n32\n64\n', ''),
    'inferred_capture_all_instances': (IO + TAGS + 'func classifier:func(u64)->u64=tag;'
        'func held:func(u64)->u64=func(x:T)->u64 where T{return classifier(x);};'
        'behave @print(held(1u8));behave @print(held(1u64));', 0, '8\n64\n', ''),
    'captured_snapshot_survives_replacement': (IO + TAGS + 'func fixed(x:u64)->u64{return 7u64;}func classifier:func(u64)->u64=tag;'
        'func held:func(u64)->u64=func[classifier](x:T)->u64 where T{return classifier(x);};func copy=held;classifier<-fixed;'
        'behave @print(copy(1u8));behave @print(held(1u64));behave @print(classifier(1u8));', 0, '8\n64\n7\n', ''),
    'returned_generic_capture': (IO + TAGS + 'func factory(f:func(u64)->u64)->func(u64)->u64{return func[f](x:T)->u64 where T{return f(x);};}'
        'func classifier:func(u64)->u64=tag;func held=factory(classifier);behave @print(held(1u8));behave @print(held(1u64));', 0, '8\n64\n', ''),
    'named_recursive_fixed_capture': (IO + TAGS + 'func factory(f:func(u64)->u64)->func(u64,u64)->u64{'
        'func recur[f](x:u64,n:u64)->u64{if(n==0u64){return f(x);}else{return recur(x,n-1u64);};};return recur;}'
        'func classifier:func(u64)->u64=tag;func held=factory(classifier);behave @print(held(1u8,20u64));behave @print(held(1u64,30u64));', 0, '64\n64\n', ''),
    'named_generic_definition_without_view': ('proc run()->void{func id(x:T)->const T where T{return x;};}run();', 0, '', ''),
    'named_open_view_remains_unproved': ('proc run()->void{func id(x:T)->const T where T{return x;};func value=id;}run();', 2, '', 'require an explicit concrete view type'),
    'retained_and_own_types': (IO + TAGS + 'func factory(sample:T)->func(u64)->u64 where T{'
        'return func[](x:U)->u64 where U{val retained:T<-1u8;return tag(retained)+tag(x);};}'
        'func a=factory(1);func b=factory(1f64);behave @print(a(1u8));behave @print(b(1u8));behave @print(a(1u64));', 0, '73\n172\n129\n', ''),
    'retained_type_is_not_reinferred': (IO + TAGS + 'func factory(sample:T)->func(T)->u64 where T{return func[](x:T)->u64{return tag(x);};}'
        'func held=factory(1u64);behave @print(held(1u8));', 0, '64\n', ''),
    'nested_retained_context': (IO + TAGS + 'func factory(sample:T)->func()->func(u64)->u64 where T{return func[]()->func(u64)->u64{'
        'return func[](x:U)->u64 where U{val retained:T<-1u8;return tag(retained)+tag(x);};};}'
        'func held=factory(1f64)();behave @print(held(1u8));', 0, '172\n', ''),
    'deferred_actual_input_snapshot': (IO + TAGS + 'proc held:proc(u64)->void=proc(x:T)->void where T{behave @print(tag(x));};'
        'val x<-1u8;defer held(x);x<-2u8;defer held(1u64);', 0, '64\n8\n', ''),
    'inner_arithmetic_keeps_actual_type': ('func held:func(u64)->u64=func(x:T)->u64 where T{return x+x;};held(200u8);', 70, '', 'u8 addition overflow'),
    'retained_arithmetic_uses_fixed_type': (IO + 'func factory(sample:T)->func(T)->const T where T{return func[](x:T)->const T{return x+x;};}'
        'func held=factory(1u64);behave @print(held(200u8));', 0, '400\n', ''),
    'unused_argument_instance_is_checked': ('func held:func(u64)->const u64=func(x:T)->const T where T{return 1u64;};held(1u64);', 2, '', 'return type'),
    'existing_value_not_retyped': ('func first=func()->u32{return 1u32;};func second:func()->u64=first;', 2, '', 'initializer does not satisfy'),
    'ordinary_value_capture': ('val x<-1u64;func held:func(u64)->u64=func[x](n:T)->u64 where T{return x;};', 0, '', ''),
    'borrowed_result_retains_owner': (IO + 'func held=func(ref x:i64 in O)->ref i64 in O where {owner O}{return x;};'
        'val x<-1;held(x)<-7;behave @print(x);', 0, '7\n', ''),
    'dynamic_capture_covers_unused_inputs': (IO + 'symbol @a:u64,@b:u64,@q:proc(u64)->u64;proc pick(x:u64)->u64{return @a;}'
        'proc pick(x:T)->u64 where T{return @b;}effect @a<-1u64;effect @b<-7u64;'
        'proc held:proc(u64)->u64=proc(x:T)->u64 where T{return pick(x);};effect @a<-3u64;effect @b<-9u64;effect @q=held;'
        'behave @print(perform @q(1u8));behave @print(perform @q(1u64));behave @print(behave @q(1u8));', 0, '7\n1\n9\n', ''),
    'unused_dynamic_capture_required': ('symbol @a:u64,@b:u64;proc pick(x:u64)->u64{return @a;}proc pick(x:T)->u64 where T{return @b;}'
        'effect @a<-1u64;proc held:proc(u64)->u64=proc(x:T)->u64 where T{return pick(x);};', 2, '', 'cannot prove an installed binding'),
    'common_exception_actual_identity': (IO + 'using "std:exceptions" Exception;using "std:exceptions" MissingReturnError;'
        'func missing()->u64{}func error()->const MissingReturnError{try{missing();}catch(e:const MissingReturnError){return e;};}'
        'func tag(e:MissingReturnError)->u64{return 7u64;}func tag(e:T)->u64 where T{return e.category;}'
        'func held:func(Exception)->u64=func(e:T)->u64 where T{return tag(e);};val e:Exception<-error();behave @print(held(e));', 0, '7\n', '[-Wmissing-return]'),
}


def wide_retained_factory(width, generic, repeated=False):
    view = 'func(' + ','.join(['u64', 'u32'] + ['u64'] * (width - 2)) + ')->u64'
    formals = ['first:U' if generic else 'first:u64', 'second:T' if generic else 'second:u32']
    formals.extend(f'extra{i}:' + ('U' if repeated and i == 2 else 'u64') for i in range(2, width))
    body = ('return tag(first)+tag(second);' if generic else
            'val retained:T<-1u8;return tag(retained)+first+second;')
    return ('func factory(sample:T)->' + view + ' where T{'
            'return func[](' + ','.join(formals) + ')->u64' + (' where U' if generic else '') + '{' + body + '};}')


for width in (16, 32):
    for generic in (False, True):
        source = IO + TAGS + wide_retained_factory(width, generic) + 'func a=factory(1u32);func b=factory(1f64);'
        for holder, first, second in (('a', '1u8', '2u8'), ('a', '1u32', '2u32'),
                                       ('b', '1u8', '2u32'), ('b', '1u32', '2u8')):
            arguments = ','.join([first, second] + ['3u64'] * (width - 2))
            source += f'behave @print({holder}({arguments}));'
        expected = '40\n64\n172\n196\n' if generic else '35\n35\n167\n167\n'
        runner.CASES[f'wide_retained_{"generic" if generic else "fixed"}_{width}'] = (source, 0, expected, '')

runner.CASES['wide_retained_own_binder_still_requires_same_type'] = (
    IO + TAGS + wide_retained_factory(16, True, repeated=True) + 'func held=factory(1f64);',
    2, '', 'no applicable method')
runner.CASES['wide_retained_input_rejects_narrowing'] = (
    IO + TAGS + wide_retained_factory(16, True) + 'func held=factory(1u8);',
    2, '', 'no applicable method')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {
    'deferred_actual_input_snapshot': ['x', 'x', 'x', 'x', 'x', 'x', 'x'],
    'borrowed_result_retains_owner': ['x'],
    'common_exception_actual_identity': ['e', 'e', 'e', 'e'],
}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
