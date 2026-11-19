#!/usr/bin/env python3
"""Checked obligations cross immediate full-scope calls after local handling."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
TYPES = 'using "std:exceptions" Exception;using "std:exceptions" NullPointerException;'
PAYLOAD = ('func sample_error()->const NullPointerException{ptr absent:u64=null in here;'
           'try{absent;{}}catch(caught:const NullPointerException){return caught;};return sample_error();}')
FORWARD = 'proc forward(val e:NullPointerException)->void throws NullPointerException{throw e;}'
DECLARED = 'proc declared()->void throws NullPointerException{}'
STOP = ('using "std:exceptions" MissingReturnError;'
        'func stop(val e:MissingReturnError)->void{throw e;}')
ROOT = 'proc broad(val e:NullPointerException)->void throws Exception{throw e;}'
TAKE = 'proc take(value:u64)->void{behave @print(value);}'
OBSERVE = 'proc observe(value:u64)->void{behave @print(value);}'
NPE = "checked exception 'NullPointerException' requires a covering catch or throws declaration"
BROAD = "declared checked exception 'Exception' requires a covering catch or throws declaration"
DEFER = 'checked failures involving deferred execution require coverage within the owning pass'


def run(body, output, declarations=''):
    return IO + TYPES + PAYLOAD + declarations + body, 0, output, ''


def reject(source, diagnostic=NPE):
    return TYPES + PAYLOAD + source, 2, '', diagnostic


def query(body, prelude=TYPES):
    return prelude + 'val result<-make local typeof({{' + body + '}}){4u64};'


CONDITIONAL_READ = (' {{val local<-make local u64;'
                    'if(flag){local<-7u64;{}}else{};behave @print(local);}}; ')
UNINITIALIZED_LOCAL = "read of 'local' before definite initialization"
BORROWED_BOOLEAN = 'val off<-false,on<-true;ref flag=off;ref next=on;'
DEFER_CONDITIONAL_READ = 'defer{'+CONDITIONAL_READ+'};'
UNWRITTEN = 'val local<-make local u64;'
DEFER_READ_LOCAL = 'defer{behave @print(local);}'
GUARDED_WRITES = ('val a<-make local u64,b<-make local u64;'
                  'if(flag){a<-7u64;{}}else{b<-8u64;{}};')
SELECT_WRITTEN = 'ptr p=if(flag){a}else{b};'
PICK = 'func choose(ptr left:T in O,ptr right:T in O)->ptr T in O where {T,owner O}'
PICK_QUERY = (UNWRITTEN+'val ready<-8u64;ptr chosen=choose(local,ready);'
              'try{return chosen;}catch(e:const NullPointerException){return 99u64;};')
UNWRITTEN_RESULT = "read of 'chosen' before definite initialization"
PREPARE = ('proc prepare(ptr first:u64 in O,ptr second:u64 in O)'
           '->ptr u64 in O throws NullPointerException where {owner O}')
PREPARE_INPUTS = UNWRITTEN+'val other<-make local u64;'
PREPARE_READ = ('try{ptr chosen=prepare(local,other);return chosen;}'
                'catch(e:const NullPointerException){return 99u64;};')
RECURSIVE_PICK = PICK.replace('right:T in O)', 'right:T in O,n:u64)')
RECURSIVE_QUERY = PICK_QUERY.replace('choose(local,ready)', 'choose(local,ready,3u64)')
RECURSIVE_STEP = 'return choose(left,right,n-1u64);'


def guarded_reference(body):
    return IO+'proc run(flag:bool)->void{val result<-{{'+body+'}};behave @print(result);}run(true);run(false);'


def queried_pick(body):
    return IO+PICK+'{'+body+'}'+query(PICK_QUERY)+'behave @print(result);'


def queried_preparation(body, caller=PREPARE_READ):
    return IO+TYPES+PREPARE+'{'+body+'}'+query(PREPARE_INPUTS+caller, '')+'behave @print(result);'


def recursive_pick(base, step=RECURSIVE_STEP, declaration=RECURSIVE_PICK):
    return declaration+'{if(n==0u64){'+base+'}else{'+step+'};}'


EXPANDING_PICK = recursive_pick('return right;', 'return choose(left,right,n-1u64,func[tag]()->const A{return tag;});',
    RECURSIVE_PICK.replace('n:u64)', 'n:u64,tag:A)').replace('{T,owner O}', '{T,A,owner O}'))


runner.CASES = {
    'local_read_with_known_capture': (IO+'val flag<-true;'+CONDITIONAL_READ, 0, '7\n', ''),
    'local_read_with_guarded_capture': (
        IO+'proc run(flag:bool)->void{if(flag){'+CONDITIONAL_READ+'}else{};}run(true);run(false);', 0, '7\n', ''),
    'local_read_with_unknown_capture': (
        IO+'proc run(flag:bool)->void{'+CONDITIONAL_READ+'}run(true);', 2, '', UNINITIALIZED_LOCAL),
    'local_read_with_late_true_capture': (
        IO+'val flag<-false;defer{ {{'+CONDITIONAL_READ+'}}; };flag<-true;', 0, '7\n', ''),
    'local_read_with_late_false_capture': (
        IO+'val flag<-true;defer{'+CONDITIONAL_READ+'};flag<-false;', 2, '', UNINITIALIZED_LOCAL),
    'local_read_with_failed_registration_prefix': (
        IO+'val flag<-false;defer{'+CONDITIONAL_READ+'};defer{flag<-true;};', 2, '', UNINITIALIZED_LOCAL),
    'local_read_uses_its_own_storage': (
        IO+'val local<-7u64;val flag<-false;'+CONDITIONAL_READ, 2, '', UNINITIALIZED_LOCAL),
    'local_read_uses_rebound_borrowed_target': (
        IO+BORROWED_BOOLEAN+'{{'+DEFER_CONDITIONAL_READ+'flag=next;}};', 0, '7\n', ''),
    'local_read_rejects_false_rebound_target': (
        IO+'val off<-false,on<-true;ref flag=on;ref next=off;{{'+DEFER_CONDITIONAL_READ+'flag=next;}};',
        2, '', UNINITIALIZED_LOCAL),
    'local_read_precedes_write_to_shared_target': (
        IO+BORROWED_BOOLEAN+'ref peer=on;{{flag=next;defer{peer<-false;};'+DEFER_CONDITIONAL_READ+'}};',
        0, '7\n', ''),
    'local_read_follows_body_write_to_distinct_target': (
        IO+BORROWED_BOOLEAN+'ref peer=off;{{'+DEFER_CONDITIONAL_READ+'flag=next;peer<-false;}};',
        0, '7\n', ''),
    'local_read_follows_deferred_write_to_distinct_target': (
        IO+BORROWED_BOOLEAN+'ref peer=off;{{flag=next;'+DEFER_CONDITIONAL_READ+'defer{peer<-false;};}};',
        0, '7\n', ''),
    'local_read_observes_earlier_write_to_shared_target': (
        IO+BORROWED_BOOLEAN+'ref peer=on;{{flag=next;'+DEFER_CONDITIONAL_READ+'defer{peer<-false;};}};',
        2, '', UNINITIALIZED_LOCAL),
    'local_read_observes_targets_shared_after_registration': (
        IO+BORROWED_BOOLEAN+'ref peer=off;{{'+DEFER_CONDITIONAL_READ+'defer{peer<-false;};flag=next;peer=next;}};',
        2, '', UNINITIALIZED_LOCAL),
    'local_read_keeps_target_when_writer_rebinds_after_write': (
        IO+'val off<-false,on<-true;ref flag=on;ref peer=off;{{'+DEFER_CONDITIONAL_READ+
        'defer{peer=flag;};defer{peer<-false;};}};', 0, '7\n', ''),
    'local_read_observes_shared_target_when_writer_rebinds_before_write': (
        IO+'val off<-false,on<-true;ref flag=on;ref peer=off;{{'+DEFER_CONDITIONAL_READ+
        'defer{peer<-false;};defer{peer=flag;};}};', 2, '', UNINITIALIZED_LOCAL),
    'local_read_consumes_conditionally_initialized_target': (
        guarded_reference(GUARDED_WRITES+SELECT_WRITTEN+'return p;'), 0, '7\n8\n', ''),
    'local_read_rejects_opposite_conditional_target': (
        guarded_reference(GUARDED_WRITES+'ptr p=if(flag){b}else{a};return p;'),
        2, '', "read of 'p' before definite initialization"),
    'local_read_keeps_initialized_selection_across_rebind': (
        guarded_reference(GUARDED_WRITES+SELECT_WRITTEN+
            'val other<-make local u64;ptr saved=p;p=other;return saved;'), 0, '7\n8\n', ''),
    'local_read_nested_after_conditional_target_initialization': (
        guarded_reference(GUARDED_WRITES+SELECT_WRITTEN+'return {{return p;}};'), 0, '7\n8\n', ''),
    'local_read_during_cleanup_after_conditional_target_initialization': (
        guarded_reference(GUARDED_WRITES+SELECT_WRITTEN+'defer{behave @print(p);};return 11u64;'),
        0, '7\n11\n8\n11\n', ''),
    'throw_only_typed': run('val e<-sample_error();try{ {{throw e;}}; }'
                            'catch(caught:const NullPointerException){behave @print(caught.category);};', '4\n'),
    'throw_only_void_result': reject('val e<-sample_error();val result<-make local typeof({{throw e;}}){7u64};',
                                     'initializer does not satisfy local storage type'),
    'throw_only_query_has_no_reads': run('val e<-make local NullPointerException;'
                                         'val result<-make local typeof({ {{throw e;}};7u64 }){9u64};behave @print(result);', '9\n'),
    'throw_only_unchecked_query_validity': reject('val result<-make local typeof({{throw 7u64;}});',
                                                  'throw requires an intrinsic exception value'),
    'throw_only_uses_argument_signature': run(
        'val e<-sample_error();try{consume({{defer observe(7u64);observe(1u64);throw e;}},'
        '{observe(98u64);2u64});{}}catch(caught:const NullPointerException){behave @print(caught.category);};',
        '1\n7\n4\n', OBSERVE + 'proc consume(x:T,y:T)->void where T{behave @print(y);}'),
    'throw_only_uses_callable_view_argument_signature': run(
        'proc invoke=take;val e<-sample_error();try{invoke({{throw e;}});{}}'
        'catch(caught:const NullPointerException){behave @print(caught.category);};', '4\n', TAKE),
    'throw_only_supplies_no_callable_payload': run(
        'val e<-sample_error();try{apply({{throw e;}});{}}'
        'catch(caught:const NullPointerException){behave @print(caught.category);};', '4\n',
        'proc apply(fn:func(u64)->u64)->void{behave @print(fn(7u64));}'),
    'throw_only_uses_declared_result_signature': run(
        'try{take(expose(sample_error()));{}}catch(caught:const NullPointerException){behave @print(caught.category);};',
        '4\n', TAKE + 'func expose(val e:NullPointerException)->u64 throws NullPointerException;'
        'func expose(val e:NullPointerException) throws NullPointerException{return {{throw e;}};}'),
    'throw_only_argument_query_does_not_read_or_execute': run(
        'val e<-make local NullPointerException;'
        'val result<-make local typeof(select({{observe(98u64);throw e;}},1u64)){17u64};behave @print(result);',
        '17\n', OBSERVE + 'func select(x:T,y:T)->const T where T{return y;}'),
    'throw_only_scope_composes_through_nested_return': run(
        'val e<-sample_error();try{take({{return {{throw e;}};}});{}}'
        'catch(caught:const NullPointerException){behave @print(caught.category);};', '4\n', TAKE),
    'throw_only_result_keeps_checked_obligation': reject(IO + TAKE + 'val e<-sample_error();take({{throw e;}});'),
    'ordinary_void_scope_cannot_supply_argument': reject(TAKE + 'take({{7u64;}});',
        'non-value or abruptly completing operand'),
    'ambiguous_contract_cannot_type_throw_only_result': reject(
        TAKE + 'proc take(value:bool)->void{}val e<-sample_error();'
        'try{take({{throw e;}});{}}catch(caught:const NullPointerException){};',
        'non-value or abruptly completing operand'),
    'query_checks_catch_category': (query('try{}catch(e:const u64){};return 7u64;'),
        2, '', 'catch requires a selected intrinsic exception type'),
    'query_rejects_unwritten_private_storage': (
        query(UNWRITTEN+'return local;'), 2, '', UNINITIALIZED_LOCAL),
    'query_checks_read_after_separate_abrupt_arm': (
        'proc run(flag:bool)->void{'+query(UNWRITTEN+'if(flag){if(true){return 7u64;}else{};}else{};return local;', '')+
        '}run(true);', 2, '', UNINITIALIZED_LOCAL),
    'query_keeps_outer_input_unevaluated': (
        IO+UNWRITTEN+query('return local;')+'behave @print(result);', 0, '4\n', ''),
    'query_carries_nested_borrowed_write': (
        IO+query(UNWRITTEN+'ptr p=local;{{defer{p<-7u64;};}};return local;')+'behave @print(result);', 0, '4\n', ''),
    'query_checks_registration_failure_before_deferred_write': (
        IO+query(UNWRITTEN+DEFER_READ_LOCAL+';defer{local<-7u64;};return 7u64;'),
        2, '', UNINITIALIZED_LOCAL),
    'query_accepts_guarded_deferred_read': (
        IO+query(UNWRITTEN+DEFER_READ_LOCAL+'finally{};defer (<-)(local,7u64);return 7u64;')+
        'behave @print(result);', 0, '4\n', ''),
    'query_checks_non_null_target_after_nullable_join': (
        TYPES+'proc run(flag:bool)->void{'+query(UNWRITTEN+
        'ptr p:u64=null in here;if(flag){p=local;{}}else{};try{return p;}'
        'catch(e:const NullPointerException){return 4u64;};', '')+'}run(true);', 2, '',
        "read of 'p' before definite initialization"),
    'query_retains_initialization_of_selected_target': (
        IO+'proc run(flag:bool)->void{'+query(UNWRITTEN+'val other<-make local u64;'
        'ptr p=if(flag){local}else{other};p<-7u64;return p;', '')+
        'behave @print(result);}run(true);', 0, '4\n', ''),
    'query_keeps_initialized_selection_across_reference_copy': (
        IO+'proc run(flag:bool)->void{'+query(UNWRITTEN+'val other<-make local u64;val ready<-1u64;'
        'ptr p=if(flag){local}else{other};p<-7u64;ptr retained=p;p=ready;return retained;', '')+
        'behave @print(result);}run(true);', 0, '4\n', ''),
    'query_checks_a_target_from_a_borrowed_result_signature': (
        'func same(ptr x:u64 in O)->ptr u64 in O where {owner O};'+query(UNWRITTEN+
        'ptr p=same(local);try{return p;}catch(e:const NullPointerException){return 4u64;};'),
        2, '', "read of 'p' before definite initialization"),
    'query_proves_the_target_returned_by_a_generic_method': (
        queried_pick('return right;'), 0, '4\n', ''),
    'query_rejects_unwritten_target_returned_by_a_generic_method': (
        queried_pick('return left;'), 2, '', UNWRITTEN_RESULT),
    'query_proves_replacement_borrowed_target_from_finally': (
        queried_pick('try{return left;}finally{return right;};'), 0, '4\n', ''),
    'query_proves_replacement_borrowed_target_from_defer': (
        queried_pick('defer{return right;};return left;'), 0, '4\n', ''),
    'query_rejects_replacement_borrowed_target_from_defer': (
        queried_pick('defer{return left;};return right;'), 2, '', UNWRITTEN_RESULT),
    'query_retains_saved_borrowed_target_across_cleanup_rebind': (
        queried_pick('ptr selected=right;defer{selected=left;};return selected;'), 0, '4\n', ''),
    'query_composes_borrowed_targets_through_generic_forwarding': (
        PICK.replace('choose', 'forward')+'{return right;}'+queried_pick('return forward(left,right);'),
        0, '4\n', ''),
    'query_falls_back_when_ref_result_can_be_null': (
        PICK.replace('->ptr T', '->ref T')+'{ptr absent:T=null in O;return absent;}'+query(PICK_QUERY),
        2, '', UNWRITTEN_RESULT),
    'query_falls_back_for_readonly_target_in_writable_result': (
        PICK.replace('ptr right:T', 'ptr right:const T')+'{return right;}'+query(PICK_QUERY),
        2, '', UNWRITTEN_RESULT),
    'query_inspects_borrowed_signature_without_validating_generic_body': (
        IO+PICK+'{return 7u64;}'+UNWRITTEN+'val ready<-8u64;'
        'val result<-make local typeof(choose(local,ready)){4u64};behave @print(result);', 0, '4\n', ''),
    'query_closes_recursive_borrowed_result': (
        IO+recursive_pick('return right;')+query(RECURSIVE_QUERY)+'behave @print(result);', 0, '4\n', ''),
    'query_rejects_recursive_permutation_with_unwritten_target': (
        recursive_pick('return right;', 'return choose(right,left,n-1u64);')+query(RECURSIVE_QUERY),
        2, '', UNWRITTEN_RESULT),
    'query_closes_mutual_borrowed_returns_with_remapped_input_positions': (
        IO+recursive_pick('return right;', 'return sibling(right,left,n-1u64);')+
        recursive_pick('return left;', 'return choose(right,left,n-1u64);', RECURSIVE_PICK.replace('choose', 'sibling'))+
        query(RECURSIVE_QUERY)+'behave @print(result);', 0, '4\n', ''),
    'query_keeps_unwritten_target_from_mutual_base_case': (
        recursive_pick('return right;', 'return sibling(left,right,n-1u64);')+
        recursive_pick('return left;', declaration=RECURSIVE_PICK.replace('choose', 'sibling'))+
        query(RECURSIVE_QUERY), 2, '', UNWRITTEN_RESULT),
    'query_closes_recursive_finally_result_replacement': (
        IO+recursive_pick('try{return left;}finally{return right;};')+
        query(RECURSIVE_QUERY)+'behave @print(result);', 0, '4\n', ''),
    'query_closes_recursive_defer_result_replacement': (
        IO+recursive_pick('defer{return right;};return left;')+
        query(RECURSIVE_QUERY)+'behave @print(result);', 0, '4\n', ''),
    'query_closes_recursive_initialization': (
        IO+TYPES+recursive_pick('left<-7u64;return left;', declaration=RECURSIVE_PICK.replace('func', 'proc')
            .replace('where', 'throws NullPointerException where'))+query(RECURSIVE_QUERY, '')+
        'behave @print(result);', 0, '4\n', ''),
    'query_closes_recursive_result_over_two_concrete_type_arguments': (
        IO+recursive_pick('return right;', 'return choose(left,right,n-1u64,0u32);',
            RECURSIVE_PICK.replace('n:u64)', 'n:u64,tag:A)').replace('{T,owner O}', '{T,A,owner O}'))+
        query(RECURSIVE_QUERY.replace('3u64)', '3u64,true)'))+'behave @print(result);', 0, '4\n', ''),
    'query_distinguishes_a_recursive_null_result_from_incomplete_evidence': (
        IO+recursive_pick('return null in O;')+query(UNWRITTEN+'val ready<-8u64;'
            'ptr chosen=choose(local,ready,3u64);if(chosen is null){return 17u64;}else{return local;};')+
        'behave @print(result);', 0, '4\n', ''),
    'query_preserves_failed_recursive_call_paths': (
        TYPES+recursive_pick('val failure<-make local NullPointerException;throw failure;',
            declaration=RECURSIVE_PICK.replace('func', 'proc').replace('where', 'throws NullPointerException where'))+
        query(UNWRITTEN+'val ready<-8u64;try{choose(local,ready,3u64);return 17u64;}'
            'catch(e:const NullPointerException){return local;};', ''), 2, '', UNINITIALIZED_LOCAL),
    'query_uses_declared_bound_for_expanding_recursive_type_arguments': (
        EXPANDING_PICK+query(RECURSIVE_QUERY.replace('3u64)', '3u64,true)')), 2, '', UNWRITTEN_RESULT),
    'query_inspects_a_signature_with_expanding_recursive_type_arguments': (
        IO+EXPANDING_PICK+UNWRITTEN+'val ready<-8u64;'
        'val result<-make local typeof(choose(local,ready,3u64,true)){4u64};behave @print(result);', 0, '4\n', ''),
    'query_reads_target_initialized_before_borrowed_return': (
        queried_preparation('first<-7u64;return first;'), 0, '4\n', ''),
    'query_reads_saved_target_initialized_in_finally': (
        queried_preparation('try{return first;}finally{first<-7u64;{}};'), 0, '4\n', ''),
    'query_reads_saved_target_initialized_in_defer': (
        queried_preparation('defer{if(first is not null){first<-7u64;{}}else{};};return first;'), 0, '4\n', ''),
    'query_matches_saved_target_and_cleanup_write_under_the_same_value_guard': (
        queried_preparation('defer{if(flag){first<-7u64;{}}else{second<-8u64;{}};};'
                            'if(flag){return first;}else{return second;};',
                            PREPARE_READ.replace('local,other', 'local,other,true'))
        .replace('second:u64 in O)', 'second:u64 in O,flag:bool)'), 0, '4\n', ''),
    'query_rejects_cleanup_writes_under_the_opposite_value_guard': (
        queried_preparation('defer{if(flag){second<-7u64;{}}else{first<-8u64;{}};};'
                            'if(flag){return first;}else{return second;};',
                            PREPARE_READ.replace('local,other', 'local,other,true'))
        .replace('second:u64 in O)', 'second:u64 in O,flag:bool)'), 2, '', UNWRITTEN_RESULT),
    'query_reads_saved_target_initialized_by_prepared_transfer': (
        queried_preparation('defer (<-)(first,7u64);return first;'), 0, '4\n', ''),
    'query_carries_callee_initialization_to_the_original_target': (
        queried_preparation('first<-7u64;return second;',
            'try{prepare(local,other);return local;}catch(e:const NullPointerException){return 99u64;};'), 0, '4\n', ''),
    'query_carries_named_deferred_initialization_at_invocation': (
        IO+PREPARE.replace(' throws NullPointerException', '')+
        '{if(first is not null){first<-7u64;{}}else{};return second;}'+query(PREPARE_INPUTS+
        'defer{behave @print(local);}finally{};defer prepare(local,other);return 5u64;')+
        'behave @print(result);', 0, '4\n', ''),
    'query_does_not_credit_write_to_rebound_slot_for_saved_result': (
        queried_preparation('ptr saved=first;defer{saved=second;'
            'if(saved is not null){saved<-7u64;{}}else{};};return saved;'), 2, '', UNWRITTEN_RESULT),
    'query_keeps_successful_callee_writes_out_of_failure_paths': (
        queried_preparation('try{return first;}finally{ptr absent:u64=null in here;absent;first<-7u64;{}};',
            PREPARE_READ.replace('return 99u64;', 'return local;')), 2, '', UNINITIALIZED_LOCAL),
    'query_invalidates_written_boolean_evidence_from_overlapping_inputs': (
        IO+TYPES+PREPARE.replace('ptr second:u64 in O', 'ref setter:bool in O,ref gate:bool in O')+
        '{if(gate){setter<-false;if(gate){first<-7u64;{}}else{};}else{first<-7u64;{}};return first;}'+
        query(UNWRITTEN+'val switch<-true;'+PREPARE_READ.replace('local,other', 'local,switch,switch'), ''),
        2, '', UNWRITTEN_RESULT),
    'query_checks_prepared_bits_swap_at_registration': (
        query(UNWRITTEN+'val ready<-3u64;defer (<->)(local,ready);local<-7u64;return local;'),
        2, '', "read of 'left swap target' before definite initialization"),
    'query_rejects_repeated_catch_category': (query('try{}catch(e:const NullPointerException){}'
        'catch(f:const NullPointerException){};return 7u64;'), 2, '', 'duplicate exception category in catch clauses'),
    'query_matches_root_and_untyped_catch_category': (query('try{}catch(e:const Exception){}catch(f){};return 7u64;'),
        2, '', 'duplicate exception category in catch clauses'),
    'query_checks_deferred_catch_category': (query('defer catch(e:const u64){};return 7u64;'),
        2, '', 'catch requires a selected intrinsic exception type'),
    'query_rejects_repeated_deferred_catch_category': (query('defer catch(e:const NullPointerException){}'
        'catch(f:const NullPointerException){};return 7u64;'), 2, '', 'duplicate exception category in catch clauses'),
    'query_checks_guarded_arm_types': (query('try{true}catch(e:const NullPointerException){7u64};return 6u64;'),
        2, '', 'try body and catch arms must have the same result type'),
    'query_uses_deferred_catch_binding_within_handler': (IO +
        query('func e()->u64{return 7u64;};defer catch(e:const NullPointerException){e.category;};return e();') +
        'behave @print(result);', 0, '4\n', ''),
    'query_checks_handlers_without_running_cleanup': (IO +
        query('defer{behave @print(88u64);};defer catch(e:const NullPointerException){};'
              'try{behave @print(99u64);}catch(e:const NullPointerException){}catch(e:const Exception){}'
              'finally{behave @print(77u64);};return 7u64;') +
        'behave @print(result);', 0, '4\n', ''),
    'ordinary_throw_only_requires_result': reject('proc plain(val e:NullPointerException) throws NullPointerException{throw e;}',
                                                  'unproved callable result'),
    'throw_only_requires_coverage': reject('val e<-sample_error();{{throw e;}};'),
    'bare_throw_remains_invalid': reject('try{sample_error();{}}catch(e:const Exception){ {{throw;}}; };',
                                         'bare throw requires an active catch'),
    'bare_throw_after_continuing_catch_remains_invalid': (
        query('val e<-make local NullPointerException;try{throw e;}catch(caught:const NullPointerException){};throw;'),
        2, '', 'bare throw requires an active catch'),
    'query_finally_checks_declared_call_failure': (DECLARED +
        query('try{declared();val late<-7u64;{}}finally{late;{}};return late;'),
        2, '', 'typeof requires a known declaration or operand type'),
    'query_finally_checks_unchecked_call_failure': (STOP +
        query('val e<-make local MissingReturnError;'
              'try{stop(e);val late<-7u64;{}}finally{late;{}};return late;'),
        2, '', 'typeof requires a known declaration or operand type'),
    'query_finally_checks_callable_view_failure': (STOP +
        query('func invoke=stop;val e<-make local MissingReturnError;'
              'try{invoke(e);val late<-7u64;{}}finally{late;{}};return late;'),
        2, '', 'typeof requires a known declaration or operand type'),
    'query_finally_observes_capture_preparation_before_arguments': (
        query('val n<-1u64;func read[n](value:u64)->u64{return n+value;};'
              'try{read({val late<-7u64;late});{}}finally{late;{}};return late;'),
        2, '', 'typeof requires a known declaration or operand type'),
    'query_does_not_capture_caller_binding_for_module_call': (IO +
        'func helper(x:u64)->u64{return x;}func forward(x:u64)->u64{return helper(x);}' +
        query('func helper=forward;try{forward({val early<-7u64;early});{}}finally{early;{}};return early;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_finally_checks_unchecked_failure_through_nested_scope': (STOP +
        query('val e<-make local MissingReturnError;'
              'try{ {{stop(e);}};val late<-7u64;{}}finally{late;{}};return late;'),
        2, '', 'typeof requires a known declaration or operand type'),
    'query_accepts_bodyless_call_after_binding': (IO + 'proc declared()->void;' +
        query('try{val early<-7u64;declared();{}}finally{early;{}};return early;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_rejects_bodyless_call_before_binding': ('proc declared()->void;' +
        query('try{declared();val late<-7u64;{}}finally{late;{}};return late;'),
        2, '', 'typeof requires a known declaration or operand type'),
    'query_handler_uses_presence_at_matching_failure': (IO +
        query('val e<-make local NullPointerException;ref item:u64=1u64;ptr p:u64=null in here;'
              'try{p=item;throw e;}catch(caught:const NullPointerException){if(true){p}else{7u64};{}};'
              'return 9u64;') + 'behave @print(result);', 0, '4\n', ''),
    'query_handler_joins_matching_payload_failure_paths': (
        query('val e<-make local NullPointerException;ref item:u64=1u64;ptr p:u64=null in here;'
              'try{p;p=item;throw e;}catch(caught:const NullPointerException){if(true){p}else{7u64};{}};'
              'return 9u64;'), 2, '', 'conditional scalar value requires a proven non-null target'),
    'query_handler_skips_nonmatching_failure_path': (IO + 'using "std:exceptions" MissingReturnError;' +
        query('val e<-make local NullPointerException;val other<-make local MissingReturnError;'
              'ref item:u64=1u64;ptr p:u64=null in here;'
              'try{if(true){throw other;}else{};p=item;throw e;}'
              'catch(caught:const NullPointerException){if(true){p}else{7u64};{}}catch(rest:const Exception){};'
              'return 9u64;') + 'behave @print(result);', 0, '4\n', ''),
    'query_identity_does_not_load_nullable_payload': (IO +
        query('val e<-make local NullPointerException;ref item:u64=1u64;ptr p:u64=null in here;'
              'try{p is null;p=item;throw e;}catch(caught:const NullPointerException){if(true){p}else{7u64};{}};'
              'return 9u64;') + 'behave @print(result);', 0, '4\n', ''),
    'query_handler_observes_callable_allocation_failure': ('using "std:exceptions" OutOfMemoryError;'
        'func one()->u64{return 1u64;}' +
        query('ref item:u64=1u64;ptr p:u64=item;try{func invoke=one;{}}'
              'catch(e:const OutOfMemoryError){p=null in here;{}};return if(true){p}else{7u64};'),
        2, '', 'conditional scalar value requires a proven non-null target'),
    'query_consumes_caught_failure_before_outer_finally': (IO +
        query('val e<-make local NullPointerException;'
              'try{try{throw e;}catch(caught:const NullPointerException){};val late<-7u64;{}}'
              'finally{late;{}};return late;') + 'behave @print(result);',
        0, '4\n', ''),
    'nested_checked_calls': run('proc pass(val e:NullPointerException)->void throws NullPointerException'
                                '{ {{ {{forward(e);}}; }}; }'
                                'try{pass(sample_error());}catch(caught:const NullPointerException){behave @print(caught.category);};',
                                '4\n', FORWARD),
    'checked_call_only_inside_scope': reject(DECLARED + '{{declared();}};'),
    'checked_call_covered_inside_scope': run('{{try{declared();}catch(e:const NullPointerException){};'
                                             'behave @print(7u64);}};', '7\n', DECLARED),
    'handler_output_is_checked': reject('val e<-sample_error();{{try{throw e;}catch(caught:const NullPointerException){throw caught;};}};'),
    'finally_output_is_checked': reject('val e<-sample_error();{{try{}finally{throw e;};}};'),
    'finally_replaces_checked': run('val e<-sample_error();val result<-{{try{throw e;}finally{return 9u64;};}};'
                                    'behave @print(result);', '9\n'),
    'checked_operation_in_scope': run('try{ {{ptr absent:u64=null in here;absent;{}}}; }'
                                      'catch(caught:const NullPointerException){behave @print(caught.category);};', '4\n'),
    'scope_local_catch': run('val e<-sample_error();{{try{throw e;}catch(caught:const NullPointerException)'
                            '{behave @print(caught.category);};}};', '4\n'),
    'root_throw_survives_narrower_evidence': reject('val e:Exception<-sample_error();'
                                                   'try{ {{throw e;}}; }catch(caught:const NullPointerException){};', BROAD),
    'root_call_survives_narrower_evidence': reject(ROOT + 'val e<-sample_error();'
                                                 'try{ {{broad(e);}}; }catch(caught:const NullPointerException){};', BROAD),
    'root_covered_in_enclosing_caller': run('val payload<-sample_error();'
                                           'try{ {{broad(payload);}}; }catch(caught:const Exception){behave @print(caught.category);};',
                                           '4\n', ROOT),
    'root_covered_locally': run('val e:Exception<-sample_error();{{try{throw e;}catch(caught:const Exception)'
                               '{behave @print(caught.category);};}};', '4\n'),
    'ordinary_defer_runs_before_propagation': run('val e<-sample_error();try{ {{defer{behave @print(7u64);};throw e;}}; }'
                                                 'catch(caught:const NullPointerException){behave @print(caught.category);};', '7\n4\n'),
    'ordinary_catch_within_defer': run('val e<-sample_error();'
                                      'defer {try{ {{throw e;}}; }catch(caught:const NullPointerException)'
                                      '{behave @print(caught.category);};};', '4\n'),
    'enclosing_throws_cannot_cover_deferred_scope': reject('proc sample(val e:NullPointerException)->void throws NullPointerException'
                                                         '{defer {{throw e;}};}', DEFER),
    'owning_scope_cannot_export_checked_defer': reject('proc sample(val e:NullPointerException)->void throws NullPointerException'
                                                      '{ {{defer {throw e;};}}; }', DEFER),
    'ordinary_outer_catch_cannot_cover_checked_defer': reject('val e<-sample_error();'
                                                             'try{ {{defer {throw e;};}}; }'
                                                             'catch(caught:const NullPointerException){};', DEFER),
    'legacy_deferred_handler_cannot_consume_body_failure': reject('val e<-sample_error();'
                                                                '{{defer catch(caught:const NullPointerException){};throw e;}};', DEFER),
    'legacy_deferred_handler_in_caller_cannot_consume_body_failure': reject('val e<-sample_error();'
        'defer catch(caught:const NullPointerException){};try{ {{throw e;}};}finally{};', DEFER),
    'eager_defer_preparation_uses_ordinary_catch': run('val flag<-true;val e<-sample_error();'
        'try{defer report({{if(flag){throw e;}else{return 7u64;};}});}'
        'catch(caught:const NullPointerException){behave @print(caught.category);};',
        '4\n', 'proc report(value:u64)->void{behave @print(value);}'),
    'generic_checked_interface': run('proc run(val e:T)->void where T{'
        'try{ {{throw e;}}; }catch(caught:const NullPointerException){behave @print(caught.category);};}'
        'run(sample_error());', '4\n'),
    'separate_scope_interfaces': (IO + TYPES + PAYLOAD +
        'using "std:exceptions" MissingReturnError;func missing()->u64{}'
        'func unchecked()->const MissingReturnError{try{missing();{}}catch(caught:const MissingReturnError)'
        '{return caught;};return unchecked();}'
        'proc checked_path(val e:NullPointerException)->void throws NullPointerException{ {{throw e;}}; }'
        'proc unchecked_path(val e:MissingReturnError)->void{ {{throw e;}}; }'
        'try{checked_path(sample_error());}catch(checked:const NullPointerException){behave @print(checked.category);};'
        'try{unchecked_path(unchecked());}catch(failed:const MissingReturnError){behave @print(failed.category);};',
        0, '4\n5\n', '[-Wmissing-return]'),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
