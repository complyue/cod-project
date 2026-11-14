#!/usr/bin/env python3
"""Selected reference slots retain identity across operand effects and cleanup."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;\n'
NPE = 'using "std:exceptions" NullPointerException;\n'
ERROR = 'using "std:exceptions" MissingReturnException;func missing()->const i64{}\n'
SETUP = 'val x=1,y=2,z=3;ptr a=x,b=y,c=z;'
SHOW = 'behave @print(a);behave @print(b);behave @print(c);'
def run(body, setup=SETUP, show=SHOW):
    return IO + 'proc run(flag:bool)->void{' + setup + body + show + '}run(true);run(false);'

runner.CASES = {
    'conditional_rebind': (run('(if(flag){a}else{b})=z;'), 0, '3\n2\n3\n1\n3\n3\n', ''),
    'conditional_swap': (run('(if(flag){a}else{b})<=>c;'), 0, '3\n2\n1\n1\n3\n2\n', ''),
    'two_selected_swap_operands': (run('(if(flag){a}else{b})<=>(if(flag){b}else{c});'), 0, '2\n1\n3\n1\n3\n2\n', ''),
    'same_selected_slot': (run('(if(flag){a}else{b})<=>(if(flag){a}else{b});'), 0, '1\n2\n3\n1\n2\n3\n', ''),
    'selection_before_rhs': (run('(if(flag){a}else{b})={flag<-!flag;z};'), 0, '3\n2\n3\n1\n3\n3\n', ''),
    'ordered_once': (run('({behave @print(4);if(flag){a}else{b}})=({behave @print(5);z});'), 0, '4\n5\n3\n2\n3\n4\n5\n1\n3\n3\n', ''),
    'swap_reloads_after_rhs': (run('(if(flag){a}else{b})<=>{a=z;b=x;c};'), 0, '3\n1\n3\n3\n3\n1\n', ''),
    'scope_retains_slot_before_cleanup': (run('({{defer{a=z;b=x;};if(flag){a}else{b}}})=y;'), 0, '2\n1\n3\n3\n2\n3\n', ''),
    'scope_swap_reloads_after_cleanup': (run('({{defer{a=z;b=x;};if(flag){a}else{b}}})<=>c;'), 0, '3\n1\n3\n3\n3\n1\n', ''),
    'finally_keeps_selected_slot': (run('(try{if(flag){a}else{b}}finally{flag<-!flag;a=z;b=x;{}})=y;'), 0, '2\n1\n3\n3\n2\n3\n', ''),
    'chained_rebind_result_slot': (run('((if(flag){a}else{b})=z)=y;'), 0, '2\n2\n3\n1\n2\n3\n', ''),
    'chained_rhs_selected_target': (run('c=(if(flag){a}else{b})=x;'), 0, '1\n2\n1\n1\n1\n1\n', ''),
    'assignment_result_target': (run('((if(flag){a}else{b})=z)<-9;'), 0, '9\n2\n9\n1\n9\n9\n', ''),
    'assignment_result_value': (run('behave @print((if(flag){a}else{b})=z);'), 0, '3\n3\n2\n3\n3\n1\n3\n3\n', ''),
    'nullable_slot_has_no_payload_read': (run('(if(flag){a}else{b})=x;', 'val x=1;ptr a:i64=null,b:i64=null;', 'behave @print(isnull(a));behave @print(isnull(b));'), 0, 'false\ntrue\ntrue\nfalse\n', ''),
    'nullable_swap_has_no_payload_read': (run('(if(flag){a}else{b})<=>c;', 'val x=1;ptr a:i64=null,b:i64=null,c=x;', 'behave @print(isnull(a));behave @print(isnull(b));behave @print(isnull(c));'), 0, 'false\ntrue\ntrue\ntrue\nfalse\ntrue\n', ''),
    'guarded_selection': (IO + ERROR + 'proc run(flag:bool)->void{' + SETUP + '(try{if(flag){a}else{missing();a}}catch(e:const MissingReturnException){b})=z;' + SHOW + '}run(true);run(false);', 0, '3\n2\n3\n1\n3\n3\n', '[-Wmissing-return]'),
    'rhs_failure_preserves_slots': (IO + ERROR + 'proc run(flag:bool)->void{' + SETUP + 'try{(if(flag){a}else{b})={missing();z};{}}catch(e:const MissingReturnException){};' + SHOW + '}run(true);run(false);', 0, '1\n2\n3\n1\n2\n3\n', '[-Wmissing-return]'),
    'swap_failure_preserves_prior_effect': (IO + ERROR + 'proc run(flag:bool)->void{' + SETUP + 'try{({a=z;if(flag){a}else{b}})<=>{missing();c};}catch(e:const MissingReturnException){};' + SHOW + '}run(true);run(false);', 0, '3\n2\n3\n3\n2\n3\n', '[-Wmissing-return]'),
    'destination_return_skips_rhs': (IO + 'proc run(flag:bool)->const i64{' + SETUP + '(if(flag){return 8;}else{b})={behave @print(4);z};return b;}behave @print(run(true));behave @print(run(false));', 0, '8\n4\n3\n', ''),
    'deferred_slot_selection': (run('defer{(if(flag){a}else{b})=z;behave @print(a);behave @print(b);};flag<-!flag;', show=''), 0, '1\n3\n3\n2\n', ''),
    'loop_slot_selection': (run('for(i in 0..3){(if(flag){a}else{b})=z;flag<-!flag;};'), 0, '3\n3\n3\n3\n3\n3\n', ''),
    'fixed_alias_names': (run('ptr var p->a;ptr var q->b;(if(flag){p}else{q})=z;'), 0, '3\n2\n3\n1\n3\n3\n', ''),
    'generic_borrowed_slots': (IO + 'func run(ref x:T in O,ref y:T in O,ref z:T in O,flag:bool)->const T where {T,owner O}{(if(flag){x}else{y})=z;return x;}val a=1,b=2,c=3,d=false,e=true;behave @print(run(a,b,c,true));behave @print(run(a,b,c,false));behave @print(run(d,e,e,true));behave @print(a);', 0, '3\n1\ntrue\n1\n', ''),
    'ref_requires_initialized_for_every_choice': ('func f(flag:bool)->void{val x=1,y=make local i64;ref a=x;ptr b=x;(if(flag){a}else{b})=y;}', 2, '', 'before definite initialization'),
    'ref_requires_nonnull_for_every_choice': ('func f(flag:bool)->void{val x=1;ref a=x;ptr b=x;(if(flag){a}else{b})=null;}', 2, '', 'ref requires a proven non-null target'),
    'weak_update_retains_uninitialized_choice': ('func f(flag:bool)->const i64{val x=1,y=make local i64;ptr a=x,b=y;(if(flag){a}else{b})=x;return b;}', 2, '', 'before definite initialization'),
    'weak_update_retains_null_choice': (NPE + 'func f(flag:bool)->const i64{val x=1;ptr a:i64=null,b:i64=null;(if(flag){a}else{b})=x;return b;}', 2, '', 'requires a covering catch or throws'),
    'weak_update_introduces_possible_null': (NPE + 'func f(flag:bool)->const i64{val x=1;ptr a=x,b=x;(if(flag){a}else{b})=null;return a;}', 2, '', 'requires a covering catch or throws'),
    'constant_slot_remains_fixed': ('func f(flag:bool)->void{val x=1;const ptr a=x;ptr b=x;(if(flag){a}else{b})=x;}', 2, '', 'cannot rebind a const reference binding'),
    'val_has_no_reference_slot': ('func f(flag:bool)->void{val x=1;ptr p=x;(if(flag){p}else{x})=x;}', 2, '', 'val storage has no reference binding'),
    'readonly_source_remains_readonly': ('func f(flag:bool)->void{val x=1;const val y=2;ptr a=x,b=x;(if(flag){a}else{b})=y;}', 2, '', 'readonly source cannot rebind'),
    'owner_remains_fixed': ('func f(flag:bool)->void{val x=1;ptr a=x,b=x;{{val y=2;(if(flag){a}else{b})=y;}};}', 2, '', 'initializer-fixed owner'),
    'ended_slot_still_rejected': ('val x=1;({{ptr p=x;p}})=x;', 2, '', "reference binding's lexical scope has ended"),
    'borrowed_result_is_not_a_slot': ('func get(ref x:i64 in O)->ref i64 in O where {owner O}{return x;}val x=1;get(x)=x;', 2, '', 'reference operation requires a binding slot'),
    'runtime_slot_alias': ('val x=1,y=2;ptr a=x,b=y;ptr var p->if(true){a}else{b};', 0, '', ''),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
