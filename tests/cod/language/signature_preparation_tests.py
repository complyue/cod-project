#!/usr/bin/env python3
"""One signature path prepares contextual values and lexical recursive borrows."""
from pathlib import Path
import foundation_tests as runner

IO='using symbol "std:io" @print;'
ID='func identity(x:T)->const T where T{return x;}'
APPLY='func apply(f:func(T)->const T,x:T)->const T where T{return f(x);}'
OBS=lambda x:'behave @print('+x+');'
NPE='using "std:exceptions" NullPointerException;'
MISS='using "std:exceptions" MissingReturnException;func missing()->const i64{}'
runner.CASES={
 'later_generic_view':(IO+ID+APPLY+OBS('apply(identity,7)'),0,'7\n',''),
 'later_overloaded_view':(IO+'func pick(x:i64)->const i64{return x+1;}func pick(x:bool)->const bool{return !x;}'+APPLY+OBS('apply(pick,7)')+OBS('apply(pick,true)'),0,'8\nfalse\n',''),
 'later_result_type':(IO+ID+APPLY+'func number()->const i64{return 8;}'+OBS('apply(identity,number())'),0,'8\n',''),
 'later_scoped_type':(IO+ID+APPLY+OBS('apply(identity,{{val x=9;x}})'),0,'9\n',''),
 'later_guarded_type':(IO+ID+APPLY+OBS('apply(identity,try{10}finally{})'),0,'10\n',''),
 'later_condition_type':(IO+ID+APPLY+OBS('apply(identity,if(true){11}else{12})'),0,'11\n',''),
 'later_fold_type':(IO+ID+APPLY+OBS('apply(identity,fold(i in 0..4,s=0){s+i})'),0,'6\n',''),
 'multiple_generic_arguments':(IO+ID+'func twice(f:func(T)->const T,g:func(T)->const T,x:T)->const T where T{return f(g(x));}'+OBS('twice(identity,identity,13)'),0,'13\n',''),
 'structural_later_view':(IO+ID+'func call(f:func(T)->const T,g:func()->const T)->const T where T{return f(g());}func number()->const i64{return 14;}func g=number;'+OBS('call(identity,g)'),0,'14\n',''),
 'contextual_anonymous_value':(IO+APPLY+OBS('apply(func(x:T)->const T where T{return x;},15)'),0,'15\n',''),
 'contextual_scope_value':(IO+ID+APPLY+OBS('apply({'+OBS('1')+'identity},{'+OBS('2')+'16})'),0,'1\n2\n16\n',''),
 'contextual_result_throws':(IO+ID+NPE+'func apply(f:func(T)->const T throws NullPointerException,x:T)->const T throws NullPointerException where T{return f(x);}try{'+OBS('apply(identity,17)')+'}catch(e:const NullPointerException){};',0,'17\n',''),
 'context_does_not_read_uninitialized':(ID+APPLY+'val x=make local i64;apply(identity,x);',2,'','before definite initialization'),
 'context_does_not_hide_same_t':(ID+'func apply(f:func(T)->const T,x:T,y:T)->const T where T{return f(x);}apply(identity,1,true);',2,'','require an explicit concrete view type'),
 'context_does_not_waive_throws':(NPE+'func identity(x:T)->const T throws NullPointerException where T{return x;}'+APPLY+'apply(identity,1);',2,'','required checked exception'),
 'context_does_not_change_callable_kind':('proc identity(x:T)->const T where T{return x;}'+APPLY+'apply(identity,1);',2,'','callable view contract'),
 'late_failure_preserves_order':(IO+ID+APPLY+MISS+'try{apply({'+OBS('1')+'identity},missing());{}}catch(e:const MissingReturnException){'+OBS('2')+'};',0,'1\n2\n','[-Wmissing-return]'),
 'deferred_contextual_value':(IO+ID+'proc show(f:func(T)->const T,x:T)->void where T{'+OBS('f(x)')+'}{{defer show(identity,18);defer show(identity,true);}};',0,'true\n18\n',''),
 'lexical_recursive_ref':(IO+'proc run()->void{val x=19;func recur(ref p:i64 in O,n:i64)->const i64 where {owner O}{if(n==0){return p;}else{return recur(p,n-1);};};'+OBS('recur(x,30)')+'}run();',0,'19\n',''),
 'lexical_recursive_ptr_write':(IO+NPE+'proc run()->void{val x=make local i64;func recur(ptr p:i64 in O,n:i64)->void throws NullPointerException where {owner O}{if(n==0){p<-20;{}}else{recur(p,n-1);};};try{recur(x,30);'+OBS('x')+'}catch(e:const NullPointerException){};}run();',0,'20\n',''),
 'lexical_recursive_read':(IO+'proc run()->void{val x=21;func recur(p:i64 in O,n:i64)->const i64 where {owner O}{if(n==0){return p;}else{return recur(p,n-1);};};'+OBS('recur(x,30)')+'}run();',0,'21\n',''),
 'lexical_recursive_owner_result':(IO+'proc run()->void{val x=22;func recur(ref p:i64 in O,n:i64)->ref i64 in O where {owner O}{if(n==0){return p;}else{return recur(p,n-1);};};ptr p=recur(x,30);p<-23;'+OBS('x')+'}run();',0,'23\n',''),
 'lexical_recursive_retained_type':(IO+'func run(ref x:T in O)->const T where {T,owner O}{func recur(ref p:T in P,n:i64)->const T where {owner P}{if(n==0){return p;}else{return recur(p,n-1);};};return recur(x,30);}val x=24,y=true;'+OBS('run(x)')+OBS('run(y)'),0,'24\ntrue\n',''),
 'lexical_recursive_capture':(IO+'func base(x:i64)->const i64{return x+1;}func run(f:func(i64)->const i64,ref x:i64 in O)->const i64 where {owner O}{func recur[f](ref p:i64 in P,n:i64)->const i64 where {owner P}{if(n==0){return f(p);}else{return recur(p,n-1);};};return recur(x,30);}val x=24;'+OBS('run(base,x)'),0,'25\n',''),
 'lexical_recursive_later_effect':(IO+'proc run()->void{val x=1;func recur(ref p:i64 in O,n:i64)->const i64 where {owner O}{if(n==0){return p;}else{return recur(p,{p<-p+1;n-1});};};'+OBS('recur(x,25)')+OBS('x')+'}run();',0,'26\n26\n',''),
 'lexical_recursive_defer':(IO+NPE+'proc run()->void{val x=make local i64;func recur(ptr p:i64 in O,n:i64)->void throws NullPointerException where {owner O}{if(n==0){p<-27;{}}else{defer recur(p,n-1);};};try{recur(x,30);'+OBS('x')+'}catch(e:const NullPointerException){};}run();',0,'27\n',''),
 'lexical_recursive_checked_failure':(IO+NPE+'proc run()->void{ptr x:i64=null;func recur(ptr p:i64 in O,n:i64)->const i64 throws NullPointerException where {owner O}{if(n==0){return p;}else{return recur(p,n-1);};};val y=1;x=y;x=null;try{recur(x,30);{}}catch(e:const NullPointerException){'+OBS('28')+'};}run();',0,'28\n',''),
 'lexical_recursive_readonly':(IO+'proc run()->void{const val x=29;func recur(ref p:const i64 in O,n:i64)->const i64 where {owner O}{if(n==0){return p;}else{return recur(p,n-1);};};'+OBS('recur(x,30)')+'}run();',0,'29\n',''),
 'lexical_recursion_respects_shadow':(IO+'func other(ref p:i64 in O,n:i64)->const i64 where {owner O}{return 30;}proc run()->void{val x=1;func recur(ref p:i64 in O,n:i64)->const i64 where {owner O}{func recur=other;return recur(p,n);};'+OBS('recur(x,3)')+'}run();',0,'30\n',''),
 'lexical_recursive_same_owner':(IO+'proc run()->void{val x=31,y=2;func recur(ref p:i64 in O,ref q:i64 in O,n:i64)->const i64 where {owner O}{if(n==0){return p;}else{return recur(p,q,n-1);};};'+OBS('recur(x,y,30)')+'}run();',0,'31\n',''),
 'lexical_recursion_uninitialized_read':('proc run()->void{val x=make local i64;func recur(ref p:i64 in O,n:i64)->const i64 where {owner O}{if(n==0){return p;}else{return recur(p,n-1);};};recur(x,30);}run();',2,'','before definite initialization'),
 'lexical_recursion_wrong_owner':('proc run()->void{val x=1;func recur(ref p:i64 in O,ref q:i64 in O,n:i64)->const i64 where {owner O}{if(n==0){return p;}else{return recur(p,q,n-1);};};{{val y=2;recur(x,y,3);}};}run();',2,'','different actual owners'),
 'lexical_recursion_readonly_write':('proc run()->void{const val x=1;func recur(ref p:i64 in O,n:i64)->const i64 where {owner O}{if(n==0){return p;}else{return recur(p,n-1);};};recur(x,30);}run();',2,'','readonly argument'),
}
runner.CASE_ARGUMENTS={}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
