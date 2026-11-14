#!/usr/bin/env python3
"""Context supplies callable signatures without materializing query operands."""
from pathlib import Path
import foundation_tests as runner

IO='using symbol "std:io" @print;'
ID='func identity(x:T)->const T where T{return x;}'
APPLY='func apply(f:func(T)->const T,x:T)->const T where T{return f(x);}'
NPE='using "std:exceptions" NullPointerException;'
def query(expression,value='7',prefix=ID+APPLY):
 return IO+prefix+'val x=make local typeof('+expression+'){'+value+'};behave @print(x);'
runner.CASES={
 'later_type':(query('apply(identity,1)'),0,'7\n',''),
 'boolean_context':(query('apply(identity,true)','false'),0,'false\n',''),
 'overloaded_name':(query('apply(pick,1)','8','func pick(x:i64)->const i64{return x;}func pick(x:bool)->const bool{return x;}'+APPLY),0,'8\n',''),
 'grouped_name':(query('apply({identity},1)'),0,'7\n',''),
 'scoped_name':(query('apply({{identity}},1)'),0,'7\n',''),
 'conditional_name':(query('apply(if(true){identity}else{identity},1)'),0,'7\n',''),
 'multiple_contextual_arguments':(query('twice(identity,identity,1)','9',ID+'func twice(f:func(T)->const T,g:func(T)->const T,x:T)->const T where T{return f(g(x));}'),0,'9\n',''),
 'earlier_argument_context':(query('reverse(1,identity)','10',ID+'func reverse(x:T,f:func(T)->const T)->const T where T{return f(x);}'),0,'10\n',''),
 'nested_queries':(query('apply(identity,apply(identity,1))','11'),0,'11\n',''),
 'local_callable_contract':(query('f(identity,1)','12',ID+'func apply(f:func(i64)->const i64,x:i64)->const i64{return f(x);}func f=apply;'),0,'12\n',''),
 'selected_callable_contract':(query('(if(true){f}else{g})(identity,1)','13',ID+'func apply(f:func(i64)->const i64,x:i64)->const i64{return f(x);}func f=apply;func g=apply;'),0,'13\n',''),
 'uninitialized_later_operand':(query('apply(identity,y)','14',ID+APPLY+'val y=make local i64;'),0,'14\n',''),
 'null_later_operand':(query('apply(identity,y)','15',ID+APPLY+'ptr y:i64=null;'),0,'15\n',''),
 'no_selected_body_demand':(query('apply(broken,true)','false','func broken(x:T)->const T where T{return x+x;}'+APPLY),0,'false\n',''),
 'no_recursive_demand':(query('apply(recursive,true)','false','func recursive(x:T)->const T where T{return recursive(x);}'+APPLY),0,'false\n',''),
 'outer_argument_evidence':(IO+ID+APPLY+'behave @print(apply(identity,{{val x=make local typeof(apply(identity,1)){16};x}}));',0,'16\n',''),
 'nested_runtime_argument_evidence':(IO+ID+APPLY+'behave @print(apply(identity,apply(identity,17)));behave @print(apply(identity,apply(identity,true)));',0,'17\ntrue\n',''),
 'anonymous_runtime_argument_evidence':(IO+ID+APPLY+'behave @print(apply(identity,apply(func(x:T)->const T where T{return x;},18)));',0,'18\n',''),
 'unselected_procedure_effects':(query('apply(identity,1)','19','proc identity(x:T)->const T where T{behave @print(99);return x;}proc apply(f:proc(T)->const T,x:T)->const T where T{return f(x);}'),0,'19\n',''),
 'borrowed_later_operand':(query('use(identity,y)','20',ID+'func use(f:func(i64)->const i64,ref x:i64 in O)->const i64 where {owner O}{return f(x);}val y=make local i64;'),0,'20\n',''),
 'added_checked_contract':(query('apply(identity,1)','21',NPE+ID+'func apply(f:func(T)->const T throws NullPointerException,x:T)->const T throws NullPointerException where T{return f(x);}'),0,'21\n',''),
 'matching_checked_contract':(query('apply(identity,1)','22',NPE+'func identity(x:T)->const T throws NullPointerException where T{return x;}func apply(f:func(T)->const T throws NullPointerException,x:T)->const T throws NullPointerException where T{return f(x);}'),0,'22\n',''),
 'checked_contract_cannot_be_erased':(query('apply(identity,1)','7',NPE+'func identity(x:T)->const T throws NullPointerException where T{return x;}'+APPLY),2,'','required checked exception'),
 'callable_kind_not_adapted':(query('apply(identity,1)','7','proc identity(x:T)->const T where T{return x;}'+APPLY),2,'','callable view contract'),
 'callable_result_not_adapted':(query('apply(wrong,1)','7','func wrong(x:T)->const bool where T{return true;}'+APPLY),2,'','callable view contract'),
 'ambiguous_callable_family':(query('apply(pick,1)','7','func pick(x:T,y:i64)->const i64 where T{return y;}func pick(x:i64,y:T)->const i64 where T{return x;}func apply(f:func(i64,i64)->const i64,x:i64)->const i64{return f(x,x);}'),2,'','ambiguous methods'),
 'unresolved_relation':(query('amb(identity,1,true)','7',ID+'func amb(f:func(T)->const T,x:T,y:T)->const T where T{return x;}'),2,'','known declaration or operand type'),
 'shadowed_name_remains_value':(query('apply(identity,1)','7',ID+APPLY+'val identity=1;'),2,'','no applicable method'),
 'query_does_not_initialize':(ID+APPLY+'val y=make local i64;val x=make local typeof(apply(identity,y)){1};y;',2,'','before definite initialization'),
 'runtime_initialization_still_reads':(IO+ID+APPLY+'val y=make local i64;val x=make local typeof(apply(identity,y)){apply(identity,y)};',2,'','before definite initialization'),
 'retained_generic_context':(IO+ID+APPLY+'func outer(x:T)->const T where T{func inner(y:T)->const T{val z=make local typeof(apply(identity,y)){y};return z;};return inner(x);}behave @print(outer(23));behave @print(outer(true));',0,'23\ntrue\n',''),
 'runtime_context_preserves_effect_order':(IO+ID+APPLY+'proc one()->const i64{behave @print(1);return 24;}behave @print(apply({behave @print(2);identity},apply(identity,one())));',0,'2\n1\n24\n',''),
 'runtime_context_preserves_throws':(NPE+'func identity(x:T)->const T throws NullPointerException where T{return x;}'+APPLY+'apply(identity,1);',2,'','required checked exception'),
 'ordinary_signature_requires_known_result':('func unknown(x:T)->const U where {T,U}{return x;}'+APPLY+'val x=make local typeof(apply(unknown,1));',2,'','type parameter is not determined'),
 'anonymous_query_boundary':(query('apply(func(x:T)->const T where T{return x;},1)'),2,'','typeof for this expression form'),
 'lexical_self_query_retains_context':(IO+APPLY+'func outer(x:T)->const T where T{func self(x:T)->const T{val y=make local typeof(apply(self,x)){x};return y;};return self(x);}behave @print(outer(25));behave @print(outer(true));',0,'25\ntrue\n',''),
}
runner.CASE_ARGUMENTS={}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
 raise SystemExit(runner.main())
