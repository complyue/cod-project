#!/usr/bin/env python3
"""Family value results share transfer rules and preserve completion and identity."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
GAP = 'unproved symbolic family result/access/view compatibility'
ERROR = '''using "std:exceptions" SystemException;
using "std:exceptions" MissingReturnException;
using "std:exceptions" NullPointerException;
func missing()->u64{}
func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}
func null_error()->const NullPointerException{val x<-1;ptr p=x;p=null;try{p;{}}catch(e:const NullPointerException){return e;};}
'''
runner.CASES = {
    'selected_methods_keep_own_results': (IO + 'symbol @f:proc(u64)->f64;proc @f(x:u64)->u32{return 7u32;}proc @f(x:T)->f32 where T{return 1.5f32;}'
        'effect @f;behave @print(behave @f(1u8));behave @print(perform @f(1u64));', 0, '1.5\n7\n', ''),
    'generic_result_uses_instance': (IO + 'symbol @f:proc(u64)->u64;proc @f(x:T) where T{return x;}'
        'effect @f;proc held=@f;behave @print(held(255u8));behave @print(held(4294967295u32));behave @print(held(18446744073709551615u64));', 0, '255\n4294967295\n18446744073709551615\n', ''),
    'shared_generic_caller': (IO + 'symbol @f:proc(u64)->f64;proc @f(x:T)->u32 where T{return 4294967295u32;}'
        'proc relay(x:T)->f64 where T{return behave @f(x);}effect @f;behave @print(relay(1u8));behave @print(relay(1u64));', 0, '4294967295\n4294967295\n', ''),
    'nested_capture_and_return': (IO + 'symbol @f:proc()->f64;proc @f()->u32{return 7u32;}'
        'proc wrap(p:proc()->f64)->proc()->f64{return proc[p]()->f64{return p();};}'
        'effect @f;proc held=wrap(@f);behave @print(held());', 0, '7\n', ''),
    'dynamic_replay_restores': (IO + 'symbol @x:u32,@f:proc()->f64;proc @f()->u32{return @x;}'
        'effect @x<-7u32;effect @f;effect @x<-9u32;behave @print(perform @f());behave @print(behave @f());behave @print(@x);', 0, '7\n9\n9\n', ''),
    'one_family_multiple_result_views': (IO + 'proc @source()->u32{return 4294967295u32;}symbol @integer:proc()->u64,@floating:proc()->f64;'
        'effect @source;effect @integer=source;effect @floating=source;behave @print(behave @source());'
        'behave @print(behave @integer());behave @print(behave @floating());', 0, '4294967295\n4294967295\n4294967295\n', ''),
    'finally_replaces_before_adaptation': (IO + 'symbol @f:proc()->f64;proc @f()->u32{try{return 1u32;}finally{return 7u32;};}'
        'effect @f;behave @print(behave @f());', 0, '7\n', ''),
    'deferred_return_before_adaptation': (IO + 'symbol @f:proc()->f64;proc @f()->u32{defer{behave @print(3u64);};defer{return 7u32;};return 1u32;}'
        'effect @f;behave @print(behave @f());', 0, '3\n7\n', ''),
    'deferred_invocation_discards_value': (IO + 'symbol @f:proc(u64)->f64;proc @f(x:T)->u32 where T{behave @print(x);return 7u32;}'
        'effect @f;defer perform @f(3u8);behave @print(1u64);', 0, '1\n3\n', ''),
    'missing_return_propagates': (IO + ERROR + 'symbol @f:proc()->f64;proc @f()->u32{}effect @f;'
        'try{behave @f();{}}catch(e:const MissingReturnException){behave @print(e.category);};', 0, '5\n', '[-Wmissing-return]'),
    'finally_exception_propagates': (IO + ERROR + 'symbol @f:proc()->f64;proc @f()->u32{try{return 7u32;}finally{missing();{}};}effect @f;'
        'try{perform @f();{}}catch(e:const MissingReturnException){behave @print(e.category);};', 0, '5\n', '[-Wmissing-return]'),
    'fatal_result_keeps_cleanup': (IO + 'symbol @f:proc()->u64;proc @f()->u8{defer{behave @print(7u64);};return 255u8+1u8;}'
        'effect @f;behave @f();', 70, '7\n', 'u8 addition overflow'),
    'exception_cause_and_caller_binding': (IO + ERROR + 'symbol @x:u32,@f:proc()->u64;proc @f()->u32{'
        'behave @print(@x);throw error() from error();}effect @x<-1u32;effect @f;effect @x<-7u32;'
        'try{perform @f();{}}catch(e:const MissingReturnException){behave @print(e.cause_count);behave @print(cause_at(e,0u64).category);'
        'behave @print(e.site>0u64);behave @print(@x);};', 0, '1\n1\n5\ntrue\n7\n', '[-Wmissing-return]'),
    'checked_failure_obligation': ('using "std:exceptions" StaticallyUnknownType;symbol @f:proc()->u64;'
        'proc @f()->u32 throws StaticallyUnknownType{return 7u32;}effect @f;', 2, '', 'callable view omits a required checked exception'),
    'unobserved_incompatible_result': ('symbol @f:proc(u64)->f64;proc @f(x:u64)->u32{return 7u32;}proc @f(x:T)->u64 where T{return 1u64;}'
        'effect @f;behave @f(1u64);', 2, '', GAP),
    'borrowed_target_not_widened': ('proc @narrow(ref x:u32 in O)->ref u32 in O where {owner O}{return x;}'
        'proc @wide(ref x:u32 in O)->ref u64 in O where {owner O};effect @wide=narrow;', 2, '', GAP),
    'result_mode_not_changed': ('proc @expected(ref x:u32 in O)->u64 where {owner O};'
        'proc @actual(ref x:u32 in O)->ref u32 in O where {owner O}{return x;}effect @expected=actual;', 2, '', GAP),
    'common_exception_result_preserves_identity': (IO + ERROR + 'symbol @f:proc(u64)->const SystemException;'
        'proc @f(x:u64)->const MissingReturnException{return error();}proc @f(x:T)->const NullPointerException where T{return null_error();}'
        'proc @classify(e:MissingReturnException)->u64{return 7u64;}proc @classify(e:T)->u64 where T{return e.category;}'
        'symbol @classify:proc(SystemException)->u64;effect @f;effect @classify;'
        'behave @print(behave @classify(behave @f(1u64)));behave @print(behave @classify(behave @f(1u8)));', 0, '7\n4\n', '[-Wmissing-return]'),
}
for source, target, literal, output in [
    ('u8', 'u32', '255u8', '255'), ('u8', 'u64', '255u8', '255'),
    ('u8', 'i64', '255u8', '255'), ('u8', 'f32', '255u8', '255'),
    ('u8', 'f64', '255u8', '255'), ('u32', 'u64', '4294967295u32', '4294967295'),
    ('u32', 'i64', '4294967295u32', '4294967295'), ('u32', 'f64', '4294967295u32', '4294967295'),
    ('f32', 'f64', '1.5f32', '1.5'),
]:
    runner.CASES[f'{source}_to_{target}'] = (IO + f'symbol @f:proc()->{target};proc @f()->{source}{{return {literal};}}'
        'effect @f;behave @print(behave @f());', 0, output + '\n', '')
for source, target, literal in [('u64', 'f64', '1u64'), ('i64', 'f64', '1'), ('u32', 'f32', '1u32'),
                                ('f64', 'f32', '1.0f64'), ('u64', 'u8', '1u64'), ('bool', 'u64', 'true')]:
    runner.CASES[f'reject_{source}_to_{target}'] = (f'symbol @f:proc()->{target};proc @f()->{source}{{return {literal};}}effect @f;', 2, '', GAP)
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
