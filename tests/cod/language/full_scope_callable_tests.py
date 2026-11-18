#!/usr/bin/env python3
"""Full scopes share ordinary value inputs, inference and local completion."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
BASE = 'func one()->const u64{return 1u64;}func two()->const u64{return 2u64;}'
runner.CASES = {
    'independent_scalar': (IO+'proc run()->u64{val n<-1u64;{{n<-2u64;}};return n;}behave @print(run());', 0, '1\n', ''),
    'local_return': (IO+'func run()->u64{ {{return 1u64;}};return 2u64;}behave @print(run());', 0, '2\n', ''),
    'explicit_result': (IO+'func run()->u64{return {{return 7u64;}};}behave @print(run());', 0, '7\n', ''),
    'independent_inference': (IO+'func run(){ {{return true;}};return 8u64;}behave @print(run());', 0, '8\n', ''),
    'tail_is_void': ('func run()->u64{return {{7u64;}};}run();', 2, '', 'return type'),
    'inferred_void': (IO+'func run(){ {{return 7u64;}};true;}run();behave @print(9u64);', 0, '9\n', ''),
    'void_fallthrough': (IO+'proc run()->void{ {{behave @print(1u64);}};behave @print(2u64);}run();', 0, '1\n2\n', ''),
    'cleanup_completion': (IO+'proc run()->u64{val n<-1u64;val inner<-{{defer{n<-3u64;};return n;}};return n+inner;}behave @print(run());', 0, '2\n', ''),
    'cleanup_local_return': (IO+'proc run()->u64{defer{{return 9u64;}};return 2u64;}behave @print(run());', 0, '2\n', ''),
    'cleanup_local_return_grouped': (IO+'proc run()->u64{defer{ {{return 9u64;}};};return 2u64;}behave @print(run());', 0, '2\n', ''),
    'delayed_capture': (IO+'proc run()->void{val n<-1u64;defer{{behave @print(n);}};n<-2u64;}run();', 0, '2\n', ''),
    'delayed_capture_grouped': (IO+'proc run()->void{val n<-1u64;defer{ {{behave @print(n);}};};n<-2u64;}run();', 0, '2\n', ''),
    'nested_cleanup': (IO+'proc run()->void{defer{behave @print(4u64);};{{defer{behave @print(3u64);};{{defer{behave @print(2u64);};behave @print(1u64);}};}};}run();', 0, '1\n2\n3\n4\n', ''),
    'argument_order': (IO+'func sum(val a:u64,val b:u64)->u64{return a+b;}proc run()->void{val n<-1u64;behave @print(sum({{n<-2u64;return n;}},n));behave @print(n);}run();', 0, '3\n1\n', ''),
    'generic_value_inputs': (IO+'func run(val x:T)->const T where T{return {{x<-x+x;return x;}};}behave @print(run(3u64));behave @print(run(4));behave @print(run(2f32));', 0, '6\n8\n4\n', ''),
    'complete_value_input': (IO+BASE+'proc run()->void{func saved=one;{{saved<-two;behave @print(saved());}};behave @print(saved());}run();', 0, '2\n1\n', ''),
    'returned_complete_value': (IO+'func factory(val seed:u64)->func()->const u64{return {{seed<-seed+1u64;return func[seed]()->const u64{return seed;};}};}func saved=factory(4u64);behave @print(saved());behave @print(saved());', 0, '5\n5\n', ''),
    'query_has_no_effects': (IO+'proc run()->void{val n<-make local u64;val result<-make local typeof({{behave @print(n);return 7u64;}}){8u64};behave @print(result);}run();', 0, '8\n', ''),
    'query_readonly_write': ('const val n<-1u64;val result<-make local typeof({{n<-2u64;return n;}}){3u64};', 2, '', 'readonly'),
    'query_function_effect': (IO+'func run()->void{val n<-make local typeof({{behave @print(1u64);return 2u64;}}){3u64};}run();', 2, '', 'function cannot invoke a dynamic procedure'),
    'query_function_direct_procedure': ('proc action()->void{}func run()->void{val n<-make local typeof({{action();return 2u64;}}){3u64};}run();', 2, '', 'function cannot invoke a procedure'),
    'query_unreachable_readonly_write': ('const val n<-1u64;val result<-make local typeof({{return n;n<-2u64;}}){3u64};', 2, '', 'readonly'),
    'query_nested_readonly_write': ('const val n<-1u64;val result<-make local typeof({{ {{n<-2u64;}};return n;}}){3u64};', 2, '', 'readonly'),
    'query_deferred_readonly_write': ('const val n<-1u64;val result<-make local typeof({{defer{n<-2u64;};return n;}}){3u64};', 2, '', 'readonly'),
    'query_needs_no_installed_binding': (IO+'symbol @bias:u64;val result<-make local typeof({{return @bias;}}){8u64};behave @print(result);', 0, '8\n', ''),
    'query_established_nested_signature': (IO+'func unused(x:T)->u64 where T{return true;}val result<-make local typeof({{return unused(1u64);}}){8u64};behave @print(result);', 0, '8\n', ''),
    'query_tail_is_void': ('val result<-make local typeof({{7u64;}}){8u64};', 2, '', 'initializer does not satisfy local storage type'),
    'function_permissions': (IO+'func run()->void{ {{behave @print(1u64);}};}run();', 2, '', 'function cannot invoke a dynamic procedure'),
    'readonly_capture': ('proc run()->void{const val n<-1u64;{{n<-2u64;}};}run();', 2, '', 'readonly'),
    'capture_requires_initialization': ('proc run()->void{val n<-make local u64;{{n<-2u64;}};}run();', 2, '', 'before definite initialization'),
    'reference_input_writes_target': (IO+'proc run()->void{ref n:u64=1u64;{{n<-2u64;}};behave @print(n);}run();', 0, '2\n', ''),
    'place_capture_requires_proof': ('proc run()->void{val n<-1u64;var p->n;{{p<-2u64;}};}run();', 2, '', 'selected-place lifetime and transport proof'),
    'missing_return_after_inner_completion': (IO+'using "std:exceptions" MissingReturnError;proc run()->u64{ {{return 1u64;}};}try{run();{}}catch(e:const MissingReturnError){behave @print(5u64);};', 0, '5\n', '[-Wmissing-return]'),
    'inner_missing_return': (IO+'using "std:exceptions" MissingReturnError;proc run(val flag:bool)->void{try{val n<-{{if(flag){return 1u64;}else{};}};behave @print(n);}catch(e:const MissingReturnError){behave @print(5u64);};}run(false);run(true);', 0, '5\n1\n', '[-Wmissing-return]'),
    'captured_value_has_invocation_owner': (IO+'func choose(ref a:u64 in O,ref b:u64 in O)->const u64 where {owner O}{return 7u64;}func choose(ref a:u64 in O,ref b:u64 in P)->const bool where {owner O,owner P}{return true;}func run(){val source<-1u64;return {{val local<-2u64;return choose(source,local);}};}behave @print(run());', 0, '7\n', ''),
    'outer_catch_is_not_ambient': ('using "std:exceptions" MissingReturnError;func missing()->const u64{}proc run()->void{try{missing();{}}catch(e:const MissingReturnError){ {{if(true){throw;}else{return 1u64;};}};{}};}run();', 2, '', 'bare throw requires an active catch'),
    'unchecked_failure_crosses_call': (IO+'using "std:exceptions" MissingReturnError;func missing()->const u64{}proc run()->void{defer{behave @print(4u64);};try{ {{defer{behave @print(2u64);};behave @print(1u64);missing();}};}catch(e:const MissingReturnError){behave @print(3u64);};}run();', 0, '1\n2\n3\n4\n', '[-Wmissing-return]'),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
