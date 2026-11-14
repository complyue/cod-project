#!/usr/bin/env python3
"""Exact intrinsic handlers retain generic identities and completion contracts."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;\n'
TYPES = 'using "std:exceptions" MissingReturnException;using "std:exceptions" NullPointerException;using "std:exceptions" SystemException;\n'
ERRORS = TYPES + '''
func missing()->const i64{}
func missing_error()->const MissingReturnException{
  try{missing();}catch(e:const MissingReturnException){return e;};
}
func null_error()->const NullPointerException{
  ptr p:i64=null in here;
  try{p+0;}catch(e:const NullPointerException){return e;};
}
'''
WARN = '[-Wmissing-return]'
PROBE = 'func probe(e:T)->const u64 where T{try{throw e;}catch(x:const T){return x.category;};}\n'
runner.CASES = {
    'exact_generic_category': (IO + ERRORS + PROBE + 'behave @print(probe(missing_error()));behave @print(probe(null_error()));', 0, '5\n4\n', WARN),
    'reverse_instance_order': (IO + ERRORS + PROBE + 'behave @print(probe(null_error()));behave @print(probe(missing_error()));', 0, '4\n5\n', WARN),
    'typed_handler_value_dispatch': (IO + ERRORS + 'func pick(e:MissingReturnException)->const i64{return 1;}func pick(e:NullPointerException)->const i64{return 2;}func probe(e:T)->const i64 where T{try{throw e;}catch(x:const T){return pick(x);};}behave @print(probe(missing_error()));behave @print(probe(null_error()));', 0, '1\n2\n', WARN),
    'mismatch_falls_through': (IO + ERRORS + 'func probe(sample:T,e:U)->const i64 where {T,U}{try{throw e;}catch(x:const T){return 1;}catch(x){return 0;};}behave @print(probe(missing_error(),null_error()));behave @print(probe(null_error(),missing_error()));behave @print(probe(null_error(),null_error()));', 0, '0\n0\n1\n', WARN),
    'two_generic_handlers': (IO + ERRORS + 'func probe(a:T,b:U)->const i64 where {T,U}{try{throw b;}catch(x:const T){return 1;}catch(x:const U){return 2;};}behave @print(probe(missing_error(),null_error()));behave @print(probe(null_error(),missing_error()));', 0, '2\n2\n', WARN),
    'caught_value_return': (IO + ERRORS + 'func keep(e:T)->const T where T{try{throw e;}catch(x:const T){return x;};}behave @print(keep(missing_error()).category);behave @print(keep(null_error()).category);', 0, '5\n4\n', WARN),
    'recursive_handler': (IO + ERRORS + 'func probe(e:T,n:i64)->const u64 where T{if(n>0){return probe(e,n-1);}else{try{throw e;}catch(x:const T){return x.category;};};}behave @print(probe(missing_error(),30));behave @print(probe(null_error(),40));', 0, '5\n4\n', WARN),
    'retained_handler_type': (IO + ERRORS + 'func factory(sample:T)->func(SystemException)->const i64 where T{return func[](e:SystemException)->const i64{try{throw e;}catch(x:const T){return 1;}catch(x){return 0;};};}func a=factory(missing_error());func b=factory(null_error());val m:SystemException<-missing_error(),n:SystemException<-null_error();behave @print(a(m));behave @print(a(n));behave @print(b(m));behave @print(b(n));', 0, '1\n0\n0\n1\n', WARN),
    'retained_copy_and_replacement': (IO + ERRORS + 'func factory(sample:T)->func(SystemException)->const i64 where T{return func[](e:SystemException)->const i64{try{throw e;}catch(x:const T){return 1;}catch(x){return 0;};};}func a=factory(missing_error());func saved=a;a<-factory(null_error());val m:SystemException<-missing_error();behave @print(saved(m));behave @print(a(m));', 0, '1\n0\n', WARN),
    'deferred_recovery_return': (IO + ERRORS + 'func recover(e:T)->const i64 where T{defer catch(x:const T){return 42;}finally{};defer{throw e;};return 1;}behave @print(recover(missing_error()));behave @print(recover(null_error()));', 0, '42\n42\n', WARN),
    'deferred_recovery_preserves_return': (IO + ERRORS + 'func recover(e:T)->const i64 where T{defer catch(x:const T){}finally{};defer{throw e;};return 7;}behave @print(recover(missing_error()));behave @print(recover(null_error()));', 0, '7\n7\n', WARN),
    'caught_snapshot_in_defer': (IO + ERRORS + 'proc probe(e:T)->void where T{try{throw e;}catch(x:const T){defer{behave @print(x.category);};};}probe(missing_error());probe(null_error());', 0, '5\n4\n', WARN),
    'handler_initializes_storage': (IO + ERRORS + 'func probe(e:T)->const i64 where T{val result<-make local i64;try{throw e;}catch(x:const T){result<-9;{}};return result;}behave @print(probe(missing_error()));behave @print(probe(null_error()));', 0, '9\n9\n', WARN),
    'generic_rethrow_sibling_exclusion': (IO + ERRORS + 'func probe(e:T)->const i64 where T{try{try{throw e;}catch(x:const T){throw;}catch(x){return 99;};}catch(x:const T){return 8;};}behave @print(probe(missing_error()));behave @print(probe(null_error()));', 0, '8\n8\n', WARN),
    'generic_name_shadows_exception_alias': (IO + ERRORS + 'func probe(e:MissingReturnException)->const u64 where MissingReturnException{try{throw e;}catch(x:const MissingReturnException){return x.category;};}behave @print(probe(null_error()));', 0, '4\n', WARN),
    'duplicate_after_substitution': (ERRORS + 'func probe(a:T,b:U)->void where {T,U}{try{}catch(x:const T){}catch(x:const U){};}probe(missing_error(),missing_error());', 2, '', 'duplicate exception category'),
    'nonexception_substitution': ('func probe(x:T)->void where T{try{}catch(e:const T){};}probe(1);', 2, '', 'catch requires a selected intrinsic exception type'),
    'common_exception_requires_catch_all': (ERRORS + 'func probe(e:T)->void where T{try{}catch(x:const T){};}val common:SystemException<-missing_error();probe(common);', 2, '', 'use an untyped catch for SystemException'),
    'undetermined_handler_type': ('func probe()->void where T{try{}catch(x:const T){};}probe();', 2, '', 'type parameter is not determined'),
    'checked_rethrow_obligation': (ERRORS + 'func probe(e:T)->void where T{try{throw e;}catch(x:const T){throw;};}probe(null_error());', 2, '', 'requires a covering catch or throws declaration'),
    'mismatched_handler_does_not_initialize': (ERRORS + 'func probe(sample:T,e:U)->const i64 where {T,U}{val result<-make local i64;try{throw e;}catch(x:const T){result<-1;{}}catch(x){};return result;}probe(missing_error(),null_error());', 2, '', 'before definite initialization'),
    'caught_generic_value_is_readonly': (ERRORS + 'func probe(e:T)->void where T{try{throw e;}catch(x:const T){x<-e;};}probe(missing_error());', 2, '', 'readonly destination'),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
