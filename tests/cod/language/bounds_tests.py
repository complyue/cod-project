#!/usr/bin/env python3
"""Source bounds failures use ordinary exception and completion contracts."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
TYPES = 'using "std:exceptions" IndexOutOfBoundsError;using "std:exceptions" MissingReturnError;using "std:exceptions" Exception;using "std:exceptions" NullPointerException;'
SEED = 'func missing()->u64{}func seed()->const MissingReturnError{try{missing();}catch(e:const MissingReturnError){return e;};}'
LINK = 'func link(e:MissingReturnError,cause:Exception)->const MissingReturnError{try{throw e from cause;}catch(x:const MissingReturnError){return x;};}'
PRELUDE = IO+TYPES+SEED+LINK
WARN = '[-Wmissing-return]'
OUT = lambda value: 'behave @print('+value+');'

runner.CASES = {
 'empty': (PRELUDE+'try{cause_at(seed(),0u64);{}}catch(e:const IndexOutOfBoundsError){'+OUT('e.category')+OUT('e.requested')+OUT('e.available')+OUT('e.cause_count')+OUT('e.site!=0u64')+'};',0,'8\n0\n0\n0\ntrue\n',WARN),
 'uncaught_cleanup': (PRELUDE+'{{defer{'+OUT('9')+'};cause_at(seed(),0u64);}};',70,'9\n','uncaught IndexOutOfBoundsError'),
 'ordered_operands': (PRELUDE+'proc source()->const MissingReturnError{'+OUT('1')+'return seed();}proc index()->u64{'+OUT('2')+'return 3u64;}try{cause_at(source(),index());{}}catch(e:const IndexOutOfBoundsError){'+OUT('e.requested')+'};',0,'1\n2\n3\n',WARN),
 'valid_and_boundary': (PRELUDE+'val original<-seed();val history<-link(original,link(original,original));'+OUT('cause_at(history,0u64).cause_count')+OUT('cause_at(history,1u64).cause_count')+'try{cause_at(history,2u64);{}}catch(e:const IndexOutOfBoundsError){'+OUT('e.requested')+OUT('e.available')+'};',0,'1\n0\n2\n2\n',WARN),
 'maximum_index': (PRELUDE+'try{cause_at(seed(),18446744073709551615u64);{}}catch(e:const IndexOutOfBoundsError){'+OUT('e.requested')+OUT('e.available')+'};',0,'18446744073709551615\n0\n',WARN),
 'native_function_without_throws': (PRELUDE+'func at(e:Exception)->const Exception{return cause_at(e,0u64);}try{at(seed());{}}catch(e:const IndexOutOfBoundsError){'+OUT('e.category')+'};',0,'8\n',WARN),
 'generic_recursion': (PRELUDE+'func at(e:T,n:u64)->const Exception where T{if(n==0u64){return cause_at(e,1u64);}else{return at(e,n-1u64);};}try{at(seed(),30u64);{}}catch(e:const IndexOutOfBoundsError){'+OUT('e.requested')+'};',0,'1\n',WARN),
 'selected_callable': (PRELUDE+'func at(e:Exception)->const Exception{return cause_at(e,0u64);}func f=at;try{f(seed());{}}catch(e:const IndexOutOfBoundsError){'+OUT('e.category')+'};',0,'8\n',WARN),
 'deferred_invocation': (PRELUDE+'try{ {{defer{'+OUT('9')+'};defer cause_at(seed(),0u64);'+OUT('1')+'}};{}}catch(e:const IndexOutOfBoundsError){'+OUT('e.category')+'};',0,'1\n9\n8\n',WARN),
 'deferred_handler': (PRELUDE+'{{defer catch(e:const IndexOutOfBoundsError){'+OUT('e.available')+'}finally{'+OUT('9')+'};defer cause_at(seed(),1u64);}};',0,'0\n9\n',WARN),
 'preserve_pending_return': (PRELUDE+'func f()->u64{defer catch(e:const IndexOutOfBoundsError){};defer cause_at(seed(),1u64);return 7u64;}'+OUT('f()'),0,'7\n',WARN),
 'replace_pending_return': (PRELUDE+'func f()->u64{defer catch(e:const IndexOutOfBoundsError){return e.requested;};defer cause_at(seed(),3u64);return 7u64;}'+OUT('f()'),0,'3\n',WARN),
 'catch_initialization': (PRELUDE+'val x<-make local u64;try{cause_at(seed(),0u64);x<-1u64;}catch(e:const IndexOutOfBoundsError){x<-2u64;};'+OUT('x'),0,'2\n',WARN),
 'catch_cannot_hide_uninitialized': (PRELUDE+'val x<-make local u64;try{cause_at(seed(),0u64);x<-1u64;{}}catch(e:const IndexOutOfBoundsError){};'+OUT('x'),2,'','before definite initialization'),
 'deferred_result_write': (PRELUDE+'val x<-make local u64;{{defer catch(e:const IndexOutOfBoundsError){x<-2u64;};defer{cause_at(seed(),0u64);x<-1u64;};}};'+OUT('x'),0,'2\n',WARN),
 'finally_preserved': (PRELUDE+'try{cause_at(seed(),0u64);{}}catch(e:const IndexOutOfBoundsError){'+OUT('1')+'}finally{'+OUT('2')+'};',0,'1\n2\n',WARN),
 'finally_return': (PRELUDE+'func f()->u64{try{cause_at(seed(),0u64);return 1u64;}finally{return 2u64;}}'+OUT('f()'),0,'2\n',WARN),
 'type_query_unevaluated': (PRELUDE+'val e<-make local typeof(cause_at(seed(),999u64)){seed()};'+OUT('e.category'),0,'5\n',WARN),
 'intrinsic': (PRELUDE+'try{intrinsic (cause_at)(seed(),0u64);{}}catch(e:const IndexOutOfBoundsError){'+OUT('e.category')+'};',0,'8\n',WARN),
 'exception_category_selection': (PRELUDE+'func error()->const IndexOutOfBoundsError{try{cause_at(seed(),0u64);}catch(e:const IndexOutOfBoundsError){return e;};}func pick(e:Exception)->u64{return 1u64;}func pick(e:IndexOutOfBoundsError)->u64{return 2u64;}'+OUT('pick(error())')+'val common:Exception<-error();'+OUT('pick(common)'),0,'2\n1\n',WARN),
 'explicit_cause': (PRELUDE+'val original<-seed();try{try{cause_at(original,0u64);}catch(e:const IndexOutOfBoundsError){throw e from original;};{}}catch(e:const IndexOutOfBoundsError){'+OUT('e.cause_count')+OUT('cause_at(e,0u64).category')+'};',0,'1\n5\n',WARN),
 'early_operand_return': (PRELUDE+'func f()->u64{cause_at({{return 6u64;}},0u64);return 9u64;}'+OUT('f()'),0,'6\n',WARN),
 'early_index_return': (PRELUDE+'func f()->u64{cause_at(seed(),{{return 7u64;}});return 9u64;}'+OUT('f()'),0,'7\n',WARN),
 'unchecked_valid': (PRELUDE+'val e<-seed();'+OUT('cause_at(link(e,e),0u64).category'),0,'5\n',WARN),
 'unchecked_retains_numeric': (IO+'{{defer{'+OUT('9')+'};1/0;}};',70,'9\n','division by zero'),
 'unchecked_retains_npe': (TYPES+'ptr p:i64=null in here;p;',2,'','requires a covering catch'),
 'unchecked_retains_initialization': ('val x<-make local u64;x;',2,'','before definite initialization'),
 'unchecked_retains_index_type': (PRELUDE+'cause_at(seed(),true);',2,'','cause_at requires an exception and a u64 index'),
}
runner.CASES['imported_policy'] = ({'lib.cod':TYPES+SEED+'func at()->const Exception{return cause_at(seed(),0u64);}export at;',
 'main.cod':IO+TYPES+'using "lib.cod" at;try{at();{}}catch(e:const IndexOutOfBoundsError){'+OUT('e.category')+'};'},0,'8\n',WARN)
runner.CASE_ARGUMENTS = {name:['--no-bounds-checks'] for name in runner.CASES if name.startswith('unchecked_')}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
