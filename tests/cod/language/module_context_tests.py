#!/usr/bin/env python3
"""Source-preserving module contexts, explicit publication and incorporation."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
BASE = 'func score(x:T)->u64 where T{return 1u64;}func relay(x:u64){return score(x);}export score;export relay;'
FAMILY = 'symbol @score:proc(u64)->u64;proc @score(x:T)->u64 where T{return 1u64;}proc @score(x:u8)->u64{return 7u64;}export @score;'

runner.CASES = {
    'private_helper_keeps_source_context': ({
        'base.cod': BASE,
        'main.cod': IO + 'using "base.cod" relay;func score(x:u64)->u64{return 9u64;}behave @print(relay(1u64));behave @print(score(1u64));',
    }, 0, '1\n9\n', ''),
    'source_callable_value_and_nested_closure': ({
        'base.cod': BASE + 'func factory(){return func(x:u64){return score(x);};}export factory;',
        'main.cod': IO + 'using "base.cod" relay;using "base.cod" factory;func score(x:u64)->u64{return 9u64;}'
            'func first=relay;func second=factory();behave @print(first(1u64));behave @print(second(1u64));',
    }, 0, '1\n1\n', ''),
    'reexport_and_repeated_paths': ({
        'base.cod': BASE,
        'adapter.cod': 'using "base.cod" relay as twice;export twice;',
        'main.cod': IO + 'using "adapter.cod" twice;using "./adapter.cod" twice;using "base.cod" relay as direct;'
            'behave @print(twice(2u64));behave @print(direct(2u64));',
    }, 0, '1\n1\n', ''),
    'same_spelling_distinct_families': ({
        'base.cod': BASE,
        'other.cod': 'func score(x:T)->u64 where T{return 7u64;}export score;',
        'main.cod': IO + 'using "base.cod" score as first;using "other.cod" score as second;using "base.cod" relay;'
            'behave @print(first(1u64));behave @print(second(1u64));behave @print(relay(1u64));',
    }, 0, '1\n7\n1\n', ''),
    'selected_symbol_family_requires_effect': ({
        'base.cod': FAMILY,
        'main.cod': IO + 'using symbol "base.cod" @score;effect @score;behave @print(behave @score(1u8));behave @print(behave @score(1u64));',
    }, 0, '7\n1\n', ''),
    'symbol_from_preserves_identity_and_semantics': ({
        'base.cod': FAMILY,
        'main.cod': IO + 'symbol from "base.cod" @score as @first;symbol from "./base.cod" @score as @second;'
            'effect @first;behave @print(behave @second(1u8));{{proc f(x:u64)->u64{return 9u64;};effect @second=f;'
            'behave @print(behave @first(1u8));}};behave @print(behave @second(1u8));',
    }, 0, '7\n9\n7\n', ''),
    'family_source_context_and_distinct_destination': ({
        'base.cod': 'func helper(x:u64)->u64{return 7u64;}proc @score(x:u64)->u64{return helper(x);}export @score;',
        'main.cod': IO + 'using symbol "base.cod" @score as @source;proc @score(x:u64)->u64{return 9u64;}'
            'func helper(x:u64)->u64{return 11u64;}effect @score=source;behave @print(behave @score(1u8));',
    }, 0, '7\n', ''),
    'cross_module_dynamic_capture': ({
        'base.cod': 'symbol @bias:u64;proc @score(x:u64)->u64{return @bias+x;}export @bias;export @score;',
        'main.cod': IO + 'using symbol "base.cod" @score;symbol from "base.cod" @bias;'
            'effect @bias<-3u64;effect @score;effect @bias<-8u64;behave @print(perform @score(1u64));behave @print(behave @score(1u64));',
    }, 0, '4\n9\n', ''),
    'different_module_symbol_spellings_do_not_merge': ({
        'base.cod': 'symbol @bias:u64;proc read()->u64{return @bias;}export read;export @bias;',
        'main.cod': IO + 'symbol @bias:u64;using "base.cod" read;symbol from "base.cod" @bias as @source_bias;effect @bias<-9u64;effect @source_bias<-3u64;behave @print(read());behave @print(@bias);',
    }, 0, '3\n9\n', ''),
    'dependency_header_order_once_only_and_explicit_cleanup': ({
        'shared.cod': IO + 'symbol @bias:u64;{{effect @bias<-7u64;behave @print(2u64);defer {behave @print(@bias);};}};export @bias;',
        'left.cod': IO + 'symbol from "shared.cod" @bias;behave @print(1u64);behave @print(3u64);export @bias;',
        'right.cod': IO + 'symbol from "./shared.cod" @bias;behave @print(5u64);behave @print(6u64);export @bias;',
        'main.cod': IO + 'symbol from "left.cod" @bias;'
            'symbol from "right.cod" @bias as @other;behave @print(0u64);behave @print(4u64);effect @other<-8u64;behave @print(@other);',
    }, 0, '2\n7\n1\n3\n5\n6\n0\n4\n8\n', ''),
    'dependency_full_scope_has_distinct_storage': ({
        'base.cod': IO + '{{val x<-1u64;defer {behave @print(x);};}};func value()->u64{return 7u64;}export value;',
        'main.cod': IO + 'using "base.cod" value;val x<-2u64;behave @print(x);defer {behave @print(x);};',
    }, 0, '1\n2\n2\n', ''),
    'dependency_failure_stops_later_initializers': ({
        'base.cod': IO + 'func f()->u64{return 1u64;}export f;{{defer {behave @print(2u64);};1u64/0u64;}};',
        'main.cod': IO + 'using "base.cod" f;defer {behave @print(1u64);};behave @print(99u64);',
    }, 70, '2\n', 'division by zero'),
    'source_exception_alias_and_body_inference': ({
        'base.cod': 'using "std:exceptions" MissingReturnException as Problem;'
            'func missing()->u64{}func make_error(){try{missing();}catch(e:const Problem){return e;};}func accept(e:Problem)->u64{return 4u64;}export make_error;export accept;export Problem;',
        'main.cod': IO + 'using "std:exceptions" OutOfMemory as Problem;using "base.cod" make_error;using "base.cod" accept;'
            'using "base.cod" Problem as SourceProblem;behave @print(accept(make_error()));',
    }, 2, '', 'inferred result mode for a non-scalar type'),
    'source_exception_signature': ({
        'base.cod': 'using "std:exceptions" MissingReturnException as Problem;'
            'func missing()->u64{}func make_error()->const Problem{try{missing();}catch(e:const Problem){return e;};}func accept(e:Problem)->u64{return 4u64;}export make_error;export accept;export Problem;',
        'main.cod': IO + 'using "std:exceptions" OutOfMemory as Problem;using "base.cod" make_error;using "base.cod" accept;'
            'using "base.cod" Problem as SourceProblem;behave @print(accept(make_error()));',
    }, 0, '4\n', '[-Wmissing-return]'),
    'no_implicit_tls_installation': ({'base.cod': FAMILY, 'main.cod': 'using symbol "base.cod" @score;behave @score(1u64);'}, 2, '', 'cannot prove an installed binding'),
    'future_installation_is_not_available': ({
        'vocab.cod': 'symbol @bias:u64;export @bias;',
        'base.cod': 'symbol from "vocab.cod" @bias;@bias;export @bias;',
        'main.cod': 'symbol from "base.cod" @bias;effect @bias<-7u64;',
    }, 2, '', 'cannot prove an installed binding'),
    'private_selection_rejected': ({'base.cod': BASE, 'main.cod': 'using "base.cod" hidden;'}, 2, '', 'does not export'),
    'selection_is_not_reexport': ({'base.cod': BASE, 'adapter.cod': 'using "base.cod" score;', 'main.cod': 'using "adapter.cod" score;'}, 2, '', 'does not export'),
    'private_symbol_rejected': ({'base.cod': 'symbol @p:u64;', 'main.cod': 'symbol from "base.cod" @p;'}, 2, '', 'does not export'),
    'using_extension_rejected': ({'base.cod': BASE, 'main.cod': 'using "base.cod" score;func score(x:u64)->u64{return 7u64;}'}, 2, '', 'does not grant callable extension'),
    'symbol_selection_extension_rejected': ({'base.cod': FAMILY, 'main.cod': 'symbol from "base.cod" @score;proc @score(x:u32)->u64{return 9u64;}'}, 2, '', 'do not grant callable extension'),
    'import_remains_unproved': ({'base.cod': BASE, 'main.cod': 'import "base.cod" score;'}, 2, '', 'receiver-adapted import'),
    'mixin_remains_unproved': ({'base.cod': BASE, 'main.cod': 'mixin "base.cod" score;'}, 2, '', 'receiver-adapted import'),
    'symbol_destination_boundary': ({'base.cod': FAMILY, 'main.cod': 'symbol @score:proc(u64)->u64;using "base.cod" @score;'}, 2, '', 'symbolic destination selection'),
    'conflicting_aliases': ({'base.cod': 'symbol @p:u64;export @p;', 'other.cod': 'symbol @p:u64;export @p;',
        'main.cod': 'symbol from "base.cod" @p;symbol from "other.cod" @p;'}, 2, '', 'conflicting symbol alias'),
    'cyclic_construction_boundary': ({'a.cod': 'using "main.cod" f;', 'main.cod': 'using "a.cod" f;func f()->void{}export f;'}, 2, '', 'cyclic module construction remains unproved'),
    'missing_source': ({'main.cod': 'using "missing.cod" f;'}, 2, '', 'cannot read CoD source'),
    'source_recursive_result_inference': ({
        'base.cod': 'func even(n:u64){if(n==0u64){return true;}else{return odd(n-1u64);};}func odd(n:u64){if(n==0u64){return false;}else{return even(n-1u64);};}export even;',
        'main.cod': IO + 'using "base.cod" even;func odd(n:u64)->u64{return n;}behave @print(even(20u64));behave @print(even(21u64));',
    }, 0, 'true\nfalse\n', ''),
    'source_declaration_result_contract': ({
        'base.cod': 'func pick(x:T)->u64 where T;func pick(x:T) where T{return 7u64;}export pick;',
        'main.cod': IO + 'using "base.cod" pick as other;func pick(x:T)->bool where T{return false;}behave @print(other(1u8));',
    }, 0, '7\n', ''),
    'source_symbol_type_query': ({
        'base.cod': 'symbol @bias:u8;func value(){val x<-make local typeof(@bias){7u8};return x;}export value;',
        'main.cod': IO + 'symbol @bias:bool;using "base.cod" value;behave @print(value());',
    }, 0, '7\n', ''),
    'source_callable_ignores_caller_lexical_shadow': ({
        'base.cod': BASE,
        'main.cod': IO + 'using "base.cod" relay;proc use(score:u64)->u64{func f=relay;return f(1u64);}behave @print(use(99u64));',
    }, 0, '1\n', ''),
    'nested_relative_source': ({
        'sub/base.cod': BASE,
        'sub/adapter.cod': 'using "base.cod" relay;export relay;',
        'main.cod': IO + 'using "sub/adapter.cod" relay;behave @print(relay(1u64));',
    }, 0, '1\n', ''),
    'source_module_values_do_not_leak': ({
        'base.cod': 'val x<-7u64;func get()->u64{return x;}export get;',
        'main.cod': IO + 'using "base.cod" get;val x<-9u64;behave @print(get());',
    }, 2, '', 'retained dependency initialization scope remains unproved'),
    'selected_type_and_symbol_collision': ({
        'base.cod': 'using "std:exceptions" MissingReturnException as Name;export Name;',
        'vocab.cod': 'symbol @Name:u64;export @Name;',
        'main.cod': 'using "base.cod" Name;symbol from "vocab.cod" @Name;',
    }, 2, '', 'same-scope type and callable/symbol'),
    'receiver_declaration_cannot_extend_using': ({
        'base.cod': BASE,
        'main.cod': 'using "base.cod" score;func score(x:u64)->u64;',
    }, 2, '', 'does not grant callable extension'),
    'dependency_scope_does_not_expose_binding': ({
        'base.cod': IO + 'symbol @bias:u64;{{effect @bias<-3u64;behave @print(@bias);}};export @bias;',
        'main.cod': IO + 'symbol from "base.cod" @bias;behave @print(@bias);',
    }, 2, '', 'cannot prove an installed binding'),
    'dependency_iteration_does_not_expose_binding': ({
        'base.cod': IO + 'symbol @bias:u64;for(i in 0u64..2u64){effect @bias<-i;behave @print(@bias);};export @bias;',
        'main.cod': IO + 'symbol from "base.cod" @bias;behave @print(@bias);',
    }, 2, '', 'cannot prove an installed binding'),
    'retained_dependency_effect_is_unproved': ({
        'base.cod': 'symbol @bias:u64;effect @bias<-7u64;export @bias;',
        'main.cod': 'symbol from "base.cod" @bias;',
    }, 2, '', 'retained dependency initialization scope remains unproved'),
    'retained_dependency_defer_is_unproved': ({
        'base.cod': IO + 'func value()->u64{return 7u64;}export value;defer {behave @print(1u64);};',
        'main.cod': 'using "base.cod" value;',
    }, 2, '', 'retained dependency initialization scope remains unproved'),
    'loop_defer_still_belongs_to_dependency': ({
        'base.cod': IO + 'func value()->u64{return 7u64;}export value;for(i in 0u64..2u64){defer {behave @print(1u64);};};',
        'main.cod': 'using "base.cod" value;',
    }, 2, '', 'retained dependency initialization scope remains unproved'),
    'forward_source_reference_requires_incorporation_proof': ({
        'base.cod': IO + 'behave @print(1u64);func f()->u64{return 7u64;}export f;',
        'main.cod': IO + 'behave @print(f());using "base.cod" f;',
    }, 2, '', 'first-reference incorporation analysis'),
    'interleaved_selection_requires_incorporation_proof': ({
        'base.cod': BASE,
        'main.cod': IO + 'behave @print(1u64);using "base.cod" relay;',
    }, 2, '', 'first-reference incorporation analysis'),
    'export_import_is_not_silently_ignored': ({
        'base.cod': 'export import "std:exceptions" MissingReturnException;',
        'main.cod': 'using "base.cod" MissingReturnException;',
    }, 2, '', 'export import publication remains unproved'),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
