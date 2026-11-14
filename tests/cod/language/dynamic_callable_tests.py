#!/usr/bin/env python3
"""Complete procedure values retain dynamic bindings and replay them on perform."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
X = 'symbol @x:u64;proc read()->u64{return @x;}'
P = 'symbol @p:proc()->u64;'
MISSING = 'using "std:exceptions" MissingReturnException;func missing()->u64{}func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}'
runner.CASES = {
    'current_and_replay': (IO+X+P+'effect @x<-1u64;proc local=read;effect @p=local;effect @x<-2u64;'
        'behave @print(local());behave @print(behave @p());behave @print(perform @p());behave @print(@x);', 0, '2\n2\n1\n2\n', ''),
    'install_materializes': (IO+X+P+'effect @x<-3u64;effect @p=read;effect @x<-4u64;behave @print(perform @p());', 0, '3\n', ''),
    'reinstall_preserves_capture': (IO+X+P+'symbol @q:proc()->u64;effect @x<-3u64;effect @p=read;effect @x<-4u64;'
        'effect @q=@p;behave @print(perform @q());behave @print(behave @q());', 0, '3\n4\n', ''),
    'independent_holder_copy': (IO+X+P+'effect @x<-3u64;proc a=read;proc b=a;effect @x<-4u64;a<-read;'
        'effect @p=b;behave @print(perform @p());effect @p=a;behave @print(perform @p());', 0, '3\n4\n', ''),
    'return_snapshot': (IO+X+P+'proc make_value()->proc()->u64{effect @x<-7u64;return read;}'
        'effect @p=make_value();behave @print(perform @p());', 0, '7\n', ''),
    'current_call_after_return': (IO+X+P+'proc make_value()->proc()->u64{effect @x<-7u64;return read;}'
        'effect @x<-8u64;effect @p=make_value();behave @print(behave @p());behave @print(perform @p());', 0, '8\n7\n', ''),
    'nested_symbol_capture': (IO+X+P+'symbol @q:proc()->u64;proc replay()->u64{return perform @p();}'
        'effect @x<-3u64;effect @p=read;effect @q=replay;effect @x<-4u64;effect @p=read;'
        'behave @print(perform @q());behave @print(behave @q());behave @print(perform @p());', 0, '3\n4\n4\n', ''),
    'nested_snapshot_outlives_binding_scopes': (IO+X+P+'symbol @q:proc()->u64;proc replay()->u64{return perform @p();}'
        'proc build()->proc()->u64{effect @x<-9u64;effect @p=read;return replay;}'
        'effect @q=build();behave @print(perform @q());', 0, '9\n', ''),
    'generic_capture_contexts': (IO+X+'symbol @p:proc(u64)->u64,@q:proc(bool)->u64;'
        'proc get(v:T)->u64 where T{return @x;}effect @x<-3u64;effect @p=get;effect @x<-4u64;effect @q=get;'
        'effect @x<-5u64;behave @print(perform @p(1u64));behave @print(perform @q(true));behave @print(behave @p(1u64));', 0, '3\n4\n5\n', ''),
    'nested_current_capture': (IO+X+P+'symbol @q:proc()->u64;proc current()->u64{return behave @p();}'
        'effect @x<-3u64;effect @p=read;effect @q=current;effect @x<-4u64;effect @p=read;'
        'behave @print(perform @q());behave @print(behave @q());', 0, '3\n4\n', ''),
    'lexical_and_dynamic_capture': (IO+X+P+'effect @x<-5u64;proc local=read;proc outer=proc[local]()->u64{return local()+@x;};'
        'effect @p=outer;effect @x<-6u64;behave @print(perform @p());behave @print(behave @p());', 0, '10\n12\n', ''),
    'nested_module_capture': (IO+X+P+'effect @x<-5u64;proc outer=proc[read]()->u64{return read()+@x;};'
        'effect @p=outer;effect @x<-6u64;behave @print(perform @p());', 0, '10\n', ''),
    'touch_only_capture': (IO+'symbol @x:u64;'+P+'proc touched()->u64{touch @x;return 7u64;}'
        'effect @x<-1u64;effect @p=touched;behave @print(perform @p());', 0, '7\n', ''),
    'symbol_read_is_complete_value': (IO+X+P+'effect @x<-3u64;effect @p=read;proc saved=@p;'
        'effect @x<-4u64;effect @p=read;effect @p=saved;behave @print(perform @p());', 0, '3\n', ''),
    'replay_without_current_capture_bindings': (IO+X+P+'proc build()->proc()->u64{effect @x<-8u64;return read;}'
        'effect @p=build();behave @print(perform @p());', 0, '8\n', ''),
    'scope_restoration': (IO+X+P+'effect @x<-1u64;effect @p=read;{{effect @x<-2u64;effect @p=read;'
        'behave @print(perform @p());}};behave @print(perform @p());', 0, '2\n1\n', ''),
    'same_scope_replacement': (IO+X+P+'effect @x<-1u64;effect @p=read;effect @x<-2u64;effect @p=read;'
        'effect @x<-3u64;effect @p=read;behave @print(perform @p());', 0, '3\n', ''),
    'iteration_restoration': (IO+X+P+'effect @x<-1u64;effect @p=read;for(i in 0u64..3u64){'
        'effect @x<-i+2u64;effect @p=read;behave @print(perform @p());};behave @print(perform @p());', 0, '2\n3\n4\n1\n', ''),
    'defer_callee_snapshot': (IO+X+'symbol @p:proc()->void;proc show()->void{behave @print(@x);}'
        'effect @x<-1u64;effect @p=show;defer perform @p();defer behave @p();effect @x<-2u64;effect @p=show;', 0, '2\n1\n', ''),
    'defer_after_iteration': (IO+X+'symbol @p:proc()->void;proc show()->void{behave @print(@x);}'
        'effect @x<-1u64;for(i in 0u64..3u64){effect @x<-i+2u64;effect @p=show;defer perform @p();};', 0, '4\n3\n2\n', ''),
    'delayed_block_reads_final_callee': (IO+X+'symbol @p:proc()->void;proc show()->void{behave @print(@x);}'
        'effect @x<-1u64;effect @p=show;defer {perform @p();};effect @x<-2u64;effect @p=show;', 0, '2\n', ''),
    'callee_selected_before_operand_effect': (IO+X+'symbol @p:proc(u64)->u64;proc a(v:u64)->u64{return @x+v;}'
        'effect @x<-1u64;effect @p=a;behave @print(perform @p({effect @x<-2u64;effect @p=a;10u64}));'
        'behave @print(perform @p(10u64));', 0, '11\n12\n', ''),
    'callable_value_argument': (IO+X+'symbol @p:proc(proc()->u64)->u64;proc apply(f:proc()->u64)->u64{return f();}'
        'effect @x<-1u64;effect @p=apply;effect @x<-2u64;behave @print(perform @p(read));behave @print(behave @p(read));', 0, '1\n2\n', ''),
    'inferred_dynamic_result': (IO+X+P+'proc replay(){return perform @p();}effect @x<-7u64;effect @p=read;behave @print(replay());', 0, '7\n', ''),
    'type_query_needs_no_installation': (IO+P+'val n<-make local typeof(perform @p()){9u64};behave @print(n);', 0, '9\n', ''),
    'replay_failure_restores': (IO+X+P+MISSING+'proc bad()->u64{behave @print(@x);throw error();}'
        'effect @x<-1u64;effect @p=bad;effect @x<-2u64;try{perform @p();{}}catch(e:const MissingReturnException){behave @print(@x);};', 0, '1\n2\n', ''),
    'failed_installation_keeps_old_callee': (IO+X+P+MISSING+'effect @x<-1u64;effect @p=read;effect @x<-2u64;'
        'try{effect @p={throw error();};}catch(e:const MissingReturnException){behave @print(perform @p());};', 0, '1\n', ''),
    'failed_installation_retains_rhs_effect': (IO+X+P+MISSING+'effect @x<-1u64;effect @p=read;effect @x<-2u64;'
        'try{effect @p={effect @p=read;throw error();};}catch(e:const MissingReturnException){behave @print(perform @p());};', 0, '2\n', ''),
    'replay_cleanup_replaces_return': (IO+X+P+'proc final()->u64{defer{return @x+1u64;};return @x;}'
        'effect @x<-3u64;effect @p=final;effect @x<-4u64;behave @print(perform @p());behave @print(@x);', 0, '4\n4\n', ''),
    'recursive_replay_restores': (IO+X+'symbol @p:proc(u64)->u64;proc recurse(n:u64)->u64{'
        'if(n==0u64){return @x;}else{effect @x<-@x+1u64;return recurse(n-1u64);};}'
        'effect @x<-3u64;effect @p=recurse;effect @x<-4u64;behave @print(perform @p(3u64));behave @print(@x);', 0, '6\n4\n', ''),
    'callee_rebinding_restores': (IO+X+P+'proc local()->u64{effect @x<-9u64;return @x;}'
        'effect @x<-1u64;effect @p=local;behave @print(perform @p());behave @print(@x);', 0, '9\n1\n', ''),
    'unbound_capture': (X+'proc p=read;', 2, '', 'cannot prove an installed binding'),
    'unbound_install_source': (X+P+'effect @p=read;', 2, '', 'cannot prove an installed binding'),
    'function_cannot_materialize_dynamic': (X+'func build()->proc()->u64{return read;}effect @x<-1u64;build();', 2, '', 'function cannot capture dynamic bindings'),
    'function_can_copy_existing_value': (IO+X+P+'func copy(p:proc()->u64)->proc()->u64{return p;}'
        'effect @x<-4u64;effect @p=copy(read);effect @x<-5u64;behave @print(perform @p());', 0, '4\n', ''),
    'behave_requires_current_bindings': (X+P+'proc build()->proc()->u64{effect @x<-8u64;return read;}effect @p=build();behave @p();', 2, '', 'cannot prove an installed binding'),
    'current_value_call_requires_bindings': (X+'proc build()->proc()->u64{effect @x<-8u64;return read;}proc p=build();p();', 2, '', 'cannot prove an installed binding'),
    'unbound_callee': (P+'perform @p();', 2, '', 'cannot prove an installed binding'),
    'wrong_effect_operator': (P+'proc read()->u64{return 1u64;}effect @p<-read;', 2, '', "procedure symbols require effect '='"),
    'wrong_effect_type': (P+'effect @p=1u64;', 2, '', 'effect value does not satisfy'),
    'wrong_call_arity': (P+'proc read()->u64{return 1u64;}effect @p=read;perform @p(2u64);', 2, '', 'argument count'),
    'not_a_procedure_symbol': ('symbol @x:u64;effect @x<-1u64;perform @x();', 2, '', 'dynamic invocation requires a procedure symbol'),
    'family_without_implementation': (P+'effect @p;', 2, '', 'symbolic family has no applicable implementation'),
}
for name, (source, status, output, diagnostic) in list(runner.CASES.items()):
    if MISSING in source and status == 0:
        runner.CASES[name] = (source, status, output, '[-Wmissing-return]')
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
