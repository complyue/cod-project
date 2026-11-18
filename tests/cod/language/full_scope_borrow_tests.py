#!/usr/bin/env python3
"""Immediate scope operands use ordinary borrowed target and owner contracts."""
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
runner.CASES = {
    'local_slot_and_deferred_target': (IO+'ref a:u64=1u64;ref b:u64=10u64;ref p:u64=a;'
        '{{p=b;defer{p<-20u64;};}};behave @print(p is a);behave @print(a+b);', 0, 'true\n21\n', ''),
    'nullable_slot_can_rebind_locally': (IO+'ptr p:u64=null in here;ref b:u64=10u64;'
        '{{behave @print(p is null);p=b;behave @print(p);p=null;}};behave @print(p is null);', 0, 'true\n10\ntrue\n', ''),
    'nonnull_ptr_can_rebind_to_null': (IO+'ref a:u64=2u64;ptr p:u64=a;'
        '{{behave @print(p);p=null;behave @print(p is null);}};behave @print(p);', 0, '2\ntrue\n2\n', ''),
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
        'ref local:u64=1u64;behave @print({{return select(outer,local);}});}'
        'ref a:u64=1u64;show(a);', 0, 'true\n', ''),
    'named_owner_keeps_same_role': (IO+'proc show(ref outer:u64 in O)->void where {owner O}{'
        ' {{ptr p:u64 in O=null in O;p=outer;p<-9u64;}};}ref a:u64=1u64;show(a);behave @print(a);', 0, '9\n', ''),
    'named_owner_without_captured_target': (IO+'proc show(ref unused:u64 in O)->void where {owner O}{'
        ' {{ptr p:u64 in O=null in O;behave @print(p is null);}};}ref a:u64=1u64;show(a);', 0, 'true\n', ''),
    'fixed_slot_preserves_writable_target': (IO+'ref a:u64=1u64;const ptr p:u64=a;{{p<-5u64;}};behave @print(a);', 0, '5\n', ''),
    'fixed_slot_rebind_rejected': ('ref a:u64=1u64;const ptr p:u64=a;{{p=null;}};', 2, '', 'cannot rebind a const reference binding'),
    'readonly_target_write_rejected': ('ref a:const u64=1u64;{{a<-2u64;}};', 2, '', 'readonly'),
    'invocation_here_cannot_replace_outer_owner': ('ref a:u64=1u64;ptr p:u64=a;'
        '{{ptr local:u64=null in here;p=local;}};', 2, '', 'initializer-fixed owner'),
    'readonly_query_rejects_borrow_write': ('ref a:const u64=1u64;val result<-make local typeof({{a<-2u64;return 1u64;}}){4u64};',
        2, '', 'readonly'),
    'query_does_not_read_uninitialized_target': (IO+'val source<-make local u64;ptr p:u64=source;'
        'val result<-make local typeof({{return p;}}){6u64};behave @print(result);', 0, '6\n', ''),
    'nested_query_retains_binding_evidence_only': (IO+'proc run()->void{val source<-make local u64;ptr p:u64=source;'
        'func inspect()->u64{val result<-make local typeof({{return p;}}){6u64};return result;};'
        'behave @print(inspect());}run();', 0, '6\n', ''),
    'query_only_owner_is_not_transported': (IO+'func run(ref item:u64 in O)->bool where {owner O}{'
        'func inspect()->bool{val answer<-make local typeof({{return null in O is null;}}){true};return answer;};'
        'return inspect();}val source<-1u64;behave @print(run(source));', 0, 'true\n', ''),
    'nested_immediate_borrow': (IO+'ref a:u64=1u64;{{ {{a<-7u64;}}; }};behave @print(a);', 0, '7\n', ''),
    'escape_through_returned_callable_rejected': ('ref a:u64=1u64;func held={{return func[a]()->u64{return a;};}};',
        2, '', 'selected-target lifetime and transport proof'),
    'value_snapshot_survives_write_through_borrow': (IO+VALUES+'func original=wrap(one);ptr target=original;'
        'behave @print({{target<-wrap(two);return original();}});behave @print(original());', 0, '1\n2\n', ''),
    'generic_default_parameter_roles': (IO+VALUES+'func retain(input:T)->const T where T{return {{return input;}};}'
        'behave @print(retain(3u64));func saved=retain(one);behave @print(saved());behave @print(retain(8));', 0, '3\n1\n8\n', ''),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
