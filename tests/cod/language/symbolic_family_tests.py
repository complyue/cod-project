#!/usr/bin/env python3
"""Symbolic families preserve actual inputs, complete captures and source identity."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
F = 'symbol @f:proc(u64)->u64;proc @f(x:u64)->u64{return 1u64;}proc @f(x:T)->u64 where T{return 7u64;}'
X = 'symbol @x:u64,@y:u64;symbol @f:proc(u64)->u64;proc @f(n:u64)->u64{return @x;}proc @f(n:T)->u64 where T{return @y;}'
runner.CASES = {
    'actual_types_before_preparation': (IO + F + 'effect @f;behave @print(behave @f(1u8));behave @print(behave @f(1u32));behave @print(behave @f(1u64));', 0, '7\n7\n1\n', ''),
    'declaration_supplies_result': (IO + 'symbol @f:proc(u64)->u64;proc @f(x:u64)->u64;proc @f(x:u64){return 7u64;}effect @f;behave @print(behave @f(1u8));', 0, '7\n', ''),
    'implicit_symbol': (IO + 'proc @f(x:u64)->u64{return x+1u64;}effect @f;behave @print(perform @f(2u8));', 0, '3\n', ''),
    'definition_does_not_install': ('proc @f()->u64{return 1u64;}behave @f();', 2, '', 'cannot prove an installed binding'),
    'definition_does_not_capture': ('symbol @x:u64;proc @f()->u64{return @x;}', 0, '', ''),
    'source_family_not_destination': (IO + F + 'symbol @g:proc(u64)->u64;proc @g(x:T)->u64 where T{return 9u64;}effect @g=f;behave @print(behave @g(1u8));effect @g;behave @print(behave @g(1u8));', 0, '7\n9\n', ''),
    'selected_context_1_7_1': (IO + 'symbol @base:proc(u64)->u64,@extended:proc(u64)->u64,@q:proc(u64)->u64;'
        'proc @base(x:T)->u64 where T{return 1u64;}proc @extended(x:T)->u64 where T{return 1u64;}proc @extended(x:u64)->u64{return 7u64;}'
        'effect @q=base;behave @print(behave @q(1u64));effect @q=extended;behave @print(behave @q(1u64));effect @q=base;behave @print(behave @q(1u64));', 0, '1\n7\n1\n', ''),
    'complete_capture_and_replay': (IO + X + 'effect @x<-1u64;effect @y<-7u64;effect @f;effect @x<-3u64;effect @y<-4u64;'
        'behave @print(perform @f(1u8));behave @print(perform @f(1u64));behave @print(behave @f(1u8));behave @print(@x);behave @print(@y);', 0, '7\n1\n4\n3\n4\n', ''),
    'unobserved_alternative_requires_capture': (X + 'effect @x<-1u64;effect @f;behave @f(1u64);', 2, '', 'cannot prove an installed binding'),
    'copy_retains_source_and_capture': (IO + X + 'symbol @q:proc(u64)->u64;effect @x<-1u64;effect @y<-7u64;effect @f;'
        'effect @x<-3u64;effect @y<-4u64;effect @q=@f;behave @print(perform @q(1u8));behave @print(perform @q(1u64));', 0, '7\n1\n', ''),
    'holder_and_forwarding': (IO + F + 'proc invoke(p:proc(u64)->u64)->u64{return p(1u8);}effect @f;proc held=@f;behave @print(held(1u8));behave @print(invoke(held));', 0, '7\n7\n', ''),
    'shared_generic_caller': (IO + F + 'proc invoke(x:T)->u64 where T{return behave @f(x);}effect @f;behave @print(invoke(1u8));behave @print(invoke(1u64));', 0, '7\n1\n', ''),
    'callable_results_follow_selected_method': (IO + 'symbol @f:proc(u64)->func()->u64;func a()->u64{return 1u64;}func b()->u64{return 7u64;}'
        'proc @f(x:u64)->func()->u64{return a;}proc @f(x:T)->func()->u64 where T{return b;}effect @f;'
        'func first=behave @f(1u8);func second=behave @f(1u64);behave @print(first());behave @print(second());', 0, '7\n1\n', ''),
    'lexical_source_symbol_is_not_a_capture': (IO + F + 'symbol @q:proc(u64)->u64;proc build=proc()->proc(u64)->u64{effect @q=f;return @q;};proc held=build();behave @print(held(1u8));', 0, '7\n', ''),
    'family_return': (IO + F + 'proc make_value()->proc(u64)->u64{effect @f;return @f;}proc held=make_value();behave @print(held(1u8));', 0, '7\n', ''),
    'recursive_dynamic_family': (IO + 'symbol @f:proc(u64)->u64;proc @f(x:u64)->u64{return if(x==0u64){1u64}else{x*behave @f(x-1u64)};}'
        'proc seed(x:u64)->u64{return x;}effect @f=seed;effect @f;behave @print(behave @f(5u64));', 0, '120\n', ''),
    'combined_installs_whole_family': (IO + 'symbol @f:proc(u64)->u64;effect @f(x:u64)->u64{return 1u64;};proc @f(x:T)->u64 where T{return 7u64;}behave @print(behave @f(1u8));', 0, '7\n', ''),
    'combined_implicit_symbol': (IO + 'effect @f()->u64{return 7u64;};behave @print(behave @f());', 0, '7\n', ''),
    'multiple_actual_arguments': (IO + 'symbol @f:proc(u64,u64)->u64;proc @f(x:T,y:T)->u64 where T{return 1u64;}proc @f(x:A,y:B)->u64 where {A,B}{return 7u64;}'
        'effect @f;behave @print(behave @f(1u8,2u32));behave @print(behave @f(1u8,2u8));', 0, '7\n1\n', ''),
    'scope_restores_complete_value': (IO + F + 'symbol @g:proc(u64)->u64;proc @g(x:T)->u64 where T{return 9u64;}effect @f;{{effect @f=g;behave @print(behave @f(1u8));}};behave @print(behave @f(1u8));', 0, '9\n7\n', ''),
    'deferred_family_snapshot': (IO + 'symbol @f:proc(u64)->void;proc @f(x:u64)->void{behave @print(1u64);}proc @f(x:T)->void where T{behave @print(7u64);}'
        'proc other(x:u64)->void{behave @print(9u64);}effect @f;defer behave @f(1u8);effect @f=other;', 0, '7\n', ''),
    'deferred_argument_snapshot': (IO + 'symbol @f:proc(u64)->void;proc @f(x:T)->void where T{behave @print(x);}effect @f;val x<-3u8;defer behave @f(x);x<-4u8;', 0, '3\n', ''),
    'callee_before_argument_effect': (IO + 'symbol @f:proc(u64)->u64;proc @f(x:T)->u64 where T{return 7u64;}proc other(x:u64)->u64{return 9u64;}'
        'effect @f;behave @print(behave @f({effect @f=other;1u8}));behave @print(behave @f(1u8));', 0, '7\n9\n', ''),
    'missing_domain_method': ('symbol @f:proc(u64)->u64;proc @f(x:u8)->u64{return 1u64;}effect @f;', 2, '', 'no applicable method'),
    'result_adapter_remains_unproved': ('symbol @f:proc(u64)->u64;proc @f(x:u64)->u32{return 1u32;}effect @f;', 2, '', 'unproved symbolic family result/access/view compatibility'),
    'ambiguous_unused_domain': ('symbol @f:proc(u64,u64)->u64;proc @f(x:u64,y:T)->u64 where T{return 1u64;}proc @f(x:T,y:u64)->u64 where T{return 2u64;}effect @f;', 2, '', 'ambiguous methods'),
    'checked_failure_unused_domain': ('using "std:exceptions" StaticallyUnknownType;symbol @f:proc(u64)->u64;proc @f(x:u64)->u64{return 1u64;}'
        'proc @f(x:T)->u64 throws StaticallyUnknownType where T{return 7u64;}effect @f;', 2, '', 'callable view omits a required checked exception'),
    'unused_generic_body_checked': ('symbol @f:proc(u64)->u64;proc @f(x:u64)->u64{return 1u64;}proc @f(x:T)->u64 where T{return true;}effect @f;behave @f(1u64);', 2, '', 'return type'),
    'method_outside_slot_signature': (IO + F + 'proc @f(x:bool)->u64{return 8u64;}symbol @q:proc(bool)->u64;effect @q=f;behave @print(behave @q(true));', 0, '8\n', ''),
    'nearest_source_binding': ('proc @f()->u64{return 1u64;}symbol @q:proc()->u64;proc go(f:u64)->void{effect @q=f;}go(1u64);', 2, '', 'effect value does not satisfy'),
    'signature_query_has_no_capture': ('symbol @x:u64;proc @f()->u64{return @x;}val result<-make local typeof(perform @f()){1u64};', 0, '', ''),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
