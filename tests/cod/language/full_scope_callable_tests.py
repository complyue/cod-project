#!/usr/bin/env python3
"""Full scopes share ordinary value inputs, inference and local completion."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
BASE = 'func one()->const u64{return 1u64;}func two()->const u64{return 2u64;}'
IDENTITY = 'func identity(x:T)->const T where T{return x;}'
APPLY = 'func apply(f:func(T)->const T,x:T)->const T where T{return f(x);}'
FIXED = 'func fixed(f:func(u64)->const u64,x:u64)->const u64{return f(x);}'
GUARDED_QUERY = ('using "std:exceptions" NullPointerException;val e<-make local NullPointerException;')
GUARDED_TRY = ('try{if(true){throw e;}else{};val n<-4294967296u64;{}}'
               'catch(c:const NullPointerException){return 7u32;}')
FINALLY_READ = 'finally{late;{}};return late;'
QUERY_EVIDENCE = 'typeof requires a known declaration or operand type'


def query(body):
    return 'val result<-make local typeof({{' + body + '}}){4u64};'


runner.CASES = {
    'supplied_scope_forwarding_preserves_live_slot': (IO+
        'func relay(here body(n:u64)->u64,n:u64)->u64{return body(n);}'
        'func outer(here body(n:u64)->u64,n:u64)->u64{return relay(body,n);}'
        'proc run()->void{val base<-20u64;behave @print(outer(7u64)(n)=>{{return base+n;}});}run();',0,'27\n',''),
    'supplied_scope_per_invocation_cleanup': (IO+
        'func twice(here body(n:u64)->void)->void{body(2u64);body(7u64);}'
        'proc run()->void{val total<-1u64;twice()(n)=>{{defer{total<-total+10u64;};total<-total+n;}};'
        'behave @print(total);}run();',0,'30\n',''),
    'supplied_scope_distinct_recursive_activations': (IO+
        'func once(here body()->void)->void{body();}'
        'proc visit(depth:u64)->void{val n<-depth;once(){{if(n>0u64){visit(n-1u64);}else{};behave @print(n);}};}visit(3u64);',
        0,'0\n1\n2\n3\n',''),
    'supplied_scope_named_operands_evaluate_in_written_order': (IO+
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'func ordered(here body(n:u64)->u64,left:u64,right:u64)->u64{return body(left*10u64+right);}'
        'proc run()->void{behave @print(ordered(mark(2u64)->right,mark(1u64)->left)(n)=>{{behave @print(9u64);return n;}});}run();',
        0,'2\n1\n9\n12\n',''),
    'supplied_scope_checked_failure_returns_to_receiver': (IO+'using "std:exceptions" NullPointerException;'+
        'func retry(here body()->u64 throws NullPointerException)->u64{'
        'try{return body();}catch(e:const NullPointerException){return 51u64;};return 0u64;}'
        'proc run()->void{behave @print(retry(){{ptr x:u64=null in here;return x;}});}run();',0,'51\n',''),
    'supplied_scope_cannot_use_lexical_here_as_scalar_operand': (
        'func body(here step(n:u64)->u64)->u64{return step(1u64);}'
        'func invoke(here body(n:u64)->u64)->u64{return body(body);}',2,'','a here argument can only be invoked or forwarded'),
    'live_scalar_slot': (IO+'proc run()->u64{val n<-1u64;{{n<-2u64;}};return n;}behave @print(run());', 0, '2\n', ''),
    'local_return': (IO+'func run()->u64{ {{return 1u64;}};return 2u64;}behave @print(run());', 0, '2\n', ''),
    'explicit_result': (IO+'func run()->u64{return {{return 7u64;}};}behave @print(run());', 0, '7\n', ''),
    'independent_inference': (IO+'func run(){ {{return true;}};return 8u64;}behave @print(run());', 0, '8\n', ''),
    'tail_is_void': ('func run()->u64{return {{7u64;}};}run();', 2, '', 'return type'),
    'inferred_void': (IO+'func run(){ {{return 7u64;}};true;}run();behave @print(9u64);', 0, '9\n', ''),
    'void_fallthrough': (IO+'proc run()->void{ {{behave @print(1u64);}};behave @print(2u64);}run();', 0, '1\n2\n', ''),
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
    'delayed_capture': (IO+'proc run()->void{val n<-1u64;defer{{behave @print(n);}};n<-2u64;}run();', 0, '2\n', ''),
    'delayed_capture_grouped': (IO+'proc run()->void{val n<-1u64;defer{ {{behave @print(n);}};};n<-2u64;}run();', 0, '2\n', ''),
    'nested_cleanup': (IO+'proc run()->void{defer{behave @print(4u64);};{{defer{behave @print(3u64);};{{defer{behave @print(2u64);};behave @print(1u64);}};}};}run();', 0, '1\n2\n3\n4\n', ''),
    'argument_order': (IO+'func sum(val a:u64,val b:u64)->u64{return a+b;}proc run()->void{val n<-1u64;behave @print(sum({{n<-2u64;return n;}},n));behave @print(n);}run();', 0, '4\n2\n', ''),
    'generic_value_inputs': (IO+'func run(val x:T)->const T where T{return {{x<-x+x;return x;}};}behave @print(run(3u64));behave @print(run(4));behave @print(run(2f32));', 0, '6\n8\n4\n', ''),
    'scope_callable_argument_from_later_operand': (IO + IDENTITY + APPLY +
        'func run(x:T) where T{return apply({{return identity;}},x);}'
        'behave @print(run(7u64));behave @print(run(true));', 0, '7\ntrue\n', ''),
    'scope_callable_argument_from_view': (IO + IDENTITY + FIXED +
        'func invoke=fixed;behave @print(invoke({{return identity;}},9u64));', 0, '9\n', ''),
    'nested_scope_callable_argument': (IO + IDENTITY + FIXED +
        'behave @print(fixed({{return {{return identity;}};}},13u64));', 0, '13\n', ''),
    'scope_callable_argument_cleanup_order': (IO + IDENTITY + FIXED +
        'proc observe(n:u64)->void{behave @print(n);}'
        'behave @print(fixed({{defer observe(2u64);observe(1u64);return identity;}},{observe(3u64);17u64}));',
        0, '1\n2\n3\n17\n', ''),
    'scope_callable_argument_cannot_guess_overload': (IDENTITY +
        'func choose(f:func(u64)->const u64)->u64{return f(7u64);}'
        'func choose(f:func(bool)->const bool)->u64{return 8u64;}'
        'choose({{return identity;}});', 2, '', 'unproved callable result'),
    'scope_cannot_use_callable_receiver': (IDENTITY +
        'func result:func(u64)->const u64={{return identity;}};', 2, '', 'unproved callable result'),
    'live_function_holder': (IO+BASE+'proc run()->void{func saved=one;{{saved<-two;behave @print(saved());}};behave @print(saved());}run();', 0, '2\n2\n', ''),
    'nested_live_slot': (IO+'proc run()->u64{val n<-1u64;{{{{n<-6u64;}};n<-n+1u64;}};return n;}behave @print(run());', 0, '7\n', ''),
    'prepared_function_keeps_its_selected_value': (IO+
        'func one(ref n:u64 in O)->void where {owner O}{n<-1u64;}'
        'func two(ref n:u64 in O)->void where {owner O}{n<-2u64;}'
        'proc run()->void{val n<-0u64;func saved=one;defer{behave @print(n);};defer saved(n);'
        '{{saved<-two;}};saved(n);behave @print(n);}run();', 0, '2\n1\n', ''),
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
    'query_has_no_effects': (IO+'proc run()->void{val n<-make local u64;val result<-make local typeof({{behave @print(n);return 7u64;}}){8u64};behave @print(result);}run();', 0, '8\n', ''),
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
    'query_tail_is_void': ('val result<-make local typeof({{7u64;}}){8u64};', 2, '', 'initializer does not satisfy local storage type'),
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
    'reference_input_writes_target': (IO+'proc run()->void{ref n:u64=1u64;{{n<-2u64;}};behave @print(n);}run();', 0, '2\n', ''),
    'declared_place_writes_supplying_value_slot': (IO+'proc run()->u64{val n<-1u64;var p->n;{{p<-2u64;}};return n;}behave @print(run());',
        0, '2\n', ''),
    'missing_return_after_inner_completion': (IO+'using "std:exceptions" MissingReturnError;proc run()->u64{ {{return 1u64;}};}try{run();{}}catch(e:const MissingReturnError){behave @print(5u64);};', 0, '5\n', '[-Wmissing-return]'),
    'inner_missing_return': (IO+'using "std:exceptions" MissingReturnError;proc run(val flag:bool)->void{try{val n<-{{if(flag){return 1u64;}else{};}};behave @print(n);}catch(e:const MissingReturnError){behave @print(5u64);};}run(false);run(true);', 0, '5\n1\n', '[-Wmissing-return]'),
    'live_slot_keeps_its_supplying_owner': (IO+'func choose(ref a:u64 in O,ref b:u64 in O)->const u64 where {owner O}{return 7u64;}func choose(ref a:u64 in O,ref b:u64 in P)->const bool where {owner O,owner P}{return true;}func run(){val source<-1u64;return {{val local<-2u64;return choose(source,local);}};}behave @print(run());', 0, 'true\n', ''),
    'outer_catch_is_not_ambient': ('using "std:exceptions" MissingReturnError;func missing()->const u64{}proc run()->void{try{missing();{}}catch(e:const MissingReturnError){ {{if(true){throw;}else{return 1u64;};}};{}};}run();', 2, '', 'bare throw requires an active catch'),
    'unchecked_failure_crosses_call': (IO+'using "std:exceptions" MissingReturnError;func missing()->const u64{}proc run()->void{defer{behave @print(4u64);};try{ {{defer{behave @print(2u64);};behave @print(1u64);missing();}};}catch(e:const MissingReturnError){behave @print(3u64);};}run();', 0, '1\n2\n3\n4\n', '[-Wmissing-return]'),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
