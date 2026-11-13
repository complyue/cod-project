#!/usr/bin/env python3
"""Scope-owned queues preserve completion identity through deferred recovery."""
from pathlib import Path
import sys
import foundation_tests as runner

PRELUDE = '''using symbol "std:io" @print;
using "std:exceptions" MissingReturnException;
using "std:exceptions" OutOfMemory;
using "std:exceptions" NullPointerException;
func missing()->const i64 {}
func error()->const MissingReturnException {try{missing();}catch(e:const MissingReturnException){return e;};}
proc observe(x:i64)->void{behave @print(x);}
'''
WARN = '[-Wmissing-return]'
runner.CASES = {
    "pending_return": (PRELUDE + 'proc f()->const i64{defer observe(1);defer catch(e:const MissingReturnException){observe(2);};defer missing();return 7;}observe(f());', 0, '2\n1\n7\n', WARN),
    "skip_body": (PRELUDE + '{{defer{observe(99);}catch(e:const MissingReturnException){observe(1);}finally{observe(2);};missing();{}}};observe(3);', 0, '1\n2\n3\n', WARN),
    "own_body_failure": (PRELUDE + '{{defer{observe(1);missing();}catch(e:const MissingReturnException){observe(2);}finally{observe(3);};}};observe(4);', 0, '1\n2\n3\n4\n', WARN),
    "body_normal": (PRELUDE + '{{defer{observe(1);}catch(e){observe(99);}finally{observe(2);};}};', 0, '1\n2\n', WARN),
    "recover_original_value": (PRELUDE + 'observe({{defer catch(e:const MissingReturnException){};defer missing();3}});', 0, '3\n', WARN),
    "value_before_any_failure": (PRELUDE + 'observe({{defer catch(e){};3}});', 0, '3\n', WARN),
    "missing_scope_value": (PRELUDE + 'observe({{defer catch(e){};defer missing();3}});', 2, '', 'scope recovery has no original value'),
    "void_body_recovery": (PRELUDE + 'proc f()->void{defer catch(e:const MissingReturnException){observe(1);};missing();observe(99);}f();observe(2);', 0, '1\n2\n', WARN),
    "explicit_handler_return": (PRELUDE + 'proc f()->const i64{defer observe(1);defer catch(e:const MissingReturnException){return 8;};missing();}observe(f());', 0, '1\n8\n', WARN),
    "handler_value_is_not_return": (PRELUDE + 'proc f()->const i64{defer catch(e:const MissingReturnException){8};missing();}f();', 70, '', 'uncaught MissingReturnException'),
    "handler_replaces_return": (PRELUDE + 'proc f()->const i64{defer catch(e:const MissingReturnException){return 8;};defer missing();return 3;}observe(f());', 0, '8\n', WARN),
    "finally_replaces_handler": (PRELUDE + 'proc f()->const i64{defer catch(e:const MissingReturnException){return 8;}finally{return 9;};missing();}observe(f());', 0, '9\n', WARN),
    "return_through_finally_failure": (PRELUDE + 'proc f()->const i64{defer catch(e:const MissingReturnException){};try{return 6;}finally{missing();{}}}observe(f());', 0, '6\n', WARN),
    "deferred_return_then_failure": (PRELUDE + 'proc f()->const i64{defer catch(e:const MissingReturnException){};defer{try{return 6;}finally{missing();{}}};return 2;}observe(f());', 0, '6\n', WARN),
    "repeated_replacements": (PRELUDE + 'proc f()->const i64{defer catch(e:const MissingReturnException){};defer missing();defer{return 7;};defer missing();return 2;}observe(f());', 0, '7\n', WARN),
    "ordinary_catch_cancels_return": (PRELUDE + 'proc f()->const i64{defer catch(e:const MissingReturnException){};try{{defer missing();return 1;}}catch(e:const MissingReturnException){observe(8);};return 2;}observe(f());', 0, '8\n2\n', WARN),
    "nested_registration": (PRELUDE + 'proc f()->const i64{defer catch(e:const MissingReturnException){observe(1);};defer catch(e:const MissingReturnException){defer missing();observe(2);};missing();}try{f();{}}catch(e:const MissingReturnException){observe(3);};', 0, '2\n1\n3\n', WARN),
    "nested_handler_snapshot": (PRELUDE + '{{defer catch(e:const MissingReturnException){defer{behave @print(e.category);};};missing();{}}};', 0, '5\n', WARN),
    "explicit_chain": (PRELUDE + 'val original=error();{{defer catch(e:const MissingReturnException){behave @print(e.cause_count);};defer catch(e:const MissingReturnException){throw original from e;};throw original;}};', 0, '1\n', WARN),
    "bare_rethrow": (PRELUDE + '{{defer catch(e:const MissingReturnException){observe(1);};defer catch(e){throw;};missing();{}}};', 0, '1\n', WARN),
    "named_rethrow": (PRELUDE + '{{defer catch(e:const MissingReturnException){observe(1);};defer catch(e){throw e;};missing();{}}};', 0, '1\n', WARN),
    "sibling_exclusion": (PRELUDE + '{{defer catch(e:const MissingReturnException){observe(1);};defer catch(e:const MissingReturnException){throw;}catch(e){observe(99);};missing();{}}};', 0, '1\n', WARN),
    "conditional_registration": (PRELUDE + 'proc f(flag:bool)->void{if(flag){defer catch(e:const MissingReturnException){observe(1);};}else{};missing();}f(true);try{f(false);}catch(e:const MissingReturnException){observe(2);};', 0, '1\n2\n', WARN),
    "loop_handlers": (PRELUDE + '{{for(i in 0..3){defer{observe(2);}catch(e:const MissingReturnException){observe(1);};};missing();{}}};', 0, '1\n2\n2\n', WARN),
    "loop_zero": (PRELUDE + 'try{{for(i in 0..0){defer catch(e:const MissingReturnException){observe(99);};};missing();{}}}catch(e:const MissingReturnException){observe(1);};', 0, '1\n', WARN),
    "loop_explicit_scope": (PRELUDE + 'for(i in 0..3){{defer catch(e:const MissingReturnException){observe(1);};missing();{}}};observe(2);', 0, '1\n1\n1\n2\n', WARN),
    "recursive_recovery": (PRELUDE + 'proc f(n:i64)->const i64{defer catch(e:const MissingReturnException){return n;};if(n==0){missing();}else{return f(n-1)+1;}}observe(f(3));', 0, '3\n', WARN),
    "generic_recovery": (PRELUDE + 'func f(x:T)->const T where T{defer catch(e:const MissingReturnException){};defer missing();return x;}behave @print(f(2u64));behave @print(f(true));', 0, '2\ntrue\n', WARN),
    "terminal_failure_not_caught": (PRELUDE + '{{defer catch(e){observe(99);}finally{observe(1);};3/0;{}}};', 70, '1\n', 'division by zero'),
    "normal_after_body_failure": (PRELUDE + 'val n=0;{{defer catch(e:const MissingReturnException){};n<-2;missing();{}}};observe(n);', 0, '2\n', WARN),
    "uninitialized_after_recovery": (PRELUDE + 'val n=make local i64;{{defer catch(e:const MissingReturnException){};missing();n<-2;{}}};observe(n);', 2, '', 'before definite initialization'),
    "initialized_before_failure": (PRELUDE + 'val n=make local i64;{{defer catch(e:const MissingReturnException){};n<-2;missing();{}}};observe(n);', 0, '2\n', WARN),
    "checked_body_covered": (PRELUDE + 'func f(e:NullPointerException)->void{defer catch(x:const NullPointerException){};throw e;}', 0, '', WARN),
    "checked_call_covered": (PRELUDE + 'func bad(e:NullPointerException)->void throws NullPointerException{throw e;}func f(e:NullPointerException)->void{defer catch(x:const NullPointerException){};defer bad(e);}', 0, '', WARN),
    "checked_block_covered": (PRELUDE + 'func f(e:NullPointerException)->void{defer catch(x:const NullPointerException){};defer{throw e;};}', 0, '', WARN),
    "checked_wrong_order": (PRELUDE + 'func f(e:NullPointerException)->void{defer{throw e;};defer catch(x:const NullPointerException){};}', 2, '', 'requires a covering catch or throws declaration'),
    "checked_conditional_gap": (PRELUDE + 'func f(e:NullPointerException,flag:bool)->void{if(flag){defer catch(x:const NullPointerException){};}else{};throw e;}', 2, '', 'requires a covering catch or throws declaration'),
    "checked_after_return_unreachable": (PRELUDE + 'func f(e:NullPointerException)->void{return;throw e;}', 0, '', WARN),
    "checked_registration_catch_wrong_scope": (PRELUDE + 'func f(e:NullPointerException)->void{try{defer{throw e;};}catch(x:const NullPointerException){};}', 2, '', 'requires a covering catch or throws declaration'),
    "checked_full_scope_catch": (PRELUDE + 'func f(e:NullPointerException)->void{try{{defer{throw e;};}}catch(x:const NullPointerException){};}', 0, '', WARN),
    "checked_one_iteration": (PRELUDE + 'func f(e:NullPointerException)->void{for(i in 0..1){defer catch(x:const NullPointerException){};};throw e;}', 0, '', WARN),
    "checked_zero_iterations": (PRELUDE + 'func f(e:NullPointerException)->void{for(i in 0..0){defer catch(x:const NullPointerException){};};throw e;}', 2, '', 'requires a covering catch or throws declaration'),
    "checked_loop_prefix": (PRELUDE + 'func f(e:NullPointerException)->void{defer catch(x:const NullPointerException){};for(i in 0..3){defer{throw e;};};}', 0, '', WARN),
    "checked_finally_does_not_recover": (PRELUDE + 'func f(e:NullPointerException)->void{defer{}finally{};throw e;}', 2, '', 'requires a covering catch or throws declaration'),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    sys.exit(runner.main())
