#!/usr/bin/env python3
"""Typed dynamic bindings compose with calls, cleanup and iteration restoration."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
X = 'symbol @x:u64;'
READ = 'proc read()->u64{return @x;}'
SHOW = 'proc show()->void{behave @print(@x);}'
MISSING = 'using "std:exceptions" MissingReturnException;func missing()->u64{}func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}'
runner.CASES = {
    'basic': (IO+X+'effect @x<-1u64;behave @print(@x);effect @x<-2u64;behave @print(@x);', 0, '1\n2\n', ''),
    'grouping_and_full_scope': (IO+X+'effect @x<-1u64;{effect @x<-2u64;};behave @print(@x);'
        '{{effect @x<-3u64;behave @print(@x);}};behave @print(@x);', 0, '2\n3\n2\n', ''),
    'rhs_sees_old_value': (IO+X+'effect @x<-1u64;effect @x<-@x+1u64;behave @print(@x);', 0, '2\n', ''),
    'primitive_types_and_widening': (IO+'symbol a:u8,b:u32,c:i64,d:f32,e:f64,f:bool;'
        'effect @a<-255u8;effect @b<-@a;effect @c<-@b;effect @d<-3u8;effect @e<-@d;effect @f<-true;'
        'behave @print(@a);behave @print(@b);behave @print(@c);behave @print(@d);behave @print(@e);behave @print(@f);',
        0, '255\n255\n255\n3\n3\ntrue\n', ''),
    'direct_procedure': (IO+X+READ+'effect @x<-3u64;behave @print(read());', 0, '3\n', ''),
    'procedure_restore': (IO+X+READ+'proc set()->u64{effect @x<-4u64;return read();}'
        'effect @x<-3u64;behave @print(set());behave @print(read());', 0, '4\n3\n', ''),
    'generic_recursive_restore': (IO+X+READ+'proc recurse(n:T)->u64 where T{'
        'if(n==0){return read();}else{effect @x<-@x+1u64;return recurse(n-1);};}'
        'effect @x<-5u64;behave @print(recurse(3));behave @print(read());', 0, '8\n5\n', ''),
    'argument_effect_belongs_to_caller': (IO+X+'proc take(v:u64)->u64{effect @x<-9u64;return v+@x;}'
        'effect @x<-1u64;behave @print(take({effect @x<-2u64;@x}));behave @print(@x);', 0, '11\n2\n', ''),
    'deferred_reads_final_binding': (IO+X+SHOW+'effect @x<-1u64;{{defer show();effect @x<-2u64;}};show();', 0, '2\n1\n', ''),
    'late_binding_before_cleanup': (IO+X+SHOW+'defer show();effect @x<-6u64;', 0, '6\n', ''),
    'late_binding_delayed_block': (IO+X+'defer {behave @print(@x);};effect @x<-7u64;', 0, '7\n', ''),
    'deferred_argument_snapshot': (IO+X+'effect @x<-1u64;defer behave @print(@x);effect @x<-2u64;', 0, '1\n', ''),
    'deferred_local_effect': (IO+X+SHOW+'effect @x<-1u64;defer show();defer {{{effect @x<-8u64;show();}}};', 0, '8\n1\n', ''),
    'return_cleanup_and_restore': (IO+X+SHOW+'proc f()->u64{effect @x<-9u64;defer show();return @x;}'
        'effect @x<-1u64;behave @print(f());show();', 0, '9\n9\n1\n', ''),
    'cleanup_replaces_return': (IO+X+SHOW+'proc f()->u64{effect @x<-2u64;defer{return @x+1u64;};return @x;}'
        'effect @x<-1u64;behave @print(f());show();', 0, '3\n1\n', ''),
    'nested_cleanup_effect': (IO+X+SHOW+'effect @x<-1u64;defer show();defer {{{effect @x<-2u64;defer show();}}};', 0, '2\n1\n', ''),
    'deferred_handler_binding': (IO+X+MISSING+SHOW+'defer catch(e:const MissingReturnException){show();};effect @x<-2u64;throw error();',
        0, '2\n', ''),
    'throw_restores': (IO+X+MISSING+SHOW+'proc f()->void{effect @x<-2u64;defer show();throw error();}'
        'effect @x<-1u64;try{f();}catch(e:const MissingReturnException){show();};', 0, '2\n1\n', ''),
    'failed_rhs_preserves_binding': (IO+X+MISSING+SHOW+'effect @x<-1u64;'
        'try{effect @x<-{throw error();};}catch(e:const MissingReturnException){show();};', 0, '1\n', ''),
    'failed_rhs_retains_its_effects': (IO+X+MISSING+SHOW+'effect @x<-1u64;'
        'try{effect @x<-{effect @x<-2u64;throw error();};}catch(e:const MissingReturnException){show();};', 0, '2\n', ''),
    'finally_uses_current_scope': (IO+X+SHOW+'effect @x<-1u64;{{try{effect @x<-2u64;}finally{show();};}};show();', 0, '2\n1\n', ''),
    'iterations_restore': (IO+X+SHOW+'effect @x<-1u64;for(i in 0u64..3u64){show();effect @x<-i+2u64;show();};show();',
        0, '1\n2\n1\n3\n1\n4\n1\n', ''),
    'nested_iterations_restore': (IO+X+SHOW+'effect @x<-1u64;for(i in 0u64..2u64){effect @x<-i+2u64;'
        'for(j in 0u64..2u64){effect @x<-@x+1u64;show();};show();};show();', 0, '3\n3\n2\n4\n4\n3\n1\n', ''),
    'iteration_full_scope_cleanup': (IO+X+SHOW+'effect @x<-1u64;for(i in 0u64..2u64){effect @x<-i+2u64;{{defer show();}};};show();',
        0, '2\n3\n1\n', ''),
    'iteration_defer_uses_outer_binding': (IO+X+SHOW+'effect @x<-1u64;for(i in 0u64..3u64){effect @x<-i+2u64;defer show();};', 0, '1\n1\n1\n', ''),
    'iteration_defer_saves_argument': (IO+X+'effect @x<-1u64;for(i in 0u64..3u64){effect @x<-i+2u64;defer behave @print(@x);};', 0, '4\n3\n2\n', ''),
    'iteration_return_restores_before_outer_defer': (IO+X+SHOW+'proc f()->u64{defer show();for(i in 0u64..3u64){effect @x<-2u64;return @x;};return 0u64;}'
        'effect @x<-1u64;behave @print(f());show();', 0, '1\n2\n1\n', ''),
    'iteration_throw_restores_before_catch': (IO+X+MISSING+SHOW+'effect @x<-1u64;'
        'try{for(i in 0u64..2u64){effect @x<-2u64;throw error();};}catch(e:const MissingReturnException){show();};', 0, '1\n', ''),
    'empty_else_restores': (IO+X+SHOW+'effect @x<-1u64;for(i in 0u64..0u64){}else{effect @x<-2u64;show();defer show();};show();', 0, '2\n1\n1\n', ''),
    'fold_restores': (IO+X+SHOW+'effect @x<-1u64;behave @print(fold(i in 0u64..3u64,a=0u64){effect @x<-i+1u64;a+@x});show();', 0, '6\n1\n', ''),
    'branch_establishes_binding': (IO+X+'if(true){effect @x<-2u64;}else{effect @x<-3u64;};behave @print(@x);', 0, '2\n', ''),
    'touch_dependency': (X+'proc f()->void{touch @x;}effect @x<-1u64;f();', 0, '', ''),
    'locally_bound_callable': (IO+X+'proc p=proc()->u64{effect @x<-2u64;return @x;};behave @print(p());', 0, '2\n', ''),
    'unbound_read': (X+'@x;', 2, '', 'cannot prove an installed binding'),
    'unbound_call': (X+READ+'read();', 2, '', 'cannot prove an installed binding'),
    'callee_binding_does_not_escape': (X+'proc set()->void{effect @x<-1u64;}set();@x;', 2, '', 'cannot prove an installed binding'),
    'full_scope_binding_does_not_escape': (X+'{{effect @x<-1u64;}};@x;', 2, '', 'cannot prove an installed binding'),
    'iteration_binding_does_not_escape': (X+'for(i in 0u64..1u64){effect @x<-1u64;};@x;', 2, '', 'cannot prove an installed binding'),
    'else_binding_does_not_escape': (X+'for(i in 0u64..0u64){}else{effect @x<-1u64;};@x;', 2, '', 'cannot prove an installed binding'),
    'partial_branch_binding': (X+'proc f(b:bool)->void{if(b){effect @x<-1u64;}else{};touch @x;}f(true);', 2, '', 'cannot prove an installed binding'),
    'early_read_not_fixed_by_later_install': (X+'proc f()->void{touch @x;effect @x<-1u64;}f();', 2, '', 'cannot prove an installed binding'),
    'deferred_iteration_read_cannot_retain_binding': (X+SHOW+IO+'for(i in 0u64..1u64){effect @x<-1u64;defer show();};', 2, '', 'cannot prove an installed binding'),
    'delayed_iteration_read_cannot_retain_binding': (X+IO+'for(i in 0u64..1u64){effect @x<-1u64;defer {behave @print(@x);};};', 2, '', 'cannot prove an installed binding'),
    'failed_late_install': (X+IO+MISSING+SHOW+'defer show();effect @x<-{throw error();};', 2, '', 'cannot prove an installed binding'),
    'unbound_touch': (X+'touch @x;', 2, '', 'cannot prove an installed binding'),
    'function_read': (X+'func f()->u64{return @x;}effect @x<-1u64;f();', 2, '', 'function cannot access dynamic bindings'),
    'function_install': (X+'func f()->void{effect @x<-1u64;}f();', 2, '', 'function cannot access dynamic bindings'),
    'function_touch': (X+'func f()->void{touch @x;}f();', 2, '', 'function cannot access dynamic bindings'),
    'bits_rebind_rejects': (X+'effect @x=1u64;', 2, '', "bits symbols require effect '<-'"),
    'wrong_type': (X+'effect @x<-true;', 2, '', 'effect value does not satisfy'),
    'bare_write_rejects': (X+'effect @x<-1u64;@x<-2u64;', 2, '', 'assignment requires a storage destination'),
    'dynamic_capture_boundary': (X+READ+'effect @x<-1u64;proc p=read;p();', 2, '', 'procedure value construction with dynamic binding captures'),
}
for name, (source, status, output, diagnostic) in list(runner.CASES.items()):
    if MISSING in source and status == 0:
        runner.CASES[name] = (source, status, output, '[-Wmissing-return]')
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
