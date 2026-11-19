#!/usr/bin/env python3
"""Full scopes share ordinary value inputs, inference and local completion."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
BASE = 'func one()->const u64{return 1u64;}func two()->const u64{return 2u64;}'
IDENTITY = 'func identity(x:T)->const T where T{return x;}'
APPLY = 'func apply(f:func(T)->const T,x:T)->const T where T{return f(x);}'
FIXED = 'func fixed(f:func(u64)->const u64,x:u64)->const u64{return f(x);}'
PTR_RESULT = 'proc run(ref picked:u64 in O,ptr spare:u64 in O)->ptr u64 in O where {owner O}{'
PTR_RESULT_INPUTS = 'val value_input<-7u64,uninitialized_input<-make local u64;'
OBSERVE_PTR_RESULT = ('if returning (ptr saved){if(saved is not null){behave @print(saved);}'
                      'else{behave @print(0u64);};}')
READ_PTR_RESULT = 'defer{' + OBSERVE_PTR_RESULT + '};'
WRITE_PTR_RESULT = 'defer{if returning (ptr saved){if(saved is not null){saved<-9u64;{}}else{}}};'
GUARDED_QUERY = ('using "std:exceptions" NullPointerException;val e<-make local NullPointerException;')
GUARDED_TRY = ('try{if(true){throw e;}else{};val n<-4294967296u64;{}}'
               'catch(c:const NullPointerException){return 7u32;}')
FINALLY_READ = 'finally{late;{}};return late;'
QUERY_EVIDENCE = 'typeof requires a known declaration or operand type'


def query(body):
    return 'val result<-make local typeof({{' + body + '}}){4u64};'


def shared_pointer_cleanup(body):
    return (IO + 'func prepare(ptr first:u64 in O,ptr second:u64 in O)->ptr u64 in O where {owner O}{' + body +
            'return first;}val raw<-make local u64;ptr received=prepare(raw,raw);'
            'if(received is not null){behave @print(received);}else{};')


runner.CASES = {
    'function_tail_and_explicit_return_share_inference': (IO+
        'func sample(flag:bool){if(flag){return 3u32;}else{};12u64;}'
        'behave @print(sample(true));behave @print(sample(false));', 0, '3\n12\n', ''),
    'recursive_function_tail_has_independent_anchor': (IO+
        'func sum_to(n:u64){if(n==0u64){return 0u64;}else{};n+sum_to(n-1u64)}'
        'behave @print(sum_to(4u64));', 0, '10\n', ''),
    'recursive_tail_requires_independent_evidence': (
        'func recur(n:u64){n+recur(n)}recur(1u64);', 2, '', 'independent result contract'),
    'prepared_local_tail_is_visible_to_cleanup_replacement': (IO+
        'func retained(val input:u64)->u64{defer{input<-9u64;};input;}'
        'func replace()->u64{defer{return 8u64;};7u64}behave @print(retained(4u64));behave @print(replace());',
        0, '4\n8\n', ''),
    'here_local_tail_and_single_brace_feedback': (IO+
        'func update(here body()->u64)->u64{body()+1u64}'
        'proc outer()->void{val local<-{{6u64;}};behave @print(local);'
        'behave @print(update(){{7u64}});behave @print(update(){7u64});return;}outer();',
        0, '6\n8\n8\n', ''),
    'local_tail_supplies_type_evidence_inside_proc': (IO+
        'proc run()->void{val n<-make local typeof({{7u64;}}){11u64};behave @print(n);return;}run();',
        0, '11\n', ''),
    'unreached_value_is_not_a_void_function_return': (
        'func done()->void{return;7u64;}done();', 0, '', ''),
    'value_tail_in_void_function_requires_discard': (
        'func wrong()->void{7u64;}wrong();', 2, '', 'value-producing tail in a void callable'),
    'ordinary_void_procedure_requires_explicit_return_after_cleanup': (
        IO+'using "std:exceptions" MissingReturnError;'
        'proc forgotten()->void{defer behave @print(5u64);}'
        'try{forgotten();}catch(e:const MissingReturnError){behave @print(6u64);};',
        0, '5\n6\n', '[-Wmissing-return]'),
    'ordinary_void_procedure_can_return_from_cleanup': (
        'proc finished()->void{defer{return;};}finished();', 0, '', ''),
    'return_scalar_filter_matches_prepared_type': (IO+
        'proc run()->u64{defer{if returning (val wrong:u32){return 11u64;}'
        'else if returning (saved:u64){if(saved==7u64){return 8u64;}else{return 99u64;}}};return 7u32;}'
        'behave @print(run());',0,'8\n',''),
    'return_scalar_filter_uses_concrete_generic_result': (IO+
        'proc run(value:T)->const T where T{defer{if returning (saved:u64){behave @print(saved);}'
        'else{behave @print(1u64);}};return value;}behave @print(run(3u32));behave @print(run(7u64));',
        0,'1\n3\n7\n7\n',''),
    'return_pattern_snapshot_can_enter_nofail_full_scope': (IO+
        'proc run()->u64{defer{if returning (val saved){saved<-12u64;{{behave @print(saved);}};}};return 7u64;}'
        'behave @print(run());',0,'12\n7\n',''),
    'return_pattern_condition_cannot_anchor_the_inferred_result': (
        'using "std:exceptions" MissingReturnError;func missing()->u64{}'
        'proc run(problem:MissingReturnError){defer{return if returning (val saved:u64){saved}else{throw problem;};};}'
        'try{missing():void;}catch(e:const MissingReturnError){run(e);};',2,'','independent result contract'),
    'return_owner_filter_compares_actual_owner_roles': (IO+
        'proc run(ref source:u64 in A,ref other:u64 in B)->ref u64 in A where {owner A,owner B}{'
        'defer{if returning (in B){behave @print(1u64);}else if returning (){behave @print(2u64);}};return source;}'
        'proc split(ref source:u64 in O)->void where {owner O}{val local<-6u64;run(source,local):void;return;}'
        'val a<-7u64,b<-8u64;run(a,b):void;split(a);',0,'1\n2\n',''),
    'return_owner_filter_matches_typed_null': (IO+
        'proc run(ref source:u64 in O)->ptr u64 in O where {owner O}{defer{'
        'if returning (in O){behave @print(1u64);}else{behave @print(2u64);}};return null;}'
        'val a<-7u64;ptr saved=run(a);behave @print((saved is null));',0,'1\ntrue\n',''),
    'return_owner_filter_rejects_nearest_non_owner': (
        'proc run(requested:u64)->u64{defer{if returning (in requested){}};return 7u64;}run(1u64);',
        2,'',"nearest binding 'requested' is not an owner"),
    'return_reference_pattern_keeps_the_selected_target': (IO+
        'proc run(ref left:u64 in O,ref right:u64 in O,which:bool)->ref u64 in O where {owner O}{'
        'defer{if returning (saved:u64 in O){behave @print(saved);}};'
        'if(which){return left;}else{return right;};}'
        'val a<-7u64,b<-8u64;behave @print(run(a,b,true));behave @print(run(a,b,false));',0,'7\n7\n8\n8\n',''),
    'rebinding_the_reference_pattern_keeps_the_prepared_return': (IO+
        'proc run(ref source:u64 in O,ref other:u64 in O)->ref u64 in O where {owner O}{'
        'defer{if returning (ref saved){saved<-9u64;saved=other;behave @print(saved);}};return source;}'
        'val a<-7u64,b<-8u64;behave @print(run(a,b));behave @print(a);',0,'8\n9\n9\n',''),
    'return_pointer_pattern_preserves_null_and_mode': (IO+
        'proc run(ref source:u64 in O,absent:bool)->ptr u64 in O where {owner O}{defer{'
        'if returning (ref wrong:u64 in O){behave @print(1u64);}'
        'else if returning (ptr saved:u64 in O){behave @print((saved is null));}};'
        'if(absent){return null;}else{return source;};}'
        'val a<-7u64;run(a,true):void;run(a,false):void;',0,'true\nfalse\n',''),
    'return_reference_pattern_compares_actual_owner_roles': (IO+
        'proc run(ref source:u64 in A,ref other:u64 in B)->ref u64 in A where {owner A,owner B}{'
        'defer{if returning (ref saved:u64 in B){behave @print(saved);}else if returning (){behave @print(9u64);}};return source;}'
        'proc split(ref source:u64 in O)->void where {owner O}{val local<-6u64;run(source,local):void;return;}'
        'val a<-7u64,b<-8u64;run(a,b):void;split(a);',0,'7\n9\n',''),
    'return_reference_pattern_observes_replacement_in_finally': (IO+
        'proc run(ref source:u64 in O,ref other:u64 in O)->ref u64 in O where {owner O}{'
        'defer{try{return other;}finally{if returning (ref saved){behave @print(saved);}};};return source;}'
        'val a<-7u64,b<-8u64;behave @print(run(a,b));',0,'8\n8\n',''),
    'return_reference_pattern_filters_access_before_branch_execution': (IO+
        'proc run(ref source:const u64 in O)->ref const u64 in O where {owner O}{'
        'defer{if returning (ref writable:u64 in O){writable<-1u64;{}}'
        'else if returning (ref saved:const u64){behave @print(saved);}};return source;}'
        'val a<-7u64;behave @print(run(a));',0,'7\n7\n',''),
    'return_pointer_pattern_does_not_initialize_its_payload': (IO+
        'proc run(ptr source:u64 in O)->ptr u64 in O where {owner O}{'
        'defer{if returning (ptr saved){if(saved is not null){behave @print(saved);}else{};}};return source;}'
        'val a<-make local u64;run(a):void;',2,'','initialization'),
    'return_pointer_pattern_reads_only_the_prepared_target': (IO+PTR_RESULT+
        READ_PTR_RESULT+'return picked;}'+PTR_RESULT_INPUTS+'run(value_input,uninitialized_input):void;',0,'7\n',''),
    'return_pointer_pattern_null_reads_no_targets': (IO+PTR_RESULT+
        READ_PTR_RESULT+'return null;}'+PTR_RESULT_INPUTS+'run(value_input,uninitialized_input):void;',0,'0\n',''),
    'return_pointer_pattern_checks_readiness_after_replacement': (IO+PTR_RESULT+
        READ_PTR_RESULT+'defer{return spare;};return picked;}'+PTR_RESULT_INPUTS+
        'run(value_input,uninitialized_input):void;',2,'',"initialization of 'uninitialized_input'"),
    'return_pointer_pattern_uses_the_result_suspended_by_finally': (IO+PTR_RESULT+
        'defer{try{return picked;}finally{'+OBSERVE_PTR_RESULT+'};};return spare;}'+PTR_RESULT_INPUTS+
        'run(value_input,uninitialized_input):void;',0,'7\n',''),
    'return_pointer_pattern_write_proves_result_ready_when_present': (IO+PTR_RESULT+
        WRITE_PTR_RESULT+'return spare;}'+PTR_RESULT_INPUTS+
        'ptr value=run(value_input,uninitialized_input);if(value is not null){behave @print(value);}else{};',0,'9\n',''),
    'return_pointer_pattern_write_prepares_later_cleanup_read': (IO+PTR_RESULT+
        READ_PTR_RESULT+WRITE_PTR_RESULT+'return spare;}'+PTR_RESULT_INPUTS+
        'run(value_input,uninitialized_input):void;',0,'9\n',''),
    'return_pointer_pattern_write_does_not_prepare_another_target': (IO+PTR_RESULT+
        READ_PTR_RESULT+'defer{return spare;};'+WRITE_PTR_RESULT+'return picked;}'+PTR_RESULT_INPUTS+
        'run(value_input,uninitialized_input):void;',2,'',"initialization of 'uninitialized_input'"),
    'failed_writer_registration_leaves_unwritten_replacement': (
        shared_pointer_cleanup('defer{return second;};'+WRITE_PTR_RESULT),2,'','initialization'),
    'same_cleanup_writes_before_return_through_shared_formal': (
        shared_pointer_cleanup('defer{if returning (ptr saved){if(saved is not null){saved<-9u64;{}}'
                               'else{};return second;}};'),0,'9\n',''),
    'guarded_replacement_requires_a_prepared_return': (
        shared_pointer_cleanup('defer{if returning (){return second;}};'+WRITE_PTR_RESULT),0,'9\n',''),
    'return_inspection_uses_closed_indirect_call_outcomes': (IO+IDENTITY+
        'proc run(f:func(u64)->const u64)->u64{defer{if returning (){return f(7u64);}};return 1u64;}'
        'behave @print(run(identity));',0,'7\n',''),
    'return_function_pattern_binds_a_proved_callable_set': (IO+IDENTITY+
        'func fixed(n:u64)->const u64{return 9u64;}'
        'proc run(flag:bool)->const func(u64)->const u64{defer{if returning (val saved){behave @print(saved(7u64));}};'
        'if(flag){return identity;}else{return fixed;};}'
        'func a=run(true);func b=run(false);behave @print(a(11u64));behave @print(b(11u64));',0,'7\n9\n11\n9\n',''),
    'writing_a_function_pattern_holder_keeps_the_pending_return': (IO+IDENTITY+
        'func fixed(n:u64)->const u64{return 9u64;}'
        'proc run()->const func(u64)->const u64{defer{if returning (val saved:func(u64)->const u64)'
        '{saved<-fixed;behave @print(saved(7u64));}};return identity;}'
        'func held=run();behave @print(held(11u64));',0,'9\n11\n',''),
    'return_function_pattern_with_no_prepared_value_uses_the_else_arm': (IO+
        'using "std:exceptions" MissingReturnError;proc run()->const func(u64)->const u64{'
        'defer{if returning (saved:func(u64)->const u64){}else{behave @print(7u64);}};}'
        'try{run():void;}catch(e:const MissingReturnError){behave @print(8u64);};',0,'7\n8\n','[-Wmissing-return]'),
    'return_inspection_preserves_closed_indirect_failure': (
        'func missing(n:u64)->const u64{}'
        'proc run(f:func(u64)->const u64)->u64{defer{if returning (){return f(7u64);}};return 1u64;}run(missing);',
        2,'','return inspection requires independent deferred failure delivery'),
    'here_group_distinct_feedback_and_break': (IO+
        'func plus_one(here body()->u64)->u64{return body()+1u64;}'
        'func relay(here body()->u64)->u64{return plus_one(body)+10u64;}'
        'behave @print(relay(){continue 7u64;});behave @print(relay(){break 7u64;});',0,'18\n7\n',''),
    'here_group_return_crosses_receiver_cleanup': (IO+
        'proc relay(here body()->void)->void{defer behave @print(42u64);'
        'try{body();}finally{behave @print(41u64);return;};}'
        'proc run()->u64{relay(){return 7u64;};return 0u64;}behave @print(run());',0,'41\n42\n7\n',''),
    'return_inspection_calls_known_forwarded_group': (IO+
        'func relay(here body()->void)->void{body();}'
        'func forward(here body()->void)->void{relay(body);}'
        'proc run()->void{defer{if returning (){forward(){};behave @print(6u64);}};return;}run();',0,'6\n',''),
    'return_inspection_calls_group_with_protected_return': (IO+
        'proc relay(here body()->void)->void{try{body();}finally{behave @print(4u64);return;};}'
        'proc run()->u64{defer{if returning (){relay(){return 9u64;};}};return 1u64;}behave @print(run());',0,'4\n9\n',''),
    'forwarded_group_observes_supplier_finally_return': (IO+
        'func relay(here body()->void)->void{body();}'
        'func forward(here body()->void)->void{relay(body);}'
        'proc run(flag:bool)->u64{defer{try{if(flag){return 7u64;}else{};}finally{forward(){'
        'if returning (){behave @print(1u64);}else{behave @print(2u64);}};};return 9u64;};}'
        'behave @print(run(true));behave @print(run(false));',0,'1\n7\n2\n9\n',''),
    'forwarded_group_binds_prepared_supplier_bits': (IO+
        'func relay(here body()->void)->void{body();}func forward(here body()->void)->void{relay(body);}'
        'proc run()->u64{defer{forward(){if returning (val saved:u64){behave @print(saved);}};};return 7u32;}'
        'behave @print(run());',0,'7\n7\n',''),
    'group_finally_binds_protected_return_separately_from_feedback': (IO+
        'func relay(here body()->u32)->void{body():void;}'
        'proc run()->u64{defer{relay(){try{return 21u64;}finally{'
        'if returning (val saved:u64){behave @print(saved);return 25u64;}};1u32};};return 7u64;}'
        'behave @print(run());',0,'21\n25\n',''),
    'group_finally_observes_supplier_return_without_observing_feedback': (IO+
        'func relay(here body(taken:bool)->void,flag:bool)->void{body(flag);}'
        'proc run(flag:bool)->u64{defer{relay(flag)(taken)=>{try{if(taken){return 7u64;}else{continue;};}finally{'
        'if returning (){behave @print(1u64);}else{behave @print(2u64);}};};return 9u64;};}'
        'behave @print(run(true));behave @print(run(false));',0,'1\n7\n2\n9\n',''),
    'group_finally_cancellation_preserves_inherited_return_only': (IO+
        'using "std:exceptions" MissingReturnError;func missing()->u64{}'
        'func relay(here body()->void)->void{body();}'
        'proc run(flag:bool)->u64{defer{relay(){try{try{return 7u64;}finally{missing():void;};}'
        'catch(e:const MissingReturnError){if returning (){behave @print(1u64);}else{behave @print(2u64);}};};'
        'return 9u64;};if(flag){return 8u64;}else{};}'
        'behave @print(run(true));behave @print(run(false));',0,'1\n9\n2\n9\n','[-Wmissing-return]'),
    'here_group_local_finally_cancels_return_before_catch': (IO+
        'using "std:exceptions" MissingReturnError;func missing()->u64{}'
        'func relay(here body()->void)->void{body();}'
        'proc run()->u64{relay(){try{try{return 7u64;}finally{missing():void;};}'
        'catch(e:const MissingReturnError){};};return 9u64;}behave @print(run());',
        0,'9\n','[-Wmissing-return]'),
    'here_group_cancelled_return_contributes_no_supplier_exit': (IO+
        'using "std:exceptions" MissingReturnError;func missing()->u64{}'
        'func error()->const MissingReturnError{try{missing():void;}catch(e:const MissingReturnError){return e;};}'
        'func relay(here body()->void)->void{body();}'
        'proc run()->u64{val n<-make local u64;defer{if returning (){behave @print(n);}};'
        'relay(){try{try{return 7u64;}finally{throw error();};}catch(e){};};n<-9u64;return n;}'
        'behave @print(run());',0,'9\n9\n','[-Wmissing-return]'),
    'here_group_local_finally_cancels_break_before_catch': (IO+
        'using "std:exceptions" MissingReturnError;func missing()->u64{}'
        'func relay(here body()->void)->u64{body();4u64}'
        'proc run()->u64{return relay(){try{try{break 7u64;}finally{missing():void;};}'
        'catch(e:const MissingReturnError){};};}behave @print(run());',0,'4\n','[-Wmissing-return]'),
    'here_group_checked_failure_bypasses_receiver_catch': (IO+'using "std:exceptions" NullPointerException;'+
        'func fail_now()->void throws NullPointerException{ptr p:u64=null in here;p;{};}'
        'proc relay(here body()->void throws NullPointerException)->void{try{body();}'
        'catch(e:const NullPointerException){behave @print(80u64);}finally{behave @print(81u64);};return;}'
        'proc run()->void{try{relay(){fail_now();};}catch(e:const NullPointerException){behave @print(82u64);};return;}run();',0,'81\n82\n',''),
    'here_group_receiver_recovers_own_cleanup_failure': (IO+'using "std:exceptions" NullPointerException;'+
        'func fail_now()->void throws NullPointerException{ptr p:u64=null in here;p;{};}'
        'proc relay(here body()->void)->void{try{try{body();}finally{fail_now();};}'
        'catch(e:const NullPointerException){behave @print(32u64);};behave @print(90u64);return;}'
        'proc run()->u64{relay(){return 7u64;};return 0u64;}behave @print(run());',0,'32\n7\n',''),
    'here_group_recursive_break_keeps_call_activation': (IO+
        'func relay(here body()->void)->u64{body();return 99u64;}'
        'func walk(depth:u64)->u64{val d<-depth;return relay(){if(d>0u64){d<-walk(d-1u64);{};}else{};break d+1u64;};}'
        'behave @print(walk(4u64));',0,'5\n',''),
    'generator_for_and_here_share_receiving': (IO+
        'func generate() yields u64->bool{if(!yield(1u64)){return;}else{};yield(2u64);}'
        'proc run()->void{val left<-0u64,right<-0u64;'
        'for val v<-generate(){left<-left*10u64+v;continue true;};'
        'generate()(v)=>{right<-right*10u64+v;true};behave @print(left);behave @print(right);return;}run();',0,'12\n12\n',''),
    'generator_single_value_receives_into_existing_slot': (IO+
        'func generate() yields u64{yield(1u64);yield(2u64);}'
        'proc run()->void{val a<-0u64,b<-0u64,rank<-0u64;'
        'for a<-generate(){rank<-rank*10u64+a;};generate()(b<-)=>{b<-b+10u64;};'
        'behave @print(rank);behave @print(b);return;}run();',0,'12\n12\n',''),
    'borrowed_handoffs_share_receiving_rules': (IO+
        'proc twice(source:u64 in O) yields ref u64 in O where {owner O}{yield(source);source<-9u64;yield(source);return;}'
        'val left<-1u64,right<-1u64,total<-0u64;'
        'for ref item=twice(left){item<-item+1u64;};'
        'twice(right)(val item<-)=>{item<-item+1u64;total<-total*10u64+item;};'
        'behave @print(left);behave @print(right);behave @print(total);',0,'10\n9\n30\n',''),
    'borrowed_handoff_uses_producer_activation_owner': (IO+
        'proc scratch() yields ref u64 in here{val cell<-3u64;yield(cell);yield(cell);return;}'
        'val peer<-3u64,total<-0u64;for ref item=scratch(){'
        'if(item is not peer){total<-total*10u64+item;{};}else{};item<-item+1u64;};'
        'behave @print(total);',0,'34\n',''),
    'borrowed_handoff_preserves_input_owner_and_target_identity': (IO+
        'proc once(ref source:u64 in O) yields ref u64 in O where {owner O}{yield(source);return;}'
        'val target<-5u64,result<-0u64;once(target)(item)=>{{'
        'if(item is target){result<-7u64;{};}else{};}};behave @print(result);',0,'7\n',''),
    'borrowed_here_forwarding_preserves_declared_owner': (IO+
        'func relay(here body(ref item:u64 in P)->void,ref source:u64 in P)->void where {owner P}{body(source);return;}'
        'func outer(here body(ref item:u64 in O)->void,ref source:u64 in O)->void where {owner O}{relay(body,source);return;}'
        'val target<-2u64;outer(target)(ref item=)=>{item<-item+3u64;};behave @print(target);',0,'5\n',''),
    'borrowed_here_forwarding_rejects_new_invocation_owner': (
        'func relay(here body(ref item:u64 in here)->void)->void{val local<-1u64;body(local);return;}'
        'func outer(here body(ref item:u64 in here)->void)->void{relay(body);return;}outer()(ref item=)=>{};',
        2,'',"new receiver's invocation owner"),
    'borrowed_handoff_rejects_different_actual_owner': (
        'proc wrong(source:u64 in O) yields ref u64 in O where {owner O}{val local<-5u64;yield(local);return;}'
        'val target<-3u64;for ref item=wrong(target){};',2,'','borrowed here input has a different actual owner'),
    'borrowed_handoff_rejects_mutation_through_readonly_control': (
        'proc inspect(source:const u64 in O) yields ref const u64 in O where {owner O}{yield(source);return;}'
        'val target<-3u64;for ref item=inspect(target){item<-4u64;};',2,'','readonly'),
    'labelled_handoff_orders_offers_and_selects_readonly_fields': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'proc scan(line:const u64 in O) yields {id:u64; ref cell:const u64 in O; audit:u64;} -> bool where {owner O}{'
        'if(!yield(mark(24u64)->id,mark(8u64)->audit,line->cell)){return;}else{};'
        'yield(mark(42u64)->id,line->cell,mark(9u64)->audit);return;}'
        'proc run()->void{val line<-75u64;'
        'for {ref payload=cell;val number<-id;} := scan(line){behave @print(payload);behave @print(number);continue false;};'
        'scan(line)({val number<-id;ref payload=cell;})=>{behave @print(payload);behave @print(number);false};return;}run();',
        0,'24\n8\n75\n24\n24\n8\n75\n24\n',''),
    'labelled_handoff_projects_two_owners_and_a_snapshot': (IO+
        'proc scan(ref left:u64 in A,ref right:u64 in B) yields {ref z:u64 in A;ref a:u64 in B;} where {owner A,owner B}{'
        'yield(left->z,right->a);return;}'
        'val x<-51u64;{{val y<-72u64;'
        'for {ref first=z;val previous<-z;ref other=a;} := scan(x,y){first<-other;other<-previous;};'
        'behave @print(x);behave @print(y);}};',0,'72\n51\n',''),
    'labelled_handoff_empty_header_preserves_offers': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'proc scan() yields {second:u64;first:u64;} -> bool{'
        'if(yield(mark(2u64)->second,mark(1u64)->first)){behave @print(3u64);}else{};return;}'
        'scan()({})=>{true};',0,'2\n1\n3\n',''),
    'labelled_handoff_type_query_does_not_offer': (IO+
        'func scan() yields {value:u64;flag:bool;} -> bool{'
        'val proof<-make local typeof(yield(99u64->value,true->flag)){false};'
        'if(proof){yield(true->flag,88u64->value):void;}else{};}'
        'proc run()->void{for {val n<-value;} := scan(){behave @print(n);continue true;};behave @print(7u64);return;}run();',
        0,'7\n',''),
    'labelled_handoff_single_field_uses_shorthand': (IO+
        'func scan() yields {id:u64;}{yield(25u64->id);}'
        'proc run()->void{for val n<-scan(){behave @print(n);};scan()(n)=>{behave @print(n);};return;}run();',
        0,'25\n25\n',''),
    'labelled_handoff_checked_failure_bypasses_producer_catch': (IO+'using "std:exceptions" NullPointerException;'+
        'func fail_now()->void throws NullPointerException{ptr p:u64=null in here;p;{};}'
        'proc scan() yields {id:u64;flag:bool;} -> bool{try{yield(true->flag,24u64->id):void;}'
        'catch(e:const NullPointerException){behave @print(80u64);}finally{behave @print(81u64);};return;}'
        'proc run()->void{try{for {val number<-id;} := scan(){behave @print(number);fail_now();true};}'
        'catch(e:const NullPointerException){behave @print(82u64);};return;}run();',0,'24\n81\n82\n',''),
    'labelled_handoff_requires_all_labels': (
        'proc scan() yields {id:u64;flag:bool;}{yield(24u64->id);return;}scan()({})=>{};',
        2,'',"missing yielded label 'flag'"),
    'labelled_handoff_rejects_duplicate_labels': (
        'proc scan() yields {id:u64;flag:bool;}{yield(24u64->id,25u64->id,true->flag);return;}scan()({})=>{};',
        2,'',"duplicate yielded label 'id'"),
    'labelled_handoff_rejects_unknown_offer_label': (
        'proc scan() yields {id:u64;}{yield(24u64->unknown);return;}scan()({})=>{};',
        2,'',"unknown yielded label 'unknown'"),
    'labelled_handoff_rejects_positional_offers': (
        'proc scan() yields {id:u64;}{yield(24u64);return;}scan()({})=>{};',
        2,'','a labelled yield requires a label for every operand'),
    'labelled_handoff_rejects_unknown_receiving_label': (
        'proc scan() yields {id:u64;}{yield(24u64->id);return;}scan()({val n<-other;})=>{};',
        2,'',"unknown receiving label 'other'"),
    'labelled_handoff_rejects_positional_multi_field_reception': (
        'proc scan() yields {id:u64;flag:bool;}{yield(24u64->id,true->flag);return;}scan()(n,b)=>{};',
        2,'','a multi-field here contract requires a labelled receiving header'),
    'labelled_handoff_rejects_an_unproved_offer_owner': (
        'proc scan(ref left:u64 in A,ref right:u64 in B) yields {id:u64;ref line:u64 in A;} where {owner A,owner B}{'
        'yield(24u64->id,right->line);return;}'
        'val left<-1u64;{{val right<-2u64;for {} := scan(left,right){};return;}};',
        2,'','borrowed here input requires an unproved fixed owner equality'),
    'labelled_handoff_rejects_a_distinct_offer_owner': (
        'proc scan(ref storage:u64 in O) yields {id:u64;ref line:u64 in O;} where {owner O}{'
        'val scratch<-3u64;yield(24u64->id,scratch->line);return;}'
        'val storage<-1u64;for {} := scan(storage){};',
        2,'','borrowed here input has a different actual owner'),
    'labelled_handoff_rejects_mutation_of_readonly_selection': (
        'proc scan(line:const u64 in O) yields {id:u64;ref cell:const u64 in O;} where {owner O}{yield(24u64->id,line->cell);return;}'
        'val line<-1u64;for {ref selected=cell;} := scan(line){selected<-2u64;};',2,'','readonly'),
    'lexical_handoff_nested_stage_keeps_labelled_borrow_and_feedback': (IO+
        'proc rows(line:const u64 in O) yields {id:u64;ref cell:const u64 in O;} -> bool where {owner O}{'
        'behave @print(17u64);if(!yield(line->cell,17u64->id)){return;}else{};'
        'behave @print(24u64);if(!yield(24u64->id,line->cell)){return;}else{};'
        'behave @print(25u64);if(!yield(line->cell,25u64->id)){return;}else{};'
        'behave @print(42u64);yield(42u64->id,line->cell);return;}'
        'proc filter(line:const u64 in O) yields {ref cell:const u64 in O;id:u64;} -> bool where {owner O}{'
        'for {val n<-id;ref item=cell;} := rows(line){'
        'if(n==17u64){continue true;}else{};continue yield(item->cell,n->id);};return;}'
        'proc run()->void{val line<-75u64;for {ref item=cell;val n<-id;} := filter(line){'
        'behave @print(item);continue n!=25u64;};return;}run();',0,'17\n24\n75\n25\n75\n',''),
    'lexical_handoff_downstream_return_runs_both_cleanups': (IO+
        'proc inner() yields u64{try{yield(7u64);behave @print(80u64);}finally{behave @print(31u64);};return;}'
        'proc stage() yields u64{try{for val n<-inner(){yield(n);};behave @print(81u64);}'
        'finally{behave @print(32u64);};return;}'
        'proc run()->u64{for val n<-stage(){return n;};return 0u64;}behave @print(run());',0,'31\n32\n7\n',''),
    'lexical_handoff_downstream_failure_bypasses_both_catches': (IO+'using "std:exceptions" NullPointerException;'+
        'func fail_now()->void throws NullPointerException{ptr p:u64=null in here;p;{};}'
        'proc inner() yields u64{try{yield(24u64);}'
        'catch(e:const NullPointerException){behave @print(80u64);}finally{behave @print(31u64);};return;}'
        'proc stage() yields u64{try{for val n<-inner(){yield(n);};}'
        'catch(e:const NullPointerException){behave @print(81u64);}finally{behave @print(32u64);};return;}'
        'proc run()->void{try{for val n<-stage(){behave @print(n);fail_now();};}'
        'catch(e:const NullPointerException){behave @print(33u64);};return;}run();',0,'24\n31\n32\n33\n',''),
    'lexical_handoff_inner_break_leaves_outer_consumer_live': (IO+
        'proc inner() yields u64{yield(7u64);behave @print(80u64);return;}'
        'proc stage() yields u64{for val n<-inner(){yield(n);break;};yield(8u64);return;}'
        'proc run()->void{for val n<-stage(){behave @print(n);};return;}run();',0,'7\n8\n',''),
    'lexical_handoff_recursive_suppliers_keep_distinct_destinations': (IO+
        'func inner() yields u64{yield(7u64);yield(80u64);}'
        'func stage() yields u64{for val n<-inner(){yield(n);};}'
        'proc run(depth:u64)->void{val remaining<-depth;for val n<-stage(){'
        'if(remaining>0u64){run(remaining-1u64);}else{};behave @print(n+remaining);break;};'
        'behave @print(depth);return;}run(2u64);',0,'7\n0\n8\n1\n9\n2\n',''),
    'lexical_handoff_inner_cleanup_recovery_keeps_downstream_break': (IO+'using "std:exceptions" NullPointerException;'+
        'func fail_now()->void throws NullPointerException{ptr p:u64=null in here;p;{};}'
        'proc inner() yields u64 throws NullPointerException{try{yield(7u64);}'
        'finally{behave @print(31u64);fail_now();};return;}'
        'proc stage() yields u64{try{for val n<-inner(){yield(n);};}'
        'catch(e:const NullPointerException){behave @print(32u64);}finally{behave @print(33u64);};'
        'behave @print(80u64);return;}'
        'proc run()->void{for val n<-stage(){behave @print(n);break;};behave @print(34u64);return;}run();',
        0,'7\n31\n32\n33\n34\n',''),
    'lexical_handoff_forwarding_preserves_break_destination': (IO+
        'func inner() yields u64{yield(7u64);yield(8u64);}'
        'func send(here step(x:u64)->void,x:u64)->void{step(x);}'
        'func stage() yields u64{for val n<-inner(){send(yield,n);};yield(80u64);}'
        'proc run()->void{for val n<-stage(){behave @print(n);break;};behave @print(9u64);return;}run();',0,'7\n9\n',''),
    'lexical_handoff_full_scope_keeps_borrowed_owner_and_feedback': (IO+
        'proc stage(ref line:const u64 in O) yields ref const u64 in O -> bool where {owner O}{'
        'val more<-{{yield(line)}};if(more){yield(line):void;}else{};return;}'
        'proc run()->void{val line<-25u64;for ref item=stage(line){behave @print(item);continue false;};return;}run();',
        0,'25\n',''),
    'lexical_handoff_full_scope_propagates_downstream_return': (IO+
        'proc stage() yields u64{try{ {{yield(7u64);return;}};behave @print(80u64);}'
        'finally{behave @print(31u64);};return;}'
        'proc run()->u64{for val n<-stage(){return n;};return 0u64;}behave @print(run());',0,'31\n7\n',''),
    'lexical_handoff_type_query_of_full_scope_is_nonexecuting': (IO+
        'func stage() yields u64 -> bool{'
        'val probe<-make local typeof({{yield(99u64)}}){true};if(probe){yield(7u64):void;}else{};}'
        'proc run()->void{for val n<-stage(){behave @print(n);continue false;};return;}run();',0,'7\n',''),
    'lexical_handoff_ordinary_callable_cannot_capture_entry': (
        'proc stage() yields u64{func escaped()->void{yield(7u64);};escaped();return;}'
        'for val n<-stage(){};',2,'','access to a here formal from a nested callable scope'),
    'lexical_handoff_full_scope_cannot_materialize_entry': (
        'proc stage() yields u64{ {{func saved=yield;return;}};return;}'
        'for val n<-stage(){};',2,'','a here argument can only be invoked or forwarded'),
    'prepared_here_source_runs_operands_once_before_the_stage': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'proc rows(day:u64) yields u64 -> bool{behave @print(40u64);'
        'if(!yield(day)){return;}else{};yield(99u64);return;}'
        'proc stage(here input() yields u64 -> bool,stamp:u64) yields u64 -> bool{behave @print(stamp);'
        'for val n<-input(){continue yield(n);};for val n<-input(){continue yield(n+1u64);};return;}'
        'proc run()->void{for val n<-stage(rows(mark(24u64)),mark(32u64)){behave @print(n);continue false;};return;}run();',
        0,'24\n32\n32\n40\n24\n40\n25\n',''),
    'prepared_here_source_is_not_entered_when_skipped': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'proc rows(day:u64) yields u64{behave @print(80u64);yield(day);return;}'
        'proc stage(here input() yields u64) yields u64{behave @print(31u64);return;}'
        'proc run()->void{for val n<-stage(rows(mark(7u64))){behave @print(n);};return;}run();',0,'7\n31\n',''),
    'prepared_here_source_uses_distinct_formals_and_forwards_inputs': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'func rows(day:u64) yields u64{yield(day);}'
        'proc visit(here first() yields u64,here second() yields u64)->void{'
        'for val n<-second(){behave @print(n+10u64);};for val n<-first(){behave @print(n+20u64);};return;}'
        'proc relay(here first() yields u64,here second() yields u64)->void{visit(first,second);return;}'
        'relay(rows(mark(1u64)),rows(mark(2u64)));',0,'1\n2\n12\n21\n',''),
    'prepared_here_source_preserves_labelled_feedback_and_stop': (IO+
        'func rows(day:u64) yields {ok:bool;id:u64;} -> bool{if(!yield(day->id,true->ok)){return;}else{};yield(80u64->id,false->ok);}'
        'func stage(here src() yields {id:u64;ok:bool;} -> bool) yields {ok:bool;id:u64;} -> bool{'
        'for {val id<-id;val ready<-ok;} := src(){continue yield(ready->ok,id->id);};}'
        'proc run()->void{stage(rows(24u64))({val id<-id;val ready<-ok;})=>{behave @print(id);behave @print(ready);false};return;}run();',
        0,'24\ntrue\n',''),
    'prepared_here_source_carries_its_invocation_owner': (IO+
        'func origin(value:u64) yields ref const u64 in here{val slot<-value;yield(slot);}'
        'func stage(here input() yields ref const u64 in here) yields u64{for ref item=input(){yield(item+1u64);};}'
        'proc run()->void{for val n<-stage(origin(24u64)){behave @print(n);};return;}run();',0,'25\n',''),
    'prepared_here_source_propagates_outer_return_through_cleanups': (IO+
        'proc rows(day:u64) yields u64{defer behave @print(31u64);yield(day);behave @print(80u64);return;}'
        'proc stage(here src() yields u64) yields u64{defer behave @print(32u64);for val n<-src(){yield(n);};return;}'
        'proc run()->u64{for val n<-stage(rows(24u64)){return n;};return 0u64;}behave @print(run());',0,'31\n32\n24\n',''),
    'prepared_here_source_downstream_failure_bypasses_both_catches': (IO+'using "std:exceptions" NullPointerException;'+
        'func fail_now()->void throws NullPointerException{ptr p:u64=null in here;p;{};}'
        'proc rows(day:u64) yields u64{try{yield(day);}'
        'catch(e:const NullPointerException){behave @print(80u64);}finally{behave @print(31u64);};return;}'
        'proc stage(here src() yields u64) yields u64{try{for val n<-src(){yield(n);};}'
        'catch(e:const NullPointerException){behave @print(81u64);}finally{behave @print(32u64);};return;}'
        'proc run()->void{try{for val n<-stage(rows(24u64)){behave @print(n);fail_now();};}'
        'catch(e:const NullPointerException){behave @print(33u64);};return;}run();',0,'24\n31\n32\n33\n',''),
    'prepared_here_source_checks_failures_on_the_selected_overload': (IO+'using "std:exceptions" Exception;'+
        'func rows(day:u64) yields u64{yield(day);}'
        'func rows(flag:bool) yields u64 throws Exception{if(flag){yield(80u64);}else{};}'
        'func stage(here src() yields u64) yields u64{for val n<-src(){yield(n);};}'
        'proc run()->void{for val n<-stage(rows(24u64)){behave @print(n);};return;}run();',0,'24\n',''),
    'prepared_here_source_preparation_failure_does_not_enter_stage': (IO+'using "std:exceptions" NullPointerException;'+
        'func input()->u64 throws NullPointerException{ptr p:u64=null in here;return p;}'
        'proc rows(day:u64) yields u64{behave @print(80u64);yield(day);return;}'
        'proc stage(here src() yields u64) yields u64{behave @print(81u64);for val n<-src(){yield(n);};return;}'
        'proc run()->void{try{for val n<-stage(rows(input())){behave @print(n);};}'
        'catch(e:const NullPointerException){behave @print(7u64);};return;}run();',0,'7\n',''),
    'prepared_here_source_requires_compatible_handoff': (
        'func rows() yields {other:u64;}{yield(1u64->other);}'
        'func stage(here input() yields {id:u64;}) yields u64{for val n<-input(){yield(n);};}'
        'for val n<-stage(rows()){};',2,'','prepared producer does not match the selected here contract'),
    'prepared_here_source_cannot_drop_spread': (
        'func rows() yields u64{yield(1u64);}'
        'proc stage(here input() yields u64)->void{return;}'
        'stage(rows()...);',2,'','spread arguments'),
    'prepared_here_source_retains_procedure_restrictions': (
        'proc rows() yields u64{yield(1u64);return;}'
        'func stage(here input() yields u64) yields u64{for val n<-input(){yield(n);};}'
        'for val n<-stage(rows()){};',2,'','a function cannot invoke a procedure producer'),
    'here_receiver_abrupt_argument_preserves_preparation_prefix': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'proc receive(here body(x:u64)->u64,first:u64,middle:u64,last:u64)->u64{behave @print(last);return body(first+middle);}'
        'proc run()->u64{return receive(mark(11u64),{return 21u64;},mark(98u64))(n)=>{behave @print(99u64);n};}'
        'behave @print(run());',0,'11\n21\n',''),
    'here_receiver_abrupt_argument_still_checks_the_body': (
        'func receive(here body(x:u64)->u64,input:u64)->u64{return body(input);}'
        'func run()->u64{return receive({return 21u64;})(n)=>{false};}run();',
        2,'','here body fallthrough requires feedback of type'),
    'query_here_receiver_checks_named_arguments_without_execution': (IO+
        'proc mark()->u64{behave @print(90u64);return 1u64;}'
        'proc apply(here body(x:u64)->u64,value:u64,choose:bool)->u64{behave @print(91u64);return body(value);}'
        'proc run()->void{val result<-make local typeof(apply(true->choose,mark()->value)(n)=>{behave @print(92u64);n+1u64}){7u64};'
        'behave @print(result);return;}run();',0,'7\n',''),
    'query_here_receiver_distinguishes_break_and_feedback_types': (IO+
        'func compare(here step()->u64)->bool{return step()==7u64;}'
        'val result<-make local typeof(compare(){if(true){break false;}else{continue 7u64;};}){true};'
        'behave @print(result);',0,'true\n',''),
    'query_here_receiver_rejects_wrong_feedback_type': (
        'func compare(here step()->u64)->bool{return step()==7u64;}'
        'val result<-make local typeof(compare(){continue false;}){true};',2,'',"continue requires 'u64'"),
    'query_here_receiver_checks_supplier_return_type': (
        'func compare(here step()->u64)->bool{return step()==7u64;}'
        'proc run()->u64{val result<-make local typeof(compare(){return false;}){true};return 7u64;}run();',
        2,'','return type does not satisfy the implicit callable result'),
    'generator_void_feedback_rejects_explicit_operand': (
        'func generate() yields u64{yield(1u64);}func action()->void{}'
        'for val v<-generate(){continue action();};',2,'','void here feedback requires bare continue'),
    'supplied_scope_forwarding_preserves_live_slot': (IO+
        'func relay(here body(n:u64)->u64,n:u64)->u64{return body(n);}'
        'func outer(here body(n:u64)->u64,n:u64)->u64{return relay(body,n);}'
        'proc run()->void{val base<-20u64;behave @print(outer(7u64)(n)=>{{return base+n;}});return;}run();',0,'27\n',''),
    'supplied_scope_per_invocation_cleanup': (IO+
        'func twice(here body(n:u64)->void)->void{body(2u64);body(7u64);}'
        'proc run()->void{val total<-1u64;twice()(n)=>{{defer{total<-total+10u64;};total<-total+n:void;}};'
        'behave @print(total);return;}run();',0,'30\n',''),
    'supplied_scope_distinct_recursive_activations': (IO+
        'func once(here body()->void)->void{body();}'
        'proc visit(depth:u64)->void{val n<-depth;once(){{if(n>0u64){visit(n-1u64);}else{};behave @print(n);}};return;}visit(3u64);',
        0,'0\n1\n2\n3\n',''),
    'supplied_scope_named_operands_evaluate_in_written_order': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'func ordered(here body(n:u64)->u64,left:u64,right:u64)->u64{return body(left*10u64+right);}'
        'proc run()->void{behave @print(ordered(mark(2u64)->right,mark(1u64)->left)(n)=>{{behave @print(9u64);return n;}});return;}run();',
        0,'2\n1\n9\n12\n',''),
    'supplied_scope_checked_failure_returns_to_receiver': (IO+'using "std:exceptions" NullPointerException;'+
        'func retry(here body()->u64 throws NullPointerException)->u64{'
        'try{return body();}catch(e:const NullPointerException){return 51u64;};return 0u64;}'
        'proc run()->void{behave @print(retry(){{ptr x:u64=null in here;return x;}});return;}run();',0,'51\n',''),
    'supplied_scope_cannot_use_lexical_here_as_scalar_operand': (
        'func body(here step(n:u64)->u64)->u64{return step(1u64);}'
        'func invoke(here body(n:u64)->u64)->u64{return body(body);}',2,'','a here argument can only be invoked or forwarded'),
    'live_scalar_slot': (IO+'proc run()->u64{val n<-1u64;{{n<-2u64;}};return n;}behave @print(run());', 0, '2\n', ''),
    'local_return': (IO+'func run()->u64{ {{return 1u64;}};return 2u64;}behave @print(run());', 0, '2\n', ''),
    'explicit_result': (IO+'func run()->u64{return {{return 7u64;}};}behave @print(run());', 0, '7\n', ''),
    'independent_inference': (IO+'func run(){ {{return true;}};return 8u64;}behave @print(run());', 0, '8\n', ''),
    'tail_returns_local_value': (IO+'func run()->u64{return {{7u64;}};}behave @print(run());', 0, '7\n', ''),
    'inferred_void': (IO+'func run(){ {{return 7u64;}};true:void;}run();behave @print(9u64);', 0, '9\n', ''),
    'void_fallthrough': (IO+'proc run()->void{ {{behave @print(1u64);}};behave @print(2u64);return;}run();', 0, '1\n2\n', ''),
    'cleanup_completion': (IO+'proc run()->u64{val n<-1u64;val inner<-{{defer{n<-3u64;};return n;}};return n+inner;}behave @print(run());', 0, '4\n', ''),
    'cleanup_loop_preserves_payload_before_each_registration': (IO +
        'func run(ptr input:u64 in O)->u64 where {owner O}{const val present<-{{'
        'val seen<-false;for(i in 0..2){defer{return seen;};defer{seen<-(input is not null);};};return false;}};'
        'if(present){return input;}else{return 7u64;};}'
        'ref item:u64=5u64;ptr absent:u64=null in here;behave @print(run(item));behave @print(run(absent));',
        0, '5\n7\n', ''),
    'cleanup_readonly_borrow_preserves_boolean_observation': (IO +
        'func inspect(ref value:const bool in B)->void where {owner B}{};'
        'func run(ptr input:u64 in O)->u64 where {owner O}{const val present<-{{'
        'val saved<-(input is not null);defer{return saved;};for(i in 0..2){defer inspect(saved);};return false;}};'
        'if(present){return input;}else{return 7u64;};}'
        'ref item:u64=5u64;ptr absent:u64=null in here;behave @print(run(item));behave @print(run(absent));',
        0, '5\n7\n', ''),
    'readonly_borrow_postcondition_refines_boolean_observation': (IO +
        'using "std:exceptions" NullPointerException;'
        'func ensure(ref value:const bool in B)->void throws NullPointerException where {owner B}{'
        'if(!value){ptr missing:u64=null in here;missing;{};}else{};};'
        'func run(ptr input:u64 in O,flag:bool)->u64 where {owner O}{val seen<-flag;'
        'try{ensure(seen);}catch(problem:const NullPointerException){return 7u64;};'
        'if(!seen){return input;}else{};return 7u64;}'
        'ptr absent:u64=null in here;behave @print(run(absent,true));behave @print(run(absent,false));',
        0, '7\n7\n', ''),
    'cleanup_local_return': (IO+'proc run()->u64{defer{{return 9u64;}};return 2u64;}behave @print(run());', 0, '2\n', ''),
    'cleanup_local_return_grouped': (IO+'proc run()->u64{defer{ {{return 9u64;}};};return 2u64;}behave @print(run());', 0, '2\n', ''),
    'delayed_capture': (IO+'proc run()->void{val n<-1u64;defer{{behave @print(n);}};n<-2u64;return;}run();', 0, '2\n', ''),
    'delayed_capture_grouped': (IO+'proc run()->void{val n<-1u64;defer{ {{behave @print(n);}};};n<-2u64;return;}run();', 0, '2\n', ''),
    'nested_cleanup': (IO+'proc run()->void{defer{behave @print(4u64);};{{defer{behave @print(3u64);};{{defer{behave @print(2u64);};behave @print(1u64);}};}};return;}run();', 0, '1\n2\n3\n4\n', ''),
    'argument_order': (IO+'func sum(val a:u64,val b:u64)->u64{return a+b;}proc run()->void{val n<-1u64;behave @print(sum({{n<-2u64;return n;}},n));behave @print(n);return;}run();', 0, '4\n2\n', ''),
    'generic_value_inputs': (IO+'func run(val x:T)->const T where T{return {{x<-x+x;return x;}};}behave @print(run(3u64));behave @print(run(4));behave @print(run(2f32));', 0, '6\n8\n4\n', ''),
    'scope_callable_argument_from_later_operand': (IO + IDENTITY + APPLY +
        'func run(x:T) where T{return apply({{return identity;}},x);}'
        'behave @print(run(7u64));behave @print(run(true));', 0, '7\ntrue\n', ''),
    'scope_callable_argument_from_view': (IO + IDENTITY + FIXED +
        'func invoke=fixed;behave @print(invoke({{return identity;}},9u64));', 0, '9\n', ''),
    'nested_scope_callable_argument': (IO + IDENTITY + FIXED +
        'behave @print(fixed({{return {{return identity;}};}},13u64));', 0, '13\n', ''),
    'scope_callable_argument_cleanup_order': (IO + IDENTITY + FIXED +
        'proc observe(n:u64)->void{behave @print(n);return;}'
        'behave @print(fixed({{defer observe(2u64);observe(1u64);return identity;}},{observe(3u64);17u64}));',
        0, '1\n2\n3\n17\n', ''),
    'scope_callable_argument_cannot_guess_overload': (IDENTITY +
        'func choose(f:func(u64)->const u64)->u64{return f(7u64);}'
        'func choose(f:func(bool)->const bool)->u64{return 8u64;}'
        'choose({{return identity;}});', 2, '', 'unproved callable result'),
    'scope_cannot_use_callable_receiver': (IDENTITY +
        'func result:func(u64)->const u64={{return identity;}};', 2, '', 'unproved callable result'),
    'live_function_holder': (IO+BASE+'proc run()->void{func saved=one;{{saved<-two;behave @print(saved());}};behave @print(saved());return;}run();', 0, '2\n2\n', ''),
    'nested_live_slot': (IO+'proc run()->u64{val n<-1u64;{{{{n<-6u64:void;}};n<-n+1u64:void;}};return n;}behave @print(run());', 0, '7\n', ''),
    'prepared_function_keeps_its_selected_value': (IO+
        'func one(ref n:u64 in O)->void where {owner O}{n<-1u64:void;}'
        'func two(ref n:u64 in O)->void where {owner O}{n<-2u64:void;}'
        'proc run()->void{val n<-0u64;func saved=one;defer{behave @print(n);};defer saved(n);'
        '{{saved<-two:void;}};saved(n);behave @print(n);return;}run();', 0, '2\n1\n', ''),
    'write_outer_slot_before_first_read': (IO+
        'proc run()->u64{val n<-make local u64;{{defer (<-)(n,9u64);}};return n;}behave @print(run());', 0, '9\n', ''),
    'read_outer_slot_before_its_cleanup_write': (
        'proc run()->u64{val n<-make local u64;return {{defer (<-)(n,9u64);return n;}};}run();', 2, '', 'initialization'),
    'outer_slot_has_no_reference_binding': (
        'proc run()->void{val n<-1u64,m<-2u64;{{n=m;}};}run();', 2, '', 'val storage has no reference binding'),
    'query_write_outer_slot_before_first_read': (IO+
        query('val n<-make local u64;{{defer (<-)(n,9u64);}};return n;')+'behave @print(result);', 0, '4\n', ''),
    'query_read_outer_slot_before_its_cleanup_write': (
        query('val n<-make local u64;return {{defer (<-)(n,9u64);return n;}};'), 2, '', 'initialization'),
    'returned_complete_value': (IO+'func factory(val seed:u64)->func()->const u64{return {{seed<-seed+1u64;return func[seed]()->const u64{return seed;};}};}func saved=factory(4u64);behave @print(saved());behave @print(saved());', 0, '5\n5\n', ''),
    'query_has_no_effects': (IO+'proc run()->void{val n<-make local u64;val result<-make local typeof({{behave @print(n);return 7u64;}}){8u64};behave @print(result);return;}run();', 0, '8\n', ''),
    'query_contextual_scope_argument_has_no_effects': (IO + IDENTITY + APPLY +
        'val result<-make local typeof(apply({{behave @print(98u64);return identity;}},1u64)){15u64};'
        'behave @print(result);', 0, '15\n', ''),
    'query_validates_contextual_scope_argument': (IDENTITY + FIXED +
        'const val frozen<-7u64;val result<-make local typeof(fixed({{frozen<-1u64;return identity;}},1u64));',
        2, '', 'readonly'),
    'query_readonly_write': ('const val n<-1u64;val result<-make local typeof({{n<-2u64;return n;}}){3u64};', 2, '', 'readonly'),
    'query_function_effect': (IO+'func run()->void{val n<-make local typeof({{behave @print(1u64);return 2u64;}}){3u64};}run();', 2, '', 'function cannot invoke a dynamic procedure'),
    'query_function_direct_procedure': ('proc action()->void{}func run()->void{val n<-make local typeof({{action();return 2u64;}}){3u64};}run();', 2, '', 'function cannot invoke a procedure'),
    'query_unreachable_readonly_write': ('const val n<-1u64;val result<-make local typeof({{return n;n<-2u64;}}){3u64};', 2, '', 'readonly'),
    'query_nested_readonly_write': ('const val n<-1u64;val result<-make local typeof({{ {{n<-2u64;}};return n;}}){3u64};', 2, '', 'readonly'),
    'query_deferred_readonly_write': ('const val n<-1u64;val result<-make local typeof({{defer{n<-2u64;};return n;}}){3u64};', 2, '', 'readonly'),
    'query_needs_no_installed_binding': (IO+'symbol @bias:u64;val result<-make local typeof({{return @bias;}}){8u64};behave @print(result);', 0, '8\n', ''),
    'query_established_nested_signature': (IO+'func unused(x:T)->u64 where T{return true;}val result<-make local typeof({{return unused(1u64);}}){8u64};behave @print(result);', 0, '8\n', ''),
    'query_discarded_tail_is_void': ('val result<-make local typeof({{7u64:void;}}){8u64};', 2, '', 'initializer does not satisfy local storage type'),
    'query_uses_continuing_arm': (IO +
        query('if(true){return 1u32;}else{val remaining<-4294967296u64;};return remaining;') + 'behave @print(result);',
        0, '4\n', ''),
    'query_uses_continuing_try_arm': (IO + GUARDED_QUERY + query(GUARDED_TRY+';return n;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_uses_continuing_catch_after_finally': (IO + GUARDED_QUERY +
        query('try{if(true){throw e;}else{return 7u32;};}'
              'catch(c:const NullPointerException){val n<-4294967296u64;{}}'
              'finally{val suffix<-2u64;};return n+suffix;') + 'behave @print(result);', 0, '4\n', ''),
    'query_finally_cannot_use_normal_only_declaration': (GUARDED_QUERY +
        query(GUARDED_TRY+'finally{n;{}};return n;'), 2, '', QUERY_EVIDENCE),
    'query_finally_observes_early_return': (
        query('try{if(true){return 3u64;}else{};val late<-7u64;{}}'+FINALLY_READ), 2, '', QUERY_EVIDENCE),
    'query_finally_observes_early_throw': (GUARDED_QUERY +
        query('try{if(true){throw e;}else{};val late<-7u64;{}}'+FINALLY_READ), 2, '', QUERY_EVIDENCE),
    'query_finally_keeps_binding_before_throw': (IO + GUARDED_QUERY +
        query('try{val late<-7u64;if(true){throw e;}else{};{}}'+FINALLY_READ) + 'behave @print(result);',
        0, '4\n', ''),
    'query_outer_finally_sees_inner_cleanup_effects': (IO +
        query('try{try{return 7u64;}finally{val late<-9u64;{}};}'+FINALLY_READ) + 'behave @print(result);',
        0, '4\n', ''),
    'query_finally_ignores_inner_callable_return': (IO +
        query('try{ {{return;}};val late<-7u64;{}}'+FINALLY_READ) + 'behave @print(result);', 0, '4\n', ''),
    'query_finally_checks_return_from_eager_defer_preparation': ('proc observe(x:u64)->void{}' +
        query('try{defer observe(if(true){return 3u64;}else{1u64});val late<-7u64;{}}'+FINALLY_READ),
        2, '', QUERY_EVIDENCE),
    'query_rejects_nonvoid_finally': (query('try{}finally{7u64};return 11u64;'),
        2, '', 'finally must complete with void or an explicit return'),
    'query_keeps_obligation_from_abrupt_condition': (IO +
        query('if({return 7u64;}){}else{};') + 'behave @print(result);', 0, '4\n', ''),
    'query_checks_arms_after_abrupt_condition': (query('if({return 7u64;}){val wrong:bool<-1u64;}else{};'),
        2, '', 'initializer does not satisfy declared val type'),
    'query_and_allows_abrupt_left_operand': (IO + query('{return 7u64;}&&false;') + 'behave @print(result);',
        0, '4\n', ''),
    'query_or_allows_abrupt_left_operand': (IO + query('{return 7u64;}||false;') + 'behave @print(result);',
        0, '4\n', ''),
    'query_and_allows_abrupt_right_operand': (IO + query('false&&{return 7u64;};return 8u64;') + 'behave @print(result);',
        0, '4\n', ''),
    'query_or_allows_abrupt_right_operand': (IO + query('true||{return 7u64;};return 8u64;') + 'behave @print(result);',
        0, '4\n', ''),
    'query_checks_invalid_right_operand_after_abrupt_left': (query('{return 7u64;}&&5u32;'),
        2, '', 'logical operation requires bool operands'),
    'query_does_not_fall_back_to_outer_callable': ('func helper()->u64{return 90u64;}'
        'proc outer(flag:bool)->void{' +
        query('if(flag){func helper()->u64{return 3u64;};return 1u64;}else{};return helper();') +
        '}outer(false);', 2, '', "nearest binding 'helper' is not callable"),
    'query_range_checks_first_endpoint': (query('for(i in true..2){};return 7u64;'), 2, '', 'a range requires integer endpoints'),
    'query_range_checks_last_endpoint': (query('for(i in 0..2f32){};return 7u64;'), 2, '', 'a range requires integer endpoints'),
    'query_range_checks_endpoint_agreement': (query('for(i in 0u32..2u64){};return 7u64;'),
        2, '', 'range endpoints with different integer types'),
    'query_range_requires_index_evidence': (query('for(i in {return 8u64;}..{return 9u64;}){};'),
        2, '', 'range index type is not established by an endpoint'),
    'query_fold_requires_value_accumulator': (query('fold(i in 0..2, acc={}){};return 7u64;'),
        2, '', 'non-value or abruptly completing operand'),
    'query_fold_checks_empty_body': (query('fold(i in 0..0, acc=0u64){false};return 7u64;'),
        2, '', 'fold body does not satisfy the accumulator type'),
    'query_fold_rejects_narrowing': (query('fold(i in 0..2, acc=0u8){1u32};return 7u64;'),
        2, '', 'fold body does not satisfy the accumulator type'),
    'query_loop_accepts_lossless_fold_without_execution': (IO +
        query('for(i in 0..2){behave @print(99);};return fold(i in 0u8..1u8, acc=0u64){i};') + 'behave @print(result);',
        0, '4\n', ''),
    'query_loop_keeps_endpoint_return': (IO + query('for(i in {return 8u64;}..2){};') + 'behave @print(result);', 0, '4\n', ''),
    'query_fold_keeps_body_return': (IO + query('fold(i in 0..1, acc=0u64){return 8u64;};return 7u64;') + 'behave @print(result);',
        0, '4\n', ''),
    'function_permissions': (IO+'func run()->void{ {{behave @print(1u64);}};}run();', 2, '', 'function cannot invoke a dynamic procedure'),
    'readonly_capture': ('proc run()->void{const val n<-1u64;{{n<-2u64;}};}run();', 2, '', 'readonly'),
    'direct_write_initializes_outer_slot': (IO+'proc run()->u64{val n<-make local u64;{{n<-2u64;}};return n;}behave @print(run());', 0, '2\n', ''),
    'reference_input_writes_target': (IO+'proc run()->void{ref n:u64=1u64;{{n<-2u64;}};behave @print(n);return;}run();', 0, '2\n', ''),
    'declared_place_writes_supplying_value_slot': (IO+'proc run()->u64{val n<-1u64;var p->n;{{p<-2u64;}};return n;}behave @print(run());',
        0, '2\n', ''),
    'missing_return_after_inner_completion': (IO+'using "std:exceptions" MissingReturnError;proc run()->u64{ {{return 1u64;}};}try{run();{}}catch(e:const MissingReturnError){behave @print(5u64);};', 0, '5\n', '[-Wmissing-return]'),
    'inner_missing_return': (IO+'using "std:exceptions" MissingReturnError;proc run(val flag:bool)->void{try{val n<-{{if(flag){return 1u64;}else{};}};behave @print(n);}catch(e:const MissingReturnError){behave @print(5u64);};return;}run(false);run(true);', 0, '5\n1\n', '[-Wmissing-return]'),
    'live_slot_keeps_its_supplying_owner': (IO+'func choose(ref a:u64 in O,ref b:u64 in O)->const u64 where {owner O}{return 7u64;}func choose(ref a:u64 in O,ref b:u64 in P)->const bool where {owner O,owner P}{return true;}func run(){val source<-1u64;return {{val local<-2u64;return choose(source,local);}};}behave @print(run());', 0, 'true\n', ''),
    'outer_catch_is_not_ambient': ('using "std:exceptions" MissingReturnError;func missing()->const u64{}proc run()->void{try{missing();{}}catch(e:const MissingReturnError){ {{if(true){throw;}else{return 1u64;};}};{}};}run();', 2, '', 'bare throw requires an active catch'),
    'unchecked_failure_crosses_call': (IO+'using "std:exceptions" MissingReturnError;func missing()->const u64{}proc run()->void{defer{behave @print(4u64);};try{ {{defer{behave @print(2u64);};behave @print(1u64);missing():void;}};}catch(e:const MissingReturnError){behave @print(3u64);};return;}run();', 0, '1\n2\n3\n4\n', '[-Wmissing-return]'),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
