#!/usr/bin/env python3
"""Named values and capture operands respect the nearest lexical identity."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
MODULE = 'func recur(n:u64)->u64{return 999u64;}'
APPLY = 'func apply(f:func(u64)->u64,n:u64)->u64{return f(n);}'
runner.CASES = {
    'recursive_value_does_not_select_module': (IO + MODULE + APPLY +
        'func factory()->func(u64)->u64{func recur(n:u64)->u64{if(n==0u64){return 7u64;}else{return apply(recur,n-1u64);};};return recur;}'
        'behave @print(factory()(1u64));', 0, '7\n', ''),
    'recursive_value_without_module': (IO + APPLY +
        'proc run()->void{func recur(n:u64)->u64{if(n==0u64){return 7u64;}else{return apply(recur,n-1u64);};};behave @print(recur(1u64));return;}run();', 0, '7\n', ''),
    'inferred_nested_capture_does_not_select_module': (IO + MODULE +
        'proc run()->void{func recur(n:u64)->u64{if(n==0u64){return 7u64;}else{};func inner=func()->u64{return recur(0u64);};return inner();};behave @print(recur(1u64));return;}run();', 0, '7\n', ''),
    'explicit_nested_capture_does_not_select_module': (IO + MODULE +
        'proc run()->void{func recur(n:u64)->u64{if(n==0u64){return 7u64;}else{};func inner=func[recur]()->u64{return recur(0u64);};return inner();};behave @print(recur(1u64));return;}run();', 0, '7\n', ''),
    'unused_self_definition_capture_retained': (IO + MODULE +
        'proc run()->void{func recur(n:u64)->u64{if(n==0u64){return 7u64;}else{};func inner=func[recur]()->u64{return 7u64;};return inner();};behave @print(recur(1u64));return;}run();', 0, '7\n', 'unused callable capture'),
    'named_initializer_sees_self_before_binding': (IO + MODULE +
        'proc run()->void{func recur(n:u64)->u64{func recur=recur;return 7u64;};behave @print(recur(1u64));return;}run();', 0, '7\n', ''),
    'direct_recursion_retains_local_identity': (IO + MODULE +
        'proc run()->void{func recur(n:u64)->u64{if(n==0u64){return 7u64;}else{return recur(n-1u64);};};behave @print(recur(20u64));return;}run();', 0, '7\n', ''),
    'query_uses_local_recursive_signature': (IO + 'func recur(n:u64)->bool{return true;}' +
        'proc run()->void{func recur(n:u64)->u64{val x<-make local typeof(recur(n)){8u64};return x;};behave @print(recur(1u64));return;}run();', 0, '8\n', ''),
    'contextual_query_does_not_materialize_self': (IO + 'func recur(n:u64)->bool{return true;}' + APPLY +
        'proc run()->void{func recur(n:u64)->u64{val x<-make local typeof(apply(recur,n)){9u64};return x;};behave @print(recur(1u64));return;}run();', 0, '9\n', ''),
    'nested_query_retains_lexical_signature': (IO + 'func recur(n:u64)->bool{return true;}' +
        'proc run()->void{func recur(n:u64)->u64{if(n==0u64){return 7u64;}else{};func inner=func()->u64{val x<-make local typeof(recur(0u64)){10u64};return x;};return inner();};behave @print(recur(1u64));return;}run();', 0, '10\n', ''),
    'nested_query_without_module': (IO +
        'proc run()->void{func recur(n:u64)->u64{if(n==0u64){return 7u64;}else{};func inner=func()->u64{val x<-make local typeof(recur(0u64)){16u64};return x;};return inner();};behave @print(recur(1u64));return;}run();', 0, '16\n', ''),
    'nested_query_retains_enclosing_types': (IO +
        'func tag(x:u64)->u64{return 64u64;}func tag(x:f64)->u64{return 164u64;}'
        'func factory(x:T)->func(T)->const T where T{func recur(y:T)->const T{'
        'func inner=func()->const T{val z<-make local typeof(recur(y)){1u8};return z;};return inner();};return recur;}'
        'behave @print(tag(factory(1u64)(2u8)));behave @print(tag(factory(1f64)(2u8)));', 0, '64\n164\n', ''),
    'parameter_value_shadows_self': (IO + MODULE + APPLY + 'func selected(n:u64)->u64{return 11u64;}' +
        'proc run()->void{func recur(recur:func(u64)->u64,n:u64)->u64{return apply(recur,n);};behave @print(recur(selected,1u64));return;}run();', 0, '11\n', ''),
    'local_value_shadows_self': (IO + MODULE + APPLY + 'func selected(n:u64)->u64{return 12u64;}' +
        'proc run()->void{func recur(n:u64)->u64{func recur=selected;return apply(recur,n);};behave @print(recur(1u64));return;}run();', 0, '12\n', ''),
    'nested_capture_uses_shadowing_parameter': (IO + MODULE + 'func selected(n:u64)->u64{return 13u64;}' +
        'proc run()->void{func recur(recur:func(u64)->u64,n:u64)->u64{func inner=func[recur](n:u64)->u64{return recur(n);};return inner(n);};behave @print(recur(selected,1u64));return;}run();', 0, '13\n', ''),
    'scalar_shadow_is_not_bypassed': (MODULE + APPLY +
        'proc run()->void{func recur(recur:u64)->u64{return apply(recur,1u64);};recur(1u64);}run();', 2, '', 'no applicable method'),
    'module_capture_still_materializes': (IO + MODULE +
        'proc run()->void{func inner=func[recur](n:u64)->u64{return recur(n);};behave @print(inner(1u64));return;}run();', 0, '999\n', ''),
    'local_capture_still_snapshots_value': (IO + 'func selected(n:u64)->u64{return 14u64;}' +
        'proc run()->void{func recur=selected;func inner=func[recur](n:u64)->u64{return recur(n);};recur<-func(n:u64)->u64{return 15u64;};behave @print(inner(1u64));return;}run();', 0, '14\n', ''),
}
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {
    'recursive_value_does_not_select_module': ['recur'],
    'inferred_nested_capture_does_not_select_module': ['recur'],
    'explicit_nested_capture_does_not_select_module': ['recur'],
    'unused_self_definition_capture_retained': ['recur'],
    'named_initializer_sees_self_before_binding': ['recur','recur'],
    'direct_recursion_retains_local_identity': ['recur'],
    'query_uses_local_recursive_signature': ['recur'],
    'contextual_query_does_not_materialize_self': ['recur'],
    'nested_query_retains_lexical_signature': ['recur'],
    'parameter_value_shadows_self': ['recur', 'recur'],
    'local_value_shadows_self': ['recur', 'recur'],
    'nested_capture_uses_shadowing_parameter': ['recur', 'recur', 'n'],
}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
