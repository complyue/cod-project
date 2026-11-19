#!/usr/bin/env python3
"""Immediate scope bodies use live reference slots and ordinary target owners."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
SELECT = (
    'func select(ref x:u64 in O,ref y:u64 in O)->u64 where {owner O}{return 1u64;}'
    'func select(ref x:u64 in P,ref y:u64 in Q)->bool where {owner P,owner Q}{return true;}'
)
VALUES = ('func one()->u64{return 1u64;}func two()->u64{return 2u64;}'
          'func wrap(val source:func()->u64)->func()->u64{return func[source]()->u64{return source();};}')
CHECKED = NPE + ('func error()->const NullPointerException{ptr nil:u64=null in here;'
                 'try{nil;{}}catch(e:const NullPointerException){return e;};return error();}')
SLOT_METHOD = 'proc (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void'
SLOT_BODY = ' where {owner O}{if(a is not null){ {{b=null;behave @print(a);}}; }else{};return;}'
SLOT_CALLS = ('val x<-1,y<-7;ptr p=x,q=y;'
              'try{p<=>p;}catch(e:const NullPointerException){behave @print(8);};behave @print(p is null);'
              'p=x;try{p<=>q;}catch(e:const NullPointerException){behave @print(9);};'
              'behave @print(p is null);behave @print(q is null);')


def query(body):
    return 'val queried<-make local typeof({{' + body + '}}){4u64};'


def saved_scope(body, before=''):
    return (IO+'func inspect(ptr input:u64 in O)->u64 where {owner O}{const val present<-(input is not null);'+
            before+'return {{'+body+'}};}'+'ref item:u64=5u64;ptr missing:u64=null in here;'
            'behave @print(inspect(item));behave @print(inspect(missing));')


def borrowed_test_scope(body, before='', declaration='ref guard:const bool=observed;'):
    return (IO+'func inspect(ptr input:u64 in O)->u64 where {owner O}{val observed<-(input is not null);'+
            declaration+before+'return {{'+body+'}};}'+'ref item:u64=5u64;ptr missing:u64=null in here;'
            'behave @print(inspect(item));behave @print(inspect(missing));')


def conditional_scope(body, before='ptr selected:u64 in O=if(flag){input}else{null in O};'):
    return (IO+'func inspect(ptr input:u64 in O,flag:bool)->u64 where {owner O}{'
            +before+'return {{'+body+'}};}'
            'ref item:u64=5u64;ptr missing:u64=null in here;'
            'behave @print(inspect(item,true));behave @print(inspect(missing,true));behave @print(inspect(item,false));')


runner.CASES = {
    'live_slot_and_deferred_write': (IO+'ref a:u64=1u64;ref b:u64=10u64;ref p:u64=a;'
        '{{p=b;defer{p<-20u64;};}};behave @print(p is b);behave @print(a+b);', 0, 'true\n21\n', ''),
    'nullable_slot_can_rebind': (IO+'ptr p:u64=null in here;ref b:u64=10u64;'
        '{{behave @print(p is null);p=b;behave @print(p);p=null:void;}};behave @print(p is null);', 0, 'true\n10\ntrue\n', ''),
    'nonnull_ptr_can_rebind_to_null': (IO+'ref a:u64=2u64;ptr p:u64=a;'
        '{{behave @print(p);p=null;behave @print(p is null);}};behave @print(p is null);', 0, '2\ntrue\ntrue\n', ''),
    'nullable_read_infers_checked_failure': (NPE+'ptr p:u64=null in here;{{return p;}};', 2, '',
        'checked exception'),
    'nullable_read_can_be_handled': (IO+NPE+'ptr p:u64=null in here;'
        'try{ {{return p;}};{} }catch(e:const NullPointerException){behave @print(e.category);};', 0, '4\n', ''),
    'capture_identity_without_payload_read': (IO+'val source<-make local u64;ptr p:u64=source;'
        'behave @print({{return p is p;}});', 0, 'true\n', ''),
    'ptr_write_initializes_outer_target': (IO+'val source<-make local u64;ptr p:u64=source;'
        '{{p<-8u64;}};behave @print(source);', 0, '8\n', ''),
    'ptr_read_requires_payload_initialization': ('val source<-make local u64;ptr p:u64=source;{{return p;}};', 2, '',
        'initialization'),
    'same_target_inputs_observe_writes': (IO+'ref a:u64=1u64;ref p:u64=a;ref q:u64=a;'
        '{{behave @print(p is q);p<-7u64;behave @print(q);}};behave @print(a);', 0, 'true\n7\n7\n', ''),
    'same_actual_owner_in_forwarded_call': (IO+SELECT+'ref a:u64=1u64;ref b:u64=2u64;'
        'behave @print({{return select(a,b);}});', 0, '1\n', ''),
    'distinct_actual_owners_in_forwarded_call': (IO+SELECT+'proc show(ref outer:u64 in O)->void where {owner O}{'
        'ref local:u64=1u64;behave @print({{return select(outer,local);}});return;}'
        'ref a:u64=1u64;show(a);', 0, 'true\n', ''),
    'named_owner_keeps_same_role': (IO+'proc show(ref outer:u64 in O)->void where {owner O}{'
        ' {{ptr p:u64 in O=null in O;p=outer;p<-9u64;}};return;}ref a:u64=1u64;show(a);behave @print(a);', 0, '9\n', ''),
    'named_owner_without_captured_target': (IO+'proc show(ref unused:u64 in O)->void where {owner O}{'
        ' {{ptr p:u64 in O=null in O;behave @print(p is null);}};return;}ref a:u64=1u64;show(a);', 0, 'true\n', ''),
    'fixed_slot_preserves_writable_target': (IO+'ref a:u64=1u64;const ptr p:u64=a;{{p<-5u64;}};behave @print(a);', 0, '5\n', ''),
    'fixed_slot_rebind_rejected': ('ref a:u64=1u64;const ptr p:u64=a;{{p=null;}};', 2, '', 'cannot rebind a const reference binding'),
    'readonly_target_write_rejected': ('ref a:const u64=1u64;{{a<-2u64;}};', 2, '', 'readonly'),
    'invocation_here_cannot_replace_outer_owner': ('ref a:u64=1u64;ptr p:u64=a;'
        '{{ptr local:u64=null in here;p=local;}};', 2, '', 'initializer-fixed owner'),
    'readonly_query_rejects_borrow_write': ('ref a:const u64=1u64;val result<-make local typeof({{a<-2u64;return 1u64;}}){4u64};',
        2, '', 'readonly'),
    'query_checks_value_initializer': (query('val local:u64<-true;return 7u64;'),
        2, '', 'initializer does not satisfy declared val type'),
    'query_checks_local_storage_initializer': (query('val local<-make local u64{true};return 7u64;'),
        2, '', 'initializer does not satisfy local storage type'),
    'query_checks_reference_initializer_type': ('ref source:u8=1u8;' + query('ref local:u64=source;return 7u64;'),
        2, '', 'initializer does not satisfy declared ref type'),
    'query_checks_reference_initializer_access': ('ref source:const u64=1u64;' +
        query('ref local:u64=source;return 7u64;'), 2, '', 'readonly source cannot initialize a writable reference target'),
    'query_checks_reference_initializer_owner': ('func run(ref outer:u64 in O)->u64 where {owner O}{' +
        query('val local<-1u64;ptr selected:u64 in O=local;return 7u64;') +
        'return queried;}ref actual:u64=1u64;run(actual);', 2, '', 'reference initializer does not belong to the declared owner'),
    'query_rejects_untyped_null_initializer': (query('ptr local=null in here;return 7u64;'),
        2, '', 'a null initializer requires an explicit target type'),
    'query_rejects_uninitialized_fresh_ref': (query('ref local:u64=make local u64;return 7u64;'),
        2, '', 'ref requires definitely initialized target storage'),
    'query_checks_init_without_execution': (IO +
        query('val local:u64<-make local u64{behave @print(999u64);1u8};return local;') +
        'behave @print(queried);', 0, '4\n', ''),
    'query_keeps_local_return_in_initializer': (IO + query('val local<-make local u64{return 7u64;};') +
        'behave @print(queried);', 0, '4\n', ''),
    'query_accepts_existing_declared_owner': (IO + 'func run(ref outer:u64 in O)->u64 where {owner O}{' +
        query('ptr selected:u64 in O=outer;return 7u64;') +
        'return queried;}ref actual:u64=1u64;behave @print(run(actual));', 0, '4\n', ''),
    'query_preserves_continuing_borrow_owner': (IO + 'func inspect(ref a:const u64 in O,ref b:const u64 in O)->u64'
        ' where {owner O}{return b;}func run(ref external:const u64 in E,flag:bool)->u64 where {owner E}{' +
        query('if(flag){return 1u32;}else{ref selected:const u64 in E=external;};return inspect(external,selected);') +
        'return queried;}ref actual:const u64=1u64;behave @print(run(actual,false));', 0, '4\n', ''),
    'query_preserves_continuing_borrow_access': ('ref source:const u64=1u64;' +
        query('if(false){return 1u64;}else{ref selected:const u64=source;};selected<-2u64;return 3u64;'),
        2, '', 'cannot write through a readonly destination'),
    'query_does_not_read_uninitialized_target': (IO+'val source<-make local u64;ptr p:u64=source;'
        'val result<-make local typeof({{return p;}}){6u64};behave @print(result);', 0, '6\n', ''),
    'nested_query_retains_binding_evidence_only': (IO+'proc run()->void{val source<-make local u64;ptr p:u64=source;'
        'func inspect()->u64{val result<-make local typeof({{return p;}}){6u64};return result;};'
        'behave @print(inspect());return;}run();', 0, '6\n', ''),
    'query_only_owner_is_not_transported': (IO+'func run(ref item:u64 in O)->bool where {owner O}{'
        'func inspect()->bool{val answer<-make local typeof({{return null in O is null;}}){true};return answer;};'
        'return inspect();}val source<-1u64;behave @print(run(source));', 0, 'true\n', ''),
    'nested_immediate_borrow': (IO+'ref a:u64=1u64;{{ {{a<-7u64;}}; }};behave @print(a);', 0, '7\n', ''),
    'saved_boolean_with_captured_selection': (saved_scope('return if(present){input}else{7u64};'), 0, '5\n7\n', ''),
    'saved_boolean_with_twice_captured_selection': (
        saved_scope('return {{return if(present){input}else{7u64};}};'), 0, '5\n7\n', ''),
    'saved_boolean_does_not_prove_rebound_source': (
        saved_scope('return if(present){input}else{7u64};', 'input=null in O;'), 2, '', 'checked exception'),
    'saved_boolean_does_not_prove_rebound_scope_input': (
        saved_scope('input=null in O;return if(present){input}else{7u64};'), 2, '', 'checked exception'),
    'saved_boolean_in_readonly_borrowed_input': (
        borrowed_test_scope('return if(guard){input}else{7u64};'), 0, '5\n7\n', ''),
    'saved_boolean_in_nullable_borrowed_input': (borrowed_test_scope(
        'return if(guard is not null&&guard){input}else{7u64};', declaration='ptr guard:const bool=observed;'),
        0, '5\n7\n', ''),
    'saved_boolean_copied_before_overlapping_borrow_write': (borrowed_test_scope(
        'const val saved<-guard;writer<-true;return if(saved){input}else{7u64};',
        'ref writer:bool=observed;'), 0, '5\n7\n', ''),
    'saved_boolean_lost_after_overlapping_borrow_write': (borrowed_test_scope(
        'writer<-true;return if(guard){input}else{7u64};', 'ref writer:bool=observed;'),
        2, '', 'checked exception'),
    'captured_independent_reference_bindings_share_selection': (
        saved_scope('return if(same is not null){input}else{7u64};','ptr same:u64 in O=input;'),
        0, '5\n7\n', ''),
    'nested_scope_receives_proven_selection_equality': (
        saved_scope('return {{return if(same is not null){input}else{7u64};}};','ptr same:u64 in O=input;'),
        0, '5\n7\n', ''),
    'captured_selection_equality_ends_at_rebind': (
        saved_scope('input=null in O;return if(same is not null){input}else{7u64};','ptr same:u64 in O=input;'),
        2, '', 'checked exception'),
    'scope_captures_conditional_target_outcomes': (
        conditional_scope('return if(selected is not null){input}else{7u64};'), 0, '5\n7\n7\n', ''),
    'nested_scope_captures_conditional_target_outcomes': (
        conditional_scope('return {{return if(selected is not null){input}else{7u64};}};'), 0, '5\n7\n7\n', ''),
    'conditional_target_outcome_cannot_refine_rebound_input': (
        conditional_scope('input=null in O;return if(selected is not null){input}else{7u64};'), 2, '', 'checked exception'),
    'conditional_boolean_result_keeps_input_observation': (
        conditional_scope('const val present<-{{return if(flag){input is not null}else{true};}};'
                          'return if(present&&flag){input}else{7u64};', ''), 0, '5\n7\n7\n', ''),
    'boolean_result_and_guard_retained_at_scope_entry': (
        conditional_scope('return if(present&&flag){input}else{7u64};',
                          'const val present<-{{if(flag){return input is not null;}else{return true;};}};'),
        0, '5\n7\n7\n', ''),
    'escape_through_returned_callable_rejected': ('ref a:u64=1u64;func held={{return func[a]()->u64{return a;};}};',
        2, '', 'selected-target lifetime and transport proof'),
    'live_holder_observes_write_through_borrow': (IO+VALUES+'func original=wrap(one);ptr target=original;'
        'behave @print({{target<-wrap(two);return original();}});behave @print(original());', 0, '2\n2\n', ''),
    'generic_default_parameter_roles': (IO+VALUES+'func retain(input:T)->const T where T{return {{return input;}};}'
        'behave @print(retain(3u64));func saved=retain(one);behave @print(saved());behave @print(retain(8));', 0, '3\n1\n8\n', ''),
    'prepared_transfer_keeps_target_across_live_rebind': (IO+
        'proc run()->void{val a<-11u64,b<-22u64;ptr p=a;defer{behave @print(a);behave @print(b);};'
        'defer (<-)(p,7u64);{{p=b;}};behave @print(p);return;}run();', 0, '22\n7\n22\n', ''),
    'overlapping_supplier_binding_slots': (
        IO+NPE+SLOT_METHOD+' throws NullPointerException'+SLOT_BODY+SLOT_CALLS, 0, '8\ntrue\n1\nfalse\ntrue\n', ''),
    'overlapping_slots_keep_nullable_read_obligation': (
        IO+NPE+SLOT_METHOD+SLOT_BODY+SLOT_CALLS, 2, '', 'checked exception'),
}

READY_REPLACEMENT = 'val a<-make local u64;val b<-22u64;ptr p=a;'
UNWRITTEN_REPLACEMENT = 'val a<-11u64;val b<-make local u64;ptr p=a;'
for name, declarations, body, expected, diagnostic in (
    ('nested_rebind', '', READY_REPLACEMENT+'{{ {{p=b;}}; }};return p;', '22\n', ''),
    ('prepared_rebind_in_cleanup', '', READY_REPLACEMENT+'{{defer (=)(p,b);}};return p;', '22\n', ''),
    ('discarded_rebind_does_not_read_payload', '', UNWRITTEN_REPLACEMENT+'ptr saved=p;{{p=b:void;}};return saved;', '11\n', ''),
    ('rebind_drops_old_readiness', '', UNWRITTEN_REPLACEMENT+'{{defer (<-)(p,7u64);p=b;}};return p;', '', 'initialization'),
    ('checked_failure_keeps_unwritten_target', CHECKED, UNWRITTEN_REPLACEMENT+
        'val e<-error();try{ {{p=b;throw e;}}; }catch(x:const NullPointerException){return p;};return p;', '', 'initialization'),
    ('checked_failure_before_rebind_keeps_old_selection', CHECKED, UNWRITTEN_REPLACEMENT+
        'val e<-error();try{ {{throw e;p=b;}}; }catch(x:const NullPointerException){return p;};return p;', '11\n', ''),
):
    status = 2 if diagnostic else 0
    runner.CASES[name] = (IO+declarations+'proc run()->u64{'+body+'}behave @print(run());', status, expected, diagnostic)
    runner.CASES['query_'+name] = (IO+declarations+query(body)+'behave @print(queried);', status,
                                  '' if diagnostic else '4\n', diagnostic)

PLACE_REFERENCES = 'val x<-11u64,y<-22u64;ptr p=x;'
SELECTED_PLACE = 'val x<-11u64,y<-22u64,z<-33u64;ptr p=x,q=y;var selected->if(flag){p}else{q};'
SELECTED_VALUE_PLACE = 'val x<-make local u64,y<-make local u64;var selected->if(flag){x}else{y};'
OVERLAPPING_PLACES = ('val x<-11u64,y<-22u64,z<-make local u64;ptr p=x,q=y;var selected->if(flag){p}else{q};'
                      'var other->if(other_flag){p}else{q};')
for name, body, expected, diagnostic in (
    ('declared_place_shares_reference_slot', PLACE_REFERENCES+'var selected->p;return {{selected=y;return p;}};',
     '22\n', ''),
    ('declared_place_preserves_selected_reference_slot', SELECTED_PLACE+
     'flag<-!flag;{{selected=z;}};return p+q*100u64;', '2233\n3311\n', ''),
    ('declared_place_nested_names_share_slot', SELECTED_PLACE+
     'var other->selected;return {{ {{other=z;}};return selected;}};', '33\n33\n', ''),
    ('declared_place_initializes_selected_value', SELECTED_VALUE_PLACE+
     '{{selected<-41u64:void;}};return selected;', '41\n41\n', ''),
    ('declared_place_does_not_initialize_unselected_value', SELECTED_VALUE_PLACE+
     '{{selected<-41u64:void;}};return x;', '', 'initializ'),
    ('declared_place_preserves_prepared_target', PLACE_REFERENCES+
     'var selected->p;{{defer (<-)(selected,8u64);p=y;}};return x+p;', '30\n', ''),
    ('declared_place_keeps_name_specific_const', PLACE_REFERENCES+
     'const var selected->p;return {{selected<-9u64;p=y;return selected+x;}};', '31\n', ''),
    ('declared_place_const_binding_rejects_rebind', PLACE_REFERENCES+
     'const var selected->p;{{selected=y;}};return p;', '', 'const reference binding'),
    ('declared_place_cannot_relax_slot_target_access',
     'val x<-11u64;const val y<-22u64;ptr p=x;var selected:const->p;{{selected=y;}};return p;',
     '', 'readonly source'),
    ('declared_place_inner_owner', 'return {{val x<-11u64;var selected in here->x;return selected;}};', '11\n', ''),
    ('declared_place_keeps_default_parameter_kind',
     'return {{ptr var selected->flag;return if(selected){11u64}else{22u64};}};', '11\n22\n', ''),
    ('declared_place_owner_survives_scope_call', PLACE_REFERENCES+
     'var selected->p;{{var wrong in here->selected;}};return p;', '', 'selected binding storage'),
    ('declared_place_weak_rebind_cannot_hide_unwritten_target',
     SELECTED_PLACE.replace('z<-33u64', 'z<-make local u64')+'{{selected=z;}};return p;', '', 'initializ'),
    ('declared_place_prepared_write_does_not_initialize_rebound_target',
     OVERLAPPING_PLACES+'{{defer (<-)(selected,8u64);other=z;}};return selected;',
     '', 'initializ'),
):
    if 'other_flag' in body:
        prefix = IO+'proc run(flag:bool,other_flag:bool)->u64{'
        calls = 'behave @print(run(true,true));'
    else:
        prefix = IO+'proc run(flag:bool)->u64{'
        calls = 'behave @print(run(true));'
        if 'flag' in body:
            calls += 'behave @print(run(false));'
    status = 2 if diagnostic else 0
    runner.CASES[name] = (prefix+body+'}'+calls, status, expected, diagnostic)
    runner.CASES['query_'+name] = (prefix+query(body)+'return queried;}'+calls, status,
                                  '' if diagnostic else '4\n' * (2 if 'flag' in body else 1), diagnostic)

for name, body, expected, diagnostic in (
    ('declared_place_target_owner_is_not_place_owner',
     'var selected->p;{{var wrong in O->selected;}};return 1u64;', '', 'selected binding storage'),
    ('declared_place_preserves_external_target_owner',
     'var selected->p;{{ptr local:u64 in O=null in O;selected=local:void;}};return if(p is null){3u64}else{8u64};',
     '3\n', ''),
):
    prefix = IO+'proc run(ptr p:u64 in O)->u64 where {owner O}{'
    calls = '}val x<-1u64;behave @print(run(x));'
    status = 2 if diagnostic else 0
    runner.CASES[name] = (prefix+body+calls, status, expected, diagnostic)
    runner.CASES['query_'+name] = (prefix+query(body)+'return queried;'+calls, status, '' if diagnostic else '4\n', diagnostic)

runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
