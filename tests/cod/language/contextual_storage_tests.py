#!/usr/bin/env python3
"""Shared frames preserve context-dependent local storage and borrowed targets."""
from pathlib import Path
import foundation_tests as runner
import result_consumption_tests as contracts
from result_consumption_tests import IO, NPE, PICK, NULL, CALLABLES

runner.CASES = {
 'borrow_or_construct_local':(IO+PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{ref y=pick(tag,x);x<-9;return y;}val x=1;behave @print(read(false,x));x<-1;behave @print(read(0,x));',0,'9\n1\n',''),
 'reverse_demand_order':(IO+PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{ref y=pick(tag,x);x<-9;return y;}val x=1;behave @print(read(0,x));x<-1;behave @print(read(false,x));',0,'1\n9\n',''),
 'later_storage_identity':(IO+PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{ref y=pick(tag,x);val z=3;ptr p=z;val var alias->z;alias<-4;x<-9;return y+p+alias;}val x=1;behave @print(read(false,x));x<-1;behave @print(read(0,x));',0,'17\n9\n',''),
 'two_contextual_declarations':(IO+PICK+'func read(tag:T,other:U,ref x:i64 in O)->const i64 where {T,U,owner O}{ref a=pick(tag,x);ref b=pick(other,x);val c=3;x<-9;return a+b+c;}val x=1;behave @print(read(false,0,x));x<-1;behave @print(read(0,false,x));x<-1;behave @print(read(false,false,x));x<-1;behave @print(read(0,0,x));',0,'14\n13\n21\n6\n',''),
 'ordinary_group_initializer':(IO+PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{ref y={val z=3;pick(tag,x)};x<-9;return y+z;}val x=1;behave @print(read(false,x));x<-1;behave @print(read(0,x));',0,'12\n4\n',''),
 'scoped_guarded_initializer':(IO+PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{ref y=try{{defer{x<-8;};pick(tag,x)}}finally{x<-9;{}};return y;}val x=1;behave @print(read(false,x));x<-1;behave @print(read(0,x));',0,'9\n1\n',''),
 'conditional_initializer':(IO+PICK+'func read(tag:T,ref x:i64 in O,c:bool)->const i64 where {T,owner O}{ref y=if(c){pick(tag,x)}else{pick(tag,x)};x<-9;return y;}val x=1;behave @print(read(false,x,true));x<-1;behave @print(read(0,x,false));',0,'9\n1\n',''),
 'per_iteration_storage':(IO+PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{val sum=0;for(i in 0..3){{ref y=pick(tag,x);val z=1;x<-x+1;sum<-sum+y+z;}};return sum;}val x=1;behave @print(read(false,x));x<-1;behave @print(read(0,x));',0,'18\n12\n',''),
 'fold_storage':(IO+PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{return fold(i in 0..3,sum=0){{ref y=pick(tag,x);x<-x+1;sum+y}};}val x=1;behave @print(read(false,x));x<-1;behave @print(read(0,x));',0,'15\n9\n',''),
 'recursive_frames':(IO+PICK+'func walk(tag:T,ref x:i64 in O,n:i64)->const i64 where {T,owner O}{ref y=pick(tag,x);val z=n;if(n>0){walk(tag,x,n-1);{}}else{};return y+z;}val x=1;behave @print(walk(false,x,30));x<-1;behave @print(walk(0,x,40));',0,'62\n41\n',''),
 'deferred_local_reads':(IO+PICK+'proc run(tag:T,ref x:i64 in O)->void where {T,owner O}{ref y=pick(tag,x);val z=3;defer{behave @print(y);behave @print(z);};x<-9;}val x=1;run(false,x);x<-1;run(0,x);',0,'9\n3\n1\n3\n',''),
 'deferred_prepared_borrows':(IO+PICK+'proc show(ref y:const i64 in O)->void where {owner O}{behave @print(y);}proc run(tag:T,ref x:i64 in O)->void where {T,owner O}{ref y=pick(tag,x);defer show(y);x<-9;}val x=1;run(false,x);x<-1;run(0,x);',0,'9\n1\n',''),
 'nested_deferred_declarations':(IO+PICK+'proc run(tag:T,ref x:i64 in O)->void where {T,owner O}{defer{{ref y=pick(tag,x);val z=3;x<-9;behave @print(y+z);}};}val x=1;run(false,x);x<-1;run(0,x);',0,'12\n4\n',''),
 'nullable_storage':(IO+NPE+NULL+'proc run(tag:T,ptr x:i64 in O)->void throws NullPointerException where {T,owner O}{ptr y=pick(tag,x);behave @print(isnull(y));try{behave @print(y);}catch(e:const NullPointerException){behave @print(8);};}val x=1;ptr p=x;p=null;try{run(false,p);run(0,p);}catch(e:const NullPointerException){};',0,'true\n8\nfalse\n7\n',''),
 'fixed_binding_alias':(IO+PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{ref y=pick(tag,x);const ref var z->y;x<-9;return z;}val x=1;behave @print(read(false,x));x<-1;behave @print(read(0,x));',0,'9\n1\n',''),
 'same_owner_rebinding':(IO+PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{ref y=pick(tag,x),z=pick(tag,x);y=z;x<-9;return y;}val x=1;behave @print(read(false,x));x<-1;behave @print(read(0,x));',0,'9\n2\n',''),
 'explicit_copy_keeps_local_storage':(IO+PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{ref y<-pick(tag,x);x<-9;return y;}val x=1;behave @print(read(false,x));x<-1;behave @print(read(0,x));',0,'2\n1\n',''),
 'runtime_value_view_join_remains_separate':('func read(ref x:i64 in O,c:bool)->const i64 where {owner O}{ref y=if(c){x}else{1};return y;}val x=1;read(x,true);',2,'','reference join mixing fresh values and storage views'),
 'initializer_effects_once':(IO+PICK+'proc run(tag:T,ref x:i64 in O)->void where {T,owner O}{ref y=pick({behave @print(3);tag},{behave @print(4);x});behave @print(5);x<-9;behave @print(y);}val x=1;run(false,x);x<-1;run(0,x);',0,'3\n4\n5\n9\n3\n4\n5\n1\n',''),
 'owner_contract_remains_checked':(PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{ref y in O=pick(tag,x);return y;}val x=1;read(false,x);read(0,x);',2,'','fresh storage with an explicit external owner'),
 'owner_cannot_change':(PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{ref y=pick(tag,x);y=x;return y;}val x=1;read(false,x);read(0,x);',2,'','initializer-fixed owner'),
 'readonly_borrow_remains_checked':('func pick(tag:bool,ref x:const i64 in O)->ref const i64 in O where {owner O}{return x;}func pick(tag:i64,ref x:i64 in O)->const i64 where {owner O}{return x;}func read(tag:T,ref x:i64 in O)->void where {T,owner O}{ref y=pick(tag,x);y<-2;}val x=1;read(0,x);read(false,x);',2,'','readonly destination'),
 'local_borrow_cannot_escape':(PICK+'func read(tag:T,ref x:i64 in O)->ref i64 in O where {T,owner O}{ref y=pick(tag,x);return y;}val x=1;read(false,x);read(0,x);',2,'','declared external owner'),
 'initialization_remains_checked':(PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{ref y=pick(tag,x);val z=make local i64;return y+z;}val x=1;read(false,x);read(0,x);',2,'','before definite initialization'),
 'complete_callable_local_or_borrowed':(IO+CALLABLES+'func read(tag:T,ref f:func(u64)->const u64 in O)->const u64 where {T,owner O}{ptr p=pick_fn(tag,f);func g=wrap(base);f<-base;return p(1u64)+g(1u64);}{{func f=wrap(base);behave @print(read(false,f));}};{{func f=wrap(base);behave @print(read(0,f));}};',0,'3\n4\n',''),
 'callable_slot_replacement':(IO+CALLABLES+'func read(tag:T,ref f:func(u64)->const u64 in O)->const u64 where {T,owner O}{ptr p=pick_fn(tag,f);p<-wrap(other);return f(1u64)+p(1u64);}{{func f=wrap(base);behave @print(read(false,f));}};{{func f=wrap(base);behave @print(read(0,f));}};',0,'24\n24\n',''),
 'caught_snapshot_after_contextual_storage':(IO+PICK+'using "std:exceptions" MissingReturnException;func missing()->const i64{}proc run(tag:T,ref x:i64 in O)->void where {T,owner O}{ref y=pick(tag,x);try{missing();{}}catch(e:const MissingReturnException){defer{behave @print(e.category);behave @print(y);};};x<-9;}val x=1;run(false,x);x<-1;run(0,x);',0,'5\n9\n5\n1\n','[-Wmissing-return]'),
 'local_captured_callable_after_storage':(IO+CALLABLES+PICK+'func read(tag:T,ref x:i64 in O)->const u64 where {T,owner O}{ref y=pick(tag,x);func g=wrap(base);func h=func[g](v:u64)->const u64{return g(v);};return h(1u64);}val x=1;behave @print(read(false,x));behave @print(read(0,x));',0,'2\n2\n',''),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__), Path(contracts.__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
