#!/usr/bin/env python3
"""Initialization follows actual cleanup order, registration and failure paths."""
from pathlib import Path
import sys
import foundation_tests as runner

PRELUDE = '''using symbol "std:io" @print;
using "std:exceptions" MissingReturnError;
using "std:exceptions" OutOfMemoryError;
func missing()->const i64 {}
func error()->const MissingReturnError {try{missing();}catch(e:const MissingReturnError){return e;};}
proc observe(x:i64)->void{behave @print(x);}
'''
WARN = '[-Wmissing-return]'
UNPROVED = 'cannot prove initialization'
BORROWED = 'val target<-make local i64;ptr destination=target;'
runner.CASES = {
    'late_write': (PRELUDE + '{{val x<-make local i64;defer{observe(x);};x<-1;}};', 0, '1\n', WARN),
    'prepared_read_is_eager': (PRELUDE + '{{val x<-make local i64;defer observe(x);x<-1;}};', 2, '', 'before definite initialization'),
    'post_scope': (PRELUDE + BORROWED + '{{defer{destination<-3;};}};observe(target);', 0, '3\n', WARN),
    'both_branch_setters': (PRELUDE + 'proc f(flag:bool)->void{' + BORROWED
        + '{{if(flag){defer{destination<-1;};}else{defer{destination<-2;};};}};observe(target);}f(true);f(false);',
        0, '1\n2\n', WARN),
    'optional_setter': (PRELUDE + 'proc f(flag:bool)->void{' + BORROWED
        + '{{if(flag){defer{destination<-1;};}else{};}};observe(target);}',
        2, '', 'before definite initialization'),
    'ordered_guarded_read': (PRELUDE + '{{val x<-make local i64;defer{observe(x);}finally{};defer{x<-4;};}};', 0, '4\n', WARN),
    'setter_registration_can_fail': (PRELUDE + '{{val x<-make local i64;defer{observe(x);};defer{x<-4;};}};', 2, '', UNPROVED),
    'wrong_execution_order': (PRELUDE + '{{val x<-make local i64;defer{x<-4;};defer{observe(x);}finally{};}};', 2, '', UNPROVED),
    'write_before_failure': (PRELUDE + '{{val x<-make local i64;defer{observe(x);}catch(e:const MissingReturnError){observe(x);};defer{x<-5;missing();};}};', 0, '5\n', WARN),
    'write_after_failure': (PRELUDE + '{{val x<-make local i64;defer{observe(x);}catch(e:const MissingReturnError){observe(x);};defer{missing();x<-5;};}};', 2, '', UNPROVED),
    'nested_scope_write': (PRELUDE + '{{' + BORROWED
        + 'defer{ {{ {{defer{destination<-6;};}};observe(destination);}};};}};',
        0, '6\n', WARN),
    'nested_scope_late_read': (PRELUDE + '{{' + BORROWED
        + 'defer{ {{defer{observe(destination);};destination<-7;}};};}};', 0, '7\n', WARN),
    'nested_registration': (PRELUDE + '{{val x<-make local i64;defer{defer{observe(x);};x<-8;};}};',
        2, '', 'currently draining scope'),
    'nested_reads_use_invocation': (PRELUDE +
        '{{val x<-make local i64;defer{ {{defer{observe(x);};}};};x<-9;}};', 0, '9\n', WARN),
    'read_before_write': (PRELUDE + '{{val x<-make local i64;defer{observe(x);x<-1;};}};', 2, '', UNPROVED),
    'local_is_fresh': (PRELUDE + 'for(i in 0..3){ {{defer{ {{val x<-make local i64;observe(x);x<-1;}};};}};};',
        2, '', "read of 'x' before definite initialization"),
    'local_nested_read': (PRELUDE + '{{defer{{val x<-make local i64;defer{observe(x);};x<-10;}};}};', 0, '10\n', WARN),
    'local_nested_uninitialized': (PRELUDE + '{{defer{{val x<-make local i64;defer{observe(x);};}};}};', 2, '', UNPROVED),
    'loop_late_write': (PRELUDE + 'proc f(count:i64)->void{val x<-make local i64;for(i in 0..count){defer{observe(x);};x<-3;};}f(0);f(3);', 0, '3\n3\n3\n', WARN),
    'loop_no_write': (PRELUDE + 'proc f(count:i64)->void{val x<-make local i64;for(i in 0..count){defer{observe(x);};};}', 2, '', UNPROVED),
    'loop_one_setter': (PRELUDE + BORROWED + '{{for(i in 0..1){defer{destination<-11;};};}};observe(target);',
        0, '11\n', WARN),
    'loop_zero_setters': (PRELUDE + BORROWED + '{{for(i in 0..0){defer{destination<-11;};};}};observe(target);',
        2, '', 'before definite initialization'),
    'loop_else_initializes': (PRELUDE + 'proc f(count:i64)->void{' + BORROWED
        + '{{for(i in 0..count){defer{destination<-11;};}else{destination<-12;};}};observe(target);}f(0);f(3);',
        0, '12\n11\n', WARN),
    'conditional_read_registration': (PRELUDE + 'proc f(flag:bool)->void{val x<-make local i64;if(flag){defer{observe(x);};x<-13;{}}else{};}f(true);f(false);', 0, '13\n', WARN),
    'conditional_distinct_cells': (PRELUDE + 'proc f(flag:bool)->void{val x<-make local i64,y<-make local i64;if(flag){defer{observe(x);};x<-14;}else{defer{observe(y);};y<-15;};}f(true);f(false);', 0, '14\n15\n', WARN),
    'wrong_cell': (PRELUDE + '{{val x<-make local i64,y<-make local i64;defer{observe(x);}finally{};defer{y<-1;};}};', 2, '', UNPROVED),
    'handler_normal_initialization': (PRELUDE + '{{val x<-make local i64;defer{x<-16;}catch(e){x<-17;}finally{observe(x);};}};', 0, '16\n', WARN),
    'handler_failed_entry_initialization': (PRELUDE + 'val e<-error();{{val x<-make local i64;defer{x<-16;}catch(e){x<-17;}finally{observe(x);};throw e;}};', 0, '17\n', WARN),
    'handler_failed_entry_uninitialized': (PRELUDE + 'val e<-error();{{val x<-make local i64;defer{x<-16;}catch(e){}finally{observe(x);};throw e;}};', 2, '', UNPROVED),
    'unmatched_handler_read': (PRELUDE + 'val e<-error();try{{val x<-make local i64;defer catch(e:const OutOfMemoryError){observe(x);};throw e;}}catch(e:const MissingReturnError){observe(18);};', 0, '18\n', WARN),
    'handler_write_continuation': (PRELUDE + 'val incoming<-error();' + BORROWED
        + '{{defer catch(e:const MissingReturnError){destination<-19;};throw incoming;}};observe(target);',
        0, '19\n', WARN),
    'failure_continuation': (PRELUDE + 'val incoming<-error();' + BORROWED
        + 'try{ {{defer{destination<-20;};throw incoming;}}; }catch(e:const MissingReturnError){observe(target);};',
        0, '20\n', WARN),
    'registration_failure_continuation': (PRELUDE + 'val incoming<-error();' + BORROWED
        + 'try{ {{defer{destination<-20;};throw incoming;}}; }catch(e){observe(target);};',
        2, '', 'before definite initialization'),
    'replacement_return': (PRELUDE + 'func f()->const i64{val x<-make local i64;defer{return x;};x<-21;return 1;}observe(f());', 0, '21\n', WARN),
    'generic_setter': (PRELUDE + 'proc f(value:T)->const T where T{val x<-make local T;ptr destination=x;'
        '{{defer{destination<-value;};}};return x;}behave @print(f(22u64));behave @print(f(true));',
        0, '22\ntrue\n', WARN),
    'read_requirement_survives_return': (PRELUDE + 'func f()->const i64{val x<-make local i64;defer{x;return 1;};return 2;}f();', 2, '', UNPROVED),
    'read_requirement_survives_catch': (PRELUDE + 'val e<-error();{{val x<-make local i64;defer{try{x;throw e;}catch(e){x<-1;};};}};', 2, '', UNPROVED),
    'prepared_nested_argument': (PRELUDE + '{{val x<-make local i64;defer{ {{defer observe(x);}};};x<-23;}};',
        0, '23\n', WARN),
    'action_local_branch_write': (PRELUDE + 'proc f(flag:bool)->void{val x<-make local i64;defer{if(flag){x<-24;}else{x<-25;};observe(x);};}f(true);f(false);', 0, '24\n25\n', WARN),
    'post_scope_partial_write': (PRELUDE + 'proc f(flag:bool)->void{' + BORROWED
        + '{{defer{if(flag){destination<-1;{}}else{};};}};observe(target);}', 2, '', 'before definite initialization'),
    'late_write_after_failure': (PRELUDE + '{{val x<-make local i64;defer{observe(x);};missing();x<-1;}};', 2, '', UNPROVED),
    'cycle_cannot_supply_own_read': (PRELUDE + 'proc f(count:i64)->void{val x<-make local i64;for(i in 0..count){defer{observe(x);x<-1;}finally{};};}', 2, '', UNPROVED),
    'nested_read_precedes_write': (PRELUDE + '{{' + BORROWED
        + 'defer{ {{ {{defer{observe(destination);};}};destination<-1;}};};}};', 2, '', UNPROVED),
    'read_requirement_survives_finally': (PRELUDE + '{{val x<-make local i64;defer{try{x;}finally{x<-1;{}};};}};', 2, '', UNPROVED),
    'category_specific_finally': (PRELUDE + 'val e<-error();{{val x<-make local i64;defer{}catch(e:const OutOfMemoryError){observe(x);}catch(e:const MissingReturnError){x<-26;}finally{observe(x);};throw e;}};', 0, '26\n', WARN),
    'finally_body_skipped_on_failure': (PRELUDE + 'val e<-error();{{val x<-make local i64;defer{x<-1;}finally{observe(x);};throw e;}};', 2, '', UNPROVED),
}
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {
    'handler_failed_entry_initialization': ['e', 'e'],
    'unmatched_handler_read': ['e', 'e', 'e'],
    'category_specific_finally': ['e', 'e', 'e'],
}
runner.EXTRA_SOURCES = [Path(__file__)]


if __name__ == '__main__':
    sys.exit(runner.main())
