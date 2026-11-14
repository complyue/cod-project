#!/usr/bin/env python3
"""Compile-time type queries share checked declarations without evaluating them."""
from pathlib import Path
import foundation_tests as runner

IO='using symbol "std:io" @print;'
NPE='using "std:exceptions" NullPointerException;'
EX='using "std:exceptions" SystemException;'
ID='func id(x:T)->const T where T{return x;}'
def storage(query, value, prefix=''):
    return IO+prefix+'val result=make local typeof('+query+'){'+value+'};behave @print(result);'
runner.CASES={
 'literal_i64':(storage('3','8'),0,'8\n',''),
 'literal_u8':(storage('3u8','8u8'),0,'8\n',''),
 'literal_u32':(storage('3u32','8u32'),0,'8\n',''),
 'literal_u64':(storage('3u64','8u64'),0,'8\n',''),
 'literal_f32':(storage('3f32','8f32'),0,'8\n',''),
 'literal_f64':(storage('3f64','8f64'),0,'8\n',''),
 'literal_bool':(storage('true','false'),0,'false\n',''),
 'minimum_literal':(storage('-9223372036854775808','8'),0,'8\n',''),
 'arithmetic_without_division':(storage('3/0+4','9'),0,'9\n',''),
 'numeric_widening':(storage('3u8+4u64','9u64'),0,'9\n',''),
 'intrinsic_selected':(storage('intrinsic (+)(3u64,4u64)','9u64'),0,'9\n',''),
 'logical':(storage('true&&!false','true'),0,'true\n',''),
 'conditional':(storage('if(true){1}else{2}','9'),0,'9\n',''),
 'uninitialized_operands':(storage('a+b','10','val a=make local i64,b=make local i64;'),0,'10\n',''),
 'null_operand':(storage('p+1','11','ptr p:i64=null;'),0,'11\n',''),
 'declared_generic_result':(storage('id(3)+4','12',ID),0,'12\n',''),
 'no_generic_body_demand':(storage('unproved(true)','false','func unproved(x:T)->const T where T{return x+x;}'),0,'false\n',''),
 'no_recursive_body_demand':(storage('unproved(true)','false','func unproved(x:T)->const T where T{return unproved(unproved(x));}'),0,'false\n',''),
 'procedure_not_invoked':(storage('noisy(1)','13','proc noisy(x:i64)->const i64{behave @print(99);return x;}'),0,'13\n',''),
 'function_can_query_procedure':(IO+'proc noisy(x:i64)->const i64{behave @print(99);return x;}func query()->const i64{val x=make local typeof(noisy(1)){14};return x;}behave @print(query());',0,'14\n',''),
 'initializer_runs_once':(IO+'proc next(ref x:i64 in O)->const i64 where {owner O}{x<-x+1;return x;}val count=0;val x=make local typeof(next(count)){next(count)};behave @print(x);behave @print(count);',0,'1\n1\n',''),
 'no_initialization_from_query':('func init(ref x:i64 in O)->const i64 where {owner O}{x<-1;return x;}val x=make local i64;val y=make local typeof(init(x)){0};x;',2,'','before definite initialization'),
 'known_callable_value':(storage('f(1)','15',ID+'func f:func(i64)->const i64=id;'),0,'15\n',''),
 'known_callable_name_argument':(storage('apply(one,1)','16','func one(x:i64)->const i64{return x;}func apply(f:func(i64)->const i64,x:i64)->const i64{return f(x);}'),0,'16\n',''),
 'uninitialized_exception_field':(storage('e.category','17u64',EX+'val e=make local SystemException;'),0,'17\n',''),
 'cause_field':(storage('cause_at(e,0u64).site','18u64',EX+'val e=make local SystemException;'),0,'18\n',''),
 'borrowed_result_isnull':(storage('isnull(get(p))','true','func get(ptr x:i64 in O)->ptr i64 in O where {owner O}{return x;}val x=1;ptr p=x;'),0,'true\n',''),
 'checked_failure_not_raised':(storage('read(p)','19',NPE+'func read(ptr x:i64 in O)->const i64 throws NullPointerException where {owner O}{return x;}val x=1;ptr p=x;p=null;'),0,'19\n',''),
 'overload_result':(storage('choose(true)','20u64','func choose(x:i64)->const i64{return x;}func choose(x:bool)->const u64{return 1u64;}'),0,'20\n',''),
 'generic_local_storage':(IO+'func query(x:T)->const T where T{val y=make local typeof(id(x)){x};return y;}'+ID+'behave @print(query(21));behave @print(query(true));',0,'21\ntrue\n',''),
 'later_operand_evidence':(IO+ID+'func apply(f:func(T)->const T,x:T)->const T where T{return f(x);}behave @print(apply(id,{{val x=make local typeof(1){22};x}}));',0,'22\n',''),
 'unknown_name':('val x=make local typeof(missing);',2,'','known declaration or operand type'),
 'unknown_callable':('val x=make local typeof(missing(1));',2,'','known callable signature'),
 'ambiguous_same_result':('func f(x:T,y:i64)->const i64 where T{return y;}func f(x:i64,y:T)->const i64 where T{return x;}val x=make local typeof(f(1,2));',2,'','ambiguous methods'),
 'wrong_same_t':(ID+'val x=make local typeof(id(true,1));',2,'','no applicable method'),
 'bad_logic':('val x=make local typeof(1&&2);',2,'','logical operation requires bool'),
 'bad_condition':('val x=make local typeof(if(1){2}else{3});',2,'','condition requires bool'),
 'incompatible_branches':('val x=make local typeof(if(true){1}else{false});',2,'','known declaration or operand type'),
 'bad_numeric':('val x=make local typeof(true+1);',2,'','no applicable numeric operation'),
 'integer_range':('val x=make local typeof(256u8);',2,'','integer literal is outside u8'),
 'real_range':('val x=make local typeof(1e99f32);',2,'','floating literal is outside f32'),
 'unknown_field':(EX+'val e=make local SystemException;val x=make local typeof(e.invalid);',2,'','unknown intrinsic exception field'),
 'invalid_isnull':('val x=make local typeof(isnull(1));',2,'','isnull requires one reference view'),
 'invalid_cause_index':(EX+'val e=make local SystemException;val x=make local typeof(cause_at(e,true));',2,'','cause_at requires an exception and a u64 index'),
 'shadowed_call':('func f(x:i64)->const i64{return x;}val f=1;val x=make local typeof(f(1));',2,'','nearest binding'),
 'callable_argument_type':(ID+'func f:func(i64)->const i64=id;val x=make local typeof(f(true));',2,'','argument does not satisfy the callable view type'),
 'borrow_needs_view':('func get(ref x:i64 in O)->const i64 where {owner O}{return x;}val x=make local typeof(get(1));',2,'','existing view'),
 'borrow_respects_access':('func get(ref x:i64 in O)->const i64 where {owner O}{return x;}const val x=1;val y=make local typeof(get(x));',2,'','readonly argument'),
 'borrow_respects_owner':('func get(ref x:i64 in O,ref y:i64 in O)->const i64 where {owner O}{return x;}val x=1;{{val y=2;val z=make local typeof(get(x,y));}};',2,'','different actual owners'),
 'body_inference_unavailable':('func f(x:i64){return x;}val x=make local typeof(f(1));',2,'','omitted callable result type'),
 'query_cannot_declare_runtime_storage':('val x=make local typeof({val y=1;y});',2,'','typeof for this expression form'),
 'outer_query_does_not_capture':('func id(x:i64)->const i64{return x;}proc run()->void{func f=id;func nested()->const i64{val x=make local typeof(f(1)){2};return x;};nested();}run();',0,'',''),
 'mixed_isnull_view':('val a=1;val b=make local typeof(isnull(if(true){a}else{1}));',2,'','isnull requires one reference view'),
 'mixed_borrowed_view':('func f(ref x:i64 in O)->const i64 where {owner O}{return x;}val a=1;val b=make local typeof(f(if(true){a}else{1}));',2,'','existing view'),
 'callable_arity':(ID+'func f:func(i64)->const i64=id;val x=make local typeof(f(1,2));',2,'','callable argument count'),
 'callable_access':('func f(ref x:i64 in O)->const i64 where {owner O}{return x;}func g=f;const val a=1;val x=make local typeof(g(a));',2,'','readonly argument'),
 'callable_owner':('func f(ref x:i64 in O,ref y:i64 in O)->const i64 where {owner O}{return x;}func g=f;val a=1;{{val b=2;val x=make local typeof(g(a,b));}};',2,'','different actual owners'),
 'lexical_recursive_query':(IO+'func run(x:T)->const T where T{func recurse(x:T,n:i64)->const T{val y=make local typeof(recurse(x,n-1)){x};if(n==0){return y;}else{return recurse(y,n-1);};};return recurse(x,4);}behave @print(run(23));behave @print(run(true));',0,'23\ntrue\n',''),
 'delayed_loop_type_only':(IO+'for(i in 0..2){defer{{val x=make local typeof(i){24};behave @print(x);}};};',0,'24\n24\n',''),
 'deferred_exception_type_only':(IO+'using "std:exceptions" MissingReturnException;func missing()->const i64{}try{missing();{}}catch(e:const MissingReturnException){defer{{val x=make local typeof(e.category){25u64};behave @print(x);}};};',0,'25\n','[-Wmissing-return]'),
 'initializer_still_captures':(IO+'func id(x:i64)->const i64{return x;}proc run()->void{func f=id;func nested[f]()->const i64{val x=make local typeof(1){f(26)};return x;};behave @print(nested());}run();',0,'26\n',''),
}
runner.CASE_ARGUMENTS={}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
