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
MISSING = 'using "std:exceptions" MissingReturnError;func missing()->i64{}'
runner.CASES = {
 'select_target_or_value': (run(BASE+'ref y=if(flag){x}else{7};x<-9;'+OUT('y')+'y<-4;'+OUT('x')),0,'1\n9\n7\n9\n',''),
 'reverse_branches': (run(BASE+'ref y=if(flag){7}else{x};x<-9;'+OUT('y')),0,'7\n1\n',''),
 'pointer_receiver': (run(BASE+'ptr y=if(flag){x}else{7};x<-9;'+OUT('y')+'y=z;'+OUT('y')),0,'1\n3\n7\n3\n',''),
 'target_identity': (run(BASE+'ref y=if(flag){x}else{7};'+OUT('y is x')+OUT('y is y')),0,'false\ntrue\nfalse\ntrue\n',''),
 'same_owner_targets': (run(BASE+'ref y=if(flag){x}else{z};y<-9;'+OUT('x')+OUT('z')),0,'9\n3\n1\n9\n',''),
 'grouped_arm_supplies_bits': (run(BASE+'ref y=if(flag){x}else{(z)};y<-9;'+OUT('x')+OUT('z')),0,'1\n3\n1\n3\n',''),
 'grouped_arm_supplies_bits_reversed': (run(BASE+'ref y=if(flag){(x)}else{z};y<-9;'+OUT('x')+OUT('z')),0,'1\n3\n1\n3\n',''),
 'alias_of_receiver': (run(BASE+'ref y=if(flag){x}else{7};ref var alias->y;alias=z;'+OUT('y')+OUT('x')),0,'3\n1\n3\n1\n',''),
 'ordinary_group': (run(BASE+'ref y={val extra<-2;if(flag){x}else{7}};x<-9;'+OUT('y+extra')),0,'3\n9\n',''),
 'scope_cleanup_keeps_pending_bits': (run(
     'ref x:i64=1,z:i64=3;ptr p=x;val y<-{{defer{p=z;x<-9;};return if(flag){p+0}else{x+1};}};'
     +OUT('y')+OUT('p')+OUT('p is x')),0,'1\n3\nfalse\n2\n3\nfalse\n',''),
 'finally_keeps_snapshot': (run(BASE+'ref y=try{if(flag){x}else{x+1}}finally{x<-9;{}};'+OUT('y')),0,'1\n2\n',''),
 'selected_reference_reads_after_its_finally': (run(BASE+'ref y=if(flag){try{x}finally{x<-2;{}}}else{7};x<-9;'+OUT('y')),0,'2\n7\n',''),
 'guarded_result_snapshot_after_finally': (MISSING+run(BASE+'ref y=(try{if(!flag){missing();{}}else{};x}'
     'catch(e:const MissingReturnError){7}finally{x<-2;{}});x<-9;'+OUT('y')),0,'2\n7\n','[-Wmissing-return]'),
 'mixed_arm_projects_after_finally': (MISSING+run(BASE+'ref y=if(true){try{if(!flag){missing();{}}else{};x}'
     'catch(e:const MissingReturnError){7}finally{x<-2;{}}}else{8};x<-9;'+OUT('y')),0,'2\n7\n','[-Wmissing-return]'),
 'grouped_boolean_proof_uses_final_payload': (IO+'proc observe()->u64{val flag<-false;val ready<-make local u64;'
     'val saved<-(try{flag<-false}finally{flag<-true;{}});'
     'if(saved){ready<-8u64;{}}else{};return ready;}'+OUT('observe()'),0,'8\n',''),
 'value_operand_reads_after_arm_finally': (run('val x<-make local i64;'+OUT('if(flag){try{x}finally{x<-2;{}}}else{7}')),
     0,'2\n7\n',''),
 'selected_borrow_reads_after_its_cleanup': (IO+'func expose(ref source:i64 in O)->ref i64 in O where {owner O}{'
     'defer{source<-2;};return source;}'+run(BASE+'ref y=if(flag){expose(x)}else{7};x<-9;'+OUT('y')).removeprefix(IO),
     0,'2\n7\n',''),
 'nested_joins': (run(BASE+'ref y=if(flag){if(flag){x}else{2}}else{try{7}finally{z<-5;{}}};x<-9;'+OUT('y')+OUT('z')),0,'1\n3\n7\n5\n',''),
 'catch_supplies_value': (MISSING+run(BASE+'ref y=try{if(flag){x}else{missing()}}catch(e:const MissingReturnError){7};x<-9;'+OUT('y')),0,'1\n7\n','[-Wmissing-return]'),
 'catch_supplies_target': (MISSING+run(BASE+'ref y=try{if(flag){missing()}else{7}}catch(e:const MissingReturnError){x};x<-9;'+OUT('y')),0,'9\n7\n','[-Wmissing-return]'),
 'once_in_order': (run('val x<-1;ref y=if({'+OUT('1')+'flag}){{'+OUT('2')+'defer{'+OUT('4')+'};return x;}}'
     'else{{'+OUT('3')+'defer{'+OUT('4')+'};return 7;}};'+OUT('y')),0,'1\n2\n4\n1\n1\n3\n4\n7\n',''),
 'grouped_receivers': (run(BASE+'ref a=if(flag){x}else{7},b=if(flag){8}else{z};x<-9;z<-10;'+OUT('a')+OUT('b')),0,'1\n8\n7\n3\n',''),
 'fixed_binding': (run(BASE+'const ref y=if(flag){x}else{7};y<-4;'+OUT('x')+OUT('y')),0,'1\n4\n1\n4\n',''),
 'readonly_target': (run('const val x<-1;ref y=if(flag){x}else{7};'+OUT('y')),0,'1\n7\n',''),
 'deferred_local_read': (run(BASE+'ref y=if(flag){x}else{7};defer{'+OUT('y')+'};x<-9;'),0,'1\n7\n',''),
 'prepared_borrow': (IO+'proc show(ref value:const i64 in O)->void where {owner O}{'+OUT('value')+'}'+run(BASE+'ref y=if(flag){x}else{7};defer show(y);x<-9;').removeprefix(IO),0,'1\n7\n',''),
 'deferred_block_receiver': (run('defer{{val x<-1;ref y=if(flag){x}else{7};x<-9;'+OUT('y')+'}};'),0,'1\n7\n',''),
 'per_iteration': (run('ref sum:i64=0;for(i in 0..4){{ref x:i64=1,fallback:i64=7;'
     'ref y=if(flag){x}else{fallback};x<-9;sum<-sum+y;}};'+OUT('sum')),0,'36\n28\n',''),
 'scope_result_uses_receiver_storage': (run('ref x:i64=1,z:i64=3;ref y:i64={{return if(flag){x+0}else{7};}};'
     +OUT('y is x')+'y=z;'+OUT('y')+OUT('x')),0,'false\n3\n1\nfalse\n3\n1\n',''),
 'selected_target_initialization': (run('val x<-make local i64,z<-make local i64;ptr selected=if(flag){x}else{z};selected<-9;ref y=if(flag){selected}else{7};'+OUT('y')),0,'9\n7\n',''),
 'guarded_target_initialization': (MISSING+run('val x<-make local i64,z<-make local i64;ptr selected=if(flag){x}else{z};selected<-9;ref y=try{if(flag){selected}else{missing()}}catch(e:const MissingReturnError){7};'+OUT('y')),0,'9\n7\n','[-Wmissing-return]'),
 'conditional_initialization': (run('val x<-make local i64,z<-make local i64;ptr y=if(flag){x}else{z};y<-9;'+OUT('y')),0,'9\n9\n',''),
 'cleanup_initializes_caller_target': (run(
     'val raw<-make local i64;ptr x:i64=raw;{{defer{x<-9;};}};ref y=if(flag){x+0}else{7};'+OUT('y')),0,'9\n7\n',''),
 'nullable_target': (NPE+run('ptr absent:i64=null in here;ref present:i64=7;ptr y=if(flag){absent}else{present};'+OUT('y is null')+'try{'+OUT('y')+'}catch(e:const NullPointerException){'+OUT('8')+'};'),0,'true\n8\nfalse\n7\n',''),
 'explicit_copy_stays_independent': (run(BASE+'val y<-if(flag){x}else{7};x<-9;'+OUT('y')),0,'1\n7\n',''),
 'recursive_generic_modes': (IO+PICK+'func work(tag:T,flag:bool,n:u64)->i64 where T{val local<-1;ref y=if(flag){pick(tag,local)}else{7};local<-9;if(n==0u64){return y;}else{return work(tag,flag,n-1u64);};}'+OUT('work(false,true,30u64)')+OUT('work(0,true,30u64)')+OUT('work(false,false,30u64)')+OUT('work(0,false,30u64)'),0,'1\n1\n7\n7\n',''),
 'receiver_type_query': (run(BASE+'ref y=if(flag){x}else{7};val copied<-make local typeof(y){3};'+OUT('copied')),0,'3\n3\n',''),
 'result_owner_evidence': ('func same(ref a:i64 in O,ref b:i64 in O)->i64 where {owner O}{return a+b;}'+IO+'func work(flag:bool){val x<-1;ref y=if(flag){x}else{7};return same(y,x);}'+OUT('work(true)')+OUT('work(false)'),0,'2\n8\n',''),
 'abrupt_branch': (IO+'proc work(flag:bool)->i64{val x<-1;ref y=if(flag){x}else{return 7;};'
     +OUT('99')+'return y;}'+OUT('work(true)')+OUT('work(false)'),0,'99\n1\n7\n',''),
 'cleanup_return_abandons_receiver': (IO+'proc work(flag:bool)->i64{ref x:i64=1,z:i64=7;'
     'ref y:i64=try{if(flag){x}else{z}}finally{return 8;};'
     +OUT('99')+'return y;}'+OUT('work(true)')+OUT('work(false)'),0,'8\n8\n',''),
 'abrupt_value_initializer': (IO+'proc work()->i64{ref y:i64=try{7}finally{return 8;};'
     +OUT('99')+'return 0;}'+OUT('work()'),0,'8\n',''),
 'abrupt_owner_initializer': (IO+'proc work(ref x:i64 in O)->i64 where {owner O}{ref y:i64 in O={return 8;};'
     +OUT('99')+'return x;}val input<-1;'+OUT('work(input)'),0,'8\n',''),
 'abrupt_target_branch': (IO+'proc work(flag:bool)->i64{val x<-1;ref y=if(flag){try{x}finally{return 8;};}else{x};'
     +OUT('99')+'return y;}'+OUT('work(true)')+OUT('work(false)'),0,'8\n99\n1\n',''),
 'abrupt_slot_branch': (IO+'proc work(flag:bool)->i64{val x<-1,z<-7;ptr p=x;'
     ' { if(flag){try{p}finally{return 8;};}else{p} } =z;return p;}'+OUT('work(true)')+OUT('work(false)'),0,'8\n7\n',''),
 'abrupt_borrow_argument': (IO+'func read(ref value:i64 in O)->i64 where {owner O}{return value;}'
     'proc work(flag:bool)->i64{val x<-1;return read(if(flag){try{x}finally{return 8;};}else{x});}'
     +OUT('work(true)')+OUT('work(false)'),0,'8\n1\n',''),
 'abrupt_shared_context': (IO+'proc work(seed:T,flag:bool)->const T where T{val local<-seed;'
     'ref y=if(flag){try{local}finally{return seed;};}else{local};'
     +OUT('99')+'return y;}'+OUT('work(true,true)')+OUT('work(7u8,false)'),0,'true\n99\n7\n',''),
 'abrupt_retains_source_validity': ('func work(flag:bool)->i64{val raw<-make local i64;'
     'val y:i64<-if(flag){try{raw}finally{return 8;};}else{1};return y;}work(true);',2,'','before definite initialization'),
 'abrupt_branch_with_scalar_survivor': (IO+'proc work(flag:bool)->i64{val x<-1;'
     'ref y=if(flag){try{x}finally{return 8;};}else{7};'
     +OUT('99')+'return y;}'+OUT('work(true)')+OUT('work(false)'),0,'8\n99\n7\n',''),
 'deferred_outer_owner_rejected': (run('ref x:i64=1;defer{{ref local:i64=7;ref y=if(flag){x}else{local};}};'),
     2,'','reference initializer joins different owners'),
 'scalar_representations': (IO+'func independent(value:T)->const T where T{return value;}func work(value:T,flag:bool)->const T where T{val local<-value;ref y=if(flag){local}else{independent(value)};return y;}'+OUT('work(false,false)')+OUT('work(true,true)')+OUT('work(7u8,false)')+OUT('work(8u32,true)')+OUT('work(1f32,false)')+OUT('work(2f64,true)'),0,'false\ntrue\n7\n8\n1\n2\n',''),
 'exception_payload': (IO+MISSING+'using "std:exceptions" Exception;func error()->const MissingReturnError{try{missing();}catch(e:const MissingReturnError){return e;};}func independent(value:Exception)->const Exception{return value;}proc work(flag:bool)->void{val original:Exception<-error();ref y=if(flag){original}else{independent(original)};'+OUT('y is original')+OUT('y.category')+'}work(true);work(false);',0,'true\n5\nfalse\n5\n','[-Wmissing-return]'),
 'callable_mixture': (IO+'func base()->u64{return 1u64;}func value=base;ptr p=if(true){value}else{base};'+OUT('p()')+OUT('p is value'),0,'1\ntrue\n',''),
 'different_owners': ('ref x:i64=1;{{ref local:i64=7;ref y=if(true){x}else{local};}};',
     2,'','reference initializer joins different owners'),
 'value_receiver_cannot_resolve_different_owners': ('func read(ref x:i64 in O,flag:bool)->i64 where {owner O}{'
     'ref local:i64=7;val y<-if(flag){x}else{local};return y;}ref original:i64=1;read(original,true);',
     2,'','conditional reference result joins different owners'),
 'query_checks_different_owners': ('ref x:i64=1;{{ref local:i64=7;val y<-make local typeof(if(true){x}else{local}){1};}};',
     2,'','conditional reference result joins different owners'),
 'external_owner': (IO+'func work(ref x:i64 in O,flag:bool)->i64 where {owner O}{ref y=if(flag){x}else{7};y<-9;return x;}val input<-1;'+OUT('work(input,true)')+OUT('work(input,false)'),0,'1\n1\n',''),
 'readonly_write': (run('const val x<-1;ref y:i64=if(flag){x}else{7};y<-9;'+OUT('x')+OUT('y')),0,'1\n9\n1\n9\n',''),
 'readonly_escalation': ('const val x<-1;val z<-7;ref y:i64=if(true){x}else{z};',2,'','readonly source cannot initialize'),
 'uninitialized_reference': ('val x<-make local i64;ref y=if(true){x}else{7};',2,'','before definite initialization'),
 'nullable_reference': ('ptr absent:i64=null in here;ref y=if(true){absent}else{7};',2,'','conditional scalar value requires a proven non-null target'),
 'does_not_initialize_other_target': ('val x<-make local i64;ptr y=if(true){x}else{7};y<-9;x;',2,'','before definite initialization'),
 'binding_stays_fixed': ('val x<-1;const ref y=if(true){x}else{7};y=x;',2,'','const reference binding'),
 'returned_local_still_invalid': ('func work(ref x:i64 in O)->ref i64 in O where {owner O}{val local<-1;ref y=if(true){local}else{7};return y;}val argument<-1;work(argument);',2,'','declared external owner'),
 'query_conditional_value_cannot_be_borrowed': ('val x<-1;val y<-make local typeof(if(true){x}else{7} is x){false};',2,'','reference identity requires existing target views'),
 'borrow_argument_does_not_materialize': ('func take(ref x:i64 in O)->void where {owner O}{}val x<-1;take(if(true){x}else{7});',2,'','borrowed arguments require existing target views'),
 'assignment_does_not_materialize': ('val x<-1;ptr p=x;p=if(true){x}else{7};',2,'','fresh value without an existing target'),
}
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {'borrow_argument_does_not_materialize':[]}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
