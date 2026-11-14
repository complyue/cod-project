#!/usr/bin/env python3
"""Checked argument preparation across method, callable and deferred invocation."""
from pathlib import Path
import foundation_tests as runner
from generic_catch_tests import IO, ERRORS, WARN

COMMON = 'func category(e:SystemException)->const u64{return e.category;}\n'
SELECT = 'func pick(e:SystemException)->const i64{return 1;}func pick(e:MissingReturnException)->const i64{return 2;}func pick(e:NullPointerException)->const i64{return 3;}\n'
MIXED = '''
func inc(x:i64)->const i64{return x+1;}
func wrap(f:func(i64)->const i64)->func(i64)->const i64{return func[f](x:i64)->const i64{return f(x);};}
func make_callback()->const func(i64)->const i64{return wrap(inc);}
func read_callback(f:func(i64)->const i64)->const i64{return f(10);}
func read_error(e:SystemException)->const i64{return 100;}
func apply(make_value:func()->const U,read:func(T)->const i64)->const i64 where {T,U}{return read(make_value());}
'''
runner.CASES = {
    'common_exception_callable': (IO+ERRORS+COMMON+'func f=category;behave @print(f(missing_error()));behave @print(f(null_error()));',0,'5\n4\n',WARN),
    'expression_target': (IO+ERRORS+COMMON+'behave @print((if(true){category}else{category})(missing_error()));',0,'5\n',WARN),
    'generic_callable_context': (IO+ERRORS+COMMON+'func apply(f:F,e:E)->const u64 where {F,E}{return f(e);}behave @print(apply(category,missing_error()));behave @print(apply(category,null_error()));',0,'5\n4\n',WARN),
    'generic_selected_method': (IO+ERRORS+SELECT+'func inspect(e:T)->const i64 where T{return pick(e);}func f:func(SystemException)->const i64=inspect;behave @print(f(missing_error()));behave @print(f(null_error()));behave @print(inspect(missing_error()));',0,'1\n1\n2\n',WARN),
    'direct_common_parameter': (IO+ERRORS+SELECT+'func inspect(e:SystemException)->const i64{return pick(e);}behave @print(inspect(missing_error()));behave @print(inspect(null_error()));',0,'1\n1\n',WARN),
    'direct_overload_retains_actual_type': (IO+ERRORS+SELECT+'behave @print(pick(missing_error()));behave @print(pick(null_error()));val e:SystemException=missing_error();behave @print(pick(e));',0,'2\n3\n1\n',WARN),
    'recursive_generic_call': (IO+ERRORS+COMMON+'func relay(f:F,e:E,n:i64)->const u64 where {F,E}{if(n>0){return relay(f,e,n-1);}else{return f(e);};}behave @print(relay(category,missing_error(),30));behave @print(relay(category,null_error(),40));',0,'5\n4\n',WARN),
    'retained_generic_handler': (IO+ERRORS+'func factory(sample:T)->func(SystemException)->const i64 where T{return func[](e:SystemException)->const i64{try{throw e;}catch(x:const T){return 1;}catch(x){return 0;};};}func a=factory(missing_error());func b=factory(null_error());behave @print(a(missing_error()));behave @print(a(null_error()));behave @print(b(missing_error()));behave @print(b(null_error()));',0,'1\n0\n0\n1\n',WARN),
    'captured_callable_argument': (IO+ERRORS+COMMON+'func wrap(f:func(SystemException)->const u64)->func(SystemException)->const u64{return func[f](e:SystemException)->const u64{return f(e);};}func f=wrap(category);behave @print(f(missing_error()));behave @print(f(null_error()));',0,'5\n4\n',WARN),
    'mixed_fresh_value_ownership': (IO+ERRORS+MIXED+'behave @print(apply(missing_error,read_error));behave @print(apply(make_callback,read_callback));',0,'100\n11\n',WARN),
    'mixed_fresh_value_reverse_order': (IO+ERRORS+MIXED+'behave @print(apply(make_callback,read_callback));behave @print(apply(missing_error,read_error));',0,'11\n100\n',WARN),
    'ordered_operands': (IO+ERRORS+'proc first()->const MissingReturnException{behave @print(1);return missing_error();}proc second()->const i64{behave @print(2);return 3;}proc observe(e:SystemException,n:i64)->void{behave @print(e.category);behave @print(n);}proc f=observe;f(first(),second());',0,'1\n2\n5\n3\n',WARN),
    'later_failure_prevents_invocation': (IO+ERRORS+'proc first()->const MissingReturnException{behave @print(1);return missing_error();}proc observe(e:SystemException,n:i64)->void{behave @print(99);}proc f=observe;try{f(first(),missing());}catch(e:const MissingReturnException){behave @print(2);};',0,'1\n2\n',WARN),
    'deferred_value_argument': (IO+ERRORS+'proc observe(e:SystemException)->void{behave @print(e.category);}proc f=observe;{{defer f(missing_error());defer f(null_error());}};',0,'4\n5\n',WARN),
    'deferred_direct_argument': (IO+ERRORS+'proc observe(e:SystemException)->void{behave @print(e.category);}{{defer observe(missing_error());defer observe(null_error());}};',0,'4\n5\n',WARN),
    'deferred_generic_selected_method': (IO+ERRORS+SELECT+'proc observe(e:T)->void where T{behave @print(pick(e));}proc f:proc(SystemException)->void=observe;{{defer f(missing_error());defer f(null_error());}};',0,'1\n1\n',WARN),
    'deferred_snapshot': (IO+ERRORS+'proc observe(e:SystemException)->void{behave @print(e.category);}proc f=observe;val e:SystemException=missing_error();{{defer f(e);e<-null_error();}};f(e);',0,'5\n4\n',WARN),
    'deferred_target_snapshot': (IO+ERRORS+'proc first(e:SystemException)->void{behave @print(e.category);}proc second(e:SystemException)->void{behave @print(99);}proc f=first;{{defer f({{f<-second;missing_error()}});}};f(null_error());',0,'5\n99\n',WARN),
    'null_target_prevents_operands': (IO+ERRORS+COMMON+'proc arg()->const MissingReturnException{behave @print(99);return missing_error();}ptr f:func(SystemException)->const u64=null;try{f(arg());{}}catch(e:const NullPointerException){behave @print(1);};',0,'1\n',WARN),
    'cause_payload_preserved': (IO+ERRORS+'func inspect(e:SystemException)->const u64{return cause_at(e,0u64).category;}func f=inspect;val e=try{throw missing_error() from null_error();}catch(x:const MissingReturnException){x};behave @print(f(e));behave @print(e.cause_count);',0,'4\n1\n',WARN),
    'common_to_exact_rejected': (ERRORS+'func inspect(e:MissingReturnException)->void{}func f=inspect;val e:SystemException=missing_error();f(e);',2,'','argument does not satisfy the callable view type'),
    'different_exact_category_rejected': (ERRORS+'func inspect(e:MissingReturnException)->void{}func f=inspect;f(null_error());',2,'','argument does not satisfy the callable view type'),
    'numeric_call_adaptation_remains_gap': ('func wide(x:f64)->const f64{return x;}func f=wide;f(1u8);',2,'','callable view argument adaptation'),
    'nested_callable_variance_rejected': (ERRORS+'func inspect(e:MissingReturnException)->void{}func use(f:func(SystemException)->void)->void{}func u=use;u(inspect);',2,'','no applicable method'),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__), Path(__file__).with_name('generic_catch_tests.py')]
if __name__ == '__main__':
    raise SystemExit(runner.main())
