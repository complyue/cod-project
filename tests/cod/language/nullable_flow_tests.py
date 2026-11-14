#!/usr/bin/env python3
"""Borrowed results preserve parameter-relative null alternatives."""
from pathlib import Path
import foundation_tests as runner

PRE='using symbol "std:io" @print;using "std:exceptions" NullPointerException;\n'
ID='func identity(ptr x:i64 in O)->ptr i64 in O where {owner O}{return x;}\n'
OBS=lambda expr:'behave @print('+expr+');'
LOC='val x=7;ptr p=x;'
CHECK='requires a covering catch'
runner.CASES={
 'direct_identity':(PRE+ID+LOC+OBS('identity(p)'),0,'7\n',''),
 'ref_from_forwarded_nonnull':(PRE+ID+LOC+'ref q=identity(p);q<-8;'+OBS('x'),0,'8\n',''),
 'selected_identity':(PRE+ID+LOC+'func f=identity;'+OBS('f(p)'),0,'7\n',''),
 'captured_identity':(PRE+ID+LOC+'func f=identity;func wrap=func[f](ptr a:i64 in O)->ptr i64 in O where {owner O}{return f(a);};'+OBS('wrap(p)'),0,'7\n',''),
 'higher_order_identity':(PRE+ID+'func apply(f:F,ptr x:T in O)->ptr T in O where {F,T,owner O}{return f(x);}'+LOC+'func f=identity;'+OBS('apply(f,p)'),0,'7\n',''),
 'nested_results':(PRE+ID+LOC+OBS('identity(identity(p))'),0,'7\n',''),
 'recursive_identity':(PRE+'func walk(ptr x:i64 in O,n:i64)->ptr i64 in O where {owner O}{if(n==0){return x;}else{return walk(x,n-1);};}'+LOC+OBS('walk(p,40)'),0,'7\n',''),
 'mutual_identity':(PRE+'func walk(ptr x:i64 in O,n:i64)->ptr i64 in O where {owner O}{if(n==0){return x;}else{return relay(x,n-1);};}func relay(ptr x:i64 in O,n:i64)->ptr i64 in O where {owner O}{return walk(x,n);}'+LOC+OBS('walk(p,40)'),0,'7\n',''),
 'generic_identity':(PRE+'func identity(ptr x:T in O)->ptr T in O where {T,owner O}{return x;}val x=7,y=true;'+OBS('identity(x)')+OBS('identity(y)'),0,'7\ntrue\n',''),
 'renamed_local_alias':(PRE+'func identity(ptr x:i64 in O)->ptr i64 in O where {owner O}{ptr p=x;var alias->p;return alias;}'+LOC+OBS('identity(p)'),0,'7\n',''),
 'conditional_inputs':(PRE+'func choose(ptr x:i64 in O,ptr y:i64 in O,flag:bool)->ptr i64 in O where {owner O}{return if(flag){x}else{y};}val x=7,y=8;'+OBS('choose(x,y,false)'),0,'8\n',''),
 'unused_nullable_input':(PRE+'func choose(ptr x:i64 in O,ptr y:i64 in O)->ptr i64 in O where {owner O}{return x;}'+LOC+'ptr q=x;q=null;'+OBS('choose(p,q)'),0,'7\n',''),
 'forwarded_null_remains_null':(PRE+ID+LOC+'p=null;'+OBS('isnull(identity(p))'),0,'true\n',''),
 'forwarded_null_read_rejected':(PRE+ID+LOC+'p=null;'+OBS('identity(p)'),2,'',CHECK),
 'forwarded_null_caught':(PRE+ID+LOC+'p=null;try{'+OBS('identity(p)')+'}catch(e:const NullPointerException){'+OBS('9')+'};',0,'9\n',''),
 'one_nullable_input_rejected':(PRE+'func choose(ptr x:i64 in O,ptr y:i64 in O,flag:bool)->ptr i64 in O where {owner O}{return if(flag){x}else{y};}'+LOC+'ptr q=x;q=null;'+OBS('choose(p,q,false)'),2,'',CHECK),
 'explicit_null_rejected':(PRE+'func clear(ptr x:i64 in O)->ptr i64 in O where {owner O}{return null;}'+LOC+OBS('clear(p)'),2,'',CHECK),
 'rebound_null_rejected':(PRE+'func clear(ptr x:i64 in O)->ptr i64 in O where {owner O}{x=null;return x;}'+LOC+OBS('clear(p)'),2,'',CHECK),
 'local_null_rejected':(PRE+'func clear(ptr x:i64 in O)->ptr i64 in O where {owner O}{ptr q:i64 in O=null;return q;}'+LOC+OBS('clear(p)'),2,'',CHECK),
 'conditional_null_rejected':(PRE+'func clear(ptr x:i64 in O,flag:bool)->ptr i64 in O where {owner O}{if(flag){x=null;{}}else{};return x;}'+LOC+OBS('clear(p,true)'),2,'',CHECK),
 'conditional_expression_null_rejected':(PRE+'func clear(ptr x:i64 in O,flag:bool)->ptr i64 in O where {owner O}{ptr q:i64 in O=null;return if(flag){x}else{q};}'+LOC+OBS('clear(p,false)'),2,'',CHECK),
 'swap_null_rejected':(PRE+'func clear(ptr x:i64 in O)->ptr i64 in O where {owner O}{ptr q:i64 in O=null;x<=>q;return x;}'+LOC+OBS('clear(p)'),2,'',CHECK),
 'swap_input_forwarding':(PRE+'func swap(ptr x:i64 in O,ptr y:i64 in O)->ptr i64 in O where {owner O}{x<=>y;return x;}val x=7,y=8;'+OBS('swap(x,y)'),0,'8\n',''),
 'swap_uses_second_null_origin':(PRE+'func swap(ptr x:i64 in O,ptr y:i64 in O)->ptr i64 in O where {owner O}{x<=>y;return x;}'+LOC+'ptr q=x;q=null;'+OBS('swap(q,p)'),0,'7\n',''),
 'swap_null_source_rejected':(PRE+'func swap(ptr x:i64 in O,ptr y:i64 in O)->ptr i64 in O where {owner O}{x<=>y;return x;}'+LOC+'ptr q=x;q=null;'+OBS('swap(p,q)'),2,'',CHECK),
 'recursive_reordered_inputs':(PRE+'func walk(ptr x:i64 in O,ptr y:i64 in O,n:i64)->ptr i64 in O where {owner O}{if(n==0){return y;}else{return walk(x,y,n-1);};}'+LOC+'ptr q=x;q=null;'+OBS('walk(q,p,40)'),0,'7\n',''),
 'nonnull_refinement':(PRE+ID+LOC+'if(!isnull(p)){'+OBS('identity(p)')+'}else{};',0,'7\n',''),
 'later_rebind_preserves_prepared_argument':(PRE+'func first(ptr x:i64 in O,ignored:i64)->ptr i64 in O where {owner O}{return x;}'+LOC+OBS('first(p,{p=null;0})')+OBS('isnull(p)'),0,'7\ntrue\n',''),
 'finally_preserves_return_selection':(PRE+'func identity(ptr x:i64 in O)->ptr i64 in O where {owner O}{try{return x;}finally{x=null;{}};}'+LOC+OBS('identity(p)'),0,'7\n',''),
 'finally_null_return_rejected':(PRE+'func clear(ptr x:i64 in O)->ptr i64 in O where {owner O}{try{return x;}finally{return null;};}'+LOC+OBS('clear(p)'),2,'',CHECK),
 'deferred_return':(PRE+'func identity(ptr x:i64 in O)->ptr i64 in O where {owner O}{defer{return x;};}'+LOC+OBS('identity(p)'),0,'7\n',''),
 'deferred_null_return_rejected':(PRE+'func clear(ptr x:i64 in O)->ptr i64 in O where {owner O}{defer{return null;};return x;}'+LOC+OBS('clear(p)'),2,'',CHECK),
 'deferred_late_rebind_rejected':(PRE+'func clear(ptr x:i64 in O)->ptr i64 in O where {owner O}{defer{return x;};x=null;}'+LOC+OBS('clear(p)'),2,'',CHECK),
 'loop_forwarding':(PRE+'func identity(ptr x:i64 in O)->ptr i64 in O where {owner O}{ptr p=x;for(i in 0..3){p=x;};return p;}'+LOC+OBS('identity(p)'),0,'7\n',''),
 'loop_null_rejected':(PRE+'func clear(ptr x:i64 in O)->ptr i64 in O where {owner O}{for(i in 0..3){x=null;};return x;}'+LOC+OBS('clear(p)'),2,'',CHECK),
 'null_branch_fallback':(PRE+'func choose(ptr x:i64 in O,ref fallback:i64 in O)->ptr i64 in O where {owner O}{if(isnull(x)){return fallback;}else{return x;};}'+LOC+'ptr q=x;q=null;'+OBS('choose(q,x)'),0,'7\n',''),
 'deferred_preserved_return':(PRE+'func identity(ptr x:i64 in O)->ptr i64 in O where {owner O}{defer{x=null;{}};return x;}'+LOC+OBS('identity(p)'),0,'7\n',''),
 'finalizer_replaces_null':(PRE+'func identity(ptr x:i64 in O)->ptr i64 in O where {owner O}{try{return null;}finally{return x;};}'+LOC+OBS('identity(p)'),0,'7\n',''),
 'refined_nullable_formal':(PRE+ID+'func choose(ptr x:i64 in O)->const i64 where {owner O}{if(isnull(x)){return 9;}else{return identity(x);};}'+LOC+OBS('choose(p)'),0,'7\n',''),
 'guarded_return':(PRE+'func identity(ptr x:i64 in O)->ptr i64 in O where {owner O}{return try{x}finally{};}'+LOC+OBS('identity(p)'),0,'7\n',''),
 'selected_null_alternative_rejected':(PRE+ID+'func clear(ptr x:i64 in O)->ptr i64 in O where {owner O}{return null;}'+LOC+'func f=if(false){identity}else{clear};'+OBS('f(p)'),2,'',CHECK),
 'unrelated_null_implementation':(PRE+ID+'func clear(ptr x:i64 in O)->ptr i64 in O where {owner O}{return null;}'+LOC+'func unused=clear;func f=identity;'+OBS('f(p)'),0,'7\n',''),
 'unrelated_null_ref_target':(PRE+ID+'func clear(ptr x:i64 in O)->ptr i64 in O where {owner O}{return null;}'+LOC+'func unused=clear;func f=identity;ref q=f(p);'+OBS('q'),0,'7\n',''),
 'unrelated_null_ref_argument':(PRE+ID+'func clear(ptr x:i64 in O)->ptr i64 in O where {owner O}{return null;}func read(ref x:i64 in O)->const i64 where {owner O}{return x;}'+LOC+'func unused=clear;func f=identity;'+OBS('read(f(p))'),0,'7\n',''),
 'declared_exception_not_waived':(PRE+'func identity(ptr x:i64 in O)->ptr i64 in O throws NullPointerException where {owner O}{return x;}'+LOC+OBS('identity(p)'),2,'',CHECK),
 'uninitialized_target_rejected':(PRE+ID+'val x=make local i64;'+OBS('identity(x)'),2,'','before definite initialization'),
 'readonly_target_preserved':(PRE+'func identity(ptr x:const i64 in O)->ptr const i64 in O where {owner O}{return x;}'+LOC+'identity(p)<-8;',2,'','readonly destination'),
 'expired_owner_rejected':(PRE+ID+'ptr p:i64=null;{{val x=7;p=identity(x);}};'+OBS('p'),2,'','owner has ended'),
}
runner.CASE_ARGUMENTS={}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
