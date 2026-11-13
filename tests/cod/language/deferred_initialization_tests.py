#!/usr/bin/env python3
"""Initialization follows actual cleanup order, registration and failure paths."""
from pathlib import Path
import sys
import foundation_tests as runner

PRELUDE = '''using symbol "std:io" @print;
using "std:exceptions" MissingReturnException;
using "std:exceptions" OutOfMemory;
func missing()->const i64 {}
func error()->const MissingReturnException {try{missing();}catch(e:const MissingReturnException){return e;};}
proc observe(x:i64)->void{behave @print(x);}
'''
WARN = '[-Wmissing-return]'
UNPROVED = 'cannot prove initialization'
runner.CASES = {
    'late_write': (PRELUDE + '{{val x=make local i64;defer{observe(x);};x<-1;}};', 0, '1\n', WARN),
    'prepared_read_is_eager': (PRELUDE + '{{val x=make local i64;defer observe(x);x<-1;}};', 2, '', 'before definite initialization'),
    'post_scope': (PRELUDE + 'val x=make local i64;{{defer{x<-3;};}};observe(x);', 0, '3\n', WARN),
    'both_branch_setters': (PRELUDE + 'proc f(flag:bool)->void{val x=make local i64;{{if(flag){defer{x<-1;};}else{defer{x<-2;};};}};observe(x);}f(true);f(false);', 0, '1\n2\n', WARN),
    'optional_setter': (PRELUDE + 'proc f(flag:bool)->void{val x=make local i64;{{if(flag){defer{x<-1;};}else{};}};observe(x);}', 2, '', 'before definite initialization'),
    'ordered_guarded_read': (PRELUDE + '{{val x=make local i64;defer{observe(x);}finally{};defer{x<-4;};}};', 0, '4\n', WARN),
    'setter_registration_can_fail': (PRELUDE + '{{val x=make local i64;defer{observe(x);};defer{x<-4;};}};', 2, '', UNPROVED),
    'wrong_execution_order': (PRELUDE + '{{val x=make local i64;defer{x<-4;};defer{observe(x);}finally{};}};', 2, '', UNPROVED),
    'write_before_failure': (PRELUDE + '{{val x=make local i64;defer{observe(x);}catch(e:const MissingReturnException){observe(x);};defer{x<-5;missing();};}};', 0, '5\n', WARN),
    'write_after_failure': (PRELUDE + '{{val x=make local i64;defer{observe(x);}catch(e:const MissingReturnException){observe(x);};defer{missing();x<-5;};}};', 2, '', UNPROVED),
    'nested_scope_write': (PRELUDE + '{{val x=make local i64;defer{{{{defer{x<-6;};}};observe(x);}};}};', 0, '6\n', WARN),
    'nested_scope_late_read': (PRELUDE + '{{val x=make local i64;defer{{defer{observe(x);};x<-7;}};}};', 0, '7\n', WARN),
    'nested_registration': (PRELUDE + '{{val x=make local i64;defer{defer{observe(x);};x<-8;};}};', 0, '8\n', WARN),
    'nested_reads_use_invocation': (PRELUDE + '{{val x=make local i64;defer{defer{observe(x);};};x<-9;}};', 0, '9\n', WARN),
    'read_before_write': (PRELUDE + '{{val x=make local i64;defer{observe(x);x<-1;};}};', 2, '', UNPROVED),
    'local_is_fresh': (PRELUDE + 'for(i in 0..3){{defer{{val x=make local i64;observe(x);x<-1;}};}};', 2, '', 'before definite initialization'),
    'local_nested_read': (PRELUDE + '{{defer{{val x=make local i64;defer{observe(x);};x<-10;}};}};', 0, '10\n', WARN),
    'local_nested_uninitialized': (PRELUDE + '{{defer{{val x=make local i64;defer{observe(x);};}};}};', 2, '', UNPROVED),
    'loop_late_write': (PRELUDE + 'proc f(count:i64)->void{val x=make local i64;for(i in 0..count){defer{observe(x);};x<-3;};}f(0);f(3);', 0, '3\n3\n3\n', WARN),
    'loop_no_write': (PRELUDE + 'proc f(count:i64)->void{val x=make local i64;for(i in 0..count){defer{observe(x);};};}', 2, '', UNPROVED),
    'loop_one_setter': (PRELUDE + 'val x=make local i64;{{for(i in 0..1){defer{x<-11;};};}};observe(x);', 0, '11\n', WARN),
    'loop_zero_setters': (PRELUDE + 'val x=make local i64;{{for(i in 0..0){defer{x<-11;};};}};observe(x);', 2, '', 'before definite initialization'),
    'loop_else_initializes': (PRELUDE + 'proc f(count:i64)->void{val x=make local i64;{{for(i in 0..count){defer{x<-11;};}else{x<-12;};}};observe(x);}f(0);f(3);', 0, '12\n11\n', WARN),
    'conditional_read_registration': (PRELUDE + 'proc f(flag:bool)->void{val x=make local i64;if(flag){defer{observe(x);};x<-13;{}}else{};}f(true);f(false);', 0, '13\n', WARN),
    'conditional_distinct_cells': (PRELUDE + 'proc f(flag:bool)->void{val x=make local i64,y=make local i64;if(flag){defer{observe(x);};x<-14;}else{defer{observe(y);};y<-15;};}f(true);f(false);', 0, '14\n15\n', WARN),
    'wrong_cell': (PRELUDE + '{{val x=make local i64,y=make local i64;defer{observe(x);}finally{};defer{y<-1;};}};', 2, '', UNPROVED),
    'handler_normal_initialization': (PRELUDE + '{{val x=make local i64;defer{x<-16;}catch(e){x<-17;}finally{observe(x);};}};', 0, '16\n', WARN),
    'handler_failed_entry_initialization': (PRELUDE + 'val e=error();{{val x=make local i64;defer{x<-16;}catch(e){x<-17;}finally{observe(x);};throw e;}};', 0, '17\n', WARN),
    'handler_failed_entry_uninitialized': (PRELUDE + 'val e=error();{{val x=make local i64;defer{x<-16;}catch(e){}finally{observe(x);};throw e;}};', 2, '', UNPROVED),
    'unmatched_handler_read': (PRELUDE + 'val e=error();try{{val x=make local i64;defer catch(e:const OutOfMemory){observe(x);};throw e;}}catch(e:const MissingReturnException){observe(18);};', 0, '18\n', WARN),
    'handler_write_continuation': (PRELUDE + 'val e=error(),x=make local i64;{{defer catch(e:const MissingReturnException){x<-19;};throw e;}};observe(x);', 0, '19\n', WARN),
    'failure_continuation': (PRELUDE + 'val e=error(),x=make local i64;try{{defer{x<-20;};throw e;}}catch(e:const MissingReturnException){observe(x);};', 0, '20\n', WARN),
    'registration_failure_continuation': (PRELUDE + 'val e=error(),x=make local i64;try{{defer{x<-20;};throw e;}}catch(e){observe(x);};', 2, '', 'before definite initialization'),
    'replacement_return': (PRELUDE + 'func f()->const i64{val x=make local i64;defer{return x;};x<-21;return 1;}observe(f());', 0, '21\n', WARN),
    'generic_setter': (PRELUDE + 'proc f(value:T)->const T where T{val x=make local T;{{defer{x<-value;};}};return x;}behave @print(f(22u64));behave @print(f(true));', 0, '22\ntrue\n', WARN),
    'read_requirement_survives_return': (PRELUDE + 'func f()->const i64{val x=make local i64;defer{x;return 1;};return 2;}f();', 2, '', UNPROVED),
    'read_requirement_survives_catch': (PRELUDE + 'val e=error();{{val x=make local i64;defer{try{x;throw e;}catch(e){x<-1;};};}};', 2, '', UNPROVED),
    'prepared_nested_argument': (PRELUDE + '{{val x=make local i64;defer{defer observe(x);};x<-23;}};', 0, '23\n', WARN),
    'action_local_branch_write': (PRELUDE + 'proc f(flag:bool)->void{val x=make local i64;defer{if(flag){x<-24;}else{x<-25;};observe(x);};}f(true);f(false);', 0, '24\n25\n', WARN),
    'post_scope_partial_write': (PRELUDE + 'proc f(flag:bool)->void{val x=make local i64;{{defer{if(flag){x<-1;{}}else{};};}};observe(x);}', 2, '', 'before definite initialization'),
    'late_write_after_failure': (PRELUDE + '{{val x=make local i64;defer{observe(x);};missing();x<-1;}};', 2, '', UNPROVED),
    'cycle_cannot_supply_own_read': (PRELUDE + 'proc f(count:i64)->void{val x=make local i64;for(i in 0..count){defer{observe(x);x<-1;}finally{};};}', 2, '', UNPROVED),
    'nested_read_precedes_write': (PRELUDE + '{{val x=make local i64;defer{{{{defer{observe(x);};}};x<-1;}};}};', 2, '', UNPROVED),
    'read_requirement_survives_finally': (PRELUDE + '{{val x=make local i64;defer{try{x;}finally{x<-1;{}};};}};', 2, '', UNPROVED),
    'category_specific_finally': (PRELUDE + 'val e=error();{{val x=make local i64;defer{}catch(e:const OutOfMemory){observe(x);}catch(e:const MissingReturnException){x<-26;}finally{observe(x);};throw e;}};', 0, '26\n', WARN),
    'finally_body_skipped_on_failure': (PRELUDE + 'val e=error();{{val x=make local i64;defer{x<-1;}finally{observe(x);};throw e;}};', 2, '', UNPROVED),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    sys.exit(runner.main())
