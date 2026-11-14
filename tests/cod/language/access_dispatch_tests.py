#!/usr/bin/env python3
"""Target access participates in shared method selection and preparation."""
from pathlib import Path
import foundation_tests as runner
IO='using symbol "std:io" @print;'
PAIR='func choose(ref x:i64 in O)->const i64 where {owner O}{return 1;}func choose(ref x:const i64 in O)->const i64 where {owner O}{return 2;}'
VAL='func choose(x:i64)->const i64{return 1;}func choose(x:const i64)->const i64{return 2;}'
LOC='val x=7;ref observed:const i64=x;'
OBS=lambda expression:'behave @print('+expression+');'
runner.CASES={
 'direct_targets':(IO+PAIR+LOC+OBS('choose(x)')+OBS('choose(observed)'),0,'1\n2\n',''),
 'declaration_order':(IO+PAIR[PAIR.index('func choose(ref x:const'):]+PAIR[:PAIR.index('func choose(ref x:const')]+LOC+OBS('choose(x)')+OBS('choose(observed)'),0,'1\n2\n',''),
 'fixed_binding_not_target_const':(IO+PAIR+'val x=7;const ptr p=x;'+OBS('choose(p)'),0,'1\n',''),
 'const_value_target':(IO+PAIR+'const val x=7;'+OBS('choose(x)'),0,'2\n',''),
 'readonly_weakening':(IO+'func choose(ref x:const i64 in O)->const i64 where {owner O}{return x;}val x=7;'+OBS('choose(x)'),0,'7\n',''),
 'readonly_borrow_rejected':(PAIR[:PAIR.index('func choose(ref x:const')]+LOC+'choose(observed);',2,'','readonly argument'),
 'independent_value_access':(IO+VAL+LOC+OBS('choose(x)')+OBS('choose(observed)')+OBS('choose(3)'),0,'1\n2\n1\n',''),
 'readonly_value_copy_writable':(IO+'func changed(x:i64)->const i64{x<-8;return x;}const val x=7;'+OBS('changed(x)')+OBS('x'),0,'8\n7\n',''),
 'binding_const_duplicate':('func f(x:i64)->const i64{return x;}func f(const x:i64)->const i64{return x;}',2,'','duplicate method signature'),
 'readonly_body_cannot_write':('func f(ref x:const i64 in O)->void where {owner O}{x<-8;}',2,'','readonly destination'),
 'explicit_callable_view':(IO+VAL+'func a:func(i64)->const i64=choose;func b:func(const i64)->const i64=choose;'+OBS('a(7)')+OBS('b(7)'),0,'1\n2\n',''),
 'contextual_callable_view':(IO+VAL+'func apply(f:func(const i64)->const i64)->const i64{return f(7);}'+OBS('apply(choose)'),0,'2\n',''),
 'generic_readonly':(IO+'func choose(ref x:T in O)->const i64 where {T,owner O}{return 1;}func choose(ref x:const T in O)->const i64 where {T,owner O}{return 2;}'+LOC+'const val y=true;'+OBS('choose(x)')+OBS('choose(observed)')+OBS('choose(y)'),0,'1\n2\n2\n',''),
 'type_and_access_specific':(IO+'func choose(ref x:const T in O)->const i64 where {T,owner O}{return 2;}func choose(ref x:i64 in O)->const i64 where {owner O}{return 1;}'+LOC+OBS('choose(x)')+OBS('choose(observed)'),0,'1\n2\n',''),
 'type_access_tradeoff_ambiguous':('func choose(ref x:T in O)->const i64 where {T,owner O}{return 1;}func choose(ref x:const i64 in O)->const i64 where {owner O}{return 2;}val x=7;choose(x);',2,'','ambiguous methods'),
 'crossed_qualifiers_ambiguous':('func choose(ref x:i64 in O,ref y:const i64 in O)->const i64 where {owner O}{return 1;}func choose(ref x:const i64 in O,ref y:i64 in O)->const i64 where {owner O}{return 2;}val x=7,y=8;choose(x,y);',2,'','ambiguous methods'),
 'crossed_qualifiers_exact':(IO+'func choose(ref x:i64 in O,ref y:const i64 in O)->const i64 where {owner O}{return 1;}func choose(ref x:const i64 in O,ref y:i64 in O)->const i64 where {owner O}{return 2;}'+LOC+OBS('choose(x,observed)')+OBS('choose(observed,x)'),0,'1\n2\n',''),
 'conditional_view':(IO+PAIR+LOC+OBS('choose(if(true){x}else{observed})'),0,'2\n',''),
 'scoped_view':(IO+PAIR+LOC+OBS('choose({{ref y:const i64=x;y}})'),0,'2\n',''),
 'scope_keeps_outer_lookup':(IO+PAIR+LOC+OBS('choose({{ref observed=x;observed}})')+OBS('choose(observed)'),0,'1\n2\n','[-Wshadow]'),
 'guarded_view':(IO+PAIR+LOC+OBS('choose(try{observed}finally{})'),0,'2\n',''),
 'alias_access':(IO+PAIR+LOC+'var alias->observed;'+OBS('choose(alias)'),0,'2\n',''),
 'readonly_alias_of_value':(IO+PAIR+'val x=7;const var alias->x;'+OBS('choose(alias)'),0,'2\n',''),
 'fixed_alias_of_reference':(IO+PAIR+'val x=7;ptr p=x;const ptr var alias->p;'+OBS('choose(alias)'),0,'1\n',''),
 'copy_from_readonly_view':(IO+PAIR+LOC+'{{ptr y<-observed;'+OBS('choose(y)')+'}};',0,'1\n',''),
 'transfer_result_readonly':(IO+PAIR+'val x=7;'+OBS('choose(x<-8)'),0,'2\n',''),
 'borrowed_result_access':(IO+PAIR+'func readonly(ref x:i64 in O)->ref const i64 in O where {owner O}{return x;}val x=7;'+OBS('choose(readonly(x))'),0,'2\n',''),
 'ordinary_value_result':(IO+VAL+'func value()->const i64{return 7;}'+OBS('choose(value())'),0,'1\n',''),
 'operand_effects_once':(IO+PAIR+LOC+OBS('choose({'+OBS('3')+'observed})'),0,'3\n2\n',''),
 'eager_access_selection':(IO+'func (+)(ref x:i64 in O,ref y:i64 in O)->const i64 where {owner O}{return 1;}func (+)(ref x:const i64 in O,ref y:const i64 in O)->const i64 where {owner O}{return 2;}'+LOC+OBS('x+x')+OBS('observed+x'),0,'1\n2\n',''),
 'builtin_fallback_snapshot':(IO+'func (+)(ref a:i64 in O,ref b:i64 in O)->const i64 where {owner O}{return 99;}val x=3;ref observed:const i64=x;'+OBS('observed+{x<-9;x}')+OBS('x'),0,'12\n9\n',''),
 'borrowed_operator_observes_later_write':(IO+'func (+)(ref a:const i64 in O,ref b:const i64 in O)->const i64 where {owner O}{return intrinsic (+)(a,b);}val x=3;ref observed:const i64=x;'+OBS('observed+{x<-9;x}'),0,'18\n',''),
 'shared_result_context':(IO+PAIR+'func writable(ref x:i64 in O)->ref i64 in O where {owner O}{return x;}func readonly(ref x:i64 in O)->ref const i64 in O where {owner O}{return x;}func run(f:F,ref x:i64 in O)->const i64 where {F,owner O}{ptr p=f(x);return choose(p);}val x=7;func a=writable;func b=readonly;'+OBS('run(a,x)')+OBS('run(b,x)'),0,'1\n2\n',''),
 'recursive_access_context':(IO+PAIR+'func run(ref x:const i64 in O,n:i64)->const i64 where {owner O}{if(n==0){return choose(x);}else{return run(x,n-1);};}val x=7;'+OBS('run(x,40)'),0,'2\n',''),
 'deferred_selection':(IO+'proc show(ref x:i64 in O)->void where {owner O}{'+OBS('1')+'}proc show(ref x:const i64 in O)->void where {owner O}{'+OBS('2')+'}'+LOC+'{{defer show(x);defer show(observed);}};',0,'2\n1\n',''),
 'inferred_local_readonly_view':(IO+PAIR+LOC+OBS('choose({ptr alias=observed;alias})'),0,'2\n',''),
 'annotated_alias_access':(IO+PAIR+'val x=7;var alias:const i64->x;'+OBS('choose(alias)'),0,'2\n',''),
 'readonly_loop_controls':(IO+VAL+'for(i in 0..3){'+OBS('choose(i)')+'};',0,'2\n2\n2\n',''),
 'later_qualified_result_preparation':(IO+'func tag(ref x:i64 in O)->const bool where {owner O}{return true;}func tag(ref x:const i64 in O)->const i64 where {owner O}{return 0;}func pick(ref x:i64 in O,tag:bool)->const i64 where {owner O}{return x;}func pick(x:i64,tag:i64)->const i64{return x;}'+LOC+OBS('pick(x,{x<-9;tag(observed)})')+'x<-7;'+OBS('pick(x,{x<-9;tag(x)})'),0,'7\n9\n',''),
 'late_block_selection':(IO+PAIR+LOC+'{{defer{'+OBS('choose(observed)')+'};}};',0,'2\n',''),
}
runner.CASE_ARGUMENTS={}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
