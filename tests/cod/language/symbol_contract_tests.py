#!/usr/bin/env python3
"""Compile-time symbol contracts preserve identity, lookup and phase boundaries."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
SYMBOL = 'symbol @gain:u64;'
runner.CASES = {
    'unused_declarations': ('symbol @count:u64,@enabled:bool;symbol callback:proc(u64)->const u64;', 0, '', ''),
    'declared_scalar_query': (IO + SYMBOL + 'val x<-make local typeof(@gain){7u64};behave @print(x);', 0, '7\n', ''),
    'query_precedes_declaration': (IO + 'val x<-make local typeof(@gain){8u64};symbol @gain:u64;behave @print(x);', 0, '8\n', ''),
    'all_primitive_contracts': (IO + 'symbol a:u8,b:u32,c:u64,d:i64,e:f32,f:f64,g:bool;'
        'val x<-make local typeof(@a){1u8},y<-make local typeof(@b){2u32},z<-make local typeof(@c){3u64};'
        'val n<-make local typeof(@d){-4},p<-make local typeof(@e){0.5f32},q<-make local typeof(@f){0.25f64},r<-make local typeof(@g){true};'
        'behave @print(x);behave @print(y);behave @print(z);behave @print(n);behave @print(p);behave @print(q);behave @print(r);',
        0, '1\n2\n3\n-4\n0.5\n0.25\ntrue\n', ''),
    'conditional_query': (IO + 'symbol a:u64,b:u64;val x<-make local typeof(if(true){@a}else{@b}){9u64};behave @print(x);', 0, '9\n', ''),
    'procedure_signature_query': (IO + 'symbol operation:proc(u64)->const u64;'
        'val x<-make local typeof(@operation(1u64)){10u64};behave @print(x);', 0, '10\n', ''),
    'checked_signature_query': (IO + 'using "std:exceptions" NullPointerException;'
        'symbol operation:proc(u64)->const u64 throws NullPointerException;'
        'val x<-make local typeof(@operation(1u64)){11u64};behave @print(x);', 0, '11\n', ''),
    'function_type_inspection': (IO + SYMBOL + 'func f(){val x<-make local typeof(@gain){12u64};return x;}behave @print(f());', 0, '12\n', ''),
    'nested_type_inspection': (IO + SYMBOL + 'func outer(){func inner(){val x<-make local typeof(@gain){13u64};return x;};return inner();}'
        'behave @print(outer());', 0, '13\n', ''),
    'print_identity_aliases': ('using symbol "std:io" @print as @first;using symbol "std:io" @print as @second;'
        'behave @first(1u64);perform @second(2u64);', 0, '1\n2\n', ''),
    'caller_shadow_does_not_rebind': (IO + 'proc emit()->void{behave @print(14u64);}proc caller(print:u64)->void{emit();}caller(1u64);', 0, '14\n', ''),
    'module_value_keeps_definition_context': (IO + 'proc emit()->void{behave @print(14u64);}proc caller(print:u64)->void{proc p=emit;p();}caller(1u64);', 0, '14\n', ''),
    'lexical_definition_context': (IO + 'proc p=proc()->void{behave @print(15u64);};{{val print<-0u64;p();}};', 0, '15\n', ''),
    'recursive_definition_context': (IO + 'proc outer()->void{proc p(n:u64)->void{'
        'if(n==0u64){behave @print(16u64);}else{{{val print<-0u64;p(n-1u64);}}};};p(2u64);}outer();', 0, '16\n', ''),
    'queried_anonymous_result': (IO + SYMBOL + 'val x<-make local typeof((func(){return @gain;})()){17u64};behave @print(x);', 0, '17\n', ''),
    'queried_anonymous_shadow': (SYMBOL + 'func outer(gain:u64){val x<-make local typeof((func(){return @gain;})()){0u64};return x;}outer(1u64);',
        2, '', "nearest binding 'gain' is not a symbol"),
    'queried_nested_shadow': (SYMBOL + 'func outer(gain:u64){func inner(){val x<-make local typeof((func(){return @gain;})()){0u64};return x;};return inner();}outer(1u64);',
        2, '', "nearest binding 'gain' is not a symbol"),
    'explicit_query_avoids_body_lookup': (IO + SYMBOL + 'func outer(gain:u64){func inner(){'
        'val x<-make local typeof((func()->u64{return @gain;})()){18u64};return x;};return inner();}behave @print(outer(1u64));', 0, '18\n', ''),
    'closure_shadowed_symbol': (IO + 'proc outer(print:u64)->void{proc inner=proc()->void{behave @print(1u64);};inner();}outer(1u64);',
        2, '', "nearest binding 'print' is not a symbol"),
    'nested_closure_shadowed_symbol': (IO + 'proc outer(print:u64)->void{proc middle=proc()->void{proc inner=proc()->void{behave @print(1u64);};inner();};middle();}outer(1u64);',
        2, '', "nearest binding 'print' is not a symbol"),
    'closure_shadowed_query': (SYMBOL + 'func outer(gain:u64)->u64{func inner(){val x<-make local typeof(@gain){0u64};return x;};return inner();}outer(1u64);',
        2, '', "nearest binding 'gain' is not a symbol"),
    'lexical_self_shadows_symbol': ('using symbol "std:io" @print as @p;proc outer()->void{proc p()->void{behave @p(1u64);};p();}outer();',
        2, '', "nearest binding 'p' is not a symbol"),
    'owner_shadows_symbol': (SYMBOL + 'func f(ref x:u64 in gain)->u64 where {owner gain}{val y<-make local typeof(@gain){0u64};return y;}val x<-1u64;f(x);',
        2, '', "nearest binding 'gain' is not a symbol"),
    'unknown_symbol': ('val x<-make local typeof(@missing){0u64};', 2, '', "unknown symbol 'missing'"),
    'duplicate_symbol': ('symbol gain:u64;symbol @gain:u64;', 2, '', 'duplicate symbol binding'),
    'bare_symbol_is_not_payload': (SYMBOL + 'val x<-gain;', 2, '', 'materialization of a compile-time symbol object'),
    'unbound_read_requires_installation': (SYMBOL + '@gain;', 2, '', 'cannot prove an installed binding'),
    'installation_boundary': ('symbol p:proc(u64)->const u64;effect @p;', 2, '', 'installation'),
    'inline_installation_boundary': ('symbol p:proc(u64)->const u64;{{effect @p}};', 2, '', 'installation'),
    'source_dynamic_call_boundary': ('symbol p:proc(u64)->const u64;behave @p(1u64);', 2, '', 'dynamic procedure call other than'),
    'polymorphic_intrinsic_query_boundary': (IO + 'val x<-make local typeof(@print);', 2, '', 'typeof requires a known declaration'),
    'same_scope_callable_boundary': ('symbol p:proc(u64)->const u64;proc p(x:u64)->const u64{return x;}', 2, '', 'same-scope callable and symbol'),
    'bodyless_callable_boundary': ('symbol p:proc(u64)->const u64;proc p(x:u64)->const u64;', 2, '', 'same-scope callable and symbol'),
    'type_namespace_boundary': ('using "std:exceptions" MissingReturnException;symbol MissingReturnException:u64;', 2, '', 'same-scope type and symbol'),
    'value_namespace_boundary': (SYMBOL + 'val gain<-0u64;', 2, '', 'same-scope value and symbol'),
    'nested_shadow_ends': (IO + SYMBOL + '{{val gain<-0u64;}};val x<-make local typeof(@gain){19u64};behave @print(x);', 0, '19\n', ''),
    'deferred_outer_shadow': (IO + 'proc outer(print:u64)->void{proc inner=proc()->void{defer behave @print(1u64);};inner();}outer(1u64);',
        2, '', "nearest binding 'print' is not a symbol"),
}
for name, expression in {
    'read': '@gain;', 'query': 'val x<-make local typeof(@gain){0u64};',
    'install': 'effect @gain<-1u64;', 'own_family_install': 'effect @gain;',
    'touch': 'touch @gain;', 'behave': 'behave @gain(1u64);', 'perform': 'perform @gain(1u64);',
}.items():
    runner.CASES['nearest_'+name] = (SYMBOL + 'proc f(gain:u64)->void{'+expression+'}f(1u64);',
        2, '', "nearest binding 'gain' is not a symbol")
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
