#!/usr/bin/env python3
"""Structured returns, finally and delayed scalar blocks share cleanup control."""
from pathlib import Path
import sys
import foundation_tests as runner

PRINT = 'using symbol "std:io" @print;\n'
OBSERVE = PRINT + 'proc observe(x:i64)->void {behave @print(x);}\n'
BAD = OBSERVE + 'func bad()->const i64 {}\n'
runner.CASES = {
    "finally_value": (OBSERVE + 'behave @print(try{4}finally{observe(7);});', 0, "7\n4\n", ""),
    "finally_return": (OBSERVE + 'proc f()->const i64{defer observe(1);try{return 3;}finally{observe(2);};} behave @print(f());', 0, "2\n1\n3\n", ""),
    "finally_nested_scope": (OBSERVE + 'proc f()->const i64{defer observe(1);try{{defer observe(2);return 6;}}finally{{defer observe(3);observe(4);}}} behave @print(f());', 0, "2\n4\n3\n1\n6\n", ""),
    "finally_replace_return": (OBSERVE + 'proc f()->const i64{defer observe(1);try{return 3;}finally{{defer observe(2);return 4;}}} behave @print(f());', 0, "2\n1\n4\n", ""),
    "finally_replace_failure": (BAD + 'proc f()->const i64{defer observe(1);try{bad();}finally{return 8;}} behave @print(f());', 0, "1\n8\n", "[-Wmissing-return]"),
    "finally_failure_cleanup": (BAD + 'proc f()->const i64{defer observe(1);try{return 3;}finally{bad();{}}} f();', 70, "1\n", "uncaught MissingReturnException"),
    "finally_nested_return": (OBSERVE + 'proc f()->const i64{try{try{return 1;}finally{return 2;}}finally{observe(3);}} behave @print(f());', 0, "3\n2\n", ""),
    "generic_completion": (PRINT + 'proc f(x:T)->const T where T {defer{behave @print(x);}finally{};try{return x;}finally{x<-x+x;{}}} behave @print(f(3u32));behave @print(f(1.5f32));', 0, "6\n3\n3\n1.5\n", ""),
    "generic_replacement": (PRINT + 'func f(x:T)->const T where T {try{return x;}finally{return x+x;}} behave @print(f(3u64));behave @print(f(1.5f32));', 0, "6\n3\n", ""),
    "finally_registration": (OBSERVE + '{{defer observe(1);try{defer observe(2);observe(3);}finally{defer observe(4);observe(5);};observe(6);}};', 0, "3\n5\n6\n4\n2\n1\n", ""),
    "deferred_late_read": (OBSERVE + '{{val n=1;defer observe(n);defer{observe(n);};n<-2;}};', 0, "2\n1\n", ""),
    "deferred_return_snapshot": (OBSERVE + 'proc f()->const i64{val n=1;defer{observe(n);};defer{n<-2;};return n;} behave @print(f());', 0, "2\n1\n", ""),
    "deferred_return": (OBSERVE + 'proc f()->const i64{defer observe(1);defer{return 2;};return 3;} behave @print(f());', 0, "1\n2\n", ""),
    "deferred_fallthrough_return": (OBSERVE + 'proc f()->const i64{defer{return 2;};} behave @print(f());', 0, "2\n", ""),
    "deferred_failure_return": (BAD + 'proc f()->const i64{defer observe(1);defer{return 2;};return bad();} behave @print(f());', 0, "1\n2\n", "[-Wmissing-return]"),
    "deferred_plain_failure": (BAD + '{{defer{observe(1);};bad();}};', 70, "1\n", "uncaught MissingReturnException"),
    "deferred_nested_registration": (OBSERVE + '{{defer{observe(1);};defer{defer{observe(2);};observe(3);};}};', 0, "3\n2\n1\n", ""),
    "deferred_nested_finally": (OBSERVE + 'proc f()->const i64{defer observe(1);defer{try{{defer observe(2);return 3;}}finally{observe(4);}};return 5;} behave @print(f());', 0, "2\n4\n1\n3\n", ""),
    "deferred_attached_finally": (OBSERVE + '{{defer observe(1);defer{observe(2);}finally{observe(3);};}};', 0, "2\n3\n1\n", ""),
    "deferred_finally_return": (OBSERVE + 'proc f()->const i64{defer observe(1);defer{return 2;}finally{return 3;};return 4;} behave @print(f());', 0, "1\n3\n", ""),
    "deferred_bodyless_finally": (OBSERVE + '{{defer finally{observe(1);};}};', 0, "1\n", ""),
    "deferred_finally_failure": (BAD + '{{defer observe(1);defer{bad();}finally{observe(2);};}};', 70, "2\n1\n", "uncaught MissingReturnException"),
    "deferred_finally_pending_failure": (BAD + '{{defer{observe(99);}finally{observe(1);};bad();}};', 70, "1\n", "uncaught MissingReturnException"),
    "deferred_finally_pending_return": (OBSERVE + 'proc f()->const i64{defer{observe(1);}finally{observe(2);};return 3;} behave @print(f());', 0, "1\n2\n3\n", ""),
    "deferred_finally_skipped_init": (OBSERVE + '{{val n=make local i64;defer{n<-1;}finally{observe(n);};}};', 2, "", "deferred read requires initialization proved before registration"),
    "deferred_own_loop": (OBSERVE + '{{defer{for(i in 0..3){observe(i);};};}};', 0, "0\n1\n2\n", ""),
    "deferred_loop_mutation": (OBSERVE + '{{val n=0;for(i in 0..3){defer{n<-n+1;observe(n);};};}};', 0, "1\n2\n3\n", ""),
    "deferred_local_scope": (OBSERVE + '{{defer{{val n=4;defer observe(n);observe(n+1);}};}};', 0, "5\n4\n", ""),
    "finally_initialize": (OBSERVE + 'val n=make local i64;try{n<-1;}finally{observe(n);};observe(n);', 0, "1\n1\n", ""),
    "finally_write": (OBSERVE + 'val n=make local i64;try{}finally{n<-1;{}};observe(n);', 0, "1\n", ""),
    "finally_call_before_init": (BAD + 'val n=make local i64;try{bad();n<-1;}finally{observe(n);};', 2, "", "before definite initialization"),
    "finally_return_before_init": ('func f(flag:bool)->const i64{val n=make local i64;try{if(flag){return 1;}else{};n<-2;}finally{n;{}};return 3;} f(true);', 2, "", "before definite initialization"),
    "finally_projection": (OBSERVE + 'val n=make local i64;try{{val local=3;n<-local;}}finally{observe(n);};', 0, "3\n", ""),
    "finally_nonvoid": ('try{}finally{1};', 2, "", "finally must complete with void"),
    "deferred_index_capture": (OBSERVE + 'for(i in 0..3){defer{observe(i);};};', 2, "", "cannot capture loop-control"),
    "deferred_accumulator_capture": (OBSERVE + 'fold(i in 0..3,n=0){defer{observe(n);};n+i};', 2, "", "cannot capture loop-control"),
    "deferred_capture_inside_scope": (OBSERVE + 'for(i in 0..3){{defer{observe(i);};}};', 2, "", "cannot capture loop-control"),
    "deferred_later_initialization": (OBSERVE + '{{val n=make local i64;defer{observe(n);};n<-1;}};', 2, "", "deferred read requires initialization proved before registration"),
    "deferred_write_not_immediate": (OBSERVE + 'val n=make local i64;defer{n<-1;};observe(n);', 2, "", "before definite initialization"),
    "deferred_direct_declaration": ('defer{val n=1;};', 2, "", "deferred-block variable declaration"),
    "deferred_function_effect": (OBSERVE + 'func f()->void{defer{observe(1);};} f();', 2, "", "function cannot invoke a procedure"),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == "__main__":
    sys.exit(runner.main())
