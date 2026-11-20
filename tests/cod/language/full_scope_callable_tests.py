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
READONLY_HERE_ARRAY = ('array Rows[u64]:u64;'
                       'func inspect(here read(ref view:const Rows in O)->u64,rows:const Rows in O)->u64 '
                       'where {const owner O}{return read(rows);}'
                       'func through(rows:const Rows in O)->u64 where {const owner O}{return inspect(rows)')
READ_PTR_RESULT = 'defer{' + OBSERVE_PTR_RESULT + '};'
WRITE_PTR_RESULT = 'defer{if returning (ptr saved){if(saved is not null){saved<-9u64;{}}else{}}};'
GUARDED_QUERY = ('using "std:exceptions" NullPointerException;val e<-make local NullPointerException;')
GUARDED_TRY = ('try{if(true){throw e;}else{};val n<-4294967296u64;{}}'
               'catch(c:const NullPointerException){return 7u32;}')
FINALLY_READ = 'finally{late;{}};return late;'
QUERY_EVIDENCE = 'typeof requires a known declaration or operand type'
QUERY_PAIR = 'data Pair {first:u64;second:u64;}'
QUERY_HOLDER = 'data Holder {ptr selected:u64;}'
QUERY_HOLDER_CHOICE = ('ptr left=make Holder;ptr right=make Holder;'
                       'val a<-make local u64;val b<-make local u64;val ready<-7u64;'
                       'ptr chosen=if(pick_left){left}else{right};')
QUERY_BOUND_HOLDERS = QUERY_HOLDER_CHOICE + 'left.selected=a;right.selected=b;'
READ_SELECTED_HOLDER = ('func peek(ptr p:const Holder in O)->u64 where {const owner O}{'
                        'if(p is null){return 0u64;}else{};'
                        'if(p.selected is null){return 0u64;}else{};return p.selected;}')
REBIND_SELECTED_HOLDER = ('proc redirect(ptr p:Holder in O,ptr target:u64 in O)->void where {owner O}{'
                          'if(p is null){return;}else{};p.selected=target;return;}')
RETURN_SELECTED_HOLDER = ('func get_link(ptr p:Holder in O)->ptr u64 in O where {owner O}{'
                          'if(p is null){return null in O;}else{};return p.selected;}')
RETURN_AND_REBIND_HOLDER = ('func get_link(ptr p:Holder in O,ptr replacement:u64 in O)->ptr u64 in O where {owner O}{'
                           'if(p is null){return null in O;}else{};'
                           'defer (=)(p.selected,replacement);return p.selected;}')
QUERY_OUTER = 'data Outer {ptr child:Holder;}'
QUERY_OUTER_CHOICE = ('ptr left=make Outer;ptr right=make Outer;'
                      'ptr first_child=make Holder;ptr second_child=make Holder;'
                      'left.child=first_child;right.child=second_child;'
                      'ptr chosen=if(pick_left){left}else{right};')
QUERY_NESTED_HOLDERS = (QUERY_OUTER_CHOICE + 'val a<-make local u64;val b<-make local u64;'
                        'first_child.selected=a;second_child.selected=b;')
QUERY_NULLABLE_CHAIN = QUERY_NESTED_HOLDERS + 'right.child=null in here;'
QUERY_OPTIONAL_OUTER = 'ptr empty=make Outer;empty.child=null in here;chosen=if(pick_other){empty}else{chosen};'
QUERY_NULLABLE_CHAIN_CHOICES = QUERY_NESTED_HOLDERS + QUERY_OPTIONAL_OUTER
READ_GUARDED_CHAIN = 'if(chosen.child is null){return 0u64;}else{return chosen.child.selected;};'
QUERY_INLINE_HOLDER = 'data Payload {leaf:u64;other:u64;}data Holder {payload:Payload;}'
QUERY_INLINE_RECEIVERS = QUERY_OUTER_CHOICE + QUERY_OPTIONAL_OUTER
READ_GUARDED_INLINE = 'if(chosen.child is null){return 0u64;}else{return chosen.child.payload.leaf;};'
WRITE_GUARDED_INLINE = 'if(chosen.child is not null){chosen.child.payload.leaf<-7u64;{}}else{};'
WRITE_INLINE_HOLDER = ('proc fill(ptr p:Outer in O)->void where {owner O}{'
                       'if(p is null){return;}else{};if(p.child is null){return;}else{};p.child.payload.leaf<-7u64;return;}')
READ_INLINE_HOLDER = ('proc peek(ptr p:Outer in O)->u64 where {owner O}{if(p is null){return 0u64;}else{};'
                       'if(p.child is null){return 0u64;}else{};return p.child.payload.leaf;}')
READ_INLINE_WHEN_ENABLED = ('proc peek(ptr p:Outer in O,skip:bool)->u64 where {owner O}{'
                            'if(p is null){return 0u64;}else{};if(skip){return 0u64;}else{};'
                            'if(p.child is null){return 0u64;}else{};return p.child.payload.leaf;}')
WRITE_INLINE_WHEN_ENABLED = ('proc fill(ptr p:Outer in O,skip:bool)->void where {owner O}{'
                             'if(p is null){return;}else{};if(p.child is null){return;}else{};'
                             'if(skip){return;}else{};p.child.payload.leaf<-7u64;return;}')
WRITE_INLINE_WITH_TWO_FLAGS = ('proc fill(ptr p:Outer in O,skip:bool,other:bool)->void where {owner O}{'
                               'if(p is null){return;}else{};if(p.child is null){return;}else{};'
                               'if(skip && other){return;}else{};p.child.payload.leaf<-7u64;return;}')
WRITE_REFERENT_WHEN_ENABLED = ('proc fill(ptr p:Outer in O,skip:bool,spare:bool)->void where {owner O}{'
                               'if(p is null){return;}else{};if(p.child is null){return;}else{};'
                               'if(p.child.selected is null){return;}else{};'
                               'if(skip){return;}else{};p.child.selected<-7u64;return;}')
WRITE_NESTED_HOLDER = ('proc fill(ptr p:Outer in O)->void where {owner O}{'
                       'if(p is null){return;}else{};if(p.child is null){return;}else{};'
                       'if(p.child.selected is null){return;}else{};p.child.selected<-7u64;return;}')
READ_NESTED_HOLDER = ('proc peek(ptr p:Outer in O)->u64 where {owner O}{if(p is null){return 0u64;}else{};'
                      'if(p.child is null){return 0u64;}else{};'
                      'if(p.child.selected is null){return 0u64;}else{};return p.child.selected;}')
RETURN_NESTED_HOLDER = ('func deep(ptr p:Outer in O)->ptr u64 in O where {owner O}{'
                        'if(p is null){return null in O;}else{};if(p.child is null){return null in O;}else{};'
                        'return p.child.selected;}')
QUERY_NESTED_REPLACEMENT = 'ptr replacement=make Holder;val fresh<-make local u64;replacement.selected=fresh;'
QUERY_NODE = 'data Node {value:u64;ref next:Node;}'
READ_QUERY_PAIR = ('func inspect(ptr p:const Pair in O)->u64 where {const owner O}{'
                   'if(p is null){return 0u64;}else{};return p.second;}')
WRITE_QUERY_PAIR = ('proc populate(ptr p:Pair in O)->void where {owner O}{'
                    'if(p is null){return;}else{};p.second<-7u64;return;}')
QUERY_PRODUCT_CHOICE = ('ptr left=make Pair;ptr right=make Pair;'
                        'ptr selected=if(pick_left){left}else{right};')
QUERY_PRODUCT_FLAGS = 'val pick_left<-true;val pick_other<-false;'
PROJECT_QUERY_FIELD = ('func project_field(ptr p:Pair in O)->ptr u64 in O where {owner O}{'
                       'if(p is null){return null in O;}else{};return p.second;}')
MAYBE_QUERY_FIELD = ('func maybe_field(ptr p:Pair in O,enabled:bool)->ptr u64 in O where {owner O}{'
                     'if(enabled){if(p is null){return null in O;}else{};return p.second;}'
                     'else{return null in O;};}')
WRITE_QUERY_FIELD = ('proc fill_field(ptr field:u64 in O)->void where {owner O}{'
                     'if(field is null){return;}else{};field<-7u64;return;}')
WRITE_QUERY_HOLDER = ('proc populate(ref p:Holder in O)->void where {owner O}{'
                      'if(p.selected is null){return;}else{};p.selected<-7u64;return;}')
QUERY_CALL_FAILURE = ('using "std:exceptions" NullPointerException;'
                      'proc populate(ptr p:Pair in O)->void throws NullPointerException where {owner O}{'
                      'if(p is null){return;}else{};')
READ_QUERY_AFTER_FAILURE = ('ptr item=make Pair;try{populate(item);}'
                            'catch(e:const NullPointerException){return item.second;};return item.second;')


def query(body):
    return 'val result<-make local typeof({{' + body + '}}){4u64};'


def query_with_flag_inputs(body):
    return ('proc probe(pick_left:bool,pick_other:bool,skip:bool)->void{' + query(body) +
            'behave @print(result);return;}probe(true,false,false);')


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
    'readonly_here_array_receives_ref': (READONLY_HERE_ARRAY+
        '(view)=>{{return intrinsic (len)(view)+view[1u64];}};}export through;',0,'',''),
    'readonly_here_array_cannot_strengthen_access': (READONLY_HERE_ARRAY+
        '(view:Rows)=>{{return intrinsic (len)(view);}};}export through;',
        2,'','here body header access does not match the selected contract'),
    'readonly_here_array_cannot_snapshot_whole_value': (READONLY_HERE_ARRAY+
        '(val view<-)=>{return intrinsic (len)(view);};}export through;',
        2,'','receiving a here array as an independent value'),
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
    'pipeline_terminal_direct_spellings_match_a_supplied_body': (IO+
        'proc source(base:u64) yields u64{yield(base);yield(base);return;}'
        'proc mark(x:u64)->u64{behave @print(x);return x;}'
        'proc sink(x:u64)->u64{behave @print(x+10u64);return x;}'
        'source(mark(1u64))(n)=>{sink(n);};source(mark(1u64)) $> sink;sink <$ source(mark(1u64));',
        0,'1\n11\n11\n1\n11\n11\n1\n11\n11\n',''),
    'pipeline_terminal_zero_offers_do_not_call_a_sink': (IO+
        'proc source() yields u64{behave @print(1u64);return;}'
        'proc sink(n:u64)->void{behave @print(n);return;}'
        'source $> sink;sink <$ source;',0,'1\n1\n',''),
    'pipeline_terminal_returns_bool_feedback_in_both_directions': (IO+
        'proc source() yields u64 -> bool{if(!yield(7u64)){return;}else{};yield(80u64);return;}'
        'proc stop(n:u64)->bool{behave @print(n);return false;}'
        'source()(n)=>{continue stop(n);};source() $> stop;stop <$ source();',0,'7\n7\n7\n',''),
    'pipeline_terminal_passes_the_selected_ref_to_the_sink': (IO+
        'proc source(ref backing:u64 in O) yields ref u64 in O where {owner O}{yield(backing);return;}'
        'proc sink(ref item:u64 in P)->void where {owner P}{item<-item+1u64;return;}'
        'val current<-5u64;source(current)(item)=>{sink(item);};source(current) $> sink;'
        'sink <$ source(current);behave @print(current);',0,'8\n',''),
    'pipeline_terminal_preserves_an_abrupt_preparation_prefix': (IO+
        'proc mark(x:u64)->u64{behave @print(x);return x;}'
        'proc source(first:u64,second:u64,last:u64) yields u64{yield(first+second+last);return;}'
        'proc sink(n:u64)->void{behave @print(n);return;}'
        'proc run()->u64{sink <$ source(mark(1u64),{return 42u64;},mark(80u64));return 99u64;}'
        'behave @print(run());',0,'1\n42\n',''),
    'pipeline_terminal_checked_failure_bypasses_the_producer_catch': (IO+'using "std:exceptions" NullPointerException;'+
        'proc sink(n:u64)->void throws NullPointerException{behave @print(n);ptr p:u64=null in here;p;return;}'
        'proc source() yields u64{try{yield(7u64);behave @print(80u64);}'
        'catch(e:const NullPointerException){behave @print(81u64);}finally{behave @print(31u64);};return;}'
        'try{sink <$ source();}catch(e:const NullPointerException){behave @print(32u64);};',0,'7\n31\n32\n',''),
    'pipeline_terminal_rejects_a_feedback_type_mismatch': (
        'func source() yields u64 -> bool{yield(7u64):void;}'
        'func sink(n:u64)->u64{return n;}source() $> sink;',2,'',"continue type 'u64'"),
    'pipeline_terminal_inherits_function_permissions': (
        'func source() yields u64{yield(7u64);}'
        'proc sink(n:u64)->void{return;}'
        'func run()->void{source() $> sink;return;}run();',2,'','a function cannot invoke a procedure'),
    'pipeline_attachment_mirrors_preparation_without_mirroring_traversal': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'proc rows(day:u64) yields u64{behave @print(40u64);yield(day);return;}'
        'proc stage(here input() yields u64,offset:u64) yields u64{behave @print(30u64);val amount<-offset;'
        'for val item<-input(){yield(item+amount);};return;}'
        'proc sink(n:u64)->void{behave @print(n);return;}'
        'stage(rows(mark(1u64)),mark(2u64))(item)=>{sink(item);};'
        'rows(mark(1u64)) !> stage(mark(2u64)) $> sink;'
        'sink <$ stage(mark(2u64)) <! rows(mark(1u64));'
        'for val item<-stage(mark(2u64)) <! rows(mark(1u64)){sink(item);};',
        0,'1\n2\n30\n40\n3\n1\n2\n30\n40\n3\n2\n1\n30\n40\n3\n2\n1\n30\n40\n3\n',''),
    'pipeline_attachment_keeps_the_target_selected_in_written_order': (IO+
        'proc rows(line:const u64 in P) yields {id:u64;ref cell:const u64 in P;} -> bool where {owner P}{'
        'if(!yield(24u64->id,line->cell)){return;}else{};yield(80u64->id,line->cell);return;}'
        'proc stage(here source() yields {id:u64;ref cell:const u64 in S;} -> bool,witness:const u64 in S,enabled:bool) '
        'yields {ref cell:const u64 in S;id:u64;} -> bool where {owner S}{'
        'if(!enabled){return;}else{};for {val n<-id;ref item=cell;} := source(){continue yield(item->cell,n->id);};return;}'
        'proc run()->void{val first<-75u64;val second<-99u64;ref selected=first;'
        'for {val n<-id;ref item=cell;} := rows(selected) !> stage(first,{selected=second;true}){'
        'behave @print(n);behave @print(item);continue false;};selected=first;'
        'for {val n<-id;ref item=cell;} := stage(first,{selected=second;true}) <! rows(selected){'
        'behave @print(n);behave @print(item);continue false;};return;}run();',0,'24\n75\n24\n99\n',''),
    'pipeline_attachment_source_return_keeps_the_written_prefix': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'proc rows(first:u64,second:u64) yields u64{behave @print(80u64);yield(first+second);return;}'
        'proc stage(here input() yields u64,stamp:u64) yields u64{behave @print(stamp+80u64);'
        'for val item<-input(){yield(item);};return;}'
        'proc sink(n:u64)->void{behave @print(n);return;}'
        'proc forward()->u64{rows(mark(1u64),{return 7u64;}) !> stage(mark(2u64)) $> sink;return 99u64;}'
        'proc reverse()->u64{sink <$ stage(mark(2u64)) <! rows(mark(1u64),{return 7u64;});return 99u64;}'
        'behave @print(forward());behave @print(reverse());',0,'1\n7\n2\n1\n7\n',''),
    'pipeline_attachment_stage_return_keeps_the_written_prefix': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'proc rows(day:u64) yields u64{behave @print(80u64);yield(day);return;}'
        'proc stage(here input() yields u64,first:u64,second:u64) yields u64{'
        'val amount<-first+second;for val item<-input(){yield(item+amount);};return;}'
        'proc sink(n:u64)->void{behave @print(n);return;}'
        'proc forward()->u64{rows(mark(1u64)) !> stage(mark(2u64),{return 7u64;}) $> sink;return 99u64;}'
        'proc reverse()->u64{sink <$ stage(mark(2u64),{return 7u64;}) <! rows(mark(1u64));return 99u64;}'
        'behave @print(forward());behave @print(reverse());',0,'1\n2\n7\n2\n7\n',''),
    'pipeline_attachment_prepares_an_unopened_source': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'proc rows(day:u64) yields u64{behave @print(80u64);yield(day);return;}'
        'proc skip(here input() yields u64,stamp:u64) yields u64{behave @print(stamp+30u64);return;}'
        'proc sink(n:u64)->void{behave @print(n);return;}'
        'rows(mark(1u64)) !> skip(mark(2u64)) $> sink;'
        'sink <$ skip(mark(2u64)) <! rows(mark(1u64));',0,'1\n2\n32\n2\n1\n32\n',''),
    'pipeline_attachment_selector_failure_stops_in_written_order': (IO+'using "std:exceptions" NullPointerException;'+
        'proc select(n:u64,failed:bool)->u64 throws NullPointerException{try{behave @print(n);'
        'if(failed){ptr p:u64=null in here;p;{};}else{};return n;}finally{behave @print(n+10u64);};}'
        'proc rows(day:u64) yields u64{behave @print(80u64);yield(day);return;}'
        'proc stage(here input() yields u64,stamp:u64) yields u64{behave @print(81u64);'
        'for val item<-input(){yield(item);};return;}'
        'proc sink(n:u64)->void{behave @print(n);return;}'
        'try{rows(select(1u64,true)) !> stage(select(2u64,false)) $> sink;}catch(e:const NullPointerException){behave @print(9u64);};'
        'try{sink <$ stage(select(2u64,false)) <! rows(select(1u64,true));}catch(e:const NullPointerException){behave @print(9u64);};'
        'try{rows(select(1u64,false)) !> stage(select(2u64,true)) $> sink;}catch(e:const NullPointerException){behave @print(9u64);};'
        'try{sink <$ stage(select(2u64,true)) <! rows(select(1u64,false));}catch(e:const NullPointerException){behave @print(9u64);};',
        0,'1\n11\n9\n2\n12\n1\n11\n9\n1\n11\n2\n12\n9\n2\n12\n9\n',''),
    'pipeline_attachment_supplier_return_crosses_entered_cleanups': (IO+
        'proc rows() yields u64{try{yield(7u64);behave @print(80u64);}finally{behave @print(31u64);};return;}'
        'proc stage(here input() yields u64) yields u64{try{for val item<-input(){yield(item);};'
        'behave @print(81u64);}finally{behave @print(32u64);};return;}'
        'proc run()->u64{for val item<-stage <! rows{return item;};return 99u64;}'
        'behave @print(run());',0,'31\n32\n7\n',''),
    'pipeline_attachment_checked_sink_failure_crosses_entered_cleanups': (IO+'using "std:exceptions" NullPointerException;'+
        'proc rows() yields u64{try{yield(7u64);}catch(e:const NullPointerException){behave @print(80u64);}'
        'finally{behave @print(31u64);};return;}'
        'proc stage(here input() yields u64) yields u64{try{for val item<-input(){yield(item);};}'
        'catch(e:const NullPointerException){behave @print(81u64);}finally{behave @print(32u64);};return;}'
        'proc sink(n:u64)->void throws NullPointerException{behave @print(n);ptr p:u64=null in here;p;return;}'
        'try{rows !> stage $> sink;}catch(e:const NullPointerException){behave @print(33u64);};',
        0,'7\n31\n32\n33\n',''),
    'pipeline_attachment_rejects_incompatible_feedback': (
        'func rows() yields u64 -> bool{yield(7u64):void;}'
        'func stage(here input() yields u64) yields u64{for val item<-input(){yield(item);};}'
        'for val item<-stage <! rows(){};',2,'','prepared producer does not match the selected here contract'),
    'pipeline_attachment_establishes_the_processor_role': (IO+
        'func rows() yields u64{yield(7u64);}'
        'func both() yields u64{yield(80u64);}'
        'func both(here input() yields u64) yields u64{for val item<-input(){yield(item+1u64);};}'
        'proc sink(n:u64)->void{behave @print(n);return;}'
        'for val item<-both(){sink(item);};rows !> both $> sink;sink <$ both <! rows;',0,'80\n8\n8\n',''),
    'pipeline_composition_keeps_written_order_in_grouped_and_mirrored_forms': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'proc rows(day:u64) yields u64{behave @print(10u64);yield(day);return;}'
        'proc add(here input() yields u64,offset:u64) yields u64{behave @print(20u64);val amount<-offset;'
        'for val item<-input(){yield(item+amount);};return;}'
        'proc scale(here input() yields u64,factor:u64) yields u64{behave @print(30u64);val amount<-factor;'
        'for val item<-input(){yield(item*amount);};return;}'
        'proc sink(n:u64)->void{behave @print(n);return;}'
        'rows(mark(1u64)) !> add(mark(2u64)) |> scale(mark(3u64)) |> add(mark(4u64)) $> sink;'
        'sink <$ add(mark(4u64)) <| scale(mark(3u64)) <| add(mark(2u64)) <! rows(mark(1u64));'
        'for val item<-rows(mark(1u64)) !> (scale(mark(3u64)) <| add(mark(2u64))) |> add(mark(4u64)){sink(item);};',
        0,'1\n2\n3\n4\n20\n30\n20\n10\n13\n4\n3\n2\n1\n20\n30\n20\n10\n13\n1\n3\n2\n4\n20\n30\n20\n10\n13\n',''),
    'pipeline_composition_abrupt_preparation_skips_remaining_written_operands': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'proc rows(day:u64) yields u64{behave @print(80u64);yield(day);return;}'
        'proc pass(here input() yields u64,stamp:u64) yields u64{behave @print(81u64);'
        'for val item<-input(){yield(item);};return;}'
        'proc sink(n:u64)->void{behave @print(n);return;}'
        'proc run()->u64{rows(mark(1u64)) !> (pass({return 7u64;}) <| pass(mark(2u64))) |> pass(mark(3u64)) $> sink;'
        'return 99u64;}behave @print(run());',0,'1\n7\n',''),
    'pipeline_composition_reopens_the_same_prepared_source': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'proc rows(day:u64) yields u64{behave @print(10u64);yield(day);return;}'
        'proc pass(here input() yields u64,stamp:u64) yields u64{behave @print(stamp);'
        'for val item<-input(){yield(item);};return;}'
        'proc repeat(here input() yields u64) yields u64{for val item<-input(){yield(item);};'
        'for val item<-input(){yield(item+1u64);};return;}'
        'proc sink(n:u64)->void{behave @print(n);return;}'
        'rows(mark(1u64)) !> pass(mark(2u64)) |> repeat $> sink;',0,'1\n2\n2\n10\n1\n2\n10\n2\n',''),
    'pipeline_composition_can_skip_all_prepared_upstream_stages': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'proc rows(day:u64) yields u64{behave @print(80u64);yield(day);return;}'
        'proc pass(here input() yields u64,stamp:u64) yields u64{behave @print(81u64);'
        'for val item<-input(){yield(item);};return;}'
        'proc skip(here input() yields u64,stamp:u64) yields u64{behave @print(stamp+30u64);return;}'
        'for val item<-skip(mark(3u64)) <| pass(mark(2u64)) <! rows(mark(1u64)){behave @print(item);};',
        0,'3\n2\n1\n33\n',''),
    'pipeline_composition_uses_a_forwarded_producer_in_both_attachment_shapes': (IO+
        'proc rows(day:u64) yields u64{behave @print(10u64);yield(day);return;}'
        'proc pass(here input() yields u64,stamp:u64) yields u64{behave @print(stamp);'
        'for val item<-input(){yield(item);};return;}'
        'proc relay(here input() yields u64)->void{'
        'for val item<-pass(20u64) <! input{behave @print(item);};'
        'for val item<-pass(22u64) <| pass(21u64) <! input{behave @print(item);};return;}'
        'relay(rows(7u64));',0,'20\n10\n7\n22\n21\n10\n7\n',''),
    'pipeline_composition_preserves_labelled_feedback_and_the_actual_owner': (IO+
        'proc rows(line:const u64 in P) yields {id:u64;ref cell:const u64 in P;} -> bool where {owner P}{'
        'if(!yield(17u64->id,line->cell)){return;}else{};if(!yield(24u64->id,line->cell)){return;}else{};'
        'yield(80u64->id,line->cell);return;}'
        'proc screen(here input() yields {id:u64;ref cell:const u64 in S;} -> bool,witness:const u64 in S) '
        'yields {ref cell:const u64 in S;id:u64;} -> bool where {owner S}{'
        'for {val n<-id;ref item=cell;} := input(){if(n==17u64){continue true;}else{};'
        'continue yield(item->cell,n->id);};return;}'
        'proc forward(here input() yields {ref cell:const u64 in R;id:u64;} -> bool,witness:const u64 in R) '
        'yields {id:u64;ref cell:const u64 in R;} -> bool where {owner R}{'
        'for {val n<-id;ref item=cell;} := input(){continue yield(n->id,item->cell);};return;}'
        'proc run()->void{val line<-75u64;for {val n<-id;ref item=cell;} := forward(line) <| screen(line) <! rows(line){'
        'behave @print(n);behave @print(item);behave @print(item is line);continue false;};return;}run();',0,'24\n75\ntrue\n',''),
    'pipeline_composition_checked_failure_cleans_up_every_entered_stage': (IO+'using "std:exceptions" NullPointerException;'+
        'proc rows() yields u64{try{yield(7u64);}catch(e:const NullPointerException){behave @print(80u64);}'
        'finally{behave @print(31u64);};return;}'
        'proc stage(here input() yields u64,stamp:u64) yields u64{try{for val item<-input(){yield(item);};}'
        'catch(e:const NullPointerException){behave @print(81u64);}finally{behave @print(stamp);};return;}'
        'proc sink(n:u64)->void throws NullPointerException{behave @print(n);ptr p:u64=null in here;p;return;}'
        'try{rows !> stage(32u64) |> stage(33u64) $> sink;}catch(e:const NullPointerException){behave @print(34u64);};',
        0,'7\n31\n32\n33\n34\n',''),
    'pipeline_composition_rejects_feedback_mismatch_between_stages': (
        'func rows() yields u64{yield(7u64);}'
        'func first(here input() yields u64) yields u64 -> bool{for val item<-input(){yield(item):void;};}'
        'func second(here input() yields u64) yields u64{for val item<-input(){yield(item);};}'
        'for val item<-rows !> first |> second{};',2,'','prepared producer does not match the selected here contract'),
    'pipeline_composition_overload_prepares_data_before_the_attachment_runs': (IO+
        'proc selected_row()->u64{behave @print(10u64);return 20u64;}'
        'proc window()->u32{behave @print(11u64);return 2u32;}'
        'proc limit()->u64{behave @print(12u64);return 3u64;}'
        'proc seed()->u64{behave @print(14u64);return 40u64;}'
        'proc (|>)(first:u32,second:u64)->u64{behave @print(13u64);return first+second;}'
        'proc (!>)(row:u64,plan:u64) yields u64 -> bool{behave @print(16u64);yield(row+plan):void;return;}'
        'proc ($>)(here traversal() yields u64 -> bool,sink:u64)->u64{behave @print(15u64);val total<-sink;'
        'for val item<-traversal(){behave @print(item);total<-total+item;continue false;};return total;}'
        'behave @print(selected_row() !> window() |> limit() $> seed());'
        'behave @print(seed() <$ limit() <| window() <! selected_row());',
        0,'10\n11\n12\n13\n14\n15\n16\n25\n65\n14\n12\n11\n13\n10\n15\n16\n25\n65\n',''),
    'pipeline_attachment_overload_saves_operands_for_multiple_consumers': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'proc (<|)(limit:u64,window:u64)->u64{behave @print(10u64);return window+limit;}'
        'proc (<!)(plan:u64,row:u64) yields u64{behave @print(20u64);yield(row+plan);return;}'
        'proc ($>)(here traversal() yields u64,sink:u64)->u64{val total<-sink;'
        'for val item<-traversal(){total<-total+item;};return total;}'
        'proc report(item:u64)->void{behave @print(item);return;}'
        'here held=mark(1u64) !> mark(2u64) |> mark(3u64);behave @print(99u64);'
        'held $> report;behave @print(($>)(held,40u64));behave @print((<$)(50u64,held));'
        'behave @print(($>)((<!)(mark(4u64),(<|)(mark(6u64),mark(5u64))),60u64));',
        0,'1\n2\n3\n10\n99\n20\n6\n20\n46\n20\n56\n4\n6\n5\n10\n20\n75\n',''),
    'pipeline_composition_overload_preserves_a_borrowed_result_owner': (IO+
        'func (|>)(ref witness:const u64 in P,n:u64)->ref const u64 in P where {owner P}{return witness;}'
        'func (!>)(n:u64,ref witness:const u64 in P) yields ref const u64 in P where {owner P}{yield(witness);}'
        'proc ($>)(here traversal() yields ref const u64 in P,witness:const u64 in P)->bool where {owner P}{'
        'for ref item=traversal(){return item is witness;};return false;}'
        'val original<-75u64;behave @print(3u64 !> original |> 7u64 $> original);',0,'true\n',''),
    'pipeline_composition_falls_back_to_builtin_for_structural_stages': (IO+
        'func (|>)(left:u64,right:u64)->u64{return left+right;}'
        'func (!>)(row:u64,plan:u64) yields u64{yield(row+plan);}'
        'func rows() yields u64{yield(1u64);}'
        'func step(here input() yields u64,amount:u64) yields u64{'
        'val offset<-amount;for val item<-input(){yield(item+offset);};}'
        'proc report(item:u64)->void{behave @print(item);return;}'
        'rows() !> step(2u64) |> step(4u64) $> report;3u64 !> 5u64 |> 7u64 $> report;',0,'7\n15\n',''),
    'pipeline_terminal_forwards_labelled_borrows_and_boolean_feedback': (IO+
        'func offers(line:const u64 in P) yields {id:u64;ref cell:const u64 in P;} -> bool where {owner P}{'
        'if(!yield(7u64->id,line->cell)){return;}else{};yield(80u64->id,line->cell):void;}'
        'proc audit(ref cell:const u64 in A,id:u64)->bool where {owner A}{'
        'behave @print(id);behave @print(cell);return false;}'
        'val entry_line<-76u64;offers(entry_line) $> audit;audit <$ offers(entry_line);',0,'7\n76\n7\n76\n',''),
    'pipeline_terminal_uses_the_labels_of_each_selected_source': (IO+
        'func rows(n:u32) yields {id:u32;}{yield(n->id);}'
        'func rows(n:u64) yields {code:u64;}{yield(n->code);}'
        'proc audit(id:u32)->void{behave @print(id+10u32);return;}'
        'proc audit(code:u64)->void{behave @print(code+20u64);return;}'
        'proc drive(n:T)->void where {T}{rows(n) $> audit;return;}drive(7u32);drive(8u64);',0,'17\n28\n',''),
    'pipeline_terminal_whole_traversal_returns_a_count_for_saved_and_empty_sources': (IO+
        'func rows(active:bool) yields u64 -> bool{if(!active){return;}else{};'
        'if(!yield(17u64)){return;}else{};if(!yield(24u64)){return;}else{};yield(42u64):void;}'
        'func step(here input() yields u64 -> bool,offset:u64) yields u64 -> bool{'
        'val increment<-offset;for val item<-input(){continue yield(item+increment);};}'
        'proc ($>)(here traversal() yields u64 -> bool,starting:u64)->u64{val count<-starting;behave @print(100u64);'
        'for val item<-traversal(){behave @print(item);count<-count+1u64;continue count<2u64;};return count;}'
        'here stages=step(1u64) |> step(2u64);here scan=rows(true) !> stages;here empty_scan=rows(false) !> stages;'
        'val counter<-0u64;behave @print(scan $> counter);behave @print(counter <$ scan);behave @print(($>)(empty_scan,counter));',
        0,'100\n20\n27\n2\n100\n20\n27\n2\n100\n0\n',''),
    'pipeline_terminal_whole_traversal_prepares_inline_stages_in_written_order': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}func rows(n:u64) yields u64{yield(n);}'
        'func step(here input() yields u64,offset:u64) yields u64{'
        'val increment<-offset;for val item<-input(){yield(item+increment);};}'
        'proc (<$)(sink:u64,here traversal() yields u64)->u64{'
        'val result<-sink;for val item<-traversal(){result<-result+item;};return result;}'
        'behave @print(rows(mark(10u64)) !> step(mark(1u64)) |> step(mark(2u64)) $> mark(20u64));'
        'behave @print(mark(30u64) <$ step(mark(4u64)) <| step(mark(3u64)) <! rows(mark(40u64)));'
        'behave @print((<$)(mark(50u64),rows(mark(60u64)) !> step(mark(6u64))));',
        0,'10\n1\n2\n20\n33\n30\n4\n3\n40\n77\n50\n60\n6\n116\n',''),
    'pipeline_terminal_whole_traversal_stops_preparation_at_the_failing_operand': (IO+
        'using "std:exceptions" NullPointerException;'
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'proc fail_operand(n:u64)->u64 throws NullPointerException{'
        'behave @print(n);ptr absent:u64=null in here;absent;return n;}'
        'func rows(n:u64) yields u64{yield(n);}'
        'func step(here input() yields u64,offset:u64) yields u64{'
        'val increment<-offset;for val item<-input(){yield(item+increment);};}'
        'proc ($>)(here traversal() yields u64,sink:u64)->u64{behave @print(100u64);return sink;}'
        'try{rows(mark(1u64)) !> step(fail_operand(2u64)) |> step(mark(3u64)) $> mark(4u64):void;}'
        'catch(e:const NullPointerException){behave @print(5u64);};'
        'try{fail_operand(6u64) <$ step(mark(7u64)) <! rows(mark(8u64)):void;}'
        'catch(e:const NullPointerException){behave @print(9u64);};',
        0,'1\n2\n5\n6\n9\n',''),
    'pipeline_terminal_whole_traversal_selects_overloads_by_source_protocol': (IO+
        'func small() yields u32{yield(17u32);}func wide() yields u64{yield(24u64);}'
        'proc ($>)(here traversal() yields u32,sink:u64)->u64{for val item<-traversal(){behave @print(item);};return sink+1u64;}'
        'proc ($>)(here traversal() yields u64,sink:u64)->u64{for val item<-traversal(){behave @print(item);};return sink+2u64;}'
        'proc item(value:u64)->void{behave @print(value);return;}'
        'val count<-10u64;behave @print(small() $> count);behave @print(count <$ wide());wide() $> item;',
        0,'17\n11\n24\n12\n24\n',''),
    'pipeline_terminal_whole_traversal_rejects_a_mismatched_sink_owner': (
        'func rows(line:const u64 in R) yields ref const u64 in R where {owner R}{yield(line);}'
        'proc ($>)(here traversal() yields ref const u64 in A,sink:const u64 in A)->void where {owner A}{return;}'
        'proc query(line:const u64 in R)->void where {owner R}{val other<-75u64;rows(line) $> other;return;}'
        'val input_line<-75u64;query(input_line);',2,'','no whole-traversal terminal matches the source and sink contracts'),
    'named_scalar_call_prepares_written_order_and_allows_an_in_position_label': (IO+
        'proc trace(n:u64)->u64{behave @print(n);return n;}'
        'proc apply(left:u64,right:u64)->void{behave @print(10u64*left+right);return;}'
        'apply(trace(2u64)->right,trace(1u64)->left);apply(trace(3u64)->left,trace(4u64));',
        0,'2\n1\n12\n3\n4\n34\n',''),
    'named_scalar_call_rejects_a_positional_operand_after_reordering': (
        'proc apply(left:u64,right:u64)->void{return;}apply(2u64->right,1u64);',
        2,'','written arguments must bind distinct ordinary parameters of the callable'),
    'lexical_pipeline_reuses_stages_with_independent_sources_and_bound_sink': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'func rows(day:u64) yields u64{yield(day);}'
        'func add(here input() yields u64,offset:u64) yields u64{val amount<-offset;for val item<-input(){yield(item+amount);};}'
        'func scale(here input() yields u64,factor:u64) yields u64{val amount<-factor;for val item<-input(){yield(item*amount);};}'
        'proc sink(n:u64)->void{behave @print(n);return;}'
        'proc run()->void{here stages=add(mark(2u64)) |> scale(mark(3u64));'
        'here first=rows(mark(10u64)) !> stages;here second=stages <! rows(mark(20u64));'
        'first $> sink;sink <$ second;for val item<-first(){sink(item);};'
        'here to_sink=stages $> sink;rows(mark(30u64)) !> to_sink;return;}run();',
        0,'2\n3\n10\n20\n36\n66\n36\n30\n96\n',''),
    'lexical_pipeline_keeps_the_selected_readonly_targets': (IO+
        'func rows(line:const u64 in P) yields ref const u64 in P -> bool where {owner P}{'
        'if(!yield(line)){return;}else{};yield(line):void;}'
        'func stage(here input() yields ref const u64 in O -> bool,witness:const u64 in O) '
        'yields ref const u64 in O -> bool where {owner O}{for ref item=input(){continue yield(item);};}'
        'proc run()->void{val first<-75u64;val second<-99u64;ref selected=first;'
        'here stages=stage(selected) |> stage(selected);here attached=rows(selected) !> stages;'
        'selected=second;first<-76u64;for ref item=attached(){behave @print(item);behave @print(item is first);continue false;};'
        'return;}run();',0,'76\ntrue\n',''),
    'lexical_pipeline_retains_the_terminal_at_its_binding_site': (IO+
        'func rows() yields u64{yield(7u64);}'
        'func stage(here input() yields u64) yields u64{for val item<-input(){yield(item);};}'
        'proc report(item:u64)->void{behave @print(item);return;}'
        'proc run()->void{here stages=stage |> stage;here to_sink=stages $> report;'
        'val report<-99u64;rows !> to_sink;behave @print(report);return;}run();',0,'7\n99\n','[-Wshadow]'),
    'lexical_pipeline_cannot_escape_as_an_ordinary_result': (
        'func stage(here input() yields u64) yields u64{for val item<-input(){yield(item);};}'
        'proc run()->u64{here stages=stage |> stage;return stages;}run();',2,'','a bounded here binding requires a pipeline connection or consumer'),
    'lexical_pipeline_drives_a_saved_source_during_deferred_execution': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'func rows(day:u64) yields u64{yield(day);}'
        'func stage(here input() yields u64) yields u64{for val item<-input(){yield(item);};}'
        'proc sink(n:u64)->void{behave @print(n);return;}'
        'proc run()->void{here stages=stage |> stage;here prepared=rows(mark(1u64)) !> stages;'
        'defer{prepared $> sink;};behave @print(2u64);return;}run();',0,'1\n2\n1\n',''),
    'lexical_pipeline_requires_the_binding_on_every_reaching_path': (
        'func rows() yields u64{yield(1u64);}'
        'func stage(here input() yields u64) yields u64{for val item<-input(){yield(item);};}'
        'proc sink(n:u64)->void{return;}'
        'proc run(ready:bool)->void{if(ready){here stages=stage |> stage;}else{};'
        'rows !> stages $> sink;return;}run(false);',2,'',"here binding 'stages' is not available on every reaching path"),
    'lexical_pipeline_rejects_unbound_ordinary_arguments': (
        'func rows() yields u64{yield(1u64);}'
        'func stage(here input() yields u64) yields u64{for val item<-input(){yield(item);};}'
        'proc sink(n:u64)->void{return;}'
        'proc run()->void{here stages=stage |> stage;rows !> stages(9u64) $> sink;return;}run();',
        2,'','a lexical pipeline has no unbound ordinary arguments'),
    'lexical_pipeline_forwards_independent_sources_through_a_helper': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'func rows(day:u64) yields u64{yield(day);}'
        'func add(here input() yields u64,offset:u64) yields u64{val amount<-offset;for val item<-input(){yield(item+amount);};}'
        'func scale(here input() yields u64,factor:u64) yields u64{val amount<-factor;for val item<-input(){yield(item*amount);};}'
        'proc drive(here input() yields u64)->void{for val item<-input(){behave @print(item);};return;}'
        'proc relay(here left() yields u64,here right() yields u64)->void{drive(left);drive(right);drive(left);return;}'
        'proc run()->void{here stages=add(mark(2u64)) |> scale(mark(3u64));'
        'here first=rows(mark(10u64)) !> stages;here second=stages <! rows(mark(20u64));'
        'relay(first,second);return;}run();',0,'2\n3\n10\n20\n36\n66\n36\n',''),
    'lexical_pipeline_forwarding_carries_the_selected_borrow': (IO+
        'func rows(line:const u64 in P) yields {id:u64;ref cell:const u64 in P;} -> bool where {owner P}{'
        'if(!yield(7u64->id,line->cell)){return;}else{};yield(80u64->id,line->cell):void;}'
        'func stage(here input() yields {id:u64;ref cell:const u64 in S;} -> bool,witness:const u64 in S) '
        'yields {id:u64;ref cell:const u64 in S;} -> bool where {owner S}{'
        'for {val n<-id;ref item=cell;} := input(){continue yield(n->id,item->cell);};}'
        'proc drive(here input() yields {id:u64;ref cell:const u64 in D;} -> bool,witness:const u64 in D)->void where {owner D}{'
        'for {val n<-id;ref item=cell;} := input(){behave @print(n);behave @print(item);continue false;};return;}'
        'proc relay(here input() yields {id:u64;ref cell:const u64 in R;} -> bool,witness:const u64 in R)->void where {owner R}{'
        'drive(input,witness);return;}'
        'proc run()->void{val first<-75u64;val second<-99u64;ref selected=first;'
        'here stages=stage(selected) |> stage(selected);here attached=rows(selected) !> stages;'
        'selected=second;first<-76u64;relay(attached,first);return;}run();',0,'7\n76\n',''),
    'lexical_pipeline_forwarding_rejects_a_different_expected_owner': (
        'func rows(line:const u64 in P) yields ref const u64 in P where {owner P}{yield(line);}'
        'func stage(here input() yields ref const u64 in S,witness:const u64 in S) '
        'yields ref const u64 in S where {owner S}{for ref item=input(){yield(item);};}'
        'proc drive(here input() yields ref const u64 in D,witness:const u64 in D)->void where {owner D}{return;}'
        'proc run(outside:const u64 in O)->void where {owner O}{val inside<-99u64;'
        'here stages=stage(outside) |> stage(outside);here attached=rows(outside) !> stages;'
        'drive(attached,inside);return;}val outside<-75u64;run(outside);',
        2,'','borrowed here input has a different actual owner from its contract'),
    'lexical_pipeline_forwarding_preserves_a_helper_return_through_cleanup': (IO+
        'proc rows() yields u64{try{yield(7u64);behave @print(80u64);}finally{behave @print(31u64);};return;}'
        'proc stage(here input() yields u64,stamp:u64) yields u64{try{for val item<-input(){yield(item);};'
        'behave @print(81u64);}finally{behave @print(stamp);};return;}'
        'proc drive(here input() yields u64)->u64{for val item<-input(){return item;};return 99u64;}'
        'proc relay(here input() yields u64)->u64{return drive(input);}'
        'proc run()->void{here stages=stage(32u64) |> stage(33u64);here attached=rows !> stages;'
        'behave @print(relay(attached));return;}run();',0,'31\n32\n33\n7\n',''),
    'lexical_pipeline_can_supply_a_fresh_stage_as_its_source': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'func rows(day:u64) yields u64{yield(day);}'
        'func add(here input() yields u64,offset:u64) yields u64{val amount<-offset;for val item<-input(){yield(item+amount);};}'
        'func scale(here input() yields u64,factor:u64) yields u64{val amount<-factor;for val item<-input(){yield(item*amount);};}'
        'proc sink(n:u64)->void{behave @print(n);return;}'
        'proc drive(here input() yields u64)->void{for val item<-input(){sink(item);};return;}'
        'proc run()->void{here stages=add(mark(2u64)) |> scale(mark(3u64));here reviewed=rows(10u64) !> stages;'
        'reviewed !> add(mark(4u64)) $> sink;here rerouted=scale(mark(5u64)) <! reviewed;'
        'drive(rerouted);reviewed $> sink;return;}run();',0,'2\n3\n4\n40\n5\n180\n36\n',''),
    'lexical_pipeline_forwards_open_stages_and_an_attached_terminal': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'func rows(day:u64) yields u64{yield(day);}'
        'func add(here input() yields u64,offset:u64) yields u64{val amount<-offset;for val item<-input(){yield(item+amount);};}'
        'func scale(here input() yields u64,factor:u64) yields u64{val amount<-factor;for val item<-input(){yield(item*amount);};}'
        'proc sink(n:u64)->void{behave @print(n);return;}'
        'proc process(here plan(here input() yields u64) yields u64)->void{'
        'for val item<-rows(mark(10u64)) !> plan{sink(item);};'
        'for val item<-plan <! rows(mark(20u64)){sink(item);};return;}'
        'proc submit(here terminal(here input() yields u64)->void,here input() yields u64)->void{input !> terminal;return;}'
        'proc relay(here terminal(here input() yields u64)->void)->void{'
        'submit(terminal,rows(mark(30u64)));submit(terminal,rows(mark(40u64)));return;}'
        'proc run()->void{here stages=add(mark(2u64)) |> scale(mark(3u64));'
        'process(stages);here to_sink=stages $> sink;val sink<-99u64;relay(to_sink);behave @print(sink);return;}run();',
        0,'2\n3\n10\n36\n20\n66\n30\n96\n40\n126\n99\n','[-Wshadow]'),
    'lexical_pipeline_open_forwarding_preserves_a_helper_return_through_cleanup': (IO+
        'proc rows() yields u64{try{yield(7u64);behave @print(80u64);}finally{behave @print(31u64);};return;}'
        'proc stage(here input() yields u64,stamp:u64) yields u64{try{for val item<-input(){yield(item);};'
        'behave @print(81u64);}finally{behave @print(stamp);};return;}'
        'proc drive(here plan(here input() yields u64) yields u64)->u64{'
        'for val item<-rows !> plan{return item;};return 99u64;}'
        'proc run()->void{here stages=stage(32u64) |> stage(33u64);'
        'behave @print(drive(stages));return;}run();',0,'31\n32\n33\n7\n',''),
    'lexical_pipeline_open_forwarding_preserves_hidden_stage_borrows': (IO+
        'func rows() yields u64{yield(100u64);}'
        'func stage(here input() yields u64,amount:const u64 in O) yields u64 where {owner O}{'
        'val offset<-amount;for val item<-input(){yield(item+offset);};}'
        'proc drive(here plan(here input() yields u64) yields u64)->void{'
        'for val item<-rows !> plan{behave @print(item);};return;}'
        'proc run(outside:const u64 in P)->void where {owner P}{val first<-5u64;val replacement<-50u64;ref selected=first;'
        'here stages=stage(selected) |> stage(outside);selected=replacement;first<-6u64;drive(stages);return;}'
        'val supplied_amount<-8u64;run(supplied_amount);',0,'114\n',''),
    'lexical_pipeline_open_forwarding_preserves_labelled_borrows_and_feedback': (IO+
        'func rows(line:const u64 in P) yields {id:u64;ref cell:const u64 in P;} -> bool where {owner P}{'
        'if(!yield(7u64->id,line->cell)){return;}else{};yield(80u64->id,line->cell):void;}'
        'func stage(here input() yields {id:u64;ref cell:const u64 in S;} -> bool,witness:const u64 in S) '
        'yields {id:u64;ref cell:const u64 in S;} -> bool where {owner S}{'
        'for {val n<-id;ref item=cell;} := input(){continue yield(n->id,item->cell);};}'
        'proc drive(here plan(here input() yields {id:u64;ref cell:const u64 in D;} -> bool) '
        'yields {id:u64;ref cell:const u64 in D;} -> bool,line:const u64 in D)->void where {owner D}{'
        'for {val n<-id;ref item=cell;} := rows(line) !> plan{'
        'behave @print(n);behave @print(item);behave @print(item is line);continue false;};return;}'
        'proc relay(here plan(here input() yields {id:u64;ref cell:const u64 in R;} -> bool) '
        'yields {id:u64;ref cell:const u64 in R;} -> bool,line:const u64 in R)->void where {owner R}{drive(plan,line);return;}'
        'proc run()->void{val first<-75u64;val second<-99u64;ref selected=first;'
        'here stages=stage(selected) |> stage(selected);selected=second;first<-76u64;relay(stages,first);return;}run();',
        0,'7\n76\ntrue\n',''),
    'lexical_pipeline_open_forwarding_accepts_independent_port_owners': (IO+
        'func rows(line:const u64 in R) yields ref const u64 in R where {owner R}{yield(line);}'
        'func pass(here input() yields ref const u64 in S,witness:const u64 in S) yields ref const u64 in S where {owner S}{'
        'for ref cell=input(){yield(cell);};}'
        'func replace(here input() yields ref const u64 in A,origin:const u64 in A,replacement:const u64 in B) '
        'yields ref const u64 in B where {owner A,owner B}{for ref cell=input(){yield(replacement);};}'
        'proc drive(here plan(here input() yields ref const u64 in A) yields ref const u64 in B,'
        'line:const u64 in A,output_value:const u64 in B)->void where {owner A,owner B}{'
        'for ref cell=plan <! rows(line){behave @print(cell);behave @print(cell is output_value);};return;}'
        'proc run(original:const u64 in P)->void where {owner P}{val replacement<-42u64;'
        'here stages=pass(original) |> replace(original,replacement);drive(stages,original,replacement);return;}'
        'val supplied_line<-75u64;run(supplied_line);',0,'42\ntrue\n',''),
    'lexical_pipeline_open_forwarding_rejects_the_wrong_input_owner': (
        'func stage(here input() yields ref const u64 in O,witness:const u64 in O) yields ref const u64 in O where {owner O}{'
        'for ref cell=input(){yield(cell);};}'
        'proc accepts(here plan(here input() yields ref const u64 in A) yields ref const u64 in B,'
        'source:const u64 in A,destination:const u64 in B)->void where {owner A,owner B}{return;}'
        'proc run(outer:const u64 in R)->void where {owner R}{val inner<-42u64;'
        'here stages=stage(outer) |> stage(outer);accepts(stages,inner,outer);return;}val line<-75u64;run(line);',
        2,'','borrowed here input has a different actual owner from its contract'),
    'lexical_pipeline_open_forwarding_rejects_the_wrong_output_owner': (
        'func stage(here input() yields ref const u64 in O,witness:const u64 in O) yields ref const u64 in O where {owner O}{'
        'for ref cell=input(){yield(cell);};}'
        'proc accepts(here plan(here input() yields ref const u64 in A) yields ref const u64 in B,'
        'source:const u64 in A,destination:const u64 in B)->void where {owner A,owner B}{return;}'
        'proc run(outer:const u64 in R)->void where {owner R}{val inner<-42u64;'
        'here stages=stage(outer) |> stage(outer);accepts(stages,outer,inner);return;}val line<-75u64;run(line);',
        2,'','borrowed here input has a different actual owner from its contract'),
    'lexical_pipeline_terminal_forwarding_preserves_a_source_invocation_owner': (IO+
        'func origin(n:u64) yields ref const u64 in here{val cell<-n;yield(cell);}'
        'func convert(here input() yields ref const u64 in here) yields u64{for ref item=input(){yield(item+1u64);};}'
        'func pass(here input() yields u64) yields u64{for val item<-input(){yield(item);};}'
        'proc report(n:u64)->void{behave @print(n);return;}'
        'proc submit(here terminal(here input() yields ref const u64 in here)->void,'
        'here input() yields ref const u64 in here)->void{input !> terminal;return;}'
        'proc relay(here terminal(here input() yields ref const u64 in here)->void)->void{submit(terminal,origin(24u64));return;}'
        'here stages=convert |> pass;here terminal=stages $> report;relay(terminal);',0,'25\n',''),
    'supplied_scope_requires_proof_for_rebinding_a_live_reference_capture': (
        'proc twice(here action()->void)->void{action();action();return;}'
        'proc run(ref selection:const u64 in O,ref other:const u64 in O)->void where {owner O}{'
        'twice(){{selection=other;return;}};return;}'
        'val one<-1u64,two<-2u64;run(one,two);',
        2,'','rebinding a supplier reference slot from a repeatedly callable here body'),
    'lexical_pipeline_open_forwarding_checks_the_upstream_feedback': (
        'func stage(here input() yields u64) yields u64{for val item<-input(){yield(item);};}'
        'proc accepts(here plan(here input() yields u64 -> bool) yields u64)->void{return;}'
        'here stages=stage |> stage;accepts(stages);',2,'','open pipeline does not match the selected here contract'),
    'lexical_pipeline_forwarded_terminal_failure_crosses_entered_cleanups': (IO+'using "std:exceptions" NullPointerException;'+
        'proc rows() yields u64{try{yield(7u64);}catch(e:const NullPointerException){behave @print(80u64);}'
        'finally{behave @print(31u64);};return;}'
        'proc stage(here input() yields u64,stamp:u64) yields u64{try{for val item<-input(){yield(item);};}'
        'catch(e:const NullPointerException){behave @print(81u64);}finally{behave @print(stamp);};return;}'
        'proc sink(n:u64)->void throws NullPointerException{behave @print(n);ptr p:u64=null in here;p;return;}'
        'proc drive(here terminal(here input() yields u64)->void throws NullPointerException)->void throws NullPointerException{'
        'rows !> terminal;return;}'
        'here stages=stage(32u64) |> stage(33u64);here terminal=stages $> sink;'
        'try{drive(terminal);}catch(e:const NullPointerException){behave @print(34u64);};',
        0,'7\n31\n32\n33\n34\n',''),
    'lexical_pipeline_forwarding_cannot_erase_a_named_terminal_failure': ('using "std:exceptions" NullPointerException;'
        'func stage(here input() yields u64) yields u64{for val item<-input(){yield(item);};}'
        'proc sink(n:u64)->void throws NullPointerException{ptr p:u64=null in here;p;return;}'
        'proc discard(here terminal(here input() yields u64)->void)->void{return;}'
        'here stages=stage |> stage;here terminal=stages $> sink;discard(terminal);',
        2,'','open pipeline checked failures do not match the selected here contract'),
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
    'prepared_here_source_retains_borrow_and_owner_through_forwarding': (IO+
        'proc rows(day:u64,source:const u64 in Ledger) yields {id:u64;ref line:const u64 in Ledger;} -> bool where {owner Ledger}{'
        'if(!yield(day->id,source->line)){return;}else{};yield(42u64->id,source->line);return;}'
        'proc stage(here input() yields {id:u64;ref line:const u64 in S;} -> bool,witness:const u64 in S,enabled:bool) '
        'yields {id:u64;ref line:const u64 in S;} -> bool where {owner S}{'
        'if(!enabled){return;}else{};for {val n<-id;ref item=line;} := input(){continue yield(item->line,n->id);};return;}'
        'proc relay(here upstream() yields {ref line:const u64 in O;id:u64;} -> bool,witness:const u64 in O,enabled:bool) '
        'yields {ref line:const u64 in O;id:u64;} -> bool where {owner O}{'
        'for {val n<-id;ref item=line;} := stage(upstream,witness,enabled){continue yield(n->id,item->line);};return;}'
        'proc run()->void{val first<-75u64;val second<-99u64;ref selected=first;'
        'for {val n<-id;ref item=line;} := relay(rows(24u64,selected),first,{selected=second;first<-76u64;true}){'
        'behave @print(n);behave @print(item);behave @print(item is first);continue false;};'
        'behave @print(selected);return;}run();',0,'24\n76\ntrue\n99\n',''),
    'prepared_here_source_rejects_a_different_borrowed_owner': (
        'func rows(source:const u64 in P) yields ref const u64 in P where {owner P}{yield(source);}'
        'proc consume(here src() yields ref const u64 in O,witness:const u64 in O)->void where {owner O}{return;}'
        'proc run()->void{val outside<-1u64;{{val inside<-2u64;consume(rows(outside),inside);return;}};return;}run();',
        2,'','borrowed here input has a different actual owner from its contract'),
    'prepared_here_source_retains_two_distinct_ordinary_owners': (IO+
        'proc rows(line:const u64 in P,id:const u64 in Q) yields {id:u64;ref line:const u64 in P;} where {owner P,owner Q}{'
        'yield(id->id,line->line);return;}'
        'proc stage(here source() yields {id:u64;ref line:const u64 in O;},witness:const u64 in O) '
        'yields {id:u64;ref line:const u64 in O;} where {owner O}{'
        'for {val n<-id;ref item=line;} := source(){yield(item->line,n->id);};return;}'
        'proc run()->void{val outside<-70u64;{{val inside<-24u64;'
        'for {val n<-id;ref item=line;} := stage(rows(outside,inside),outside){behave @print(n);behave @print(item);};'
        'return;}};return;}run();',0,'24\n70\n',''),
    'prepared_here_source_distinguishes_its_owner_from_an_external_owner': (
        'func rows() yields ref const u64 in here{val local_value<-1u64;yield(local_value);}'
        'proc consume(here src() yields ref const u64 in O,witness:const u64 in O)->void where {owner O}{return;}'
        'val n<-2u64;consume(rows(),n);',2,'','prepared producer handoff owner does not match the selected contract'),
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
    'query_here_receiver_borrow_keeps_the_actual_argument_owner': (IO+
        'func same(ref a:u64 in O,ref b:u64 in O)->bool where {owner O}{return a is b;}'
        'func inspect(here step(ref item:u64 in O)->bool,ref source:u64 in O)->bool where {owner O}{return step(source);}'
        'proc run()->void{val n<-7u64;val proof<-make local typeof(inspect(n)(ref item=)=>{same(item,n)}){true};'
        'val local_proof<-make local typeof(inspect(n)(item)=>{{return same(item,n);}}){true};'
        'behave @print(proof);behave @print(local_proof);return;}run();',0,'true\ntrue\n',''),
    'query_here_receiver_keeps_invocation_owner_separate_from_supplier': (
        'func same(ref a:u64 in O,ref b:u64 in O)->bool where {owner O}{return a is b;}'
        'func inspect(here step(ref item:u64 in here)->bool)->bool{val n<-7u64;return step(n);}'
        'proc run()->void{val n<-7u64;val proof<-make local typeof(inspect()(ref item=)=>{same(item,n)}){true};return;}run();',
        2,'','arguments for one owner role have different actual owners'),
    'query_here_receiver_retains_readonly_borrow': (
        'func inspect(here step(ref item:const u64 in O)->u64,source:const u64 in O)->u64 where {owner O}{return step(source);}'
        'val n<-7u64;val proof<-make local typeof(inspect(n)(ref item=)=>{item<-3u64;item}){7u64};',2,'','readonly'),
    'query_here_receiver_rejects_readonly_ordinary_writable_borrow': (
        'func inspect(here step(ref item:u64 in O)->u64,ref source:u64 in O)->u64 where {owner O}{return step(source);}'
        'const val frozen<-7u64;val proof<-make local typeof(inspect(frozen)(ref item=)=>{item}){7u64};',
        2,'','readonly argument cannot supply a writable borrowed parameter'),
    'query_here_receiver_snapshot_has_independent_access': (IO+
        'func inspect(here step(ref item:const u64 in O)->u64,source:const u64 in O)->u64 where {owner O}{return step(source);}'
        'val n<-7u64;val proof<-make local typeof(inspect(n)(val item<-)=>{item<-3u64;item}){n};'
        'behave @print(proof);',0,'7\n',''),
    'query_here_receiver_snapshot_does_not_keep_the_lent_owner': (
        'func same(ref a:u64 in O,ref b:u64 in O)->bool where {owner O}{return a is b;}'
        'func inspect(here step(ref item:u64 in O)->bool,ref source:u64 in O)->bool where {owner O}{return step(source);}'
        'proc run()->void{val n<-7u64;val proof<-make local typeof(inspect(n)(val item<-)=>{same(item,n)}){true};return;}run();',
        2,'','arguments for one owner role have different actual owners'),
    'query_here_receiver_double_brace_has_local_return': (IO+
        'func compare(here step(n:u64)->u64,input:u64)->bool{return step(input)==7u64;}'
        'proc run()->bool{val result<-make local typeof(compare(7u64)(n)=>{{behave @print(90u64);return n;}}){true};'
        'return result;}behave @print(run());',0,'true\n',''),
    'query_here_receiver_double_brace_checks_declared_failures': (IO+'using "std:exceptions" NullPointerException;'+
        'proc may_fail()->void throws NullPointerException{return;}'
        'func admits(here step()->u64 throws NullPointerException)->u64 throws NullPointerException{return step();}'
        'func recovers(here step()->u64)->u64{return step();}'
        'proc run()->void{val first<-make local typeof(admits(){{may_fail();return 1u64;}}){11u64};'
        'val second<-make local typeof(recovers(){{try{may_fail();}catch(e:const NullPointerException){};return 1u64;}}){12u64};'
        'behave @print(first);behave @print(second);return;}run();',0,'11\n12\n',''),
    'query_here_receiver_double_brace_rejects_failure_covered_only_by_receiver': (
        'using "std:exceptions" NullPointerException;proc may_fail()->void throws NullPointerException{return;}'
        'func receive(here step()->u64)->u64 throws NullPointerException{return step();}'
        'proc run()->void{val v<-make local typeof(receive(){{may_fail();return 1u64;}}){2u64};return;}run();',
        2,'',"checked exception 'NullPointerException' requires a covering catch or throws declaration"),
    'query_here_receiver_double_brace_preserves_root_failure_contract': (IO+'using "std:exceptions" Exception;'+
        'proc broad()->void throws Exception{return;}'
        'func admits(here step()->u64 throws Exception)->u64 throws Exception{return step();}'
        'func recovers(here step()->u64)->u64{return step();}'
        'proc run()->void{val first<-make local typeof(admits(){{broad();return 1u64;}}){11u64};'
        'val second<-make local typeof(recovers(){{try{broad();}catch(e:const Exception){};return 1u64;}}){12u64};'
        'behave @print(first);behave @print(second);return;}run();',0,'11\n12\n',''),
    'query_here_receiver_double_brace_does_not_discharge_root_via_finally': (
        'using "std:exceptions" Exception;proc broad()->void throws Exception{return;}'
        'func receive(here step()->u64)->u64{return step();}'
        'proc run()->void{val v<-make local typeof(receive(){{try{broad();}finally{return 1u64;};}}){2u64};return;}run();',
        2,'',"declared checked exception 'Exception' requires a covering catch or throws declaration"),
    'query_here_receiver_double_brace_ignores_unreachable_root_failure': (IO+
        'using "std:exceptions" Exception;using "std:exceptions" NullPointerException;'
        'proc broad()->void throws Exception{return;}func receive(here step()->u64)->u64{return step();}'
        'proc run()->void{val v<-make local typeof(receive(){{return 1u64;try{broad();}catch(e:const NullPointerException){};}}){7u64};'
        'behave @print(v);return;}run();',0,'7\n',''),
    'supplied_scope_explicit_root_returns_to_receiver_catch': (IO+'using "std:exceptions" Exception;'+
        'proc broad()->void throws Exception{ptr p:u64=null in here;p;return;}'
        'func handle(here step()->u64 throws Exception)->u64{try{return step();}catch(e:const Exception){return 9u64;};}'
        'proc run()->void{behave @print(handle(){{broad();return 7u64;}});return;}run();',0,'9\n',''),
    'supplied_scope_narrow_catches_do_not_cover_declared_root': (
        'using "std:exceptions" Exception;using "std:exceptions" NullPointerException;'
        'using "std:exceptions" StaticallyUnknownTypeException;'
        'func handle(here step()->u64 throws Exception)->u64{try{return step();}'
        'catch(a:const NullPointerException){return 8u64;}catch(b:const StaticallyUnknownTypeException){return 9u64;};}'
        'handle(){{return 7u64;}};',2,'',"declared checked exception 'Exception' requires a covering catch or throws declaration"),
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
    'query_product_field_inspection_leaves_outer_payload_unevaluated': (
        IO + QUERY_PAIR + 'ptr item=make Pair;' + query('ptr local=make Pair;return item.second;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_nested_outer_product_field_inspection_leaves_payload_unevaluated': (
        IO + QUERY_PAIR + 'data Outer {child:Pair;}ptr item=make Outer;' +
        query('ptr local=make Pair;return item.child.second;') + 'behave @print(result);', 0, '4\n', ''),
    'query_product_local_field_read_requires_initialization': (
        QUERY_PAIR + query('ptr item=make Pair;return item.second;'), 2, '', 'initialization'),
    'query_discarded_product_field_read_requires_initialization': (
        QUERY_PAIR + query('ptr item=make Pair;item.second;return 7u64;'), 2, '', 'initialization'),
    'query_product_write_through_another_reference_reaches_the_same_field': (
        IO + QUERY_PAIR + query('ptr item=make Pair;ptr another=item;another.second<-7u64;'
                               'behave @print(97u64);return item.second;') + 'behave @print(result);', 0, '4\n', ''),
    'query_product_write_does_not_reach_a_rebound_target': (
        QUERY_PAIR + query('ptr first=make Pair;ptr second=make Pair;ptr chosen=first;'
                           'chosen.second<-7u64;chosen=second;return chosen.second;'), 2, '', 'initialization'),
    'query_nested_product_write_preserves_sibling_readiness': (
        QUERY_PAIR + 'data Outer {child:Pair;}' +
        query('ptr item=make Outer;item.child.first<-7u64;return item.child.second;'), 2, '', 'initialization'),
    'query_product_slot_read_requires_its_own_initialization': (
        QUERY_HOLDER + query('ptr item=make Holder;return item.selected;'), 2, '', 'initialization'),
    'query_product_slot_binding_does_not_initialize_its_referent': (
        QUERY_HOLDER + query('val unread<-make local u64;ptr item=make Holder;'
                             'item.selected=unread;return item.selected;'), 2, '', 'initialization'),
    'query_nested_full_scope_rebinds_the_embedded_product_slot': (
        IO + QUERY_HOLDER + query('val ready<-7u64;ptr item=make Holder;{{item.selected=ready;return;}};'
                                  'return item.selected;') + 'behave @print(result);', 0, '4\n', ''),
    'query_product_cleanup_write_only_applies_after_the_inner_scope': (
        IO + QUERY_PAIR + query('ptr item=make Pair;{{defer{item.second<-7u64;};}};return item.second;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_product_read_before_its_own_cleanup_write_rejects': (
        QUERY_PAIR + query('ptr item=make Pair;defer{item.second<-7u64;};return item.second;'), 2, '', 'initialization'),
    'query_complete_reference_checks_required_product_fields_across_a_cycle': (
        QUERY_NODE + query('ptr a=make Node;ptr b=make Node;a.value<-7u64;a.next=b;b.next=a;'
                           'ref complete=a;return complete.next.value;'), 2, '', 'initialization'),
    'query_complete_reference_accepts_a_ready_required_product_cycle': (
        IO + QUERY_NODE + query('ptr a=make Node;ptr b=make Node;a.value<-7u64;b.value<-8u64;'
                                'a.next=b;b.next=a;ref complete=a;return complete.next.value;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_helper_rejects_an_unwritten_product_field': (
        QUERY_PAIR + READ_QUERY_PAIR + query('ptr item=make Pair;return inspect(item);'),
        2, '', "read of 'second' before definite initialization"),
    'query_helper_does_not_require_an_unread_sibling': (
        IO + QUERY_PAIR + READ_QUERY_PAIR +
        query('ptr item=make Pair;item.second<-7u64;return inspect(item);') + 'behave @print(result);', 0, '4\n', ''),
    'query_helper_writes_follow_the_actual_product': (
        IO + QUERY_PAIR + WRITE_QUERY_PAIR + query('ptr item=make Pair;populate(item);return item.second;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_helper_write_does_not_initialize_a_sibling': (
        QUERY_PAIR + WRITE_QUERY_PAIR + query('ptr item=make Pair;populate(item);return item.first;'),
        2, '', "read of 'first' before definite initialization"),
    'query_write_prepares_field_of_selected_product': (
        IO + QUERY_PAIR + QUERY_PRODUCT_FLAGS +
        query(QUERY_PRODUCT_CHOICE + 'selected.second<-7u64;return selected.second;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_product_field_proof_follows_retained_selection': (
        IO + QUERY_PAIR + QUERY_PRODUCT_FLAGS +
        query(QUERY_PRODUCT_CHOICE +
              'ptr retained=selected;selected.second<-7u64;selected=right;return retained.second;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_same_product_candidates_do_not_prove_same_selection': (
        QUERY_PAIR + QUERY_PRODUCT_FLAGS +
        query(QUERY_PRODUCT_CHOICE + 'ptr other=if(pick_other){left}else{right};'
              'selected.second<-7u64;return other.second;'), 2, '', 'initialization'),
    'query_product_field_proof_does_not_follow_rebound_selection': (
        QUERY_PAIR + QUERY_PRODUCT_FLAGS +
        query(QUERY_PRODUCT_CHOICE + 'selected.second<-7u64;selected=right;return selected.second;'),
        2, '', 'initialization'),
    'query_helpers_share_the_selected_product_field_proof': (
        IO + QUERY_PAIR + QUERY_PRODUCT_FLAGS + READ_QUERY_PAIR + WRITE_QUERY_PAIR +
        query(QUERY_PRODUCT_CHOICE + 'ptr retained=selected;populate(selected);selected=right;return inspect(retained);') +
        'behave @print(result);', 0, '4\n', ''),
    'query_helper_write_leaves_other_product_candidate_unproved': (
        QUERY_PAIR + QUERY_PRODUCT_FLAGS + READ_QUERY_PAIR + WRITE_QUERY_PAIR +
        query(QUERY_PRODUCT_CHOICE + 'populate(selected);return inspect(right);'), 2, '', 'initialization'),
    'query_helper_formals_can_share_the_same_selected_product': (
        IO + QUERY_PAIR + QUERY_PRODUCT_FLAGS + READ_QUERY_PAIR + WRITE_QUERY_PAIR +
        'proc prepare_then_read(ptr p:Pair in O,ptr other:const Pair in O)->u64 where {owner O}{'
        'populate(p);return inspect(other);}' +
        query(QUERY_PRODUCT_CHOICE + 'return prepare_then_read(selected,selected);') +
        'behave @print(result);', 0, '4\n', ''),
    'query_product_field_proof_projects_through_inline_children': (
        IO + QUERY_PAIR + QUERY_PRODUCT_FLAGS + 'data Outer {child:Pair;}' + READ_QUERY_PAIR + WRITE_QUERY_PAIR +
        query('ptr left=make Outer;ptr right=make Outer;ptr selected=if(pick_left){left}else{right};'
              'populate(selected.child);return inspect(selected.child);') +
        'behave @print(result);', 0, '4\n', ''),
    'query_product_field_cleanup_keeps_registration_selection': (
        IO + QUERY_PAIR + QUERY_PRODUCT_FLAGS + READ_QUERY_PAIR + WRITE_QUERY_PAIR +
        query(QUERY_PRODUCT_CHOICE + 'ptr retained=selected;{{defer populate(selected);selected=right;return;}};'
              'return inspect(retained);') + 'behave @print(result);', 0, '4\n', ''),
    'query_product_field_cleanup_leaves_rebound_selection_unproved': (
        QUERY_PAIR + QUERY_PRODUCT_FLAGS + READ_QUERY_PAIR + WRITE_QUERY_PAIR +
        query(QUERY_PRODUCT_CHOICE + '{{defer populate(selected);selected=right;return;}};return inspect(selected);'),
        2, '', 'initialization'),
    'query_prepared_transfer_writes_field_of_saved_product': (
        IO + QUERY_PAIR + QUERY_PRODUCT_FLAGS +
        query(QUERY_PRODUCT_CHOICE + 'ptr retained=selected;{{defer (<-)(selected.second,7u64);selected=right;return;}};'
              'return retained.second;') + 'behave @print(result);', 0, '4\n', ''),
    'query_write_through_borrow_prepares_its_selected_field': (
        IO + QUERY_PAIR + QUERY_PRODUCT_FLAGS +
        query(QUERY_PRODUCT_CHOICE + 'ptr field=selected.second;field<-7u64;return selected.second;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_borrow_follows_a_retained_receiver': (
        IO + QUERY_PAIR + QUERY_PRODUCT_FLAGS +
        query(QUERY_PRODUCT_CHOICE + 'ptr field=selected.second;ptr retained=selected;'
              'selected=right;field<-7u64;return retained.second;') + 'behave @print(result);', 0, '4\n', ''),
    'query_borrow_write_does_not_reach_a_rebound_receiver': (
        QUERY_PAIR + QUERY_PRODUCT_FLAGS +
        query(QUERY_PRODUCT_CHOICE + 'ptr field=selected.second;selected=right;field<-7u64;return selected.second;'),
        2, '', "read of 'second' before definite initialization"),
    'query_rebound_borrow_does_not_prepare_the_earlier_field': (
        QUERY_PAIR + QUERY_PRODUCT_FLAGS +
        query(QUERY_PRODUCT_CHOICE + 'ptr field=selected.second;field=right.first;field<-7u64;return selected.second;'),
        2, '', "read of 'second' before definite initialization"),
    'query_helper_can_return_a_borrow_of_the_selected_field': (
        IO + QUERY_PAIR + QUERY_PRODUCT_FLAGS + PROJECT_QUERY_FIELD +
        'func forward_field(ptr p:Pair in O)->ptr u64 in O where {owner O}{return project_field(p);}' +
        query(QUERY_PRODUCT_CHOICE + 'ptr field=forward_field(selected);field<-7u64;return selected.second;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_scalar_helper_can_forward_a_borrow_of_the_selected_field': (
        IO + QUERY_PAIR + QUERY_PRODUCT_FLAGS +
        'func forward_field(ptr p:u64 in O)->ptr u64 in O where {owner O}{return p;}' +
        query(QUERY_PRODUCT_CHOICE + 'ptr field=forward_field(selected.second);field<-7u64;return selected.second;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_helper_result_write_does_not_reach_a_rebound_receiver': (
        QUERY_PAIR + QUERY_PRODUCT_FLAGS + PROJECT_QUERY_FIELD +
        query(QUERY_PRODUCT_CHOICE + 'ptr field=project_field(selected);selected=right;field<-7u64;return selected.second;'),
        2, '', "read of 'second' before definite initialization"),
    'query_conditional_write_through_a_nullable_borrow_leaves_the_field_unproved': (
        QUERY_PAIR + QUERY_PRODUCT_FLAGS + MAYBE_QUERY_FIELD + WRITE_QUERY_FIELD +
        query(QUERY_PRODUCT_CHOICE + 'ptr field=maybe_field(selected,pick_other);fill_field(field);return selected.second;'),
        2, '', "read of 'second' before definite initialization"),
    'query_proved_present_borrowed_field_can_be_prepared_by_a_helper': (
        IO + QUERY_PAIR + QUERY_PRODUCT_FLAGS + MAYBE_QUERY_FIELD + WRITE_QUERY_FIELD +
        query(QUERY_PRODUCT_CHOICE + 'ptr field=maybe_field(selected,pick_left);if(field is null){return 0u64;}else{};'
              'fill_field(field);return selected.second;') + 'behave @print(result);', 0, '4\n', ''),
    'query_correlated_field_writes_prepare_a_complete_product': (
        IO + QUERY_PAIR + QUERY_PRODUCT_FLAGS +
        query(QUERY_PRODUCT_CHOICE + 'selected.first<-1u64;selected.second<-2u64;'
              'ref complete=selected;return complete.second;') + 'behave @print(result);', 0, '4\n', ''),
    'query_complete_product_still_requires_an_unwritten_sibling': (
        QUERY_PAIR + QUERY_PRODUCT_FLAGS +
        query(QUERY_PRODUCT_CHOICE + 'selected.first<-1u64;ref complete=selected;return complete.first;'),
        2, '', "read of 'second' before definite initialization"),
    'query_complete_product_checks_required_referents_separately': (
        QUERY_PAIR + QUERY_PRODUCT_FLAGS + 'data Box {head:u64;ref required:Pair;}' +
        query('ptr left=make Box;ptr right=make Box;ptr missing=make Pair;left.required=missing;right.required=missing;'
              'ptr selected=if(pick_left){left}else{right};selected.head<-7u64;ref complete=selected;return complete.head;'),
        2, '', "read of 'first' before definite initialization"),
    'query_nested_scope_can_write_through_the_borrow_of_a_selected_field': (
        IO + QUERY_PAIR + QUERY_PRODUCT_FLAGS +
        query(QUERY_PRODUCT_CHOICE + 'ptr field=selected.second;{{field<-7u64;return;}};return selected.second;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_prepared_transfer_can_write_through_the_borrow_of_a_selected_field': (
        IO + QUERY_PAIR + QUERY_PRODUCT_FLAGS +
        query(QUERY_PRODUCT_CHOICE + 'ptr field=selected.second;ptr retained=selected;'
              '{{defer (<-)(field,7u64);selected=right;return;}};return retained.second;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_first_bind_initializes_the_selected_embedded_slot': (
        IO + QUERY_HOLDER + QUERY_PRODUCT_FLAGS +
        query(QUERY_HOLDER_CHOICE + 'chosen.selected=ready;return chosen.selected;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_first_bind_leaves_another_embedded_slot_unproved': (
        QUERY_HOLDER + QUERY_PRODUCT_FLAGS +
        query(QUERY_HOLDER_CHOICE + 'chosen.selected=ready;return right.selected;'),
        2, '', "read of 'selected' before definite initialization"),
    'query_first_bind_does_not_initialize_the_embedded_slots_referent': (
        QUERY_HOLDER + QUERY_PRODUCT_FLAGS +
        query(QUERY_HOLDER_CHOICE + 'chosen.selected=a;return chosen.selected;'),
        2, '', "read of 'a' before definite initialization"),
    'query_selected_embedded_slot_reads_its_new_referent_after_rebind': (
        IO + QUERY_HOLDER + QUERY_PRODUCT_FLAGS +
        query(QUERY_BOUND_HOLDERS + 'chosen.selected=ready;return chosen.selected;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_selected_embedded_slot_can_prepare_its_current_referent': (
        IO + QUERY_HOLDER + QUERY_PRODUCT_FLAGS +
        query(QUERY_BOUND_HOLDERS + 'chosen.selected<-7u64;return chosen.selected;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_overlapping_rebind_invalidates_the_selected_slots_referent': (
        QUERY_HOLDER + QUERY_PRODUCT_FLAGS +
        query(QUERY_BOUND_HOLDERS + 'chosen.selected=ready;right.selected=b;return chosen.selected;'),
        2, '', "read of 'b' before definite initialization"),
    'query_independent_choices_do_not_share_embedded_slot_writes': (
        QUERY_HOLDER + QUERY_PRODUCT_FLAGS +
        query(QUERY_BOUND_HOLDERS + 'ptr other=if(pick_other){left}else{right};'
              'chosen.selected=ready;return other.selected;'), 2, '', 'initialization'),
    'query_rebinding_embedded_slot_preserves_an_earlier_borrow': (
        IO + QUERY_HOLDER + QUERY_PRODUCT_FLAGS +
        query(QUERY_BOUND_HOLDERS + 'ptr saved=chosen.selected;chosen.selected=ready;saved<-7u64;return saved;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_write_through_earlier_borrow_leaves_new_referent_unwritten': (
        QUERY_HOLDER + QUERY_PRODUCT_FLAGS +
        query(QUERY_BOUND_HOLDERS + 'val new_cell<-make local u64;ptr saved=chosen.selected;'
              'chosen.selected=new_cell;saved<-7u64;return chosen.selected;'),
        2, '', "read of 'new_cell' before definite initialization"),
    'query_join_keeps_the_same_selected_embedded_slot': (
        IO + QUERY_HOLDER + QUERY_PRODUCT_FLAGS +
        query(QUERY_HOLDER_CHOICE + 'if(pick_other){chosen.selected=ready;}else{chosen.selected=ready;};'
              'return chosen.selected;') + 'behave @print(result);', 0, '4\n', ''),
    'query_full_scope_publishes_a_first_bind_to_the_selected_slot': (
        IO + QUERY_HOLDER + QUERY_PRODUCT_FLAGS +
        query(QUERY_HOLDER_CHOICE + '{{chosen.selected=ready;return;}};return chosen.selected;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_deferred_bind_keeps_the_selected_embedded_slot_address': (
        IO + QUERY_HOLDER + QUERY_PRODUCT_FLAGS +
        query(QUERY_BOUND_HOLDERS + 'ptr kept=chosen;{{defer (=)(chosen.selected,ready);chosen=right;return;}};'
              'return kept.selected;') + 'behave @print(result);', 0, '4\n', ''),
    'query_deferred_bind_does_not_follow_the_rebound_receiver': (
        QUERY_HOLDER + QUERY_PRODUCT_FLAGS +
        query(QUERY_BOUND_HOLDERS + '{{defer (=)(chosen.selected,ready);chosen=right;return;}};'
              'return chosen.selected;'), 2, '', "read of 'b' before definite initialization"),
    'query_named_helper_reads_the_dynamically_prepared_slot': (
        IO + QUERY_HOLDER + QUERY_PRODUCT_FLAGS + READ_SELECTED_HOLDER +
        query(QUERY_HOLDER_CHOICE + 'chosen.selected=ready;return peek(chosen);') +
        'behave @print(result);', 0, '4\n', ''),
    'query_named_helper_prepares_the_callers_selected_slot': (
        IO + QUERY_HOLDER + QUERY_PRODUCT_FLAGS + REBIND_SELECTED_HOLDER +
        query(QUERY_HOLDER_CHOICE + 'redirect(chosen,ready);return chosen.selected;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_read_only_helper_preserves_an_earlier_borrow_of_the_current_referent': (
        IO + QUERY_HOLDER + QUERY_PRODUCT_FLAGS +
        'proc probe(ptr p:const Holder in O)->void where {const owner O}{'
        'if(p is null){return;}else{};if(p.selected is null){return;}else{};return;}' +
        query(QUERY_BOUND_HOLDERS + 'ptr saved=chosen.selected;probe(chosen);saved<-7u64;return chosen.selected;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_deferred_helper_checks_the_slot_at_invocation': (
        QUERY_HOLDER + QUERY_PRODUCT_FLAGS + READ_SELECTED_HOLDER +
        query(QUERY_HOLDER_CHOICE + 'chosen.selected=ready;'
              '{{defer peek(chosen);chosen.selected=a;return;}};return 0u64;'),
        2, '', "read of 'a' before definite initialization"),
    'query_returned_referent_prepares_the_slots_current_target': (
        IO + QUERY_HOLDER + QUERY_PRODUCT_FLAGS + RETURN_SELECTED_HOLDER +
        query(QUERY_BOUND_HOLDERS + 'ptr held=get_link(chosen);held<-7u64;return chosen.selected;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_returned_referent_of_a_fixed_slot_retains_the_selected_target': (
        IO + QUERY_HOLDER + QUERY_PRODUCT_FLAGS + RETURN_SELECTED_HOLDER +
        query(QUERY_HOLDER_CHOICE + 'ptr initial=if(pick_left){a}else{b};left.selected=initial;'
              'ptr held=get_link(left);held<-7u64;return left.selected;') + 'behave @print(result);',
        0, '4\n', ''),
    'query_returned_referent_can_be_forwarded_by_a_product_helper': (
        IO + QUERY_HOLDER + QUERY_PRODUCT_FLAGS + RETURN_SELECTED_HOLDER +
        'func forward(ptr p:Holder in O)->ptr u64 in O where {owner O}{return get_link(p);}' +
        query(QUERY_BOUND_HOLDERS + 'ptr held=forward(chosen);held<-7u64;return chosen.selected;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_returned_referent_can_be_forwarded_by_a_scalar_helper': (
        IO + QUERY_HOLDER + QUERY_PRODUCT_FLAGS + RETURN_SELECTED_HOLDER +
        'func through(ptr p:u64 in O)->ptr u64 in O where {owner O}{return p;}' +
        query(QUERY_BOUND_HOLDERS + 'ptr held=through(get_link(chosen));held<-7u64;return chosen.selected;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_nullable_scalar_forwarding_does_not_equate_an_explicit_null_with_the_input': (
        QUERY_HOLDER + QUERY_PRODUCT_FLAGS + RETURN_SELECTED_HOLDER +
        'func through(ptr p:u64 in O,flag:bool)->ptr u64 in O where {owner O}{'
        'if(flag){return p;}else{return null in O;};}' +
        query(QUERY_BOUND_HOLDERS + 'ptr held=through(get_link(chosen),pick_other);'
              'if(held is null){return chosen.selected;}else{return 0u64;};'),
        2, '', "read of 'a' before definite initialization"),
    'query_scalar_helper_can_return_the_original_input_after_a_local_rebind': (
        IO + QUERY_HOLDER + QUERY_PRODUCT_FLAGS + RETURN_SELECTED_HOLDER +
        'func through(ptr p:u64 in O,ptr replacement:u64 in O)->ptr u64 in O where {owner O}{'
        'ptr old=p;p=replacement;return old;}' +
        query(QUERY_BOUND_HOLDERS + 'val fresh<-make local u64;ptr held=through(get_link(chosen),fresh);'
              'held<-7u64;return chosen.selected;') + 'behave @print(result);', 0, '4\n', ''),
    'query_scalar_helper_returning_a_local_rebind_does_not_prepare_the_original_input': (
        QUERY_HOLDER + QUERY_PRODUCT_FLAGS + RETURN_SELECTED_HOLDER +
        'func through(ptr p:u64 in O,ptr replacement:u64 in O)->ptr u64 in O where {owner O}{'
        'p=replacement;return p;}' +
        query(QUERY_BOUND_HOLDERS + 'val fresh<-make local u64;ptr held=through(get_link(chosen),fresh);'
              'held<-7u64;return chosen.selected;'), 2, '', "read of 'a' before definite initialization"),
    'query_returned_referent_does_not_follow_a_caller_rebind': (
        QUERY_HOLDER + QUERY_PRODUCT_FLAGS + RETURN_SELECTED_HOLDER +
        query(QUERY_BOUND_HOLDERS + 'val fresh<-make local u64;ptr held=get_link(chosen);'
              'chosen.selected=fresh;held<-7u64;return chosen.selected;'),
        2, '', "read of 'fresh' before definite initialization"),
    'query_returned_referent_can_follow_a_callee_rebind_before_return': (
        IO + QUERY_HOLDER + QUERY_PRODUCT_FLAGS +
        'func get_link(ptr p:Holder in O,ptr replacement:u64 in O)->ptr u64 in O where {owner O}{'
        'if(p is null){return null in O;}else{};p.selected=replacement;return p.selected;}' +
        query(QUERY_BOUND_HOLDERS + 'val fresh<-make local u64;val another<-make local u64;'
              'ptr replacement=if(pick_other){fresh}else{another};ptr held=get_link(chosen,replacement);'
              'held<-7u64;return chosen.selected;') + 'behave @print(result);', 0, '4\n', ''),
    'query_prepared_return_does_not_follow_a_rebind_in_callee_cleanup': (
        QUERY_HOLDER + QUERY_PRODUCT_FLAGS + RETURN_AND_REBIND_HOLDER +
        query(QUERY_BOUND_HOLDERS + 'val fresh<-make local u64;ptr held=get_link(chosen,fresh);'
              'held<-7u64;return chosen.selected;'), 2, '', "read of 'fresh' before definite initialization"),
    'query_prepared_return_keeps_the_earlier_target_after_callee_cleanup': (
        IO + QUERY_HOLDER + QUERY_PRODUCT_FLAGS + RETURN_AND_REBIND_HOLDER +
        query(QUERY_BOUND_HOLDERS + 'val fresh<-make local u64;ptr held=get_link(chosen,fresh);'
              'held<-7u64;return held;') + 'behave @print(result);', 0, '4\n', ''),
    'query_cleanup_can_initialize_the_prepared_result_after_rebinding_its_slot': (
        IO + QUERY_HOLDER + QUERY_PRODUCT_FLAGS +
        'func get_link(ptr p:Holder in O,ptr replacement:u64 in O)->ptr u64 in O where {owner O}{'
        'if(p is null){return null in O;}else{};if(p.selected is null){return null in O;}else{};'
        'defer (<-)(p.selected,7u64);defer (=)(p.selected,replacement);return p.selected;}' +
        query(QUERY_BOUND_HOLDERS + 'val fresh<-make local u64;ptr held=get_link(chosen,fresh);return held;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_returned_referent_shares_nullness_with_the_unchanged_slot': (
        IO + QUERY_HOLDER + QUERY_PRODUCT_FLAGS + RETURN_SELECTED_HOLDER +
        query(QUERY_HOLDER_CHOICE + 'left.selected=ready;right.selected=null in here;ptr held=get_link(chosen);'
              'if(held is null){return 0u64;}else{ref observed=chosen.selected;return observed;};') +
        'behave @print(result);', 0, '4\n', ''),
    'query_explicit_null_return_does_not_establish_nullness_for_the_slot': (
        QUERY_HOLDER + QUERY_PRODUCT_FLAGS +
        'func get_link(ptr p:Holder in O,flag:bool)->ptr u64 in O where {owner O}{'
        'if(p is null){return null in O;}else{};if(flag){return p.selected;}else{return null in O;};}' +
        query(QUERY_BOUND_HOLDERS + 'ptr held=get_link(chosen,pick_other);'
              'if(held is null){return chosen.selected;}else{return 0u64;};'),
        2, '', "read of 'a' before definite initialization"),
    'query_full_scope_publishes_a_write_through_two_embedded_slots': (
        IO + QUERY_HOLDER + QUERY_OUTER + QUERY_PRODUCT_FLAGS +
        query(QUERY_NESTED_HOLDERS + '{{chosen.child.selected<-7u64;return;}};return chosen.child.selected;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_helper_publishes_a_write_through_two_embedded_slots': (
        IO + QUERY_HOLDER + QUERY_OUTER + QUERY_PRODUCT_FLAGS + WRITE_NESTED_HOLDER +
        query(QUERY_NESTED_HOLDERS + 'fill(chosen);return chosen.child.selected;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_scope_preserves_a_write_guarded_by_an_embedded_reference': (
        IO + QUERY_HOLDER + QUERY_OUTER + QUERY_PRODUCT_FLAGS +
        query(QUERY_NULLABLE_CHAIN + '{{if(chosen.child is null){}else{chosen.child.selected<-7u64;{}};return;}};' +
              READ_GUARDED_CHAIN) + 'behave @print(result);', 0, '4\n', ''),
    'query_helper_preserves_a_write_guarded_by_an_embedded_reference': (
        IO + QUERY_HOLDER + QUERY_OUTER + QUERY_PRODUCT_FLAGS + WRITE_NESTED_HOLDER +
        query(QUERY_NULLABLE_CHAIN + 'fill(chosen);' + READ_GUARDED_CHAIN) + 'behave @print(result);', 0, '4\n', ''),
    'query_guarded_write_remains_available_to_the_next_helper': (
        IO + QUERY_HOLDER + QUERY_OUTER + QUERY_PRODUCT_FLAGS + WRITE_NESTED_HOLDER + READ_NESTED_HOLDER +
        query(QUERY_NULLABLE_CHAIN + 'fill(chosen);return peek(chosen);') + 'behave @print(result);', 0, '4\n', ''),
    'query_guard_does_not_prove_a_write_skipped_by_a_present_path': (
        IO + QUERY_HOLDER + QUERY_OUTER +
        'proc fill(ptr p:Outer in O,skip:bool)->void where {owner O}{if(p is null){return;}else{};'
        'if(p.child is null){return;}else{};if(p.child.selected is null){return;}else{};'
        'if(skip){return;}else{};p.child.selected<-7u64;return;}' +
        query_with_flag_inputs(QUERY_NULLABLE_CHAIN + 'fill(chosen,skip);' + READ_GUARDED_CHAIN),
        2, '', "read of 'a' before definite initialization"),
    'query_guard_after_rebinding_does_not_prove_a_write_to_an_earlier_candidate': (
        QUERY_HOLDER + QUERY_OUTER + QUERY_PRODUCT_FLAGS + WRITE_NESTED_HOLDER +
        query(QUERY_NULLABLE_CHAIN + 'fill(chosen);chosen.child=first_child;' + READ_GUARDED_CHAIN),
        2, '', "read of 'a' before definite initialization"),
    'query_scope_preserves_a_guarded_write_to_either_selected_referent': (
        IO + QUERY_HOLDER + QUERY_OUTER +
        query_with_flag_inputs(QUERY_NULLABLE_CHAIN_CHOICES +
                               '{{if(chosen.child is null){}else{chosen.child.selected<-7u64;{}};return;}};' +
                               READ_GUARDED_CHAIN), 0, '4\n', ''),
    'query_guarded_write_to_either_selected_referent_reaches_the_next_helper': (
        IO + QUERY_HOLDER + QUERY_OUTER + WRITE_NESTED_HOLDER + READ_NESTED_HOLDER +
        query_with_flag_inputs(QUERY_NULLABLE_CHAIN_CHOICES + 'fill(chosen);return peek(chosen);'), 0, '4\n', ''),
    'query_prepared_helper_preserves_guarded_readiness_after_cleanup': (
        IO + QUERY_HOLDER + QUERY_OUTER + WRITE_NESTED_HOLDER +
        query_with_flag_inputs(QUERY_NULLABLE_CHAIN_CHOICES + '{{defer fill(chosen);return;}};' + READ_GUARDED_CHAIN),
        0, '4\n', ''),
    'query_dynamic_guard_does_not_hide_an_unwritten_nonnull_path': (
        IO + QUERY_HOLDER + QUERY_OUTER +
        query_with_flag_inputs(QUERY_NULLABLE_CHAIN_CHOICES + 'if(chosen.child is null){}else{'
                               'if(skip){}else{chosen.child.selected<-7u64;{}};{}};' + READ_GUARDED_CHAIN),
        2, '', "read of 'a' before definite initialization"),
    'query_dynamic_guard_does_not_prove_the_rebound_referent_ready': (
        IO + QUERY_HOLDER + QUERY_OUTER + WRITE_NESTED_HOLDER +
        query_with_flag_inputs(QUERY_NULLABLE_CHAIN_CHOICES + 'fill(chosen);' + QUERY_NESTED_REPLACEMENT +
                               'chosen.child=replacement;' + READ_GUARDED_CHAIN),
        2, '', "read of 'fresh' before definite initialization"),
    'query_dynamic_guard_preserves_readiness_of_a_nested_inline_field': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + WRITE_GUARDED_INLINE + READ_GUARDED_INLINE), 0, '4\n', ''),
    'query_scope_carries_guarded_inline_readiness_to_the_caller': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + '{{' + WRITE_GUARDED_INLINE + 'return;}};' +
                               READ_GUARDED_INLINE), 0, '4\n', ''),
    'query_guarded_inline_write_reaches_a_second_helper_after_cleanup': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER + WRITE_INLINE_HOLDER + READ_INLINE_HOLDER +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + '{{defer fill(chosen);return;}};return peek(chosen);'),
        0, '4\n', ''),
    'query_helper_proves_inline_write_with_prepared_boolean_constant': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER + WRITE_INLINE_WHEN_ENABLED +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + 'fill(chosen,false);' + READ_GUARDED_INLINE),
        0, '4\n', ''),
    'query_prepared_boolean_guards_write_to_selected_inline_field': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER + WRITE_INLINE_WHEN_ENABLED +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + 'fill(chosen,skip);if(skip){return 0u64;}else{};' +
                               READ_GUARDED_INLINE), 0, '4\n', ''),
    'query_reader_can_test_the_prepared_boolean_it_receives': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER + WRITE_INLINE_WHEN_ENABLED + READ_INLINE_WHEN_ENABLED +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + 'fill(chosen,skip);return peek(chosen,skip);'), 0, '4\n', ''),
    'query_reader_with_independent_boolean_cannot_use_writer_guard': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER + WRITE_INLINE_WHEN_ENABLED + READ_INLINE_WHEN_ENABLED +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + 'fill(chosen,skip);return peek(chosen,pick_left);'),
        2, '', "read of 'leaf' before definite initialization"),
    'query_reader_can_use_compound_prepared_boolean': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER + WRITE_INLINE_WITH_TWO_FLAGS + READ_INLINE_WHEN_ENABLED +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + 'fill(chosen,skip,pick_left);return peek(chosen,skip && pick_left);'),
        0, '4\n', ''),
    'query_reader_guard_must_cover_every_path_on_which_writer_skipped': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER + WRITE_INLINE_WHEN_ENABLED + READ_INLINE_WHEN_ENABLED +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + 'fill(chosen,skip);return peek(chosen,skip && pick_left);'),
        2, '', "read of 'leaf' before definite initialization"),
    'query_reader_can_forward_guard_from_a_deferred_writer': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER + WRITE_INLINE_WHEN_ENABLED + READ_INLINE_WHEN_ENABLED +
        'proc relay(ptr p:Outer in O,skip:bool)->u64 where {owner O}{return peek(p,skip);}' +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + 'val flag<-skip;val saved<-flag;'
                               '{{defer fill(chosen,flag);flag<-false;return;}};return relay(chosen,saved);'),
        0, '4\n', ''),
    'query_reader_cannot_use_guard_after_overwriting_its_formal': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER + WRITE_INLINE_WHEN_ENABLED +
        READ_INLINE_WHEN_ENABLED.replace('if(skip)', 'skip<-false;if(skip)') +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + 'fill(chosen,skip);return peek(chosen,skip);'),
        2, '', "read of 'leaf' before definite initialization"),
    'query_two_prepared_booleans_preserve_combined_write_condition': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER + WRITE_INLINE_WITH_TWO_FLAGS +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + 'fill(chosen,skip,pick_left);'
                               'if(skip && pick_left){return 0u64;}else{};' + READ_GUARDED_INLINE), 0, '4\n', ''),
    'query_unrelated_boolean_cannot_discharge_combined_write_condition': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER + WRITE_INLINE_WITH_TWO_FLAGS +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + 'fill(chosen,skip,pick_left);'
                               'if(pick_other){return 0u64;}else{};' + READ_GUARDED_INLINE),
        2, '', "read of 'leaf' before definite initialization"),
    'query_deferred_helper_preserves_prepared_boolean_after_source_write': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER + WRITE_INLINE_WHEN_ENABLED +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + 'val argument<-skip;val saved<-argument;'
                               '{{defer fill(chosen,argument);argument<-false;return;}};'
                               'if(saved){return 0u64;}else{};' + READ_GUARDED_INLINE), 0, '4\n', ''),
    'query_boolean_operand_is_prepared_before_later_operand_writes_its_source': (
        IO + QUERY_HOLDER + QUERY_OUTER + WRITE_REFERENT_WHEN_ENABLED +
        query_with_flag_inputs(QUERY_NULLABLE_CHAIN_CHOICES + 'val argument<-skip;val saved<-argument;'
                               'fill(chosen,argument,{{argument<-false;return true;}});'
                               'if(saved){return 0u64;}else{};' + READ_GUARDED_CHAIN), 0, '4\n', ''),
    'query_rewritten_boolean_does_not_prove_earlier_helper_write': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER + WRITE_INLINE_WHEN_ENABLED +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + 'val argument<-skip;fill(chosen,argument);'
                               'argument<-false;if(argument){return 0u64;}else{};' + READ_GUARDED_INLINE),
        2, '', "read of 'leaf' before definite initialization"),
    'query_prepared_assignment_retains_the_guarded_inline_destination': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS +
                               '{{if(chosen.child is null){}else{defer (<-)(chosen.child.payload.leaf,7u64);{}};return;}};' +
                               READ_GUARDED_INLINE), 0, '4\n', ''),
    'query_prepared_inline_assignment_leaves_a_rebound_destination_unwritten': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + 'ptr replacement=make Holder;'
                               '{{if(chosen.child is null){}else{defer (<-)(chosen.child.payload.leaf,7u64);{}};'
                               'chosen.child=replacement;return;}};' + READ_GUARDED_INLINE),
        2, '', "read of 'leaf' before definite initialization"),
    'query_saved_receiver_keeps_guarded_cleanup_readiness_after_rebinding': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + 'ptr saved=chosen.child;ptr replacement=make Holder;'
                               '{{if(chosen.child is null){}else{defer (<-)(chosen.child.payload.leaf,7u64);{}};'
                               'chosen.child=replacement;return;}};'
                               'if(saved is null){return 0u64;}else{return saved.payload.leaf;};'), 0, '4\n', ''),
    'query_inline_field_must_be_written_on_each_present_path': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + 'if(chosen.child is null){}else{'
                               'if(skip){}else{chosen.child.payload.leaf<-7u64;{}};{}};' + READ_GUARDED_INLINE),
        2, '', "read of 'leaf' before definite initialization"),
    'query_guarded_inline_write_leaves_sibling_unwritten': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER + WRITE_INLINE_HOLDER +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + 'fill(chosen);'
                               'if(chosen.child is null){return 0u64;}else{return chosen.child.payload.other;};'),
        2, '', "read of 'other' before definite initialization"),
    'query_guarded_inline_write_does_not_follow_a_rebind_to_a_previous_candidate': (
        IO + QUERY_INLINE_HOLDER + QUERY_OUTER + WRITE_INLINE_HOLDER +
        query_with_flag_inputs(QUERY_INLINE_RECEIVERS + 'fill(chosen);chosen.child=first_child;' + READ_GUARDED_INLINE),
        2, '', "read of 'leaf' before definite initialization"),
    'query_helper_write_through_embedded_slots_leaves_another_chain_unwritten': (
        QUERY_HOLDER + QUERY_OUTER + QUERY_PRODUCT_FLAGS + WRITE_NESTED_HOLDER +
        query(QUERY_NESTED_HOLDERS + 'fill(chosen);return right.child.selected;'),
        2, '', "read of 'b' before definite initialization"),
    'query_helper_can_prepare_a_chain_starting_at_a_fixed_embedded_slot': (
        IO + QUERY_HOLDER + QUERY_OUTER + QUERY_PRODUCT_FLAGS + WRITE_NESTED_HOLDER +
        query(QUERY_NESTED_HOLDERS + 'left.child=if(pick_other){first_child}else{second_child};'
              'fill(left);return left.child.selected;') + 'behave @print(result);', 0, '4\n', ''),
    'query_helper_reconstructs_the_current_chain_after_an_intermediate_rebind': (
        IO + QUERY_HOLDER + QUERY_OUTER + QUERY_PRODUCT_FLAGS +
        'proc fill(ptr p:Outer in O,ptr incoming:Holder in O)->void where {owner O}{'
        'if(p is null){return;}else{};p.child=incoming;if(p.child is null){return;}else{};'
        'if(p.child.selected is null){return;}else{};p.child.selected<-7u64;return;}' +
        query(QUERY_NESTED_HOLDERS + 'ptr third=make Holder;ptr fourth=make Holder;val c<-make local u64;'
              'val d<-make local u64;third.selected=c;fourth.selected=d;'
              'ptr incoming=if(pick_other){third}else{fourth};fill(chosen,incoming);'
              'return chosen.child.selected;') + 'behave @print(result);', 0, '4\n', ''),
    'query_deferred_write_stays_with_the_earlier_intermediate_reference': (
        IO + QUERY_HOLDER + QUERY_OUTER + QUERY_PRODUCT_FLAGS +
        query(QUERY_NESTED_HOLDERS + QUERY_NESTED_REPLACEMENT + 'ptr saved=chosen.child;'
              '{{defer (<-)(chosen.child.selected,7u64);chosen.child=replacement;return;}};return saved.selected;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_deferred_write_leaves_the_rebound_intermediate_unwritten': (
        QUERY_HOLDER + QUERY_OUTER + QUERY_PRODUCT_FLAGS +
        query(QUERY_NESTED_HOLDERS + QUERY_NESTED_REPLACEMENT + 'ptr saved=chosen.child;'
              '{{defer (<-)(chosen.child.selected,7u64);chosen.child=replacement;return;}};'
              'return chosen.child.selected;'), 2, '', "read of 'fresh' before definite initialization"),
    'query_returned_referent_can_prepare_the_current_embedded_chain': (
        IO + QUERY_HOLDER + QUERY_OUTER + QUERY_PRODUCT_FLAGS + RETURN_NESTED_HOLDER +
        query(QUERY_NESTED_HOLDERS + 'ptr held=deep(chosen);held<-7u64;return chosen.child.selected;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_returned_referent_does_not_follow_an_intermediate_rebind': (
        QUERY_HOLDER + QUERY_OUTER + QUERY_PRODUCT_FLAGS + RETURN_NESTED_HOLDER +
        query(QUERY_NESTED_HOLDERS + QUERY_NESTED_REPLACEMENT + 'ptr held=deep(chosen);'
              'chosen.child=replacement;held<-7u64;return chosen.child.selected;'),
        2, '', "read of 'fresh' before definite initialization"),
    'query_returned_direct_field_can_be_reached_through_an_embedded_load': (
        IO + 'data Holder {leaf:u64;}' + QUERY_OUTER + QUERY_PRODUCT_FLAGS +
        'func deep(ptr p:Outer in O)->ptr u64 in O where {owner O}{'
        'if(p is null){return null in O;}else{};if(p.child is null){return null in O;}else{};return p.child.leaf;}' +
        query(QUERY_OUTER_CHOICE + 'ptr held=deep(chosen);held<-7u64;return chosen.child.leaf;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_returned_direct_field_keeps_its_receiver_when_cleanup_rebinds': (
        'data Holder {leaf:u64;}' + QUERY_OUTER + QUERY_PRODUCT_FLAGS +
        'func deep(ptr p:Outer in O,ptr incoming:Holder in O)->ptr u64 in O where {owner O}{'
        'if(p is null){return null in O;}else{};if(p.child is null){return null in O;}else{};'
        'defer (=)(p.child,incoming);return p.child.leaf;}' +
        query(QUERY_OUTER_CHOICE + 'ptr replacement=make Holder;ptr held=deep(chosen,replacement);'
              'held<-7u64;return chosen.child.leaf;'), 2, '', "read of 'leaf' before definite initialization"),
    'query_helper_publishes_a_write_through_three_embedded_slots': (
        IO + QUERY_HOLDER + QUERY_OUTER + 'data Top {ptr outer:Outer;}' + QUERY_PRODUCT_FLAGS +
        'proc fill(ptr p:Top in O)->void where {owner O}{if(p is null){return;}else{};'
        'if(p.outer is null){return;}else{};if(p.outer.child is null){return;}else{};'
        'if(p.outer.child.selected is null){return;}else{};p.outer.child.selected<-7u64;return;}' +
        query(QUERY_NESTED_HOLDERS + 'ptr first_top=make Top;ptr second_top=make Top;first_top.outer=left;'
              'second_top.outer=right;ptr top=if(pick_left){first_top}else{second_top};fill(top);'
              'return top.outer.child.selected;') + 'behave @print(result);', 0, '4\n', ''),
    'query_recursive_helper_preserves_field_writes': (
        IO + QUERY_PAIR + 'proc populate(ptr p:Pair in O,n:u64)->void where {owner O}{'
        'if(p is null){return;}else{};if(n==0u64){p.second<-7u64;return;}else{};populate(p,n-1u64);return;}' +
        query('ptr item=make Pair;populate(item,3u64);return item.second;') + 'behave @print(result);', 0, '4\n', ''),
    'query_helper_rebinds_a_slot_without_initializing_its_new_referent': (
        QUERY_HOLDER + 'proc redirect(ref p:Holder in O,ptr source:u64 in O)->void where {owner O}{p.selected=source;return;}' +
        query('val old<-7u64;val missing<-make local u64;ref item=make Holder {selected=old;};'
              'redirect(item,missing);return item.selected;'), 2, '', "read of 'missing' before definite initialization"),
    'query_helper_publishes_a_write_before_checked_failure': (
        IO + QUERY_PAIR + QUERY_CALL_FAILURE + 'p.second<-7u64;ptr absent:u64=null in here;absent;return;}' +
        query(READ_QUERY_AFTER_FAILURE) + 'behave @print(result);', 0, '4\n', ''),
    'query_helper_failure_omits_a_later_write': (
        QUERY_PAIR + QUERY_CALL_FAILURE + 'ptr absent:u64=null in here;absent;p.second<-7u64;return;}' +
        query(READ_QUERY_AFTER_FAILURE), 2, '', "read of 'second' before definite initialization"),
    'query_prepared_helper_keeps_the_saved_product_after_a_rebind': (
        IO + QUERY_PAIR + WRITE_QUERY_PAIR +
        query('ptr first=make Pair;ptr second=make Pair;ptr selected=first;'
              '{{defer populate(selected);selected=second;return;}};return first.second;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_prepared_helper_uses_the_embedded_slot_at_invocation': (
        IO + QUERY_HOLDER + WRITE_QUERY_HOLDER +
        query('val target<-make local u64;ref item=make Holder {selected=null;};'
              '{{defer populate(item);item.selected=target;return;}};return target;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_prepared_helper_rejects_a_late_unwritten_referent': (
        QUERY_HOLDER + 'func inspect(ref p:const Holder in O)->u64 where {const owner O}{'
        'if(p.selected is null){return 0u64;}else{};return p.selected;}' +
        query('val ready<-7u64;val target<-make local u64;ref item=make Holder {selected=ready;};'
              '{{defer inspect(item);item.selected=target;return;}};return ready;'),
        2, '', "read of 'target' before definite initialization"),
    'query_prepared_helper_registration_failure_keeps_the_field_unwritten': (
        QUERY_PAIR + 'using "std:exceptions" OutOfMemoryError;' + WRITE_QUERY_PAIR +
        query('ptr item=make Pair;try{ {{defer populate(item);}}; }catch(e:const OutOfMemoryError){return item.second;};'
              'return item.second;'), 2, '', "read of 'second' before definite initialization"),
    'query_helper_return_preserves_a_borrowed_field': (
        IO + QUERY_PAIR + 'func select_second(ptr p:Pair in O)->ptr u64 in O where {owner O}{'
        'if(p is null){return null in O;}else{};return p.second;}' +
        query('ptr item=make Pair;ptr target=select_second(item);target<-7u64;return item.second;') +
        'behave @print(result);', 0, '4\n', ''),
    'query_product_helper_without_a_body_reports_unproved_coverage': (
        QUERY_PAIR + 'func inspect(ptr p:const Pair in O)->u64 where {const owner O};' +
        query('ptr item=make Pair;return inspect(item);'), 2, '', 'no proved body'),
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
