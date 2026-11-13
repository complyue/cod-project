#!/usr/bin/env python3
"""Local bindings share storage proofs without copying or extending owners."""
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
    'shared_payload': (IO + 'val x=1;const ptr a=x;ref b=a;b<-2;behave @print(x);a<-3;behave @print(b);', 0, '2\n3\n', ''),
    'fresh_targets': (IO + 'const ptr x=1,y=x+2;const ref z=4;x<-5;y<-6;z<-7;behave @print(x);behave @print(y);behave @print(z);', 0, '5\n6\n7\n', ''),
    'copy_initializer': (IO + 'val x=1;ptr a<-x;ref b<-x;a<-2;b<-3;behave @print(x);behave @print(a);behave @print(b);', 0, '1\n2\n3\n', ''),
    'val_copy_from_alias': (IO + 'val x=1;const ptr a=x;val copy=a;copy<-2;behave @print(a);behave @print(copy);', 0, '1\n2\n', ''),
    'const_binding_writable_target': (IO + 'const ptr p=make local i64;const ptr q=p;q<-8;behave @print(p);', 0, '8\n', ''),
    'const_ptr_rebind': ('val x=1,y=2;const ptr p=x;p=y;', 2, '', 'cannot rebind a const reference binding'),
    'const_ref_rebind': ('val x=1,y=2;const ref p=x;p=y;', 2, '', 'cannot rebind a const reference binding'),
    'mutable_target': (IO + 'val x=1,y=2;ptr p=x;p=y;behave @print(p);', 0, '2\n', ''),
    'readonly_view': ('val x=1;const ptr p:const i64=x;p<-2;', 2, '', 'readonly destination'),
    'inferred_readonly': ('const val x=1;ptr p=x;p<-2;', 2, '', 'readonly destination'),
    'readonly_escalation': ('const val x=1;ref p:i64=x;', 2, '', 'readonly source cannot initialize a writable reference target'),
    'readonly_chain': ('val x=1;ptr a:const i64=x;ptr b=a;ptr c:i64=b;', 2, '', 'readonly source cannot initialize a writable reference target'),
    'readonly_observes_writes': (IO + 'val x=1;const ref r:const i64=x;ptr w=x;w<-2;behave @print(r);', 0, '2\n', ''),
    'readonly_copy': (IO + 'const val x=1;ptr a:i64<-x;a<-2;behave @print(x);behave @print(a);', 0, '1\n2\n', ''),
    'alias_type_exact': ('val x=1u8;const ptr p:u64=x;', 2, '', 'initializer does not satisfy declared ptr type'),
    'fresh_type_widens': (IO + 'const ptr p:u64=1u8;p<-2u8;behave @print(p);', 0, '2\n', ''),
    'uninitialized_alias': ('val x=make local i64;const ptr p=x;p;', 2, '', 'before definite initialization'),
    'uninitialized_ref': ('val x=make local i64;const ref p=x;', 2, '', 'before definite initialization'),
    'uninitialized_fresh_ref': ('const ref p=make local i64;', 2, '', 'ref requires definitely initialized target storage'),
    'alias_initializes_original': (IO + 'val x=make local i64;const ptr p=x;p<-9;behave @print(x);', 0, '9\n', ''),
    'original_initializes_alias': (IO + 'val x=make local i64;const ptr p=x;x<-10;behave @print(p);', 0, '10\n', ''),
    'typeof_alias': (IO + 'const ptr p=make local u64;val x=make local typeof(p);x<-11u64;behave @print(x);', 0, '11\n', ''),
    'initializer_before_binding': (IO + 'val x=1;{{const ptr x=x;x<-2;behave @print(x);}};behave @print(x);', 0, '2\n2\n', '[-Wshadow]'),
    'independent_binding_availability': ('proc f(flag:bool)->void{val x=1;const ptr a=x;if(flag){const ptr b=x;}else{};b;}f(false);', 2, '', "binding 'b' is not available"),
    'duplicate_binding': ('val x=1;ptr p=x;ptr p=x;', 2, '', 'duplicate declaration'),
    'owner_ends': ('const ptr p={{val x=1;x}};', 2, '', "reference initializer's lexical owner has ended"),
    'outer_owner_survives': (IO + 'val x=1;const ptr p={{x}};p<-12;behave @print(x);', 0, '12\n', ''),
    'source_effect_once': (IO + 'val x=1;const ptr p={behave @print(20);x};p<-2;behave @print(x);', 0, '20\n2\n', ''),
    'view_cleanup_does_not_read': (IO + 'val x=make local i64;const ptr p={{defer behave @print(21);x}};p<-3;behave @print(x);', 0, '21\n3\n', ''),
    'same_place_conditional': (IO + 'proc f(flag:bool)->void{val x=make local i64;const ptr a=x,b=x;const ptr p=if(flag){a}else{b};p<-4;behave @print(x);}f(true);f(false);', 0, '4\n4\n', ''),
    'selected_target': (IO + 'val x=1,y=2;const ptr p=if(true){x}else{y};behave @print(p);', 0, '1\n', ''),
    'alias_branch_initialization': (IO + 'proc f(flag:bool)->void{val x=make local i64;const ptr p=x;if(flag){p<-5;}else{x<-6;};behave @print(p);}f(true);f(false);', 0, '5\n6\n', ''),
    'alias_branch_missing_write': ('proc f(flag:bool)->void{val x=make local i64;const ptr p=x;if(flag){p<-5;{}}else{};x;}', 2, '', 'before definite initialization'),
    'alias_loop_write': (IO + 'val x=make local i64;const ptr p=x;for(i in 0..1){p<-i;};behave @print(x);', 0, '0\n', ''),
    'alias_loop_control_capture': ('for(i in 0..1){{{const ptr p=i;defer{p;};}}};', 2, '', 'cannot capture loop-control binding'),
    'loop_control_after_alias_scope': ('for(i in 0..1){ {{const ptr p=i;}};defer{i;};};', 2, '', 'cannot capture loop-control binding'),
    'late_alias_read': (IO + '{{val x=make local i64;const ptr p=x;defer{behave @print(p);};x<-7;}};', 0, '7\n', ''),
    'deferred_alias_write': (IO + 'val x=make local i64;const ptr p=x;{{defer{p<-8;};}};behave @print(x);', 0, '8\n', ''),
    'ordered_alias_actions': (IO + '{{val x=make local i64;const ptr p=x;defer{behave @print(x);}finally{};defer{p<-9;};}};', 0, '9\n', ''),
    'alias_setter_registration_failure': (IO + '{{val x=make local i64;const ptr p=x;defer{behave @print(x);};defer{p<-9;};}};', 2, '', 'cannot prove initialization'),
    'deferred_fresh_alias': (IO + '{{val x=make local i64;defer{{const ref p=x;behave @print(p);}};x<-10;}};', 0, '10\n', ''),
    'nested_alias_keeps_target': (IO + 'val x=make local i64;{{const ptr p=x;p<-12;}};behave @print(x);', 0, '12\n', ''),
    'const_ref_parameter_target': (IO + 'func f(const ref x:i64)->const i64{const ptr p=x;p<-13;return x;}val x=1;behave @print(f(x));behave @print(x);', 0, '13\n1\n', ''),
    'const_parameter_binding': ('func f(const ref x:i64)->void{val y=1;x=y;}', 2, '', 'cannot rebind a const reference binding'),
    'transfer_view_readonly': ('val x=1;ptr p=(x<-2);p<-3;', 2, '', 'readonly destination'),
    'late_ref_requirement': (IO + '{{val x=make local i64;defer{{const ref p=x;behave @print(p);}};}};', 2, '', 'cannot prove initialization'),
    'alias_and_parameter_copy': (IO + 'func f(x:T)->const T where T{const ptr p=x;val copy=p;return copy;}behave @print(f(11u64));behave @print(f(true));', 0, '11\ntrue\n', ''),
    'alias_then_catch_storage': (ERRORS + 'val x=1;const ptr p=x;try{missing();{}}catch(e:const MissingReturnException){behave @print(e.category);};', 0, '5\n', WARN),
    'caught_alias_snapshot': (ERRORS + '{{for(i in 0..2){try{missing();{}}catch(e:const MissingReturnException){{{const ptr p=e;defer{behave @print(p.category);};}}};};}};', 0, '5\n5\n', WARN),
    'caught_alias_expired': (ERRORS + 'try{throw error();}catch(e:const MissingReturnException){const ptr p=e;{};};p;', 2, '', "lexical owner has ended"),
    'mutable_exception_alias': (ERRORS + 'val e=error();const ptr p=e;{{defer{behave @print(p.category);};p<-error();}};', 0, '5\n', WARN),
    'rethrow_alias': (ERRORS + 'try{try{missing();{}}catch(e:const MissingReturnException){const ptr p=e;throw p;};}catch(e:const MissingReturnException){behave @print(e.category);};', 0, '5\n', WARN),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    sys.exit(runner.main())
