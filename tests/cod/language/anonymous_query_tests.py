#!/usr/bin/env python3
"""Anonymous declaration queries preserve signature checks without constructing values."""
from pathlib import Path
import foundation_tests as runner

IO='using symbol "std:io" @print;'
APPLY='func apply(f:func(T)->const T,x:T)->const T where T{return f(x);}'
NPE='using "std:exceptions" NullPointerException;'
IDENTITY='func(x:T)->const T where T{return x;}'
def query(expression,value='7',prefix=APPLY):
 return IO+prefix+'val x=make local typeof('+expression+'){'+value+'};behave @print(x);'

runner.CASES={
 'generic_context':(query('apply('+IDENTITY+',1)'),0,'7\n',''),
 'concrete_context':(query('apply(func(x:i64)->const i64{return x;},1)'),0,'7\n',''),
 'boolean_context':(query('apply('+IDENTITY+',true)','false'),0,'false\n',''),
 'grouped_context':(query('apply({'+IDENTITY+'},1)'),0,'7\n',''),
 'scoped_context':(query('apply({{'+IDENTITY+'}},1)'),0,'7\n',''),
 'conditional_context':(query('apply(if(true){'+IDENTITY+'}else{'+IDENTITY+'},1)'),0,'7\n',''),
 'nested_context':(query('apply('+IDENTITY+',apply('+IDENTITY+',1))'),0,'7\n',''),
 'earlier_argument_context':(query('reverse(1,'+IDENTITY+')','8','func reverse(x:T,f:func(T)->const T)->const T where T{return f(x);}'),0,'8\n',''),
 'declared_view_context':(query('f('+IDENTITY+',1)','9',APPLY+'func f:func(func(i64)->const i64,i64)->const i64=apply;'),0,'9\n',''),
 'concrete_value_call':(query('(func(x:i64)->const i64{return x/0;})(1)','10',''),0,'10\n',''),
 'conditional_value_call':(query('(if(true){func(x:i64)->const i64{return x;}}else{func(x:i64)->const i64{return x/0;}})(1)','11',''),0,'11\n',''),
 'procedure_body_not_executed':(query('(proc()->const i64{behave @print(99);return 1/0;})()','12',''),0,'12\n',''),
 'generic_body_not_demanded':(query('apply(func(x:T)->const T where T{return x+x;},true)','false'),0,'false\n',''),
 'concrete_body_not_demanded':(query('apply(func(x:i64)->const i64{return true;},1)'),0,'7\n',''),
 'query_body_does_not_read':(query('apply(func(x:i64)->const i64{return x+y;},1)','13',APPLY+'val y=make local i64;'),0,'13\n',''),
 'query_body_does_not_initialize':('proc apply(f:proc(i64)->const i64,x:i64)->const i64{return f(x);}val y=make local i64;val x=make local typeof(apply(proc(x:i64)->const i64{y<-x;return x;},1)){1};y;',2,'','before definite initialization'),
 'explicit_callable_capture_not_constructed':(query('apply(func [f](x:i64)->const i64{return f(x);},1)','14',APPLY+'func id(x:i64)->const i64{return x;}func f=id;'),0,'14\n',''),
 'inferred_scalar_capture_not_constructed':(query('apply(func(x:i64)->const i64{return x+y;},1)','15',APPLY+'val y=3;'),0,'15\n',''),
 'explicit_scalar_capture_not_constructed':(query('apply(func [y](x:i64)->const i64{return x+y;},1)','16',APPLY+'val y=3;'),0,'16\n',''),
 'initializer_executes_once':(query('apply('+IDENTITY+',1)','next()','proc next()->const i64{behave @print(1);return 17;}'+APPLY),0,'1\n17\n',''),
 'query_body_capture_not_inferred_by_enclosing_callable':(IO+APPLY+'func outer()->const i64{val y=3;func inner()->const i64{val z=make local typeof(apply(func(x:i64)->const i64{return x+y;},1)){18};return z;};return inner();}behave @print(outer());',0,'18\n',''),
 'retained_type_parameters':(IO+APPLY+'func outer(x:T)->const T where T{func inner(y:T)->const T{val z=make local typeof(apply(func(z:T)->const T{return z;},y)){y};return z;};return inner(x);}behave @print(outer(19));behave @print(outer(true));',0,'19\ntrue\n',''),
 'checked_throws_not_executed':(query('(func()->const i64 throws NullPointerException{throw NullPointerException;})()','20',NPE),0,'20\n',''),
 'throws_view_adaptation':(query('apply('+IDENTITY+',1)','21',NPE+'func apply(f:func(T)->const T throws NullPointerException,x:T)->const T throws NullPointerException where T{return f(x);}'),0,'21\n',''),
 'checked_throws_cannot_be_erased':(query('apply(func(x:T)->const T throws NullPointerException where T{return x;},1)','7',NPE+APPLY),2,'','required checked exception'),
 'kind_not_adapted':(query('apply(proc(x:T)->const T where T{return x;},1)'),2,'','callable view contract'),
 'result_not_adapted':(query('apply(func(x:T)->const bool where T{return true;},1)'),2,'','callable view contract'),
 'argument_count_checked':(query('(func(x:i64)->const i64{return x;})()'),2,'','argument count'),
 'argument_type_checked':(query('(func(x:i64)->const i64{return x;})(true)'),2,'','callable view type'),
 'readonly_argument_checked':(query('(func(ref x:i64 in O)->const i64 where {owner O}{return x;})(y)','7','const val y=1;'),2,'','readonly argument'),
 'fresh_borrow_rejected':(query('(func(ref x:i64 in O)->const i64 where {owner O}{return x;})(1)','7',''),2,'','existing view'),
 'uninitialized_borrow_not_read':(query('(func(ref x:i64 in O)->const i64 where {owner O}{return x;})(y)','22','val y=make local i64;'),0,'22\n',''),
 'same_owner_checked':('val a=1;{{val b=1;val z=make local typeof((func(ref x:i64 in O,ref y:i64 in O)->const i64 where {owner O}{return x;})(a,b)){7};}};',2,'','different actual owners'),
 'same_owner_preserved':(query('(func(ref x:i64 in O,ref y:i64 in O)->const i64 where {owner O}{return x;})(a,b)','23','val a=1;val b=2;'),0,'23\n',''),
 'declared_borrowed_result':(query('(func(ref x:i64 in O)->ref i64 in O where {owner O}{return x;})(a)','24','val a=make local i64;'),0,'24\n',''),
 'generic_target_needs_context':(query('('+IDENTITY+')(1)'),2,'','known declaration or operand type'),
 'default_void_not_inferred_from_body':(query('apply(func(x:i64){return x;},1)'),2,'','callable view contract'),
 'named_declaration_remains_statement':(query('{func inner(x:i64)->const i64{return x;};inner(1)}'),2,'','typeof for this expression form'),
 'runtime_body_still_checked':(IO+APPLY+'behave @print(apply(func(x:i64)->const i64{return true;},1));',2,'','return type'),
 'runtime_capture_still_checked':(IO+APPLY+'val y=3;behave @print(apply(func(x:i64)->const i64{return x+y;},1));',2,'','ordinary lexical cells'),
}
runner.CASE_ARGUMENTS={}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
 raise SystemExit(runner.main())
