#!/usr/bin/env python3
"""Selected aliases retain slot identity and independent permissions through effects."""
from pathlib import Path
import foundation_tests as runner

IO='using symbol "std:io" @print;'
NPE='using "std:exceptions" NullPointerException;'
SETUP='val x=1,y=2,z=3;ptr a=x,b=y,c=z;'
SHOW='behave @print(a);behave @print(b);behave @print(c);'
FUNCTIONS='func one()->const i64{return 1;}func two()->const i64{return 2;}func three()->const i64{return 3;}'
def run(body,setup=SETUP,show=SHOW):
 return IO+'proc run(flag:bool)->void{'+setup+body+show+'}run(true);run(false);'
runner.CASES={
 'reference_selection':(run('var p->if(flag){a}else{b};p=z;'),0,'3\n2\n3\n1\n3\n3\n',''),
 'selection_stays_fixed':(run('var p->if(flag){a}else{b};flag<-!flag;p=z;'),0,'3\n2\n3\n1\n3\n3\n',''),
 'selection_once':(run('var p->{behave @print(9);if(flag){a}else{b}};p=z;p=y;'),0,'9\n2\n2\n3\n9\n1\n2\n3\n',''),
 'alias_chain':(run('var p->if(flag){a}else{b};var q->p;q=z;behave @print(p);'),0,'3\n3\n2\n3\n3\n1\n3\n3\n',''),
 'original_rebind_observed':(run('var p->if(flag){a}else{b};a=z;b=x;behave @print(p);',show=''),0,'3\n1\n',''),
 'independent_selections':(run('var p->if(flag){a}else{b};var q->if(flag){b}else{a};p=z;behave @print(q);',show=''),0,'2\n1\n',''),
 'nested_selection':(run('var p->if(flag){a}else{b};var q->if(flag){p}else{c};q=y;behave @print(p);'),0,'2\n2\n2\n3\n2\n1\n2\n2\n',''),
 'scope_selection_precedes_cleanup':(run('var p->{{defer{a=z;b=x;};if(flag){a}else{b}}};behave @print(p);',show=''),0,'3\n1\n',''),
 'finally_selection_precedes_cleanup':(run('var p->try{if(flag){a}else{b}}finally{a=z;b=x;{}};behave @print(p);',show=''),0,'3\n1\n',''),
 'finally_value_selection':(run('var p->try{if(flag){x}else{y}}finally{x<-21;y<-22;{}};p<-23;',setup='val x=1,y=2;',show='behave @print(x);behave @print(y);'),0,'23\n22\n21\n23\n',''),
 'catch_selects_slot_before_finally':(NPE+run('ptr bad:i64=null;var p->try{if(flag){bad<-1;{}}else{};a}catch(e:const NullPointerException){b}finally{a=z;b=x;{}};behave @print(p);',show=''),0,'1\n3\n',''),
 'alias_name_scope_can_end':(run('var p->{{var inner->if(flag){a}else{b};inner}};p=z;'),0,'3\n2\n3\n1\n3\n3\n',''),
 'ended_alias_keeps_null_refinement':(run('var p->{{var inner->if(flag){a}else{b};inner}};if(isnull(p)){}else{behave @print(p);};',show=''),0,'1\n2\n',''),
 'value_selection':(run('val var p->if(flag){x}else{y};p<-7;',setup='val x=1,y=2;',show='behave @print(x);behave @print(y);'),0,'7\n2\n1\n7\n',''),
 'value_selection_chain':(run('var p->if(flag){x}else{y};var q->p;q<-8;behave @print(p);',setup='val x=1,y=2;',show=''),0,'8\n8\n',''),
 'selected_value_initialization':(run('var p->if(flag){x}else{y};p<-9;behave @print(p);',setup='val x=make local i64,y=make local i64;',show=''),0,'9\n9\n',''),
 'ended_alias_keeps_initialization':(run('var p->{{var inner->if(flag){x}else{y};inner}};p<-10;behave @print(p);',setup='val x=make local i64,y=make local i64;',show=''),0,'10\n10\n',''),
 'selected_pointer_initialization':(run('var p->if(flag){a}else{b};p<-11;behave @print(p);',setup='val x=make local i64,y=make local i64;ptr a=x,b=y;',show=''),0,'11\n11\n',''),
 'nullable_alias_does_not_read':(run('var p->if(flag){a}else{b};behave @print(isnull(p));p=x;behave @print(p);',setup='val x=12;ptr a:i64=null,b:i64=null;',show=''),0,'true\n12\ntrue\n12\n',''),
 'alias_write_selects_target_before_rhs':(run('var p->if(flag){a}else{b};p<-{a=z;b=x;13};',show='behave @print(x);behave @print(y);behave @print(z);'),0,'13\n2\n3\n1\n13\n3\n',''),
 'reference_swap':(run('var p->if(flag){a}else{b};p<=>c;behave @print(p);'),0,'3\n3\n2\n1\n3\n1\n3\n2\n',''),
 'reference_swap_reloads':(run('var p->if(flag){a}else{b};p<=>{a=z;b=x;c};'),0,'3\n1\n3\n3\n3\n1\n',''),
 'same_alias_swap':(run('var p->if(flag){a}else{b};var q->p;p<=>q;'),0,'1\n2\n3\n1\n2\n3\n',''),
 'bits_swap':(run('var p->if(flag){x}else{y};p<->z;',setup='val x=1,y=2,z=3;',show='behave @print(x);behave @print(y);behave @print(z);'),0,'3\n2\n1\n1\n3\n2\n',''),
 'readonly_alias_does_not_freeze_original':(run('var p:const->if(flag){a}else{b};a=z;b=x;behave @print(p);',show=''),0,'3\n1\n',''),
 'fixed_alias_can_write_target':(run('const var p->if(flag){a}else{b};p<-14;',show='behave @print(x);behave @print(y);'),0,'14\n2\n1\n14\n',''),
 'deferred_current_binding':(run('var p->if(flag){a}else{b};defer{behave @print(p);};a=z;b=x;',show=''),0,'3\n1\n',''),
 'deferred_rebind':(run('var p->if(flag){a}else{b};{{defer{p=z;};}};'),0,'3\n2\n3\n1\n3\n3\n',''),
 'initialization_inside_deferred_action':(run('var p->if(flag){a}else{b};defer{p<-15;behave @print(p);};',setup='val x=make local i64,y=make local i64;ptr a=x,b=y;',show=''),0,'15\n15\n',''),
 'post_cleanup_selected_initialization_unproved':(run('var p->if(flag){a}else{b};{{defer{p<-15;};}};behave @print(p);',setup='val x=make local i64,y=make local i64;ptr a=x,b=y;',show=''),2,'','before definite initialization'),
 'loop_local_alias':(run('for(i in 0..2){ {{var p->if(flag){a}else{b};p=z;flag<-!flag;}}; };'),0,'3\n3\n3\n3\n3\n3\n',''),
 'generic_borrowed_alias':(IO+'func run(ref x:T in O,ref y:T in O,flag:bool)->const T where {T,owner O}{var p->if(flag){x}else{y};return p;}val a=16,b=17,c=false,d=true;behave @print(run(a,b,true));behave @print(run(c,d,false));',0,'16\ntrue\n',''),
 'recursive_borrowed_alias':(IO+'func run(ref x:i64 in O,ref y:i64 in O,n:u64)->const i64 where {owner O}{var p->if(n%2u64==0u64){x}else{y};if(n==0u64){p<-18;return p;}else{return run(p,y,n-1u64);};}val x=1,y=2;behave @print(run(x,y,30u64));',0,'18\n',''),
 'call_initializes_selected_alias':(IO+NPE+'proc set(ptr x:i64 in O)->void throws NullPointerException where {owner O}{x<-19;}proc run(flag:bool)->void{val x=make local i64,y=make local i64;ptr a=x,b=y;var p->if(flag){a}else{b};try{set(p);behave @print(p);}catch(e:const NullPointerException){};}run(true);run(false);',0,'19\n19\n',''),
 'callable_value_alias':(run('var p->if(flag){f}else{g};behave @print(p());p<-three;behave @print(f());behave @print(g());',setup='func f=one;func g=two;',show='').replace(IO,IO+FUNCTIONS,1),0,'1\n3\n2\n2\n1\n3\n',''),
 'callable_pointer_alias':(run('var p->if(flag){a}else{b};p=h;behave @print(p());',setup='func f=one;func g=two;func h=three;ptr a=f,b=g;',show='').replace(IO,IO+FUNCTIONS,1),0,'3\n3\n',''),
 'callable_alias_snapshot_capture':(run('var p->if(flag){f}else{g};func h=func[p]()->const i64{return p();};p<-three;behave @print(h());behave @print(p());',setup='func f=one;func g=two;',show='').replace(IO,IO+FUNCTIONS,1),0,'1\n3\n2\n3\n',''),
 'query_alias_declaration_evidence':(run('var p->if(flag){a}else{b};func f=func()->const i64{val x=make local typeof(p){20};return x;};behave @print(f());',show=''),0,'20\n20\n',''),
 'kind_assertion_rejects_ptr':('val x=1;ptr a=x,b=x;ref var p->if(true){a}else{b};',2,'','must match the selected ptr storage'),
 'mixed_storage_kinds_unproved':('val x=1;ptr a=x;ref b=x;var p->if(true){a}else{b};',2,'','different storage kinds'),
 'alias_keeps_actual_ptr_kind':('val x=1;ptr a=x,b=x;ptr var p->if(true){a}else{b};ref var q->p;',2,'','must match the selected ptr storage'),
 'value_alias_cannot_rebind':('val x=1,y=2;var p->if(true){x}else{y};p=x;',2,'','val storage has no reference binding'),
 'fixed_alias_cannot_rebind':('val x=1;ptr a=x,b=x;const var p->if(true){a}else{b};p=x;',2,'','const reference binding'),
 'fixed_alternative_cannot_be_laundered':('val x=1;const ptr a=x;ptr b=x;var p->if(true){a}else{b};p=x;',2,'','const reference binding'),
 'readonly_alias_cannot_write':('val x=1;ptr a=x,b=x;var p:const->if(true){a}else{b};p<-2;',2,'','readonly destination'),
 'readonly_alias_keeps_physical_constraints':('val x=1;const val y=2;ptr a=x,b=x;var p:const->if(true){a}else{b};p=y;',2,'','readonly source cannot rebind'),
 'readonly_annotation_cannot_escalate':('val x=1;ptr a:const i64=x,b:const i64=x;var p:i64->if(true){a}else{b};',2,'','readonly source cannot supply a writable alias'),
 'type_assertion_does_not_convert':('val x=1u8,y=2u8;var p:u64->if(true){x}else{y};',2,'','exact selected type'),
 'ref_alias_requires_nonnull':('val x=1;ref a=x,b=x;var p->if(true){a}else{b};p=null;',2,'','ref requires a proven non-null target'),
 'ref_alias_requires_initialized':('val x=1,y=make local i64;ref a=x,b=x;var p->if(true){a}else{b};p=y;',2,'','before definite initialization'),
 'physical_mutation_invalidates_alias_initialization':('func run(flag:bool)->const i64{val x=make local i64,y=make local i64,z=make local i64;ptr a=x,b=y;var p->if(flag){a}else{b};p<-1;a=z;return p;}',2,'','before definite initialization'),
 'physical_mutation_invalidates_alias_null_proof':(NPE+'func run(flag:bool)->const i64{val x=1;ptr a=x,b=x;var p->if(flag){a}else{b};a=null;return p;}',2,'','requires a covering catch or throws'),
 'alias_mutation_invalidates_original_null_proof':(NPE+'func run(flag:bool)->const i64{val x=1;ptr a=x,b=x;var p->if(flag){a}else{b};p=null;return a;}',2,'','requires a covering catch or throws'),
 'overlapping_alias_mutation_invalidates_null_proof':(NPE+'func run(flag:bool)->const i64{val x=1;ptr a=x,b=x;var p->if(flag){a}else{b};var q->if(flag){b}else{a};p=null;return q;}',2,'','requires a covering catch or throws'),
 'selected_initialization_does_not_initialize_every_slot':('func run(flag:bool)->const i64{val x=make local i64,y=make local i64;var p->if(flag){x}else{y};p<-1;return x;}',2,'','before definite initialization'),
 'different_target_owners_unproved':('val x=1;{{val y=2;ptr a=x,b=y;var p->if(true){a}else{b};}};',2,'','different target owners'),
 'expired_physical_slots_rejected':('val x=1;var p->{{ptr a=x,b=x;if(true){a}else{b}}};',2,'',"aliased binding's lexical scope has ended"),
 'expired_value_storage_rejected':('var p->{{val x=1,y=2;if(true){x}else{y}}};',2,'',"aliased binding's lexical scope has ended"),
 'owner_assertion_checks_slots':('func run(ref x:i64 in O,ref y:i64 in O)->void where {owner O}{var p in O->if(true){x}else{y};}',2,'','selected binding storage, not its target owner'),
 'ordinary_capture_still_rejected':('func run(flag:bool)->void{val x=1,y=2;var p->if(flag){x}else{y};func f=func[p]()->const i64{return p;};}',2,'','ordinary lexical cells'),
}
runner.CASE_ARGUMENTS={}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
 raise SystemExit(runner.main())
