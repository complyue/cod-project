#!/usr/bin/env python3
"""Structured returns, finally and delayed scalar blocks share cleanup control."""
from pathlib import Path
import sys
import foundation_tests as runner

PRINT = 'using symbol "std:io" @print;\n'
OBSERVE = PRINT + 'proc observe(x:i64)->void {behave @print(x);return;}\n'
BAD = OBSERVE + 'func bad()->const i64 {}\n'
runner.CASES = {
    "finally_value": (OBSERVE + 'behave @print(try{4}finally{observe(7);});', 0, "7\n4\n", ""),
    "finally_return": (OBSERVE + 'proc f()->const i64{defer observe(1);try{return 3;}finally{observe(2);};} behave @print(f());', 0, "2\n1\n3\n", ""),
    "finally_nested_scope": (OBSERVE + 'proc f()->const i64{defer observe(1);return try{ {{defer observe(2);return 6;}} }finally{ {{defer observe(3);observe(4);}}; };} behave @print(f());', 0, "2\n4\n3\n1\n6\n", ""),
    "finally_replace_return": (OBSERVE + 'proc f()->const i64{defer observe(1);try{return 3;}finally{ {{defer observe(2);}};return 4;}} behave @print(f());', 0, "2\n1\n4\n", ""),
    "finally_replace_failure": (BAD + 'proc f()->const i64{defer observe(1);try{bad();}finally{return 8;}} behave @print(f());', 0, "1\n8\n", "[-Wmissing-return]"),
    "finally_failure_cleanup": (BAD + 'proc f()->const i64{defer observe(1);try{return 3;}finally{bad();{}}} f();', 70, "1\n", "uncaught MissingReturnError"),
    "finally_nested_return": (OBSERVE + 'proc f()->const i64{try{try{return 1;}finally{return 2;}}finally{observe(3);}} behave @print(f());', 0, "3\n2\n", ""),
    "generic_completion": (PRINT + 'proc f(x:T)->const T where T {defer{behave @print(x);}finally{};try{return x;}finally{x<-x+x;{}}} behave @print(f(3u32));behave @print(f(1.5f32));', 0, "6\n3\n3\n1.5\n", ""),
    "generic_replacement": (PRINT + 'func f(x:T)->const T where T {try{return x;}finally{return x+x;}} behave @print(f(3u64));behave @print(f(1.5f32));', 0, "6\n3\n", ""),
    "finally_registration": (OBSERVE + '{{defer observe(1);try{defer observe(2);observe(3);}finally{defer observe(4);observe(5);};observe(6);}};', 0, "3\n5\n6\n4\n2\n1\n", ""),
    "deferred_late_read": (OBSERVE + '{{val n<-1;defer observe(n);defer{observe(n);};n<-2:void;}};', 0, "2\n1\n", ""),
    "deferred_grouped_call_on_failure": (
        OBSERVE + 'using "std:exceptions" NullPointerException;'
        'proc prepare()->i64{observe(10);return 20;}'
        'proc f()->void throws NullPointerException{'
        'defer ((observe(prepare())));observe(15);ptr absent:i64 = null in here;observe(absent);return;}'
        'try{f();observe(99);}catch(failure:const NullPointerException){observe(30);};',
        0, "10\n15\n20\n30\n", ""),
    "deferred_grouped_noncall": ('defer ((1));', 2, "", "defer requires a callable invocation"),
    "deferred_grouped_scalar": (
        OBSERVE + 'proc compute()->i64{observe(20);return 42;}{{defer ((compute()));observe(10);}};',
        0, "10\n20\n", ""),
    "deferred_grouped_borrowed_result": (
        'func selected(ref value:i64 in R)->ref i64 in R where {owner R}{return value;}'
        'val value<-17;defer ((selected(value)));',
        2, "", "parenthesized deferred result projection"),
    "deferred_return_snapshot": (OBSERVE + 'proc f()->const i64{val n<-1;defer{observe(n);};defer{n<-2;};return n;} behave @print(f());', 0, "2\n1\n", ""),
    "deferred_return": (OBSERVE + 'proc f()->const i64{defer observe(1);defer{return 2;};return 3;} behave @print(f());', 0, "1\n2\n", ""),
    "deferred_fallthrough_return": (OBSERVE + 'proc f()->const i64{defer{return 2;};} behave @print(f());', 0, "2\n", ""),
    "deferred_failure_return": (BAD + 'proc f()->const i64{defer observe(1);defer{return 2;};return bad();} behave @print(f());', 0, "1\n2\n", "[-Wmissing-return]"),
    "deferred_plain_failure": (BAD + '{{defer{observe(1);};bad();}};', 70, "1\n", "uncaught MissingReturnError"),
    "deferred_nested_registration": (OBSERVE + '{{defer{observe(1);};defer{defer{observe(2);};observe(3);};}};', 0, "3\n2\n1\n", ""),
    "deferred_nested_finally": (OBSERVE + 'proc f()->const i64{defer observe(1);defer{try{ {{defer observe(2);return 3;}};{} }finally{observe(4);}};return 5;} behave @print(f());', 0, "2\n4\n1\n5\n", ""),
    "deferred_attached_finally": (OBSERVE + '{{defer observe(1);defer{observe(2);}finally{observe(3);};}};', 0, "2\n3\n1\n", ""),
    "deferred_finally_return": (OBSERVE + 'proc f()->const i64{defer observe(1);defer{return 2;}finally{return 3;};return 4;} behave @print(f());', 0, "1\n3\n", ""),
    "deferred_bodyless_finally": (OBSERVE + '{{defer finally{observe(1);};}};', 0, "1\n", ""),
    "deferred_finally_failure": (BAD + '{{defer observe(1);defer{bad();}finally{observe(2);};}};', 70, "2\n1\n", "uncaught MissingReturnError"),
    "deferred_finally_pending_failure": (BAD + '{{defer{observe(99);}finally{observe(1);};bad();}};', 70, "1\n", "uncaught MissingReturnError"),
    "deferred_finally_pending_return": (OBSERVE + 'proc f()->const i64{defer{observe(1);}finally{observe(2);};return 3;} behave @print(f());', 0, "1\n2\n3\n", ""),
    "return_inspection_at_module_scope": ('defer{if returning (){}};', 2, "", "requires deferred execution in a lexical return destination"),
    "return_inspection_in_ordinary_finally": ('proc f()->void{try{return;}finally{if returning (){}}}', 2, "", "requires deferred execution in a lexical return destination"),
    "return_inspection_in_eager_operand": ('proc f()->void{defer f(if returning (){} else {});return;}', 2, "", "requires deferred execution in a lexical return destination"),
    "return_inspection_in_independent_full_scope": ('proc f()->void{defer{{if returning (){};return;}};return;}', 2, "", "requires deferred execution in a lexical return destination"),
    "return_inspection_bare_binding": ('proc f()->void{defer{if returning (result){}};return;}', 2, "", "return pattern binding requires val, ref, ptr or a complete type annotation"),
    "return_inspection_explicit_void_and_fallthrough": (OBSERVE +
        'proc explicit()->void{defer{if returning (){behave @print(1);}else{behave @print(2);}};return;}'
        'func fallthrough()->void{defer{if returning (){1/0;{} }};}'
        'proc iterated()->void{defer{if returning (){behave @print(3);}else{behave @print(4);}};'
        'defer{if returning (){behave @print(5);}else{behave @print(6);return;}};}'
        'explicit();fallthrough();iterated();', 0, "1\n6\n3\n", ""),
    "return_inspection_value_tail": (OBSERVE +
        'func f(){defer{if returning (){return 8;}};7}observe(f());', 0, "8\n", ""),
    "return_inspection_calls_concrete_procedure": (OBSERVE +
        'proc f()->void{defer{if returning (){observe(9);}};return;}f();', 0, "9\n", ""),
    "return_inspection_calls_mutually_recursive_functions": (
        'func first(again:bool)->void{if(again){second(false);}else{};}'
        'func second(again:bool)->void{if(again){first(false);}else{};}'
        'proc f()->void{defer{if returning (){first(true);second(true);}};return;}f();', 0, "", ""),
    "return_inspection_calls_helper_with_local_recovery": (BAD +
        'using "std:exceptions" MissingReturnError;'
        'func handled()->void{try{bad():void;}catch(e:const MissingReturnError){};}'
        'proc f()->i64{defer{if returning (){handled();return 9;}};return 2;}observe(f());',
        0, "9\n", "[-Wmissing-return]"),
    "return_inspection_recovers_failed_preparation": (BAD +
        'proc f()->i64{defer{if returning (){return 99;}else{return 8;}};return bad();}observe(f());',
        0, "8\n", "[-Wmissing-return]"),
    "return_inspection_ordinary_suspension": (OBSERVE +
        'proc f()->i64{defer{try{return 7;}finally{if returning (){behave @print(31);}else{behave @print(32);}};};}observe(f());',
        0, "31\n7\n", ""),
    "return_inspection_projects_mixed_finally": (OBSERVE +
        'proc f(flag:bool)->i64{val n<-make local i64;defer{try{if(flag){n<-11;return 7;}else{};}'
        'finally{if returning (){behave @print(n);}else{n<-22;{}};};return n;};}'
        'observe(f(true));observe(f(false));', 0, "11\n7\n22\n", ""),
    "return_inspection_finally_write_stays_on_matching_completion": (
        'proc f(flag:bool)->i64{val n<-make local i64;defer{try{if(flag){return 7;}else{};}'
        'finally{if returning (){n<-11;{}};};return n;};}f(false);',
        2, "", "cannot prove initialization of 'n'"),
    "return_inspection_projects_nested_finally": (OBSERVE +
        'proc f(outer:bool,inner:bool)->i64{defer{try{if(outer){return 7;}else{};}finally{'
        'try{if(inner){return 9;}else{};}finally{if returning (){behave @print(41);}else{behave @print(42);}};'
        'if returning (){behave @print(43);}else{behave @print(44);};};return 8;};}'
        'observe(f(true,true));observe(f(true,false));observe(f(false,true));observe(f(false,false));',
        0, "41\n9\n41\n43\n7\n41\n9\n42\n44\n8\n", ""),
    "return_inspection_distinguishes_ready_paths": (OBSERVE +
        'proc f(flag:bool)->void{val n<-make local i64;defer{if returning (){behave @print(n);}else{return;}};'
        'if(flag){n<-1;return;}else{};}f(true);f(false);', 0, "1\n", ""),
    "return_inspection_cannot_anchor_its_result": (
        'proc f(){defer{if returning (){return 7u64;}};}f();', 2, "", "independent result contract"),
    "deferred_finally_initialized": (OBSERVE + '{{val n<-make local i64;defer{n<-1;}finally{observe(n);};}};', 0, "1\n", ""),
    "deferred_own_loop": (OBSERVE + '{{defer{for(i in 0..3){observe(i);};};}};', 0, "0\n1\n2\n", ""),
    "deferred_loop_mutation": (OBSERVE + '{{val n<-0;for(i in 0..3){defer{n<-n+1;observe(n);};};}};', 0, "1\n2\n3\n", ""),
    "deferred_local_scope": (OBSERVE + '{{defer{{val n<-4;defer observe(n);observe(n+1);}};}};', 0, "5\n4\n", ""),
    "finally_initialize": (OBSERVE + 'val n<-make local i64;try{n<-1;}finally{observe(n);};observe(n);', 0, "1\n1\n", ""),
    "finally_write": (OBSERVE + 'val n<-make local i64;try{}finally{n<-1;{}};observe(n);', 0, "1\n", ""),
    "finally_call_before_init": (BAD + 'val n<-make local i64;try{bad();n<-1;}finally{observe(n);};', 2, "", "before definite initialization"),
    "finally_return_before_init": ('func f(flag:bool)->const i64{val n<-make local i64;try{if(flag){return 1;}else{};n<-2;}finally{n;{}};return 3;} f(true);', 2, "", "before definite initialization"),
    "finally_projection": (OBSERVE + 'val n<-0;ptr output=n;try{ {{val local<-3;output<-local;}}; }finally{observe(n);};', 0, "3\n", ""),
    "finally_nonvoid": ('try{}finally{1};', 2, "", "finally must complete with void"),
    "deferred_index_capture": (OBSERVE + 'for(i in 0..3){defer{observe(i);};};', 2, "", "cannot capture loop-control"),
    "deferred_accumulator_capture": (OBSERVE + 'fold(i in 0..3,n=0){defer{observe(n);};n+i};', 2, "", "cannot capture loop-control"),
    "deferred_capture_inside_scope": (OBSERVE + 'for(i in 0..3){{val snapshot<-i;defer{observe(snapshot);};}};', 0, "0\n1\n2\n", ""),
    "deferred_later_initialization": (OBSERVE + '{{val n<-make local i64;defer{observe(n);};n<-1;}};', 0, "1\n", ""),
    "deferred_write_not_immediate": (OBSERVE + 'val n<-make local i64;defer{n<-1;};observe(n);', 2, "", "before definite initialization"),
    "deferred_direct_declaration": ('defer{val n<-1;};', 2, "", "deferred-block variable declaration"),
    "deferred_function_effect": (OBSERVE + 'func f()->void{defer{observe(1);};} f();', 2, "", "function cannot invoke a procedure"),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]


from language_test_support import expect_source_rejections
expect_source_rejections(runner, (
    'deferred_nested_registration',
), 'currently draining scope')

if __name__ == "__main__":
    sys.exit(runner.main())
