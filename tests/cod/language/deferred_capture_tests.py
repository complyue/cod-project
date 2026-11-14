#!/usr/bin/env python3
"""Delayed blocks retain immutable caught bits across catch and queue reuse."""
from pathlib import Path
import sys
import foundation_tests as runner

ERROR = '''using symbol "std:io" @print;
using "std:exceptions" MissingReturnException;
using "std:exceptions" SystemException;
using "std:exceptions" NullPointerException;
func missing()->const i64 {}
func error()->const MissingReturnException {
  try{missing();}catch(e:const MissingReturnException){return e;};
}
func chain(e:MissingReturnException)->const MissingReturnException {
  return try{throw e from e;}catch(e:const MissingReturnException){e};
}
'''
LOOP = 'val saved<-error();for(i in 0..3){saved<-chain(saved);try{throw saved;}catch(e)'
WARN = '[-Wmissing-return]'
runner.CASES = {
    "caught_after_handler": (ERROR + 'try{throw error();}catch(e){defer{behave @print(e.category);};};behave @print(1);', 0, "1\n5\n", WARN),
    "repeated_catch": (ERROR + LOOP + '{defer{behave @print(e.cause_count);};};};', 0, "3\n2\n1\n", WARN),
    "repeated_scoped_catch": (ERROR + 'val saved<-error();for(i in 0..3){{saved<-chain(saved);try{throw saved;}catch(e){defer{behave @print(e.cause_count);};};}};', 0, "1\n2\n3\n", WARN),
    "late_local_and_snapshot": (ERROR + 'val n<-0;' + LOOP + '{defer{behave @print(e.cause_count);behave @print(n);};n<-n+1;};};', 0, "3\n3\n2\n3\n1\n3\n", WARN),
    "nested_registration": (ERROR + LOOP + '{defer{defer{behave @print(e.cause_count);};};};};', 0, "3\n2\n1\n", WARN),
    "nested_prepared_call": (ERROR + 'proc observe(e:SystemException)->void{behave @print(e.cause_count);}' + LOOP + '{defer{defer observe(e);};};};', 0, "3\n2\n1\n", WARN),
    "captured_bare_rethrow": (ERROR + 'try{{try{throw chain(error());}catch(e){defer{throw;};};}}catch(e){behave @print(e.cause_count);};', 0, "1\n", WARN),
    "nested_bare_rethrow": (ERROR + 'try{{try{throw chain(error());}catch(e){defer{defer{throw;};};};}}catch(e){behave @print(e.cause_count);};', 0, "1\n", WARN),
    "bare_rethrow_each_record": (ERROR + LOOP + '{defer{try{throw;}catch(x){behave @print(x.cause_count);};};};};', 0, "3\n2\n1\n", WARN),
    "named_rethrow_each_record": (ERROR + LOOP + '{defer{try{throw e;}catch(x){behave @print(x.cause_count);};};};};', 0, "3\n2\n1\n", WARN),
    "two_captured_catches": (ERROR + 'try{throw chain(error());}catch(a){try{throw chain(chain(error()));}catch(b){defer{behave @print(a.cause_count);behave @print(b.cause_count);};};};', 0, "1\n2\n", WARN),
    "shadowed_catch": (ERROR + 'try{throw chain(error());}catch(e){try{throw chain(chain(error()));}catch(e){defer{behave @print(e.cause_count);};};defer{behave @print(e.cause_count);};};', 0, "1\n2\n", WARN),
    "catch_inside_delayed_action": (ERROR + LOOP + '{defer{try{throw e;}catch(x){defer{behave @print(x.cause_count);};};};};};', 0, "3\n2\n1\n", WARN),
    "active_bare_throw_binding": (ERROR + 'try{throw error();}catch(outer:const MissingReturnException){defer{try{throw chain(outer);}catch(inner){defer{try{throw;}catch(e){behave @print(e.cause_count);};};};};};', 0, "1\n", WARN),
    "finally_snapshot": (ERROR + LOOP + '{defer{behave @print(e.cause_count);}finally{behave @print(e.cause_count);};};};', 0, "3\n3\n2\n2\n1\n1\n", WARN),
    "bodyless_finally_snapshot": (ERROR + LOOP + '{defer finally{behave @print(e.cause_count);};};};', 0, "3\n2\n1\n", WARN),
    "captured_replacement_return": (ERROR + 'proc f()->const MissingReturnException{try{throw chain(error());}catch(e:const MissingReturnException){defer{return e;};};return error();}behave @print(f().cause_count);', 0, "1\n", WARN),
    "recursive_frame_snapshots": (ERROR + 'proc f(n:i64,e:MissingReturnException)->void{try{throw e;}catch(x){defer{behave @print(x.cause_count);};};if(n>0){f(n-1,chain(e));}else{};}f(2,error());', 0, "2\n1\n0\n", WARN),
    "generic_capture": (ERROR + 'proc f(x:T)->const T where T{try{throw error();}catch(e){defer{behave @print(e.category);behave @print(x);};};return x;}behave @print(f(2u64));behave @print(f(true));', 0, "5\n2\n2\n5\ntrue\ntrue\n", WARN),
    "captured_chain_survives_handler": (ERROR + 'try{throw chain(chain(error()));}catch(e){defer{behave @print(cause_at(e,0u64).cause_count);behave @print(e.cause_count);};};', 0, "1\n2\n", WARN),
    "caught_snapshot_readonly": (ERROR + 'try{throw error();}catch(e){defer{e<-error();};};', 2, "", "readonly destination"),
    "captured_checked_throw": (ERROR + 'func f(e:NullPointerException)->void{try{throw e;}catch(x){defer{throw;};};}', 2, "", "requires a covering catch or throws declaration"),
    "captured_checked_declared": (ERROR + 'func f(e:NullPointerException)->void throws NullPointerException{try{throw e;}catch(x){defer{throw;};};}', 0, "", WARN),
    "captured_checked_local_catch": (ERROR + 'func f(e:NullPointerException)->void{try{throw e;}catch(x){defer{try{throw;}catch(y:const NullPointerException){};};};}', 0, "", WARN),
    "deferred_empty_handler": (ERROR + 'defer catch(e){};', 0, "", WARN),
    "throw_location": (ERROR + 'func f(e:NullPointerException)->void {\n\n  throw e;\n}', 2, "", "throw_location.cod:14:3: checked exception"),
    "forwarded_throw_location": (ERROR + 'func f(e:NullPointerException)->void {\n  try{\n    throw e;\n  }finally{};\n}', 2, "", "forwarded_throw_location.cod:14:5: checked exception"),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == "__main__":
    sys.exit(runner.main())
