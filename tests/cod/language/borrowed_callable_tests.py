#!/usr/bin/env python3
"""Callable views retain external targets and the shared call obligations."""
from pathlib import Path
import foundation_tests as runner
from language_test_support import expect_source_rejections

IO='using symbol "std:io" @print;\n'
NPE='using "std:exceptions" NullPointerException;\n'
ID='func id(ref x:i64 in O)->ref i64 in O where {owner O}{return x;}\n'
PTR='func id(ptr x:i64 in O)->ptr i64 in O where {owner O}{return x;}\n'
SET='proc set(ptr x:i64 in O)->void throws NullPointerException where {owner O}{x<-7;return;}\n'
runner.CASES={
 'returned_alias':(IO+ID+'val x<-1;func f=id;ref y=f(x);y<-7;behave @print(x);x<-8;behave @print(y);',0,'7\n8\n',''),
 'parenthesized_target':(IO+ID+'val x<-1;(id)(x)<-9;behave @print(x);',0,'9\n',''),
 'nullable_forwarding':(IO+NPE+PTR+'val x<-1;ptr p=x;func f=id;try{ptr q=f(p);q<-8;behave @print(x);}catch(e:const NullPointerException){behave @print(99);};p=null;behave @print(((f(p)) is null));',0,'8\ntrue\n',''),
 'nullable_read_requires_catch':(PTR+'val x<-1;ptr p=x;p=null;func f=id;f(p)+1;',2,'','requires a covering catch'),
 'nullable_read_caught':(IO+NPE+PTR+'val x<-1;ptr p=x;p=null;func f=id;try{behave @print(f(p));}catch(e:const NullPointerException){behave @print(9);};',0,'9\n',''),
 'readonly_view':(IO+'func id(ref x:const i64 in O)->ref const i64 in O where {owner O}{return x;}val x<-1;func f=id;ref y=f(x);x<-7;behave @print(y);',0,'7\n',''),
 'readonly_write_rejected':('func id(ref x:const i64 in O)->ref const i64 in O where {owner O}{return x;}val x<-1;func f=id;f(x)<-7;',2,'','readonly destination'),
 'readonly_argument_rejected':(ID+'const val x<-1;func f=id;f(x);',2,'','readonly argument'),
 'repeated_owner':(IO+'func pick(ref x:i64 in O,ref y:i64 in O,flag:bool)->ref i64 in O where {owner O}{return if(flag){x}else{y};}val x<-1,y<-2;func f=pick;f(x,y,false)<-7;behave @print(y);',0,'7\n',''),
 'owner_mismatch':('func pick(ref x:i64 in O,ref y:i64 in O)->ref i64 in O where {owner O}{return x;}ref x:i64=1;func f=pick;{{val y<-2;f(x,y);}};',2,'','different actual owners'),
 'owner_renaming':(IO+'func first(ref x:i64 in A,ref y:i64 in B)->ref i64 in A where {owner A,owner B}{return x;}func same(ref x:i64 in X,ref y:i64 in Y)->ref i64 in X where {owner Y,owner X}{return x;}ref x:i64=1;func f=first;f<-same;{{val y<-2;f(x,y)<-7;}};behave @print(x);',0,'7\n',''),
 'owner_relationship_mismatch':('func first(ref x:i64 in A,ref y:i64 in B)->ref i64 in A where {owner A,owner B}{return x;}func same(ref x:i64 in X,ref y:i64 in X)->ref i64 in X where {owner X}{return x;}func f=first;f<-same;',2,'','unproved owner equality'),
 'returned_owner_mismatch':('func first(ref x:i64 in A,ref y:i64 in B)->ref i64 in A where {owner A,owner B}{return x;}func last(ref x:i64 in A,ref y:i64 in B)->ref i64 in B where {owner A,owner B}{return y;}func f=first;f<-last;',2,'','does not satisfy the callable view contract'),
 'borrow_and_value_contracts_distinct':(ID+'func value(x:i64)->const i64{return x;}func f=id;f<-value;',2,'','does not satisfy the callable view contract'),
 'read_and_ref_contracts_distinct':(ID+'func read(x:i64 in O)->ref i64 in O where {owner O}{return x;}func f=id;f<-read;',2,'','does not satisfy the callable view contract'),
 'local_rebind_does_not_rebind_caller':(IO+NPE+'proc clear(ptr x:i64 in O)->void where {owner O}{x=null;return;}val x<-1;ptr p=x;clear(p);behave @print((p is null));',0,'false\n',''),
 'later_argument_write_visible':(IO+'func read(ref x:i64 in O,ignored:i64)->const i64 where {owner O}{return x;}val x<-1;func f=read;behave @print(f(x,{x<-7;0}));',0,'7\n',''),
 'selected_address_before_rebind':(IO+NPE+'func read(ptr x:i64 in O,ignored:i64)->const i64 throws NullPointerException where {owner O}{return x;}val x<-1,y<-2;ptr p=x;func f=read;try{behave @print(f(p,{p=y;0}));}catch(e:const NullPointerException){};',0,'1\n',''),
 'borrow_read_before_later_effect':(IO+NPE+'proc read(x:i64 in O,ignored:i64)->void where {owner O}{return;}val x<-1;ptr p=x;p=null;proc f=read;try{f(p,{behave @print(99);0});}catch(e:const NullPointerException){behave @print(7);};',0,'7\n',''),
 'ref_requires_nonnull':(ID+'val x<-1;ptr p=x;p=null;func f=id;f(p);',2,'','proven non-null'),
 'ref_requires_initialized':(ID+'val x<-make local i64;func f=id;f(x);',2,'','before definite initialization'),
 'writer_initializes':(IO+NPE+SET+'val x<-make local i64;proc f=set;try{f(x);behave @print(x);}catch(e:const NullPointerException){};',0,'7\n',''),
 'all_selected_writers_initialize':(IO+NPE+SET+'proc other(ptr x:i64 in P)->void throws NullPointerException where {owner P}{x<-8;return;}val x<-make local i64;proc f=set;for(i in 0..2){try{f(x);behave @print(x);}catch(e:const NullPointerException){};f<-other;};',0,'7\n8\n',''),
 'selected_nonwriter_rejected':(NPE+SET+'proc other(ptr x:i64 in P)->void throws NullPointerException where {owner P}{}val x<-make local i64;proc f=set;for(i in 0..2){try{f(x);x;{}}catch(e:const NullPointerException){};f<-other;};',2,'','before definite initialization'),
 'aliased_inputs_preserve_write_order':(IO+NPE+'func setread(ptr x:i64 in O,ptr y:i64 in O)->const i64 throws NullPointerException where {owner O}{x<-7;return y;}val x<-make local i64;func f=setread;try{behave @print(f(x,x));}catch(e:const NullPointerException){};',0,'7\n',''),
 'uninitialized_return_not_invented':(NPE+PTR+'val x<-make local i64;func f=id;ptr p=f(x);try{p;{}}catch(e:const NullPointerException){};',2,'','before definite initialization'),
 'returned_target_specific':(IO+NPE+'func pick(ptr x:i64 in O,ptr y:i64 in O)->ptr i64 in O where {owner O}{return x;}val x<-7,y<-make local i64;func f=pick;try{behave @print(f(x,y));}catch(e:const NullPointerException){behave @print(99);};',0,'7\n',''),
 'deferred_writer_initializes':(IO+NPE+SET+'proc f(ptr x:i64 in O)->void throws NullPointerException where {owner O}{proc writer=set;defer writer(x);return;}val x<-make local i64;try{f(x);behave @print(x);}catch(e:const NullPointerException){};',0,'7\n',''),
 'deferred_borrow_observes_later_write':(IO+'proc show(ref x:i64 in O)->void where {owner O}{behave @print(x);return;}{{val x<-1;defer show(x);x<-7:void;}};',0,'7\n',''),
 'deferred_view_expires':('using "std:exceptions" MissingReturnError;proc show(ref x:const MissingReturnError in O)->void where {owner O}{}func bad()->const i64{}try{bad();{}}catch(e:const MissingReturnError){defer show(e);};',2,'','does not outlive'),
 'returned_view_escape_rejected':(ID+'func leak(ref seed:i64 in O)->ref i64 in O where {owner O}{'
     'val local<-1;func f=id;return f(local);}ref actual:i64=0;leak(actual);',2,'','borrowed result does not retain its declared external owner'),
 'standalone_scope_returns_independent_bits':(IO+ID+'val saved<-{{val local<-1;func f=id;return f(local);}};'
     'behave @print(saved);',0,'1\n',''),
 'generic_higher_order_borrow':(IO+ID+'func apply(f:F,ref x:i64 in O)->ref i64 in O where {F,owner O}{return f(x);}val x<-1;func f=id;apply(f,x)<-7;behave @print(x);',0,'7\n',''),
 'recursive_higher_order_borrow':(IO+ID+'func walk(f:F,ref x:i64 in O,n:i64)->ref i64 in O where {F,owner O}{if(n==0){return f(x);}else{return walk(f,x,n-1);};}val x<-1;func f=id;walk(f,x,40)<-7;behave @print(x);',0,'7\n',''),
 'explicit_borrowing_callable_input':(IO+ID+'val x<-1;func f=id;func wrap(fn:F,ref y:i64 in O)->ref i64 in O where {F,owner O}{return fn(y);};wrap(f,x)<-7;behave @print(x);',0,'7\n',''),
 'callable_holder_borrow':(IO+'func base(x:i64)->const i64{return x;}func other(x:i64)->const i64{return x+7;}proc replace(ref f:func(i64)->const i64 in O)->void where {owner O}{f<-other;return;}func f=base;replace(f);behave @print(f(1));',0,'8\n',''),
 'callable_holder_return':(IO+'func base(x:i64)->const i64{return x;}func other(x:i64)->const i64{return x+7;}func id(ref f:func(i64)->const i64 in O)->ref (func(i64)->const i64) in O where {owner O}{return f;}func f=base;func select=id;ptr p=select(f);p<-other;behave @print(f(1));',0,'8\n',''),
 'declared_exception_preserved':(NPE+'func read(ref x:i64 in O)->const i64 throws NullPointerException where {owner O}{return x;}val x<-1;func f=read;f(x);',2,'','requires a covering catch'),
 'nullable_callee_before_arguments':(IO+NPE+ID+'val x<-1;func f=id;ptr p=f;p=null;try{p({behave @print(99);x});{}}catch(e:const NullPointerException){behave @print(7);};',0,'7\n',''),
 'nullable_callee_and_result_independent':(IO+NPE+PTR+'val x<-1;ptr p=x;p=null;func f=id;ptr callable=f;behave @print(((callable(p)) is null));callable=null;try{callable({behave @print(99);p});{}}catch(e:const NullPointerException){behave @print(7);};',0,'true\n7\n',''),
 'contextual_call_preparation':(IO+'func borrow(ref x:i64 in O,ignored:i64)->const i64 where {owner O}{return x;}func copy(x:i64,ignored:i64)->const i64{return x;}proc run(f:F)->void where F{val x<-1;behave @print(f(x,{x<-7;0}));return;}func a=borrow;func b=copy;run(a);run(b);',0,'7\n1\n',''),
 'contextual_call_result':(IO+ID+'func independent(x:i64)->const i64{return x+1;}func run(f:F,ref x:i64 in O)->const i64 where {F,owner O}{return f(x);}val x<-7;func a=id;func b=independent;behave @print(run(a,x));behave @print(run(b,x));',0,'7\n8\n',''),
 'contextual_local_target':(IO+ID+'func independent(x:i64)->const i64{return x+1;}proc run(f:F,ref x:i64 in O)->void where {F,owner O}{ptr y=f(x);y<-9;behave @print(x);return;}val x<-1;func a=id;func b=independent;run(b,x);run(a,x);',0,'1\n9\n',''),
 'borrowed_signature_same_type_relation':(ID+'func value(x:i64)->const i64{return x;}func same(a:T,b:T)->void where T{}func a=id;func b=value;same(a,b);',2,'','no applicable method'),
 'deferred_contextual_preparation':(IO+'proc borrow(ref x:i64 in O)->void where {owner O}{behave @print(x);return;}proc copy(x:i64)->void{behave @print(x);return;}proc run(f:F)->void where F{val x<-1;defer f(x);x<-7;return;}proc a=borrow;proc b=copy;run(a);run(b);',0,'7\n1\n',''),
 'procedure_exclusion_preserved':('proc write(ref x:i64 in O)->void where {owner O}{x<-7;}func invoke(ref x:i64 in O)->void where {owner O}{write(x);}val x<-1;invoke(x);',2,'','function cannot invoke a procedure'),
}
runner.CASE_ARGUMENTS={}
runner.CASE_SHADOWS = {
    'returned_alias': ['x'],
    'parenthesized_target': ['x'],
    'nullable_forwarding': ['x'],
    'nullable_read_caught': ['x'],
    'readonly_view': ['x'],
    'repeated_owner': ['x', 'y'],
    'owner_renaming': ['x', 'x'],
    'local_rebind_does_not_rebind_caller': ['x'],
    'later_argument_write_visible': ['x'],
    'selected_address_before_rebind': ['x'],
    'borrow_read_before_later_effect': ['x'],
    'writer_initializes': ['x'],
    'all_selected_writers_initialize': ['x', 'x'],
    'aliased_inputs_preserve_write_order': ['x'],
    'returned_target_specific': ['x', 'y'],
    'deferred_writer_initializes': ['x', 'x'],
    'generic_higher_order_borrow': ['x', 'f', 'x'],
    'recursive_higher_order_borrow': ['x', 'f', 'x'],
    'explicit_borrowing_callable_input': ['x'],
    'callable_holder_borrow': ['f'],
    'callable_holder_return': ['f'],
    'nullable_callee_before_arguments': ['x'],
    'nullable_callee_and_result_independent': ['x'],
    'contextual_call_result': ['x', 'x', 'x'],
    'contextual_local_target': ['x', 'x', 'x'],
}
runner.EXTRA_SOURCES=[Path(__file__)]

expect_source_rejections(runner, (
    'deferred_contextual_preparation',
), 'procedure signatures cannot occupy ordinary value or reference storage')

if __name__=='__main__':
    raise SystemExit(runner.main())
