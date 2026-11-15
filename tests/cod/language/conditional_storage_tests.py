#!/usr/bin/env python3
"""Scalar receivers preserve selected targets or values across control flow."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
OUT = lambda value: 'behave @print('+value+');'
def run(body):
    return IO+'proc run(flag:bool)->void{'+body+'}run(true);run(false);'
BASE = 'val x<-1,z<-3;'
PICK = 'func pick(tag:bool,ref x:i64 in O)->ref i64 in O where {owner O}{return x;}func pick(tag:i64,ref x:i64 in O)->i64 where {owner O}{return x;}'
NPE = 'using "std:exceptions" NullPointerException;'
MISSING = 'using "std:exceptions" MissingReturnException;func missing()->i64{}'
runner.CASES = {
 'select_target_or_value': (run(BASE+'ref y=if(flag){x}else{7};x<-9;'+OUT('y')+'y<-4;'+OUT('x')),0,'9\n4\n7\n9\n',''),
 'reverse_branches': (run(BASE+'ref y=if(flag){7}else{x};x<-9;'+OUT('y')),0,'7\n9\n',''),
 'pointer_receiver': (run(BASE+'ptr y=if(flag){x}else{7};x<-9;'+OUT('y')+'y=z;'+OUT('y')),0,'9\n3\n7\n3\n',''),
 'target_identity': (run(BASE+'ref y=if(flag){x}else{7};'+OUT('y is x')+OUT('y is y')),0,'true\ntrue\nfalse\ntrue\n',''),
 'alias_of_receiver': (run(BASE+'ref y=if(flag){x}else{7};ref var alias->y;alias=z;'+OUT('y')+OUT('x')),0,'3\n1\n3\n1\n',''),
 'ordinary_group': (run(BASE+'ref y={val extra<-2;if(flag){x}else{7}};x<-9;'+OUT('y+extra')),0,'11\n9\n',''),
 'scope_cleanup_keeps_selection': (run(BASE+'ptr p=x;ref y={{defer{p=z;x<-9;};if(flag){p}else{x+1}}};'+OUT('y')+OUT('p')),0,'9\n3\n2\n3\n',''),
 'finally_keeps_selection': (run(BASE+'ref y=try{if(flag){x}else{x+1}}finally{x<-9;{}};'+OUT('y')),0,'9\n2\n',''),
 'nested_joins': (run(BASE+'ref y=if(flag){if(flag){x}else{2}}else{try{7}finally{z<-5;{}}};x<-9;'+OUT('y')+OUT('z')),0,'9\n3\n7\n5\n',''),
 'catch_supplies_value': (MISSING+run(BASE+'ref y=try{if(flag){x}else{missing()}}catch(e:const MissingReturnException){7};x<-9;'+OUT('y')),0,'9\n7\n','[-Wmissing-return]'),
 'catch_supplies_target': (MISSING+run(BASE+'ref y=try{if(flag){missing()}else{7}}catch(e:const MissingReturnException){x};x<-9;'+OUT('y')),0,'9\n7\n','[-Wmissing-return]'),
 'once_in_order': (run(BASE+'ref y=if({'+OUT('1')+'flag}){{'+OUT('2')+'defer{'+OUT('4')+'};x}}else{{'+OUT('3')+'defer{'+OUT('4')+'};7}};'+OUT('y')),0,'1\n2\n4\n1\n1\n3\n4\n7\n',''),
 'grouped_receivers': (run(BASE+'ref a=if(flag){x}else{7},b=if(flag){8}else{z};x<-9;z<-10;'+OUT('a')+OUT('b')),0,'9\n8\n7\n10\n',''),
 'fixed_binding': (run(BASE+'const ref y=if(flag){x}else{7};y<-4;'+OUT('x')+OUT('y')),0,'4\n4\n1\n4\n',''),
 'readonly_target': (run('const val x<-1;ref y=if(flag){x}else{7};'+OUT('y')),0,'1\n7\n',''),
 'deferred_local_read': (run(BASE+'ref y=if(flag){x}else{7};defer{'+OUT('y')+'};x<-9;'),0,'9\n7\n',''),
 'prepared_borrow': (IO+'proc show(ref value:const i64 in O)->void where {owner O}{'+OUT('value')+'}'+run(BASE+'ref y=if(flag){x}else{7};defer show(y);x<-9;').removeprefix(IO),0,'9\n7\n',''),
 'deferred_block_receiver': (run('defer{{val x<-1;ref y=if(flag){x}else{7};x<-9;'+OUT('y')+'}};'),0,'9\n7\n',''),
 'per_iteration': (run('val sum<-0;for(i in 0..4){{val x<-1;ref y=if(flag){x}else{7};x<-9;sum<-sum+y;}};'+OUT('sum')),0,'36\n28\n',''),
 'scope_owner_is_receiver': (run(BASE+'ref y={{if(flag){x}else{7}}};y=z;'+OUT('y')),0,'3\n3\n',''),
 'selected_target_initialization': (run('val x<-make local i64,z<-make local i64;ptr selected=if(flag){x}else{z};selected<-9;ref y=if(flag){selected}else{7};'+OUT('y')),0,'9\n7\n',''),
 'guarded_target_initialization': (MISSING+run('val x<-make local i64,z<-make local i64;ptr selected=if(flag){x}else{z};selected<-9;ref y=try{if(flag){selected}else{missing()}}catch(e:const MissingReturnException){7};'+OUT('y')),0,'9\n7\n','[-Wmissing-return]'),
 'conditional_initialization': (run('val x<-make local i64;ptr y=if(flag){x}else{7};y<-9;'+OUT('y')),0,'9\n9\n',''),
 'cleanup_initializes_target': (run('val x<-make local i64;ref y={{defer{x<-9;};if(flag){x}else{7}}};'+OUT('y')),0,'9\n7\n',''),
 'nullable_target': (NPE+run('ptr absent:i64=null in here;ptr y=if(flag){absent}else{7};'+OUT('y is null')+'try{'+OUT('y')+'}catch(e:const NullPointerException){'+OUT('8')+'};'),0,'true\n8\nfalse\n7\n',''),
 'explicit_copy_stays_independent': (run(BASE+'val y<-if(flag){x}else{7};x<-9;'+OUT('y')),0,'1\n7\n',''),
 'recursive_generic_modes': (IO+PICK+'func work(tag:T,flag:bool,n:u64)->i64 where T{val local<-1;ref y=if(flag){pick(tag,local)}else{7};local<-9;if(n==0u64){return y;}else{return work(tag,flag,n-1u64);};}'+OUT('work(false,true,30u64)')+OUT('work(0,true,30u64)')+OUT('work(false,false,30u64)')+OUT('work(0,false,30u64)'),0,'9\n1\n7\n7\n',''),
 'receiver_type_query': (run(BASE+'ref y=if(flag){x}else{7};val copied<-make local typeof(y){3};'+OUT('copied')),0,'3\n3\n',''),
 'result_owner_evidence': ('func same(ref a:i64 in O,ref b:i64 in O)->i64 where {owner O}{return a+b;}'+IO+'func work(flag:bool){val x<-1;ref y=if(flag){x}else{7};return same(y,x);}'+OUT('work(true)')+OUT('work(false)'),0,'2\n8\n',''),
 'abrupt_branch': (IO+'proc work(flag:bool)->i64{val x<-1;ref y=if(flag){x}else{{return 7;}};return y;}'+OUT('work(true)')+OUT('work(false)'),0,'1\n7\n',''),
 'cleanup_return_abandons_receiver': (IO+'proc work(flag:bool)->i64{val x<-1;ref y:i64={{defer{return 8;};if(flag){x}else{7}}};return y;}'+OUT('work(true)')+OUT('work(false)'),0,'8\n8\n',''),
 'abrupt_value_initializer': (IO+'proc work()->i64{ref y:i64={{defer{return 8;};7}};return 0;}'+OUT('work()'),0,'8\n',''),
 'abrupt_owner_initializer': (IO+'proc work(ref x:i64 in O)->i64 where {owner O}{ref y:i64 in O={{return 8;}};return 0;}val input<-1;'+OUT('work(input)'),0,'8\n',''),
 'abrupt_mixed_target_branch': (IO+'proc work(flag:bool)->i64{val x<-1;ref y=if(flag){{defer{return 8;};if(flag){x}else{7}}}else{x};return y;}'+OUT('work(true)')+OUT('work(false)'),0,'8\n1\n',''),
 'abrupt_mixed_slot_branch': (IO+'proc work(flag:bool)->i64{val x<-1,z<-7;ptr p=x;(if(flag){{defer{return 8;};if(flag){p}else{7}}}else{p})=z;return p;}'+OUT('work(true)')+OUT('work(false)'),0,'8\n7\n',''),
 'abrupt_mixed_borrow_argument': (IO+'func read(ref value:i64 in O)->i64 where {owner O}{return value;}proc work(flag:bool)->i64{val x<-1;return read(if(flag){{defer{return 8;};if(flag){x}else{7}}}else{x});}'+OUT('work(true)')+OUT('work(false)'),0,'8\n1\n',''),
 'abrupt_mixed_shared_context': (IO+'func independent(value:T)->const T where T{return value;}func work(seed:T,flag:bool)->const T where T{val local<-seed;ref y=if(flag){{defer{return seed;};if(flag){local}else{independent(seed)}}}else{local};return y;}'+OUT('work(true,true)')+OUT('work(7u8,false)'),0,'true\n7\n',''),
 'abrupt_value_still_reads': ('func work(flag:bool)->i64{val raw<-make local i64;val y:i64<-if(flag){{defer{return 8;};if(flag){raw}else{7}}}else{1};return y;}',2,'','before definite initialization'),
 'abrupt_mixed_branch': (IO+'proc work(flag:bool)->i64{val x<-1;ref y=if(flag){{defer{return 8;};if(flag){x}else{7}}}else{if(flag){x}else{7}};return y;}'+OUT('work(true)')+OUT('work(false)'),0,'8\n7\n',''),
 'deferred_outer_owner_rejected': (run(BASE+'defer{{ref y=if(flag){x}else{7};}};'),2,'','reference initializer joins different owners'),
 'scalar_representations': (IO+'func independent(value:T)->const T where T{return value;}func work(value:T,flag:bool)->const T where T{val local<-value;ref y=if(flag){local}else{independent(value)};return y;}'+OUT('work(false,false)')+OUT('work(true,true)')+OUT('work(7u8,false)')+OUT('work(8u32,true)')+OUT('work(1f32,false)')+OUT('work(2f64,true)'),0,'false\ntrue\n7\n8\n1\n2\n',''),
 'exception_payload': (IO+MISSING+'using "std:exceptions" SystemException;func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}func independent(value:SystemException)->const SystemException{return value;}proc work(flag:bool)->void{val original:SystemException<-error();ref y=if(flag){original}else{independent(original)};'+OUT('y is original')+OUT('y.category')+'}work(true);work(false);',0,'true\n5\nfalse\n5\n','[-Wmissing-return]'),
 'callable_mixture_boundary': ('func base()->u64{return 1u64;}func value=base;ptr p=if(true){value}else{base};',2,'','mixed callable initializer completion transport'),
 'different_owners': ('val x<-1;{{ref y=if(true){x}else{7};}};',2,'','reference initializer joins different owners'),
 'external_owner': ('func work(ref x:i64 in O,flag:bool)->void where {owner O}{ref y=if(flag){x}else{7};}',2,'','reference initializer joins different owners'),
 'readonly_write': ('const val x<-1;ref y=if(true){x}else{7};y<-9;',2,'','readonly destination'),
 'readonly_escalation': ('const val x<-1;ref y:i64=if(true){x}else{7};',2,'','readonly source cannot initialize'),
 'uninitialized_reference': ('val x<-make local i64;ref y=if(true){x}else{7};',2,'','before definite initialization'),
 'nullable_reference': ('ptr absent:i64=null in here;ref y=if(true){absent}else{7};',2,'','ref requires a proven non-null target'),
 'does_not_initialize_other_target': ('val x<-make local i64;ptr y=if(true){x}else{7};y<-9;x;',2,'','before definite initialization'),
 'binding_stays_fixed': ('val x<-1;const ref y=if(true){x}else{7};y=x;',2,'','const reference binding'),
 'returned_local_still_invalid': ('func work(ref x:i64 in O)->ref i64 in O where {owner O}{val local<-1;ref y=if(true){local}else{7};return y;}',2,'','declared external owner'),
 'borrow_argument_does_not_materialize': ('func take(ref x:i64 in O)->void where {owner O}{}val x<-1;take(if(true){x}else{7});',2,'','borrowed arguments require existing target views'),
 'assignment_does_not_materialize': ('val x<-1;ptr p=x;p=if(true){x}else{7};',2,'','fresh value without an existing target'),
}
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {'borrow_argument_does_not_materialize':[]}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
