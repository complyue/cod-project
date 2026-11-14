#!/usr/bin/env python3
"""Ordinary module callable values retain family dispatch and complete coverage."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
F = 'func select(x:u64)->u64{return 1u64;}func select(x:T)->u64 where T{return 7u64;}'
VIEW = 'func held:func(u64)->u64=select;'
ERROR = '''using "std:exceptions" SystemException;
using "std:exceptions" MissingReturnException;
func missing()->u64{}
func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}
'''
EXHAUSTIVE = ERROR + ''.join(
    f'using "std:exceptions" {name};func select(e:{name})->u64{{return e.category;}}'
    for name in ('OutOfMemory', 'RegionCapacityExceeded', 'InvalidCapacity', 'NullPointerException',
                 'UnsoundRuntimeTyping', 'StaticallyUnknownType'))
EXHAUSTIVE += 'func select(e:MissingReturnException)->u64{return 7u64;}'
runner.CASES = {
    'actual_type_selects_method': (IO + F + VIEW + 'behave @print(held(1u8));behave @print(held(1u32));behave @print(held(1u64));', 0, '7\n7\n1\n', ''),
    'common_result_adaptation': (IO + 'func select(x:u64)->u32{return 7u32;}func select(x:T)->f32 where T{return 1.5f32;}'
        'func held:func(u64)->f64=select;behave @print(held(1u8));behave @print(held(1u64));', 0, '1.5\n7\n', ''),
    'later_argument_supplies_view': (IO + F + 'func apply(f:func(T)->u64,x:T)->u64 where T{return f(x);}'
        'behave @print(apply(select,1u8));behave @print(apply(select,1u64));', 0, '7\n1\n', ''),
    'returned_family': (IO + F + 'func factory()->func(u64)->u64{return select;}func held=factory();behave @print(held(1u8));behave @print(held(1u64));', 0, '7\n1\n', ''),
    'nested_capture_retains_family': (IO + F + 'func wrap(f:func(u64)->u64)->func(u64)->u64{return func[f](x:u64)->u64{return f(x);};}'
        + VIEW + 'func copy=held;func wrapped=wrap(copy);behave @print(copy(1u8));behave @print(wrapped(1u8));', 0, '7\n1\n', ''),
    'joint_actual_types': (IO + 'func select(x:T,y:T)->u64 where T{return 1u64;}func select(x:A,y:B)->u64 where {A,B}{return 7u64;}'
        'func held:func(u64,u64)->u64=select;behave @print(held(1u8,2u32));behave @print(held(1u8,2u8));', 0, '7\n1\n', ''),
    'common_exception_identity': (IO + ERROR + 'func select(e:MissingReturnException)->u64{return 7u64;}func select(e:T)->u64 where T{return e.category;}'
        'func held:func(SystemException)->u64=select;val e:SystemException<-error();behave @print(held(e));', 0, '7\n', '[-Wmissing-return]'),
    'exhaustive_concrete_methods_supply_view': (IO + EXHAUSTIVE + 'func held:func(SystemException)->u64=select;behave @print(held(error()));', 0, '7\n', '[-Wmissing-return]'),
    'type_query_uses_complete_family': (IO + EXHAUSTIVE + 'func apply(f:func(SystemException)->u64,e:SystemException)->u64{return f(e);}'
        'val x<-make local typeof(apply(select,error())){9u64};behave @print(x);behave @print(apply(select,error()));', 0, '9\n7\n', '[-Wmissing-return]'),
    'type_query_rejects_incomplete_view': (ERROR + 'func select(e:MissingReturnException)->u64{return 7u64;}'
        'func apply(f:func(SystemException)->u64,e:SystemException)->u64{return f(e);}'
        'val x<-make local typeof(apply(select,error())){9u64};', 2, '', 'no applicable method'),
    'borrowed_alias_and_write': (IO + 'func identity(ref x:i64 in O)->ref i64 in O where {owner O}{return x;}func held=identity;'
        'val x<-1;held(x)<-7;behave @print(x);', 0, '7\n', ''),
    'generic_caller_preserves_evidence': (IO + F + VIEW + 'func relay(f:func(u64)->u64,x:T)->u64 where T{return f(x);}'
        'behave @print(relay(held,1u8));behave @print(relay(held,1u64));', 0, '7\n1\n', ''),
    'deferred_snapshot': (IO + 'proc select(x:u64)->void{behave @print(1u64);}proc select(x:T)->void where T{behave @print(x);}'
        'proc held:proc(u64)->void=select;val x<-7u8;defer held(x);x<-9u8;', 0, '7\n', ''),
    'complete_dynamic_capture': (IO + 'symbol @x:u64,@y:u64,@q:proc(u64)->u64;proc select(n:u64)->u64{return @x;}'
        'proc select(n:T)->u64 where T{return @y;}effect @x<-1u64;effect @y<-7u64;proc held:proc(u64)->u64=select;'
        'effect @x<-3u64;effect @y<-9u64;effect @q=held;behave @print(perform @q(1u8));behave @print(behave @q(1u8));behave @print(perform @q(1u64));', 0, '7\n9\n1\n', ''),
    'unused_alternative_requires_capture': ('symbol @x:u64,@y:u64;proc select(n:u64)->u64{return @x;}proc select(n:T)->u64 where T{return @y;}'
        'effect @x<-1u64;proc held:proc(u64)->u64=select;', 2, '', 'cannot prove an installed binding'),
    'unused_ambiguous_inputs_reject': ('func select(x:u64,y:T)->u64 where T{return 1u64;}func select(x:T,y:u64)->u64 where T{return 7u64;}'
        'func select(x:u64,y:u64)->u64{return 9u64;}func select(x:u8,y:T)->u64 where T{return 3u64;}'
        'func select(x:T,y:u8)->u64 where T{return 4u64;}func held:func(u64,u64)->u64=select;', 2, '', 'ambiguous methods'),
    'unused_result_rejects': ('func select(x:u64)->u64{return 1u64;}func select(x:T)->bool where T{return true;}'
        + VIEW, 2, '', 'callable view contract'),
    'unused_checked_failure_rejects': ('using "std:exceptions" StaticallyUnknownType;func select(x:u64)->u64{return 1u64;}'
        'func select(x:T)->u64 throws StaticallyUnknownType where T{return 7u64;}' + VIEW, 2, '', 'required checked exception'),
    'unused_body_is_checked': ('func select(x:u64)->u64{return 1u64;}func select(x:T)->u64 where T{return true;}' + VIEW, 2, '', 'return type'),
    'source_and_imported_interpretations': ({
        'base.cod': 'func choose(x:u64)->u64{return helper(x);}func choose(x:T)->u64 where T{return helper(x);}func helper(x:T)->u64 where T{return 1u64;}export choose;export helper;',
        'extended.cod': 'import "base.cod" choose;import "base.cod" helper;func helper(x:u8)->u64{return 7u64;}export choose;',
        'main.cod': IO + 'using "base.cod" choose as original;using "extended.cod" choose as extended;func a:func(u64)->u64=original;'
            'func b:func(u64)->u64=extended;behave @print(a(1u8));behave @print(b(1u8));behave @print(b(1u64));behave @print(a(1u8));',
    }, 0, '1\n7\n1\n1\n', ''),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
