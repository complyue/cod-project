#!/usr/bin/env python3
"""Receiver-owned callable identities and source-preserving import interpretation."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
BASE = 'func score(x:T)->u64 where T{return 1u64;}func twice(x:u64){return score(x)+score(x);}export score;export twice;'
EXT = 'import "base.cod" score;func score(x:u64)->u64{return 7u64;}export score;using "base.cod" twice as frozen;export frozen;export import "base.cod" twice as adapted;'

runner.CASES = {
    'base_extension_and_indirect_keep_identity': ({
        'base.cod': BASE, 'ext.cod': EXT,
        'main.cod': IO + 'using "base.cod" score as base;using "ext.cod" score as extended;func indirect(x:u64)->u64{return base(x);}'
            'behave @print(base(1u64));behave @print(extended(1u64));behave @print(indirect(1u64));',
    }, 0, '1\n7\n1\n', ''),
    'using_two_import_fourteen': ({
        'base.cod': BASE, 'ext.cod': EXT,
        'main.cod': IO + 'using "ext.cod" frozen;using "ext.cod" adapted;behave @print(frozen(1u64));behave @print(adapted(1u64));',
    }, 0, '2\n14\n', ''),
    'ordinary_values_preserve_interpretation': ({
        'base.cod': BASE, 'ext.cod': EXT,
        'main.cod': IO + 'using "ext.cod" frozen;using "ext.cod" adapted;func a=frozen;func b=adapted;behave @print(a(1u64));behave @print(b(1u64));',
    }, 0, '2\n14\n', ''),
    'deep_private_helpers_adapt_by_identity': ({
        'base.cod': 'func score(x:T)->u64 where T{return 1u64;}func private_step(x:T) where T{return score(x);}func twice(x:u64){return private_step(x)+private_step(x);}export score;export twice;',
        'ext.cod': EXT,
        'main.cod': IO + 'using "ext.cod" frozen;using "ext.cod" adapted;func private_step(x:u64)->bool{return false;}'
            'behave @print(frozen(1u64));behave @print(adapted(1u64));',
    }, 0, '2\n14\n', ''),
    'reexport_preserves_selected_context': ({
        'base.cod': BASE, 'ext.cod': EXT,
        'relay.cod': 'using "ext.cod" adapted as selected;export selected;',
        'main.cod': IO + 'using "relay.cod" selected;func score(x:u64)->u64{return 99u64;}behave @print(selected(1u64));',
    }, 0, '14\n', ''),
    'same_spelling_is_not_dependency_identity': ({
        'base.cod': BASE,
        'main.cod': IO + 'import "base.cod" twice;func score(x:u64)->u64{return 99u64;}behave @print(twice(1u64));',
    }, 0, '2\n', ''),
    'nested_closure_retains_import_context': ({
        'base.cod': BASE + 'func factory(){return func(x:u64){return score(x);};}export factory;',
        'ext.cod': EXT + 'export import "base.cod" factory;using "base.cod" factory as frozen_factory;export frozen_factory;',
        'main.cod': IO + 'using "ext.cod" factory;using "ext.cod" frozen_factory;func a=factory();func b=frozen_factory();behave @print(a(1u64));behave @print(b(1u64));',
    }, 0, '7\n1\n', ''),
    'nested_recursive_retained_type_context': ({
        'base.cod': BASE + 'func factory(x:T) where T{func go(n:T){if(n==0u64){return score(n);}else{return go(n-1u64);};};return go;}export factory;',
        'ext.cod': EXT + 'export import "base.cod" factory;',
        'main.cod': IO + 'using "ext.cod" factory;func a=factory(1u64);behave @print(a(10u64));',
    }, 0, '7\n', ''),
    'source_declaration_contract_survives_alias': ({
        'vocab.cod': 'func operation(x:T)->const T where T;export operation;',
        'ext.cod': 'import "vocab.cod" operation as local;func local(x:u64)->u64{return 7u64;}export local;',
        'main.cod': IO + 'using "ext.cod" local;behave @print(local(1u64));',
    }, 0, '7\n', ''),
    'bodyless_publication_does_not_invent_body': ({
        'vocab.cod': 'func operation(x:u64)->u64;export operation;',
        'main.cod': 'using "vocab.cod" operation;operation(1u64);',
    }, 2, '', 'no applicable implementation'),
    'bodyless_value_does_not_invent_body': ({
        'vocab.cod': 'func operation(x:u64)->u64;export operation;',
        'main.cod': 'using "vocab.cod" operation;func f:func(u64)->u64=operation;',
    }, 2, '', 'no applicable implementation'),
    'repeated_import_aliases_share_extensions': ({
        'base.cod': BASE,
        'main.cod': IO + 'import "base.cod" score as first;import "base.cod" score as second;func first(x:u64)->u64{return 7u64;}'
            'behave @print(first(1u64));behave @print(second(1u64));',
    }, 0, '7\n7\n', ''),
    'declarations_before_selection_join_identity': ({
        'base.cod': BASE,
        'main.cod': IO + 'func first(x:u64)->u64{return 7u64;}import "base.cod" score as first;import "base.cod" score as second;'
            'behave @print(second(1u64));',
    }, 0, '7\n', ''),
    'multiple_extension_aliases_warn': ({
        'base.cod': BASE,
        'main.cod': IO + 'import "base.cod" score as first;import "base.cod" score as second;func first(x:u64)->u64{return 7u64;}'
            'func second(x:bool)->u64{return 9u64;}behave @print(first(true));behave @print(second(1u64));',
    }, 0, '9\n7\n', '[-Wmulti-alias-extension]'),
    'conflicting_sibling_definitions_reject': ({
        'base.cod': BASE, 'a.cod': EXT,
        'b.cod': 'import "base.cod" score;func score(x:u64)->u64{return 11u64;}export score;',
        'main.cod': 'import "a.cod" adapted;using "b.cod" score as other;adapted(1u64);',
    }, 2, '', 'ambiguous methods'),
    'using_helper_ignores_conflicting_sibling': ({
        'base.cod': BASE, 'a.cod': EXT,
        'b.cod': 'import "base.cod" score;func score(x:u64)->u64{return 11u64;}export score;',
        'main.cod': IO + 'using "a.cod" adapted;using "b.cod" score as other;behave @print(adapted(1u64));behave @print(other(1u64));',
    }, 0, '14\n11\n', ''),
    'private_selection_stays_private': ({'base.cod': 'func private()->u64{return 7u64;}', 'main.cod': 'import "base.cod" private;'}, 2, '', 'does not export'),
    'ordinary_import_is_not_reexport': ({'base.cod': BASE, 'ext.cod': 'import "base.cod" score;', 'main.cod': 'using "ext.cod" score;'}, 2, '', 'does not export'),
    'nested_callable_capture_retains_import_context': ({
        'base.cod': BASE + 'func factory(){func value()->u64{return score(1u64);};func go(n:u64)->u64{if(n==0u64){return value();}else{return go(n-1u64);};};return go;}export factory;',
        'ext.cod': EXT + 'export import "base.cod" factory;',
        'main.cod': IO + 'using "ext.cod" factory;func f=factory();behave @print(f(10u64));',
    }, 0, '7\n', ''),
    'later_import_does_not_erase_inherited_conflict': ({
        'base.cod': BASE, 'a.cod': EXT,
        'b.cod': 'import "base.cod" score;func score(x:u64)->u64{return 11u64;}export score;',
        'c.cod': 'export import "a.cod" adapted;using "b.cod" score as other;',
        'main.cod': 'import "c.cod" adapted;import "a.cod" score;adapted(1u64);',
    }, 2, '', 'ambiguous methods'),
    'later_using_does_not_erase_inherited_conflict': ({
        'base.cod': BASE, 'a.cod': EXT,
        'b.cod': 'import "base.cod" score;func score(x:u64)->u64{return 11u64;}export score;',
        'c.cod': 'export import "a.cod" adapted;using "b.cod" score as other;',
        'main.cod': 'import "c.cod" adapted;using "a.cod" score;adapted(1u64);',
    }, 2, '', 'ambiguous methods'),
    'shadowed_dependency_does_not_demand_an_interpretation': ({
        'base.cod': BASE + 'func dangerous(x:u64){return score(x);}func call(dangerous:func(u64)->u64,x:u64){return dangerous(x);}export call;',
        'a.cod': EXT + 'export import "base.cod" call;',
        'b.cod': 'import "base.cod" score;func score(x:u64)->u64{return 11u64;}export score;',
        'main.cod': IO + 'import "a.cod" call;using "b.cod" score as other;func chosen(x:u64)->u64{return 3u64;}behave @print(call(chosen,1u64));',
    }, 0, '3\n', ''),
    'imported_declaration_preserves_checked_obligation': ({
        'base.cod': 'using "std:exceptions" NullPointerException as SourceError;func operation()->bool throws SourceError;export operation;',
        'main.cod': 'import "base.cod" operation as local;func local(){return true;}local();',
    }, 2, '', 'requires a covering catch or throws'),
    'receiver_alias_cannot_replace_source_exception': ({
        'base.cod': 'using "std:exceptions" NullPointerException as SourceError;func operation()->bool throws SourceError;export operation;',
        'main.cod': 'using "std:exceptions" MissingReturnException as SourceError;import "base.cod" operation as local;'
            'func local(){return true;}try{local();{}}catch(e:const SourceError){};',
    }, 2, '', 'requires a covering catch or throws'),
    'source_exception_obligation_survives_callable_value': ({
        'base.cod': 'using "std:exceptions" NullPointerException as SourceError;func operation()->bool throws SourceError;export operation;',
        'main.cod': 'import "base.cod" operation as local;func local(){return true;}func f:func()->bool=local;',
    }, 2, '', 'callable view omits a required checked exception'),
    'imported_declaration_preserves_generic_result': ({
        'base.cod': 'func operation(x:T)->const T where T;export operation;',
        'main.cod': 'import "base.cod" operation as local;func local(x:u64)->bool{return true;}local(1u64);',
    }, 2, '', 'does not satisfy callable declaration'),
    'imported_declaration_supports_type_query_without_body': ({
        'base.cod': 'func operation(x:T)->const T where T;export operation;',
        'main.cod': IO + 'import "base.cod" operation as local;val x<-make local typeof(local(true)){true};behave @print(x);',
    }, 0, 'true\n', ''),
    'imported_borrow_keeps_external_owner': ({
        'base.cod': 'func identity(ref x:i64 in O)->ref i64 in O where {owner O}{return x;}export identity;',
        'main.cod': IO + 'import "base.cod" identity as local;val x<-1;local(x)<-7;behave @print(x);',
    }, 0, '7\n', ''),
    'source_symbols_and_complete_capture_survive_import': ({
        'base.cod': 'symbol @x:u64;proc read()->u64{return @x;}export @x;export read;',
        'main.cod': IO + 'symbol from "base.cod" @x as @source;import "base.cod" read;symbol @x:u64,@p:proc()->u64;'
            'effect @source<-1u64;effect @x<-99u64;proc held=read;effect @source<-7u64;'
            'effect @p=held;behave @print(perform @p());behave @print(held());behave @print(read());behave @print(@x);',
    }, 0, '1\n7\n7\n99\n', ''),
    'distinct_sibling_overloads_remain_available': ({
        'base.cod': 'func score(x:T)->u64 where T{return 1u64;}func twice(x:T) where T{return score(x)+score(x);}export score;export twice;',
        'a.cod': 'import "base.cod" score;func score(x:u64)->u64{return 7u64;}export import "base.cod" twice;',
        'b.cod': 'import "base.cod" score;func score(x:bool)->u64{return 11u64;}export score;',
        'main.cod': IO + 'using "b.cod" score as other;import "a.cod" twice;behave @print(twice(true));behave @print(twice(1u64));',
    }, 0, '22\n14\n', ''),
    'sibling_interpretation_equivalence_remains_unproved': ({
        'base.cod': BASE + 'func flavor(x:T)->u64 where T{return 1u64;}export flavor;',
        'shared.cod': 'import "base.cod" score;using "base.cod" flavor;func score(x:u64)->u64{return flavor(x);}export score;',
        'a.cod': 'export import "shared.cod" score;import "base.cod" flavor;func flavor(x:u64)->u64{return 7u64;}',
        'b.cod': 'export import "shared.cod" score;import "base.cod" flavor;func flavor(x:u64)->u64{return 11u64;}',
        'main.cod': 'import "base.cod" twice;using "a.cod" score as first;using "b.cod" score as second;twice(1u64);',
    }, 2, '', 'multiple selected interpretations of one inherited definition'),
}
runner.CASES['multiple_extension_aliases_error'] = (*runner.CASES['multiple_extension_aliases_warn'][:1], 2, '', 'treated as error')
runner.CASE_ARGUMENTS = {'multiple_extension_aliases_error': ['-Werror=multi-alias-extension']}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
