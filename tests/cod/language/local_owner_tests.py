#!/usr/bin/env python3
"""Explicit reference owners remain fixed independently of target presence."""
from pathlib import Path
import foundation_tests as runner

IO='using symbol "std:io" @print;\n'
EX='using "std:exceptions" NullPointerException;\n'
runner.CASES={
 'existing_pointer_target':(IO+'proc set(x:i64 in O)->void where {owner O}{ptr p:i64 in O=x;p<-4;}val x<-1;set(x);behave @print(x);',0,'4\n',''),
 'existing_reference_target':(IO+'proc set(ref x:i64 in O)->void where {owner O}{ref p:i64 in O=x;p<-5;}val x<-1;set(x);behave @print(x);',0,'5\n',''),
 'null_then_target':(IO+'proc set(x:i64 in O)->void where {owner O}{ptr p:i64 in O=null;p=x;p<-6;}val x<-1;set(x);behave @print(x);',0,'6\n',''),
 'clear_and_restore':(IO+'proc set(x:i64 in O)->void where {owner O}{ptr p:i64 in O=x;p=null;p=x;p<-7;}val x<-1;set(x);behave @print(x);',0,'7\n',''),
 'absent_view_has_owner':(IO+'func absent(x:i64 in O)->ptr i64 in O where {owner O}{ptr p:i64 in O=null;return p;}val x<-1;ptr p=absent(x);behave @print(isnull(p));p=x;p<-8;behave @print(x);',0,'true\n8\n',''),
 'nullable_result_does_not_read':(IO+EX+'func absent(ptr x:i64 in O)->ptr i64 in O where {owner O}{ptr p:i64 in O=null;return p;}val x<-make local i64;ptr p=absent(x);behave @print(isnull(p));',0,'true\n',''),
 'null_access_still_checked':(IO+EX+'proc run(x:i64 in O)->void where {owner O}{ptr p:i64 in O=null;try{p<-99;{}}catch(e:const NullPointerException){behave @print(1);};p=x;p<-9;}val x<-0;run(x);behave @print(x);',0,'1\n9\n',''),
 'borrowed_uninitialized_destination':(IO+EX+'proc set(ptr x:i64 in O)->void throws NullPointerException where {owner O}{ptr p:i64 in O=null;p=x;p<-10;}val x<-make local i64;try{set(x);behave @print(x);}catch(e:const NullPointerException){};',0,'10\n',''),
 'external_owner_outlives_alias_scope':(IO+'func select(x:i64 in O)->ref i64 in O where {owner O}{return {{ref p:i64 in O=x;p}};}val x<-1;select(x)<-11;behave @print(x);',0,'11\n',''),
 'same_owner_conditional_target':(IO+'proc run(x:i64 in O,y:i64 in O,flag:bool)->void where {owner O}{ptr p:i64 in O=null;p=if(flag){x}else{y};p<-12;}val x<-1,y<-2;run(x,y,false);behave @print(x);behave @print(y);',0,'1\n12\n',''),
 'generic_owner_and_type':(IO+'func view(x:T in O)->ref T in O where {T,owner O}{ref p:T in O=x;return p;}val x<-13,y<-true;behave @print(view(x));behave @print(view(y));',0,'13\ntrue\n',''),
 'recursive_owner':(IO+'proc run(x:i64 in O,n:i64)->void where {owner O}{ptr p:i64 in O=x;if(n>0){run(p,n-1);}else{p<-14;{}};}val x<-0;run(x,30);behave @print(x);',0,'14\n',''),
 'per_iteration_alias_scope':(IO+'proc run(x:i64 in O)->void where {owner O}{for(i in 0 .. 3){{ptr p:i64 in O=x;p<-p+1;}};}val x<-12;run(x);behave @print(x);',0,'15\n',''),
 'deferred_annotated_alias':(IO+'proc run(x:i64 in O)->void where {owner O}{defer{{ptr p:i64 in O=x;p<-16;}};}val x<-0;run(x);behave @print(x);',0,'16\n',''),
 'late_deferred_target':(IO+EX+'proc run(x:i64 in O)->void where {owner O}{ptr p:i64 in O=null;defer{try{p<-17;{}}catch(e:const NullPointerException){behave @print(99);};};p=x;}val x<-0;run(x);behave @print(x);',0,'17\n',''),
 'owner_name_resolved_before_binding':(IO+'func run(x:i64 in O)->const bool where {owner O}{return {{ptr O:i64 in O=null;isnull(O)}};}val x<-0;behave @print(run(x));',0,'true\n','[-Wshadow]'),
 'readonly_borrow':(IO+'func read(x:const i64 in O)->const i64 where {owner O}{ref p:const i64 in O=x;return p;}val x<-18;behave @print(read(x));',0,'18\n',''),
 'callable_holder_owner':(IO+'func one()->const i64{return 1;}func two()->const i64{return 2;}proc replace(ref f:func()->const i64 in O,g:func()->const i64)->void where {owner O}{ptr p:func()->const i64 in O=f;p<-g;}func f=one;replace(f,two);behave @print(f());',0,'2\n',''),
 'same_owner_grouped_aliases':(IO+'proc run(x:i64 in O)->void where {owner O}{ptr a:i64 in O=null,b:i64 in O=x;a=b;a<-19;}val x<-0;run(x);behave @print(x);',0,'19\n',''),
 'wrong_initial_owner':('proc run(x:i64 in O,y:i64 in P)->void where {owner O,owner P}{ptr p:i64 in O=y;}',2,'','does not belong to the declared owner'),
 'null_cannot_acquire_other_owner':('proc run(x:i64 in O,y:i64 in P)->void where {owner O,owner P}{ptr p:i64 in O=null;p=y;}',2,'','initializer-fixed owner'),
 'clearing_does_not_forget_owner':('proc run(x:i64 in O,y:i64 in P)->void where {owner O,owner P}{ptr p:i64 in O=x;p=null;p=y;}',2,'','initializer-fixed owner'),
 'owned_null_initializer_mismatch':('proc run(x:i64 in O,y:i64 in P)->void where {owner O,owner P}{ptr q:i64 in P=null;ptr p:i64 in O=q;}',2,'','does not belong to the declared owner'),
 'local_null_cannot_claim_external_owner':(IO+'func run(x:i64 in O)->const bool where {owner O}{ptr q:i64=null in here;ptr p:i64 in O=q;return isnull(p);}val x<-0;behave @print(run(x));',2,'','does not belong to the declared owner'),
 'nearest_nonowner_shadow':('proc run(x:i64 in O)->void where {owner O}{{val O<-1;ptr p:i64 in O=null;}}',2,'','nearest binding \'O\' is not an owner'),
 'unknown_owner':('ptr p:i64 in Unknown=null;',2,'','unknown owner'),
 'scalar_is_not_owner':('val O<-1;ptr p:i64 in O=null;',2,'','nearest binding \'O\' is not an owner'),
 'val_cannot_change_storage_owner':('proc run(x:i64 in O)->void where {owner O}{val y:i64 in O<-1;}',2,'','val storage belongs to its lexical scope'),
 'fresh_external_storage_gap':('proc run(x:i64 in O)->void where {owner O}{ptr p:i64 in O=1;}',2,'','fresh storage with an explicit external owner'),
 'readonly_cannot_be_laundered':('proc run(x:const i64 in O)->void where {owner O}{ptr p:i64 in O=x;}',2,'','readonly source cannot initialize a writable reference'),
 'readonly_target_stays_readonly':('proc run(x:i64 in O)->void where {owner O}{ptr p:const i64 in O=x;p<-1;}',2,'','readonly destination'),
 'ref_still_requires_nonnull':('proc run(ptr x:i64 in O)->void where {owner O}{ref p:i64 in O=x;}',2,'','ref requires a proven non-null target'),
 'annotation_does_not_initialize':('proc run(ptr x:i64 in O)->void where {owner O}{if(isnull(x)){}else{ref p:i64 in O=x;};}val x<-make local i64;run(x);',2,'','initializ'),
 'returned_owner_cannot_escape':('func view(x:i64 in O)->ptr i64 in O where {owner O}{ptr p:i64 in O=x;return p;}ptr p={{val x<-1;view(x)}};p;',2,'','lexical owner has ended'),
}
runner.CASE_ARGUMENTS={}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
 raise SystemExit(runner.main())
