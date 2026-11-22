#!/usr/bin/env python3
"""Intrinsic exception values, checked boundaries and structured catch recovery."""
from pathlib import Path
import sys
import foundation_tests as runner

PRINT = 'using symbol "std:io" @print;\n'
TYPES = 'using "std:exceptions" MissingReturnError; using "std:exceptions" OutOfMemoryError; using "std:exceptions" Exception; using "std:exceptions" NullPointerException;\n'
ERROR = PRINT + TYPES + 'func missing()->const i64 {} func error()->const MissingReturnError {try{missing():void;}catch(e:const MissingReturnError){return e;};}\n'
WARN = '[-Wmissing-return]'
runner.CASES = {
    "try_destination": (PRINT + 'val x<-1; { try{x}finally{} } <-2;behave @print(x);', 0, "2\n", ""),
    "catch_result": (ERROR + 'behave @print(try{missing()}catch(e:const MissingReturnError){7}finally{behave @print(8);});', 0, "8\n7\n", WARN),
    "catch_fields": (ERROR + 'val e<-error();behave @print(e.category);behave @print(e.site>0u64);behave @print(e.requested);behave @print(e.available);behave @print(e.cause_count);', 0, "5\ntrue\n0\n0\n0\n", WARN),
    "catch_exact": (ERROR + 'val e<-error();try{throw e;}catch(x:const OutOfMemoryError){behave @print(99);}catch(x:const MissingReturnError){behave @print(1);}finally{behave @print(2);};', 0, "1\n2\n", WARN),
    "catch_all_rethrow": (ERROR + 'val e<-error();try{try{throw e;}catch(x){behave @print(x.category);throw;}}catch(x:const Exception){behave @print(x.category);};', 0, "5\n5\n", WARN),
    "catch_sibling_exclusion": (ERROR + 'val e<-error();try{try{throw e;}catch(x:const MissingReturnError){throw;}catch(x){behave @print(99);}}catch(x){behave @print(1);};', 0, "1\n", WARN),
    "catch_scope_cleanup": (ERROR + 'try{{defer{behave @print(1);};missing();{}}}catch(e){behave @print(2);}finally{behave @print(3);};', 0, "1\n2\n3\n", WARN),
    "catch_deferred_failure": (ERROR + 'try{{defer{missing();};}}catch(e:const MissingReturnError){behave @print(1);};', 0, "1\n", WARN),
    "exception_copy": (ERROR + 'val e<-error();val common:Exception<-e;val copy<-common;try{throw copy;}catch(x:const Exception){behave @print(x.category);};', 0, "5\n", WARN),
    "exception_store": (ERROR + 'val e<-error();val saved<-make local Exception;saved<-e;try{throw saved;}catch(x){behave @print(x.category);};', 0, "5\n", WARN),
    "exception_generic": (ERROR + 'func keep(x:T)->const T where T{return x;} val e<-keep(error());behave @print(e.category);behave @print(keep(3u64));', 0, "5\n3\n", WARN),
    "exception_common_parameter": (ERROR + 'func category(e:Exception)->const u64{return e.category;}func mixed(e:Exception,x:T)->const T where T{return x;}behave @print(category(error()));behave @print(mixed(error(),3u64));behave @print(mixed(error(),true));', 0, "5\n3\ntrue\n", WARN),
    "exception_overload": (ERROR + 'func pick(e:Exception)->const i64{return 1;}func pick(e:MissingReturnError)->const i64{return 2;}behave @print(pick(error()));val common:Exception<-error();behave @print(pick(common));', 0, "2\n1\n", WARN),
    "exception_overlap_relation": (ERROR + 'func pick(x:Exception,y:Exception)->const i64{return 1;}func pick(x:T,y:T)->const i64 where T{return 2;}pick(error(),error());', 2, "", "ambiguous methods"),
    "cause_name_reserved": (PRINT + 'func cause_at(x:u64,y:u64)->const u64{return x+y;}behave @print(cause_at(2u64,3u64));', 2, "", "reserved keyword 'cause_at'"),
    "exception_prepared_defer": (ERROR + 'proc observe(e:Exception)->void{behave @print(e.category);return;}{{defer observe(error());}};', 0, "5\n", WARN),
    "explicit_chain": (ERROR + 'val first<-error(),second<-error();val chained<-try{throw first from second;}catch(e:const MissingReturnError){e};behave @print(first.cause_count);behave @print(chained.cause_count);behave @print(cause_at(chained,0u64).category);behave @print(cause_at(chained,0u64).site==second.site);', 0, "0\n1\n5\ntrue\n", WARN),
    "retained_chain": (ERROR + 'func chain(e:MissingReturnError,cause:MissingReturnError)->const MissingReturnError {return try{throw e from cause;}catch(x:const MissingReturnError){x};}val e<-error();val saved<-chain(e,chain(e,e));behave @print(saved.cause_count);behave @print(cause_at(saved,1u64).category);', 0, "2\n5\n", WARN),
    "no_automatic_chain": (ERROR + 'val first<-error(),second<-error();try{try{throw first;}finally{throw second;}}catch(e){behave @print(e.cause_count);};', 0, "0\n", WARN),
    "chain_replace_copy": (ERROR + 'val e<-error();val first<-try{throw e from e;}catch(x:const MissingReturnError){x};val second<-try{throw first from e;}catch(x:const MissingReturnError){x};behave @print(first.cause_count);behave @print(second.cause_count);', 0, "1\n1\n", WARN),
    "catch_initialize_finally": (ERROR + 'val e<-error();val n<-make local i64;try{throw e;}catch(x){n<-3;{}}finally{behave @print(n);};behave @print(n);', 0, "3\n3\n", WARN),
    "catch_root_value": (ERROR + 'val e<-error();val kept<-try{throw e;}catch(x){x};try{throw kept;}catch(x:const Exception){behave @print(x.category);};', 0, "5\n", WARN),
    "catch_return_preserved": (ERROR + 'proc f()->const i64{try{return 3;}catch(x){return 99;}finally{behave @print(1);}}behave @print(f());', 0, "1\n3\n", WARN),
    "catch_return_finally_replace": (ERROR + 'proc f()->const i64{try{missing();}catch(x){return 1;}finally{return 2;}}behave @print(f());', 0, "2\n", WARN),
    "catch_declaration": (ERROR + 'val e<-error();try{throw e;}catch(x){val n<-3;};behave @print(n);', 0, "3\n", WARN),
    "catch_parameter_scope": (ERROR + 'val e<-error();try{throw e;}catch(x){};x;', 2, "", "unknown scalar value 'x'"),
    "catch_readonly": (ERROR + 'val e<-error();try{throw e;}catch(x){x<-e;};', 2, "", "readonly destination"),
    "catch_field_readonly": (ERROR + 'val e<-error();e.category<-1u64;', 2, "", "assignment requires a storage destination"),
    "catch_duplicate_alias": (TYPES + 'using "std:exceptions" MissingReturnError as Alias;try{}catch(e:const MissingReturnError){}catch(e:const Alias){};', 2, "", "duplicate exception category"),
    "catch_unknown_type": ('try{}catch(e:const MissingReturnError){};', 2, "", "selected intrinsic exception type"),
    "catch_result_mismatch": (TYPES + 'try{1}catch(e){true};', 2, "", "same result type"),
    "bare_throw_outside": ('throw;', 2, "", "bare throw requires an active catch"),
    "throw_nonexception": ('throw 1;', 2, "", "throw requires an intrinsic exception value"),
    "checked_throw": (TYPES + 'func f(e:NullPointerException)->void{throw e;}', 2, "", "requires a covering catch or throws declaration"),
    "checked_rethrow": (TYPES + 'func f(e:NullPointerException)->void{try{throw e;}catch(x:const NullPointerException){throw;};}', 2, "", "requires a covering catch or throws declaration"),
    "checked_declaration": (TYPES + 'func f(e:NullPointerException)->void throws NullPointerException{throw e;}', 0, "", ""),
    "checked_caller": (TYPES + 'func f(e:NullPointerException)->void throws NullPointerException{throw e;}func g(e:NullPointerException)->void{f(e);}', 2, "", "requires a covering catch or throws declaration"),
    "checked_caller_catch": (TYPES + 'func f(e:NullPointerException)->void throws NullPointerException{throw e;}func g(e:NullPointerException)->void{try{f(e);}catch(x:const NullPointerException){};}', 0, "", ""),
    "checked_finally_return": (TYPES + 'func f(e:NullPointerException)->const i64{try{throw e;}finally{return 3;}}', 0, "", ""),
    "voluntary_throws": (TYPES + 'func f()->void throws NullPointerException{}func g()->void{f();}', 2, "", "requires a covering catch or throws declaration"),
    "checked_defer_registration_scope": (TYPES + 'func f(e:NullPointerException)->void throws NullPointerException{throw e;}func g(e:NullPointerException)->void{try{defer f(e);}catch(x:const NullPointerException){};}', 2, "", 'coverage within the owning pass'),
    "checked_defer_declaration": (TYPES + 'func f(e:NullPointerException)->void throws NullPointerException{throw e;}func g(e:NullPointerException)->void throws NullPointerException{defer f(e);}', 2, "", "coverage within the owning pass"),
    "deferred_root_promise_caught_in_pass": (PRINT + TYPES + 'proc promised()->void throws Exception{behave @print(5u64);return;}proc run()->void{defer catch(failure){};defer promised();return;}run();', 0, "5\n", ""),
    "deferred_catch_capture": (ERROR + 'try{missing();{}}catch(e){defer{behave @print(e.category);};};', 0, "5\n", WARN),
}
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {
    'catch_fields': ['e'],
    'catch_exact': ['e'],
    'catch_all_rethrow': ['e'],
    'catch_sibling_exclusion': ['e'],
    'exception_copy': ['e'],
    'exception_store': ['e'],
    'exception_generic': ['e'],
    'retained_chain': ['e', 'e'],
    'chain_replace_copy': ['e'],
    'catch_initialize_finally': ['e'],
    'catch_narrow_common': ['e'],
    'catch_declaration': ['e'],
}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == "__main__":
    sys.exit(runner.main())
