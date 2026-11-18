#!/usr/bin/env python3
"""Compile-time type queries share checked declarations without evaluating them."""
from pathlib import Path
import foundation_tests as runner

IO='using symbol "std:io" @print;'
NPE='using "std:exceptions" NullPointerException;'
EX='using "std:exceptions" Exception;'
ID='func id(x:T)->const T where T{return x;}'
PRESENT='ref item:u64=1u64;ptr p:u64=item;'
ABSENT='ref item:u64=1u64;ptr p:u64=null in here;'
PROJECT='if(true){p}else{7u64}'
MISS='using "std:exceptions" MissingReturnError;func may_stop()->u64;'
OBSERVATION='const val present<-(input is not null);'
GUARDED_RESULT='return if(present&&flag){input}else{7u64};'
UNPROVED_TARGET='conditional scalar value requires a proven non-null target'
def storage(query, value, prefix=''):
    return IO+prefix+'val result<-make local typeof('+query+'){'+value+'};behave @print(result);'
def saved_query(body, setup='', declarations=''):
    return (IO+declarations+'func inspect(ptr input:u64 in O,flag:bool)->u64 where {owner O}{'+setup+
            'val answer<-make local typeof({{'+body+'}}){9u64};return answer;}'+
            ABSENT+'behave @print(inspect(p,true));')
runner.CASES={
 'literal_i64':(storage('3','8'),0,'8\n',''),
 'literal_u8':(storage('3u8','8u8'),0,'8\n',''),
 'literal_u32':(storage('3u32','8u32'),0,'8\n',''),
 'literal_u64':(storage('3u64','8u64'),0,'8\n',''),
 'literal_f32':(storage('3f32','8f32'),0,'8\n',''),
 'literal_f64':(storage('3f64','8f64'),0,'8\n',''),
 'literal_bool':(storage('true','false'),0,'false\n',''),
 'minimum_literal':(storage('-9223372036854775808','8'),0,'8\n',''),
 'grouped_scalar_query_does_not_read':(storage('((pending))','8u64','val pending<-make local u64;'),0,'8\n',''),
 'grouped_scalar_cannot_be_borrowed_in_query':(storage('take((x))','8',
    'func take(ref value:i64 in O)->i64 where {owner O}{return value;}val x<-1;'),2,'','borrowed parameter requires an existing view'),
 'argument_delimiters_preserve_borrow_in_query':(storage('take(x)','8',
    'func take(ref value:i64 in O)->i64 where {owner O}{return value;}val x<-1;'),0,'8\n',''),
 'grouped_nullable_query_keeps_target':(storage('(p) is null','true','ptr p:i64=null in here;'),0,'true\n',''),
 'projected_present_target':(storage(PROJECT,'9u64',PRESENT),0,'9\n',''),
 'projected_absent_target':(storage(PROJECT,'9u64',ABSENT),2,'','conditional scalar value requires a proven non-null target'),
 'projected_target_after_hypothetical_rebind':(
    storage('{p=item;'+PROJECT+'}','9u64',ABSENT)+'behave @print(p is null);',0,'9\ntrue\n',''),
 'projection_rechecks_hypothetical_absence':(
    storage('{p=null;'+PROJECT+'}','9u64',PRESENT),2,'','conditional scalar value requires a proven non-null target'),
 'projection_sees_declared_place_write':(
    storage('{selected=item;'+PROJECT+'}','9u64',ABSENT+'ptr var selected->p;')+
    'behave @print(p is null);',0,'9\ntrue\n',''),
 'projection_checks_overlapping_declared_places':(
    storage('{selected=null;'+PROJECT+'}','9u64',PRESENT+'ptr q:u64=item;ptr var selected->if(true){p}else{q};'),
    2,'','conditional scalar value requires a proven non-null target'),
 'projection_sees_builtin_slot_exchange':(
    storage('{p<=>q;'+PROJECT+'}','9u64',ABSENT+'ptr q:u64=item;')+
    'behave @print(p is null);',0,'9\ntrue\n',''),
 'projection_rechecks_exchanged_absence':(
    storage('{p<=>q;'+PROJECT+'}','9u64',PRESENT+'ptr q:u64=null in here;'),
    2,'','conditional scalar value requires a proven non-null target'),
 'projection_uses_guard_on_nullable_input':(IO+
    'func inspect(ptr input:u64 in O,flag:bool)->u64 where {owner O}{'
    'val answer<-make local typeof(if(!(input is null)&&flag){input}else{7u64}){9u64};return answer;}'
    +ABSENT+'behave @print(inspect(p,true));',0,'9\n',''),
 'projection_does_not_reuse_mutated_guard':(
    storage('if((p is not null)&&{p=null;true}){p}else{7u64}','9u64',PRESENT),
    2,'','conditional scalar value requires a proven non-null target'),
 'projection_uses_successful_transfer_destination':(IO+
    'func inspect(ptr input:u64 in O)->u64 where {owner O}{'
    'val answer<-make local typeof({input<-1u64;if(true){input}else{7u64}}){9u64};return answer;}'
    +ABSENT+'behave @print(inspect(p));',0,'9\n',''),
 'projection_ignores_stale_identity_operand':(
    storage('if(p is not {p=null;null}){p}else{7u64}','9u64',PRESENT),
    2,'','conditional scalar value requires a proven non-null target'),
 'ordinary_type_query_ignores_absent_payload':(storage('p','9u64',ABSENT),0,'9\n',''),
 'conditional_null_carrier_has_scalar_type':(storage('if(true){item}else{null}','9u64',PRESENT),0,'9\n',''),
 'projection_keeps_target_selected_before_cleanup':(
    storage('{{ref item:u64=1u64;ptr p=item;'
            'return if(true){try{p}finally{p=null;{}}}else{7u64};}}','9u64'),0,'9\n',''),
 'projection_sees_normal_finalizer_rebind':(
    storage('{{ref item:u64=1u64;ptr p:u64=null in here;try{}finally{p=item;{}};return '+PROJECT+';}}','9u64'),0,'9\n',''),
 'projection_checks_opaque_failure_entry_to_finalizer':(
    storage('{{ref item:u64=1u64;ptr p=item;try{p=null;may_stop();p=item;{}}finally{'+PROJECT+
            ';{}};return 7u64;}}','9u64',MISS),2,'','conditional scalar value requires a proven non-null target'),
 'projection_checks_opaque_failure_entry_to_catch':(
    storage('{{ref item:u64=1u64;ptr p=item;try{p=null;may_stop();p=item;{}}catch(e:const MissingReturnError){'+PROJECT+
            ';{}};return 7u64;}}','9u64',MISS),2,'','conditional scalar value requires a proven non-null target'),
 'projection_preserves_unmodified_slot_through_loop':(
    storage('{{ref item:u64=1u64;ptr p=item;for(i in 0u64..2u64){'+PROJECT+';};return '+PROJECT+';}}','9u64'),0,'9\n',''),
 'projection_checks_mutated_loop_backedge':(
    storage('{{ref item:u64=1u64;ptr p=item;for(i in 0u64..2u64){'+PROJECT+';p=null;};return 7u64;}}','9u64'),
    2,'','conditional scalar value requires a proven non-null target'),
 'projection_reads_saved_boolean':(saved_query(OBSERVATION+GUARDED_RESULT),0,'9\n',''),
 'projection_copy_survives_boolean_overwrite':(saved_query(
    'val seen<-(input is not null);const val present<-seen;seen<-flag;'+GUARDED_RESULT),0,'9\n',''),
 'projection_loses_saved_boolean_after_rebind':(saved_query(OBSERVATION+'input=null in O;'+GUARDED_RESULT),2,'',UNPROVED_TARGET),
 'projection_loses_overwritten_borrowed_boolean':(saved_query(
    'val present<-(input is not null);ref carrier:bool=present;carrier<-flag;'+GUARDED_RESULT),2,'',UNPROVED_TARGET),
 'projection_reads_boolean_through_reference':(saved_query(OBSERVATION+'ref carrier:const bool=present;'+
    GUARDED_RESULT.replace('present', 'carrier')),0,'9\n',''),
 'projection_loses_boolean_written_through_declared_place':(saved_query(
    'val present<-(input is not null);var selected->present;selected<-flag;'+GUARDED_RESULT),2,'',UNPROVED_TARGET),
 'projection_reads_shared_proof_from_either_declared_place':(saved_query(
    'val first<-(input is not null),second<-(input is not null);var present->if(flag){first}else{second};'+
    GUARDED_RESULT),0,'9\n',''),
 'projection_keeps_observation_after_independent_slot_rebind':(saved_query(
    OBSERVATION+'ptr other:u64=input;other=null in O;'+GUARDED_RESULT),0,'9\n',''),
 'projection_loses_observation_after_overlapping_slot_rebind':(saved_query(OBSERVATION+
    'ptr other:u64=input;ptr var chosen->if(flag){input}else{other};chosen=null in O;'+GUARDED_RESULT),2,'',UNPROVED_TARGET),
 'projection_joins_conditional_boolean_outcomes':(saved_query(
    'const val present<-if(flag){input is not null}else{false};'+GUARDED_RESULT),0,'9\n',''),
 'projection_imports_live_boolean_at_query_entry':(saved_query(GUARDED_RESULT,setup=OBSERVATION),0,'9\n',''),
 'projection_keeps_guarded_observation_after_unrelated_cleanup':(saved_query(
    'ptr other:u64=input;const val present<-try{input is not null}finally{other=null in O;{}};'+GUARDED_RESULT),0,'9\n',''),
 'projection_loses_observation_on_loop_backedge':(saved_query(OBSERVATION+
    'for(i in 0u64..2u64){if(present&&flag){input}else{7u64};input=null in O;};return 7u64;'),2,'',UNPROVED_TARGET),
 'projection_loses_borrowed_boolean_after_scope_cleanup':(saved_query(
    'val present<-(input is not null);ref write:bool=present;{{defer{write<-flag;};}};'+GUARDED_RESULT),2,'',UNPROVED_TARGET),
 'projection_loses_borrowed_boolean_after_prepared_write':(saved_query(
    'val present<-(input is not null);ref write:bool=present;{{defer (<-)(write,flag);}};'+GUARDED_RESULT),2,'',UNPROVED_TARGET),
 'projection_reads_returned_boolean':(saved_query(
    'return if({{return input is not null;}}){input}else{7u64};'),0,'9\n',''),
 'projection_returns_observation_without_capturing_reference':(saved_query(
    'const val seen<-(input is not null);const val present<-{{return seen;}};'+GUARDED_RESULT),0,'9\n',''),
 'projection_returns_observation_through_inner_rebind':(saved_query(
    'const val seen<-(input is not null);const val present<-{{input=null in O;return seen;}};'+GUARDED_RESULT),0,'9\n',''),
 'projection_joins_returned_boolean_outcomes':(saved_query(
    'const val present<-{{if(flag){return input is not null;}else{return false;};}};'+GUARDED_RESULT),0,'9\n',''),
 'projection_checks_every_boolean_return_path':(saved_query(
    'const val present<-{{if(flag){return input is not null;}else{return true;};}};'
    'return if(present){input}else{7u64};'),2,'',UNPROVED_TARGET),
 'projection_keeps_return_snapshot_through_finally':(saved_query(
    'const val present<-{{try{return input is not null;}finally{input=null in O;{}};}};'+GUARDED_RESULT),0,'9\n',''),
 'projection_rejects_boolean_from_rebound_inner_slot':(saved_query(
    'const val present<-{{input=null in O;return input is null;}};'+GUARDED_RESULT),2,'',UNPROVED_TARGET),
 'projection_rechecks_rebound_caller_after_returned_boolean':(saved_query(
    'const val present<-{{return input is not null;}};input=null in O;'+GUARDED_RESULT),2,'',UNPROVED_TARGET),
 'projection_reads_returned_same_owner_null_test':(saved_query(
    'const val present<-{{return input is not null in O;}};'+GUARDED_RESULT),0,'9\n',''),
 'projection_rejects_returned_different_owner_null_test':(saved_query(
    'const val present<-{{return input is not null in here;}};'+GUARDED_RESULT),2,'',UNPROVED_TARGET),
 'projection_rejects_direct_different_owner_null_test':(saved_query(
    'return if(input is not null in here){input}else{7u64};'),2,'',UNPROVED_TARGET),
 'projection_loses_replaced_return_predicate_during_defer':(saved_query(
    'const val present<-{{defer{return true;};return input is not null;}};'+GUARDED_RESULT),2,'',UNPROVED_TARGET),
 'arithmetic_without_division':(storage('3/0+4','9'),0,'9\n',''),
 'numeric_widening':(storage('3u8+4u64','9u64'),0,'9\n',''),
 'intrinsic_selected':(storage('intrinsic (+)(3u64,4u64)','9u64'),0,'9\n',''),
 'logical':(storage('true&&!false','true'),0,'true\n',''),
 'conditional':(storage('if(true){1}else{2}','9'),0,'9\n',''),
 'uninitialized_operands':(storage('a+b','10','val a<-make local i64,b<-make local i64;'),0,'10\n',''),
 'null_operand':(storage('p+1','11','ptr p:i64=null in here;'),0,'11\n',''),
 'declared_generic_result':(storage('id(3)+4','12',ID),0,'12\n',''),
 'no_generic_body_demand':(storage('unproved(true)','false','func unproved(x:T)->const T where T{return x+x;}'),0,'false\n',''),
 'no_recursive_body_demand':(storage('unproved(true)','false','func unproved(x:T)->const T where T{return unproved(unproved(x));}'),0,'false\n',''),
 'procedure_not_invoked':(storage('noisy(1)','13','proc noisy(x:i64)->const i64{behave @print(99);return x;}'),0,'13\n',''),
 'function_can_query_procedure':(IO+'proc noisy(x:i64)->const i64{behave @print(99);return x;}func query()->const i64{val x<-make local typeof(noisy(1)){14};return x;}behave @print(query());',0,'14\n',''),
 'initializer_runs_once':(IO+'proc next(ref x:i64 in O)->const i64 where {owner O}{x<-x+1;return x;}val count<-0;val x<-make local typeof(next(count)){next(count)};behave @print(x);behave @print(count);',0,'1\n1\n',''),
 'no_initialization_from_query':('func init(ref x:i64 in O)->const i64 where {owner O}{x<-1;return x;}val x<-make local i64;val y<-make local typeof(init(x)){0};x;',2,'','before definite initialization'),
 'known_callable_value':(storage('f(1)','15',ID+'func f:func(i64)->const i64=id;'),0,'15\n',''),
 'known_callable_name_argument':(storage('apply(one,1)','16','func one(x:i64)->const i64{return x;}func apply(f:func(i64)->const i64,x:i64)->const i64{return f(x);}'),0,'16\n',''),
 'uninitialized_exception_field':(storage('e.category','17u64',EX+'val e<-make local Exception;'),0,'17\n',''),
 'cause_field':(storage('cause_at(e,0u64).site','18u64',EX+'val e<-make local Exception;'),0,'18\n',''),
 'borrowed_result_identity':(storage('((get(p)) is null)','true','func get(ptr x:i64 in O)->ptr i64 in O where {owner O}{return x;}val x<-1;ptr p=x;'),0,'true\n',''),
 'checked_failure_not_raised':(storage('read(p)','19',NPE+'func read(ptr x:i64 in O)->const i64 throws NullPointerException where {owner O}{return x;}val x<-1;ptr p=x;p=null;'),0,'19\n',''),
 'overload_result':(storage('choose(true)','20u64','func choose(x:i64)->const i64{return x;}func choose(x:bool)->const u64{return 1u64;}'),0,'20\n',''),
 'generic_local_storage':(IO+'func query(x:T)->const T where T{val y<-make local typeof(id(x)){x};return y;}'+ID+'behave @print(query(21));behave @print(query(true));',0,'21\ntrue\n',''),
 'later_operand_evidence':(IO+ID+'func apply(f:func(T)->const T,x:T)->const T where T{return f(x);}behave @print(apply(id,{{val x<-make local typeof(1){22};return x;}}));',0,'22\n',''),
 'unknown_name':('val x<-make local typeof(missing);',2,'','known declaration or operand type'),
 'unknown_callable':('val x<-make local typeof(missing(1));',2,'','known callable signature'),
 'ambiguous_same_result':('func f(x:T,y:i64)->const i64 where T{return y;}func f(x:i64,y:T)->const i64 where T{return x;}val x<-make local typeof(f(1,2));',2,'','ambiguous methods'),
 'wrong_same_t':(ID+'val x<-make local typeof(id(true,1));',2,'','no applicable method'),
 'bad_logic':('val x<-make local typeof(1&&2);',2,'','logical operation requires bool'),
 'bad_condition':('val x<-make local typeof(if(1){2}else{3});',2,'','condition requires bool'),
 'incompatible_branches':('val x<-make local typeof(if(true){1}else{false});',2,'','known declaration or operand type'),
 'bad_numeric':('val x<-make local typeof(true+1);',2,'','no applicable numeric operation'),
 'integer_range':('val x<-make local typeof(256u8);',2,'','integer literal is outside u8'),
 'real_range':('val x<-make local typeof(1e99f32);',2,'','floating literal is outside f32'),
 'unknown_field':(EX+'val e<-make local Exception;val x<-make local typeof(e.invalid);',2,'','unknown intrinsic exception field'),
 'invalid_identity':('val x<-make local typeof(((1) is null));',2,'','reference identity requires existing target views'),
 'invalid_cause_index':(EX+'val e<-make local Exception;val x<-make local typeof(cause_at(e,true));',2,'','cause_at requires an exception and a u64 index'),
 'shadowed_call':('func f(x:i64)->const i64{return x;}val f<-1;val x<-make local typeof(f(1));',2,'','nearest binding'),
 'callable_argument_type':(ID+'func f:func(i64)->const i64=id;val x<-make local typeof(f(true));',2,'','argument does not satisfy the callable view type'),
 'borrow_needs_view':('func get(ref x:i64 in O)->const i64 where {owner O}{return x;}val x<-make local typeof(get(1));',2,'','existing view'),
 'borrow_respects_access':('func get(ref x:i64 in O)->const i64 where {owner O}{return x;}const val x<-1;val y<-make local typeof(get(x));',2,'','readonly argument'),
 'borrow_respects_owner':('func get(ref x:i64 in O,ref y:i64 in O)->const i64 where {owner O}{return x;}val x<-1;{{val y<-2;val z<-make local typeof(get(x,y));}};',2,'','different actual owners'),
 'body_inference_without_payload_read':('func f(x:i64){return x;}val x<-make local typeof(f(1));',0,'',''),
 'query_cannot_declare_runtime_storage':('val x<-make local typeof({val y<-1;y});',2,'','typeof for this expression form'),
 'outer_query_does_not_capture':('func id(x:i64)->const i64{return x;}proc run()->void{func f=id;func nested()->const i64{val x<-make local typeof(f(1)){2};return x;};nested();}run();',0,'',''),
 'mixed_identity_view':('val a<-1;val b<-make local typeof(((if(true){a}else{1}) is null));',2,'','reference identity requires existing target views'),
 'mixed_borrowed_view':('func f(ref x:i64 in O)->const i64 where {owner O}{return x;}val a<-1;val b<-make local typeof(f(if(true){a}else{1}));',2,'','existing view'),
 'callable_arity':(ID+'func f:func(i64)->const i64=id;val x<-make local typeof(f(1,2));',2,'','callable argument count'),
 'callable_access':('func f(ref x:i64 in O)->const i64 where {owner O}{return x;}func g=f;const val a<-1;val x<-make local typeof(g(a));',2,'','readonly argument'),
 'callable_owner':('func f(ref x:i64 in O,ref y:i64 in O)->const i64 where {owner O}{return x;}func g=f;val a<-1;{{val b<-2;val x<-make local typeof(g(a,b));}};',2,'','different actual owners'),
 'lexical_recursive_query':(IO+'func run(x:T)->const T where T{func recurse(x:T,n:i64)->const T{val y<-make local typeof(recurse(x,n-1)){x};if(n==0){return y;}else{return recurse(y,n-1);};};return recurse(x,4);}behave @print(run(23));behave @print(run(true));',0,'23\ntrue\n',''),
 'delayed_loop_type_only':(IO+'for(i in 0..2){defer{{val x<-make local typeof(i){24};behave @print(x);}};};',0,'24\n24\n',''),
 'deferred_exception_type_only':(IO+'using "std:exceptions" MissingReturnError;func missing()->const i64{}try{missing();{}}catch(e:const MissingReturnError){defer{{val x<-make local typeof(e.category){25u64};behave @print(x);}};};',0,'25\n','[-Wmissing-return]'),
 'initializer_still_captures':(IO+'func id(x:i64)->const i64{return x;}proc run()->void{func f=id;func nested[f]()->const i64{val x<-make local typeof(1){f(26)};return x;};behave @print(nested());}run();',0,'26\n',''),
}
runner.CASE_ARGUMENTS={}
runner.CASE_SHADOWS = {
    'initializer_runs_once': ['x'],
    'borrowed_result_identity': ['x'],
    'checked_failure_not_raised': ['x'],
    'body_inference_without_payload_read': ['x'],
    'lexical_recursive_query': ['x'],
}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
