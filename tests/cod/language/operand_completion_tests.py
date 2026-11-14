#!/usr/bin/env python3
"""Abrupt operands retain their completion instead of manufacturing a payload."""
from pathlib import Path
import foundation_tests as runner

IO='using symbol "std:io" @print;\n'
OBS=IO+'proc observe(x:i64)->void{behave @print(x);}\n'
EX='using "std:exceptions" MissingReturnException;func missing()->const i64{}func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}\n'
VIEW='func first(x:i64,y:i64)->const i64{return x;}\n'
CONTEXT=IO+'using "std:exceptions" NullPointerException;func pick(tag:bool,ptr x:i64 in O)->ptr i64 in O where {owner O}{return x;}func pick(tag:i64,ptr x:i64 in O)->const i64 where {owner O}{return 7;}proc show(x:i64)->void{behave @print(x);}\n'
RUN_CONTEXT='val x<-1;try{run(false,x);}catch(e:const NullPointerException){behave @print(8);};try{run(0,x);}catch(e:const NullPointerException){};'
runner.CASES={
 'logical_right_return':(IO+'func f(flag:bool)->const i64{flag&&{{return 7;}};return 8;}behave @print(f(true));behave @print(f(false));',0,'7\n8\n',''),
 'logical_or_return':(IO+'func f(flag:bool)->const i64{flag||{{return 7;}};return 8;}behave @print(f(true));behave @print(f(false));',0,'8\n7\n',''),
 'logical_left_return':(OBS+'proc f()->const i64{defer observe(1);({{return 7;}})&&{observe(99);false};return 8;}behave @print(f());',0,'1\n7\n',''),
 'condition_returns':(OBS+'proc f()->const i64{defer observe(1);if({{return 7;}}){observe(98);}else{observe(99);};return 8;}behave @print(f());',0,'1\n7\n',''),
 'conditional_initialization_after_return':(IO+'func f(flag:bool)->const i64{val x<-make local i64;if(flag&&{{return 7;}}){}else{x<-8;{}};return x;}behave @print(f(true));behave @print(f(false));',0,'7\n8\n',''),
 'live_logical_read_still_rejects':('func f(flag:bool)->const i64{val x<-make local i64;flag&&{{x;return 7;}};return 8;}f(true);',2,'','before definite initialization'),
 'logical_null_refinement':(IO+'using "std:exceptions" NullPointerException;func f(ptr p:i64 in O)->const i64 where {owner O}{isnull(p)&&{{return 7;}};return p;}val x<-8;ptr p=x;behave @print(f(p));p=null;behave @print(f(p));',0,'8\n7\n',''),
 'logical_void_operand_rejects':('func f(flag:bool)->const i64{flag&&{};return 8;}f(true);',2,'','logical operation requires bool operands'),
 'logical_wrong_normal_type_rejects':('func f(flag:bool)->const i64{flag||1;return 8;}f(true);',2,'','logical operation requires bool operands'),
 'logical_failure_cleanup':(OBS+EX+'proc f(flag:bool)->void{defer observe(1);flag&&{{throw error();}};observe(2);}try{f(true);}catch(e:const MissingReturnException){observe(3);};f(false);',0,'1\n3\n2\n1\n','[-Wmissing-return]'),
 'logical_finally_replacement':(OBS+'proc f(flag:bool)->const i64{defer observe(1);flag&&{{try{return 2;}finally{return 3;};}};return 4;}behave @print(f(true));behave @print(f(false));',0,'1\n3\n1\n4\n',''),
 'deferred_logical_return':(OBS+'proc f(flag:bool)->const i64{defer observe(1);defer{flag&&{{return 7;}};{}};return 8;}behave @print(f(true));behave @print(f(false));',0,'1\n7\n1\n8\n',''),
 'callable_view_first_operand_return':(OBS+VIEW+'proc f()->const i64{func invoke=first;defer observe(1);return invoke({{return 7;}},{observe(99);2});}behave @print(f());',0,'1\n7\n',''),
 'callable_view_later_operand_return':(OBS+VIEW+'proc f()->const i64{func invoke=first;defer observe(1);return invoke({observe(2);0},{{return 7;}});}behave @print(f());',0,'2\n1\n7\n',''),
 'callable_expression_operand_return':(OBS+VIEW+'proc f()->const i64{defer observe(1);return (first)({observe(2);0},{{return 7;}});}behave @print(f());',0,'2\n1\n7\n',''),
 'live_later_argument_read_still_rejects':(VIEW+'func f(flag:bool)->const i64{val x<-make local i64;func invoke=first;return invoke(if(flag){return 7;}else{8},x);}f(true);',2,'','before definite initialization'),
 'callable_argument_still_checked':(VIEW+'func f()->const i64{func invoke=first;return invoke({{return 7;}},false);}f();',2,'','argument does not satisfy the callable view type'),
 'callable_arity_still_checked':(VIEW+'func f()->const i64{func invoke=first;return invoke({{return 7;}});}f();',2,'','argument count'),
 'aborted_invocation_has_no_throws_obligation':(IO+'using "std:exceptions" NullPointerException;func target(x:i64)->const i64 throws NullPointerException{return x;}func f()->const i64{func invoke=target;return invoke({{return 7;}});}behave @print(f());',0,'7\n',''),
 'surviving_invocation_keeps_throws_obligation':('using "std:exceptions" NullPointerException;func target(x:i64)->const i64 throws NullPointerException{return x;}func f(flag:bool)->const i64{func invoke=target;return invoke(if(flag){return 7;}else{8});}f(true);',2,'','requires a covering catch'),
 'deferred_preparation_returns':(OBS+'proc show(x:i64,y:i64)->void{observe(x+y);}proc f()->const i64{proc invoke=show;defer observe(1);defer invoke({observe(2);0},{{return 7;}});return 8;}behave @print(f());',0,'2\n1\n7\n',''),
 'deferred_preparation_throws':(OBS+EX+'proc show(x:i64)->void{observe(x);}proc f()->void{proc invoke=show;defer observe(1);defer invoke({{throw error();}});observe(99);}try{f();}catch(e:const MissingReturnException){observe(3);};',0,'1\n3\n','[-Wmissing-return]'),
 'deferred_preparation_prior_registration':(OBS+'proc show(x:i64)->void{observe(x);}proc f()->const i64{proc invoke=show;defer invoke(1);defer invoke({{return 7;}});return 8;}behave @print(f());',0,'1\n7\n',''),
 'contextual_deferred_preparation':(CONTEXT+'proc run(tag:T,ref x:i64 in O)->void throws NullPointerException where {T,owner O}{ptr p=x;p=null;proc invoke=show;defer invoke({{pick(tag,p)}});}' + RUN_CONTEXT,0,'8\n7\n',''),
 'contextual_ordinary_preparation':(CONTEXT+'proc run(tag:T,ref x:i64 in O)->void throws NullPointerException where {T,owner O}{ptr p=x;p=null;proc invoke=show;invoke({{pick(tag,p)}});}' + RUN_CONTEXT,0,'8\n7\n',''),
 'contextual_print_operand':(CONTEXT+'proc run(tag:T,ref x:i64 in O)->void throws NullPointerException where {T,owner O}{ptr p=x;p=null;behave @print({{pick(tag,p)}});}' + RUN_CONTEXT,0,'8\n7\n',''),
 'contextual_transfer_rhs':(CONTEXT+'proc run(tag:T,ref x:i64 in O)->void throws NullPointerException where {T,owner O}{ptr p=x;p=null;x<-{{pick(tag,p)}};behave @print(x);}' + RUN_CONTEXT,0,'8\n7\n',''),
 'print_operand_return':(OBS+'proc f()->const i64{defer observe(1);behave @print({{return 7;}});return 8;}behave @print(f());',0,'1\n7\n',''),
 'assignment_rhs_return':(OBS+'proc f(ref x:i64 in O)->const i64 where {owner O}{defer{observe(x);};x<-{{return 7;}};return 8;}val x<-1;behave @print(f(x));behave @print(x);',0,'1\n7\n1\n',''),
 'abandoned_assignment_does_not_initialize':('func f(flag:bool)->const i64{val x<-make local i64;defer{x;{}};x<-{{return 7;}};return 8;}f(true);',2,'','cannot prove initialization'),
 'assignment_null_precedes_return':(OBS+'using "std:exceptions" NullPointerException;proc f(ptr p:i64 in O)->const i64 throws NullPointerException where {owner O}{p<-{{observe(99);return 7;}};return 8;}val x<-1;ptr p=x;p=null;try{f(p);{}}catch(e:const NullPointerException){observe(3);};',0,'3\n',''),
 'throw_operand_return':(OBS+'proc f()->const i64{defer observe(1);throw {{return 7;}};return 8;}behave @print(f());',0,'1\n7\n',''),
 'throw_cause_return':(OBS+EX+'proc f()->const i64{defer observe(1);throw error() from {{return 7;}};return 8;}behave @print(f());',0,'1\n7\n','[-Wmissing-return]'),
 'exception_field_operand_return':(OBS+'proc f()->const i64{defer observe(1);({{return 7;}}).category;return 8;}behave @print(f());',0,'1\n7\n',''),
 'generic_logical_completion':(IO+'func f(x:T,flag:bool)->const T where T{flag&&{{return x;}};return x+x;}behave @print(f(3u64,true));behave @print(f(1.5f32,false));',0,'3\n3\n',''),
 'recursive_logical_completion':(IO+'func f(n:i64)->const i64{(n==0)&&{{return 7;}};return f(n-1);}behave @print(f(40));',0,'7\n',''),
}
runner.CASE_ARGUMENTS={}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
