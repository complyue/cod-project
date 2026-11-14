#!/usr/bin/env python3
"""Mutable target flow preserves identity, owners, access and evaluation order."""
from pathlib import Path
import sys
import foundation_tests as runner

IO = 'using symbol "std:io" @print;\n'
ERRORS = IO + '''using "std:exceptions" MissingReturnException;
func missing()->const i64{}
func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}
'''
WARN = '[-Wmissing-return]'
runner.CASES = {
    'rebind_not_copy': (IO + 'val a<-1,b<-2;ptr p=a;p=b;p<-3;behave @print(a);behave @print(b);behave @print(p);', 0, '1\n3\n3\n', ''),
    'alias_snapshot': (IO + 'val a<-1,b<-2;ptr p=a;const ptr q=p;p=b;p<-3;q<-4;behave @print(a);behave @print(b);', 0, '4\n3\n', ''),
    'independent_bindings': (IO + 'val a<-1,b<-2,c<-3;ptr p=a,q=p;p=b;q=c;behave @print(p);behave @print(q);behave @print(a);', 0, '2\n3\n1\n', ''),
    'destination_before_rhs': (IO + 'val a<-1,b<-2;ptr p=a;p<-{p=b;7};behave @print(a);behave @print(b);behave @print(p);', 0, '7\n2\n2\n', ''),
    'rebind_rhs_effects': (IO + 'val a<-1,b<-2,c<-3;ptr p=a;p={p=c;behave @print(p);b};behave @print(p);', 0, '3\n2\n', ''),
    'rebind_chain': (IO + 'val a<-1,b<-2,c<-3;ptr p=a,q=b;(p=q=c)<-4;behave @print(p);behave @print(q);behave @print(c);behave @print(a);', 0, '4\n4\n4\n1\n', ''),
    'reference_read_after_rebind': (IO + 'val a<-1,b<-2;ref p=a;p=b;behave @print(p);', 0, '2\n', ''),
    'rebind_uninitialized_then_write': (IO + 'val a<-1,b<-make local i64;ptr p=a;p=b;p<-8;behave @print(b);', 0, '8\n', ''),
    'rebind_view_without_read': (IO + 'val a<-1,b<-make local i64;ptr p=a;const ptr q=(p=b);q<-9;behave @print(p);', 0, '9\n', ''),
    'rebind_used_value_uninitialized': ('val a<-1,b<-make local i64;ptr p=a;val x<-(p=b);', 2, '', 'before definite initialization'),
    'ref_rebind_uninitialized': ('val a<-1,b<-make local i64;ref p=a;p=b;', 2, '', 'before definite initialization'),
    'rebind_read_uninitialized': ('val a<-1,b<-make local i64;ptr p=a;p=b;p;', 2, '', 'before definite initialization'),
    'same_type_required': ('val a<-1u64,b<-2u8;ptr p=a;p=b;', 2, '', 'exact target type'),
    'same_owner_required': ('val a<-1;ptr p=a;{{val b<-2;p=b;}};', 2, '', 'initializer-fixed owner'),
    'longer_lived_owner_still_different': ('val a<-1;{{val b<-2;ptr p=b;p=a;}};', 2, '', 'initializer-fixed owner'),
    'readonly_rebind': (IO + 'val a<-1;const val b<-2;ptr p:const i64=a;p=b;behave @print(p);', 0, '2\n', ''),
    'readonly_rebind_escalation': ('val a<-1;const val b<-2;ptr p=a;p=b;', 2, '', 'readonly source cannot rebind a writable reference target'),
    'readonly_assignment_view': ('val a<-1,b<-2;ptr p:const i64=a;(p=b)<-3;', 2, '', 'readonly destination'),
    'const_binding_rebind': ('val a<-1,b<-2;const ptr p=a;p=b;', 2, '', 'cannot rebind a const reference binding'),
    'branch_rebind': (IO + 'proc f(flag:bool)->void{val a<-1,b<-2;ptr p=a;if(flag){p=b;}else{p=a;};p<-5;behave @print(a);behave @print(b);}f(true);f(false);', 0, '1\n5\n5\n2\n', ''),
    'branch_reference_selection': (IO + 'proc f(flag:bool)->void{val a<-1,b<-2;const ptr p=if(flag){a}else{b};p<-6;behave @print(a);behave @print(b);}f(true);f(false);', 0, '6\n2\n1\n6\n', ''),
    'selected_destination': (IO + 'proc f(flag:bool)->void{val a<-1,b<-2;(if(flag){a}else{b})<-7;behave @print(a);behave @print(b);}f(true);f(false);', 0, '7\n2\n1\n7\n', ''),
    'selected_destination_before_rhs': (IO + 'val a<-1,b<-2,c<-3;ptr p=a;((if(true){p}else{b}))<-{p=c;8};behave @print(a);behave @print(b);behave @print(p);', 0, '8\n2\n3\n', ''),
    'branch_uninitialized_target': ('proc f(flag:bool)->void{val a<-1,b<-make local i64;ptr p=a;if(flag){p=b;{}}else{};p;}', 2, '', 'before definite initialization'),
    'selected_write_then_read': (IO + 'proc f(flag:bool)->void{val a<-make local i64,b<-make local i64;ptr p=if(flag){a}else{b};p<-7;behave @print(p);}f(true);f(false);', 0, '7\n7\n', ''),
    'selected_initialized_ref': (IO + 'proc f(flag:bool)->void{val a<-make local i64,b<-make local i64;ptr p=if(flag){a}else{b};p<-8;const ref q=p;p=if(flag){b}else{a};behave @print(q);}f(true);f(false);', 0, '8\n8\n', ''),
    'same_target_set_rhs_mutation': ('proc f(flag:bool)->void{val a<-make local i64,b<-make local i64;ptr p=if(flag){a}else{b};p<-{p=if(flag){b}else{a};1};p;}', 2, '', 'before definite initialization'),
    'stable_selected_target_loop': (IO + 'proc f(flag:bool)->void{val a<-make local i64,b<-make local i64;ptr p=if(flag){a}else{b};p<-9;for(i in 0..2){behave @print(p);};}f(true);f(false);', 0, '9\n9\n9\n9\n', ''),
    'selected_target_cleanup': (IO + 'proc f(flag:bool)->void{val a<-make local i64,b<-make local i64;ptr p=if(flag){a}else{b};p<-10;defer{behave @print(p);};}f(true);f(false);', 0, '10\n10\n', ''),
    'selected_write_not_all_targets': ('proc f(flag:bool)->void{val a<-make local i64,b<-make local i64;ptr p=if(flag){a}else{b};p<-1;a;}', 2, '', 'before definite initialization'),
    'selected_owner_conflict': ('val a<-1;{{val b<-2;ptr p=if(true){a}else{b};}};', 2, '', 'joins different owners'),
    'selected_readonly': ('val a<-1;const val b<-2;ptr p=if(true){a}else{b};p<-3;', 2, '', 'readonly destination'),
    'mixed_value_view_gap': ('val a<-1;ptr p=if(true){a}else{2};', 2, '', 'mixing fresh values and storage views'),
    'loop_alternates': (IO + 'proc f(n:i64)->void{val a<-0,b<-0;ptr p=a;for(i in 0..n){p<-p+1;p=if(i%2==0){b}else{a};};behave @print(a);behave @print(b);behave @print(p);}f(5);f(0);', 0, '3\n2\n2\n0\n0\n0\n', ''),
    'loop_alias_snapshot': (IO + 'val a<-1,b<-2;ptr p=a;const ptr q=p;for(i in 0..3){p=b;q<-q+1;};behave @print(a);behave @print(p);', 0, '4\n2\n', ''),
    'loop_invalid_later_target': ('val a<-1,b<-make local i64;ptr p=a;for(i in 0..2){p;p=b;};', 2, '', 'before definite initialization'),
    'single_iteration_target': (IO + 'val a<-1,b<-make local i64;ptr p=a;for(i in 0..1){behave @print(p);p=b;};p<-3;behave @print(b);', 0, '1\n3\n', ''),
    'loop_rebind_before_read': (IO + 'val a<-make local i64,b<-2;ptr p=a;for(i in 0..2){p=b;behave @print(p);};', 0, '2\n2\n', ''),
    'loop_index_alias_write': ('for(i in 0..1){ {{val x<-1;ptr p=x;p=i;}};};', 2, '', 'readonly source cannot rebind a writable reference target'),
    'deferred_current_target': (IO + '{{val a<-1,b<-2;ptr p=a;defer{behave @print(p);};p=b;}};', 0, '2\n', ''),
    'deferred_alias_retains_target': (IO + '{{val a<-1,b<-2;ptr p=a;const ptr q=p;defer{behave @print(q);};p=b;}};', 0, '1\n', ''),
    'deferred_target_changes': (IO + '{{val a<-1,b<-2;ptr p=a;defer{behave @print(p);};defer{p=b;};}};', 0, '2\n', ''),
    'post_cleanup_target': (IO + 'val a<-1,b<-2;ptr p=a;{{defer{p=b;};}};behave @print(p);', 0, '2\n', ''),
    'deferred_target_write': (IO + '{{val a<-1,b<-2;ptr p=a;defer{behave @print(a);behave @print(b);};defer{p=b;p<-3;};}};', 0, '1\n3\n', ''),
    'late_uninitialized_target': ('{{val a<-1,b<-make local i64;ptr p=a;defer{p;};p=b;}};', 2, '', 'cannot prove initialization'),
    'late_target_initialized': (IO + '{{val a<-1,b<-make local i64;ptr p=a;defer{behave @print(p);};p=b;b<-4;}};', 0, '4\n', ''),
    'later_declared_target': (IO + '{{val a<-1;ptr p=a;defer{behave @print(p);};val b<-2;p=b;}};', 0, '2\n', ''),
    'future_target_in_earlier_scope': (IO + 'val a<-1;ptr p=a;{{defer{behave @print(p);};}};val b<-2;p=b;behave @print(p);', 0, '1\n2\n', ''),
    'destination_initialization_survives_rhs_rebind': (IO + 'val a<-make local i64,b<-2;ptr p=a;p<-{p=b;10};behave @print(a);behave @print(p);', 0, '10\n2\n', ''),
    'new_target_not_initialized_by_old_destination_write': ('val a<-1,b<-make local i64;ptr p=a;p<-{p=b;10};p;', 2, '', 'before definite initialization'),
    'late_setter_registration_can_fail': ('{{val a<-1,b<-make local i64;ptr p=a;defer{p;};p=b;defer{b<-4;};}};', 2, '', 'cannot prove initialization'),
    'guarded_late_target_write': (IO + '{{val a<-1,b<-make local i64;ptr p=a;defer{behave @print(p);}finally{};p=b;defer{b<-5;};}};', 0, '5\n', ''),
    'loop_queue_current_target': (IO + '{{val a<-1,b<-2;ptr p=a;for(i in 0..3){defer{behave @print(p);};p=b;};}};', 0, '2\n2\n2\n', ''),
    'scope_address_before_cleanup': (IO + 'val a<-1,b<-2;ptr p=a;const ptr q={{defer{p=b;};p}};q<-6;behave @print(a);behave @print(p);', 0, '6\n2\n', ''),
    'try_address_before_finally': (IO + 'val a<-1,b<-2;ptr p=a;const ptr q=try{p}finally{p=b;{}};q<-7;behave @print(a);behave @print(p);', 0, '7\n2\n', ''),
    'conditional_alias_effect_once': (IO + 'val a<-1,b<-2;ptr p=if({behave @print(10);true}){{behave @print(11);a}}else{{behave @print(12);b}};p<-8;behave @print(a);', 0, '10\n11\n8\n', ''),
    'generic_rebind': (IO + 'func pick(a:T,b:T)->const T where T{ptr p=a;p=b;return p;}behave @print(pick(1u64,2u64));behave @print(pick(false,true));', 0, '2\ntrue\n', ''),
    'parameter_copy_owner': (IO + 'func f(a:i64,b:i64)->const i64{ptr p=a;p=b;p<-9;return b;}val x<-1,y<-2;behave @print(f(x,y));behave @print(x);behave @print(y);', 0, '9\n1\n2\n', ''),
    'catch_after_rebinding': (ERRORS + 'val a<-1,b<-2;ptr p=a;try{p=b;missing();{}}catch(e:const MissingReturnException){behave @print(p);};', 0, '2\n', WARN),
    'finally_rebind': (IO + 'val a<-1,b<-2;ptr p=a;try{}finally{p=b;{}};behave @print(p);', 0, '2\n', ''),
    'ended_binding_live_target_write': (IO + 'val a<-1;({{ptr p=a;p}})<-12;behave @print(a);', 0, '12\n', ''),
    'ended_binding_rebind': ('val a<-1,b<-2;({{ptr p=a;p}})=b;', 2, '', "reference binding's lexical scope has ended"),
    'selected_target_before_failure': (ERRORS + 'proc f(flag:bool)->void{val a<-make local i64,b<-make local i64;ptr p=if(flag){a}else{b};try{p<-13;missing();{}}catch(e:const MissingReturnException){behave @print(p);};}f(true);f(false);', 0, '13\n13\n', WARN),
    'selected_target_after_failure': (ERRORS + 'proc f(flag:bool)->void{val a<-make local i64,b<-make local i64;ptr p=if(flag){a}else{b};try{missing();p<-13;{}}catch(e:const MissingReturnException){behave @print(p);};}', 2, '', 'before definite initialization'),
    'destination_selected_before_own_cleanup': (IO + 'val a<-make local i64,b<-make local i64;ptr p=a;({{defer{p=b;};p}})<-14;behave @print(a);', 0, '14\n', ''),
    'cleanup_rebind_does_not_initialize_current_target': ('val a<-make local i64,b<-make local i64;ptr p=a;({{defer{p=b;};p}})<-14;p;', 2, '', 'before definite initialization'),
    'finally_rebind_does_not_initialize_current_target': ('val a<-make local i64,b<-make local i64;ptr p=a;(try{p}finally{p=b;{}})<-14;p;', 2, '', 'before definite initialization'),
    'return_snapshot_before_rebinding_cleanup': (IO + 'func f()->const i64{val a<-1,b<-2;ptr p=a;defer{p=b;};return p;}behave @print(f());', 0, '1\n', ''),
    'deferred_return_reads_current_target': (IO + 'func f()->const i64{val a<-1,b<-2;ptr p=a;defer{return p;};defer{p=b;};return p;}behave @print(f());', 0, '2\n', ''),
    'exception_target_rebind': (ERRORS + 'val a<-error(),b<-error();ptr p=a;p=b;behave @print(p.category);', 0, '5\n', WARN),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    sys.exit(runner.main())
