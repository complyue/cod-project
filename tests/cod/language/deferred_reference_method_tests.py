#!/usr/bin/env python3
"""Prepared reference methods retain slots and check invocation-time target state."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
OUT = lambda value: 'behave @print('+value+');'
BASE = 'val x<-1,y<-7,z<-9;ptr p=x,q=y;'
PTR = 'func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void where {owner O}{intrinsic (<=>)(a,b);}'
REF = PTR.replace('ptr a:', 'ref a:').replace('ptr b:', 'ref b:')
READ = 'proc (<=>)(a:i64 in O,b:i64 in O)->void where {owner O}{'+OUT('a')+OUT('b')+'}'
CALL = 'defer (<=>)(p,q);'
runner.CASES = {
 'current_slots': (IO+PTR+BASE+'{{'+CALL+'q=z;}};'+OUT('p')+OUT('q'),0,'9\n1\n',''),
 'no_early_effect': (IO+PTR+BASE+'{{'+CALL+OUT('p')+OUT('q')+'}};'+OUT('p')+OUT('q'),0,'1\n7\n7\n1\n',''),
 'late_null': (IO+PTR+BASE+'{{'+CALL+'q=null;}};'+OUT('(p is null)')+OUT('q'),0,'true\n1\n',''),
 'null_initially': (IO+NPE+PTR+BASE+'p=null;{{'+CALL+'}};try{'+OUT('p')+OUT('(q is null)')+'}catch(e:const NullPointerException){'+OUT('-1')+'};',0,'7\ntrue\n',''),
 'selected_binding': (IO+PTR+BASE+'ptr r=z;val flag<-true;{{defer (<=>)((if(flag){p}else{q}),r);flag<-false;p=y;}};'+OUT('p')+OUT('q')+OUT('r'),0,'9\n7\n7\n',''),
 'temporary_alias': (IO+PTR+BASE+'{{defer (<=>)(({{ptr var chosen->p;chosen}}),q);q=z;}};'+OUT('p')+OUT('q'),0,'9\n1\n',''),
 'ordered_operands': (IO+REF+BASE+'{{defer (<=>)({'+OUT('1')+'p},{'+OUT('2')+'p=z;q});'+OUT('3')+'}};'+OUT('p')+OUT('q'),0,'1\n2\n3\n7\n9\n',''),
 'operand_cleanup': (IO+REF+BASE+'{{defer (<=>)(({{defer{p=z;};p}}),q);}};'+OUT('p')+OUT('q'),0,'7\n9\n',''),
 'same_slot': (IO+PTR+BASE+'{{defer (<=>)(p,p);p=z;}};'+OUT('p')+OUT('q'),0,'9\n7\n',''),
 'queued_rebind': (IO+PTR+BASE+'{{'+CALL+'defer (=)(q,z);}};'+OUT('p')+OUT('q'),0,'9\n1\n',''),
 'queued_twice': (IO+PTR+BASE+'{{'+CALL+CALL+'}};'+OUT('p')+OUT('q'),0,'1\n7\n',''),
 'loop_queue': (IO+REF+BASE+'{{for(i in 0..3){'+CALL+'};}};'+OUT('p')+OUT('q'),0,'7\n1\n',''),
 'observer': (IO+NPE+PTR+BASE+'{{defer{try{'+OUT('p')+OUT('q')+'}catch(e:const NullPointerException){'+OUT('-1')+'};};'+CALL+'q=z;}};',0,'9\n1\n',''),
 'ref_late_null': (REF+BASE+'{{'+CALL+'q=null;}};',2,'','cannot prove a non-null target'),
 'ref_late_uninitialized': (REF+BASE+'val empty<-make local i64;{{'+CALL+'q=empty;}};',2,'','cannot prove initialization'),
 'ref_cleanup_initialize': (IO+REF+BASE+'val empty<-make local i64;{{'+CALL+'defer (<-)(empty,9);q=empty;}};'+OUT('p'),0,'9\n',''),
 'ref_cleanup_restore': (IO+REF+BASE+'{{'+CALL+'defer (=)(q,z);q=null;}};'+OUT('p'),0,'9\n',''),
 'read_late_null': (IO+NPE+READ+BASE+'try{ {{'+CALL+'p=null;}};{}}catch(e:const NullPointerException){'+OUT('8')+'};',0,'8\n',''),
 'read_null_registration': (IO+NPE+READ+BASE+'p=null;try{ {{'+CALL+'p=z;}};{}}catch(e:const NullPointerException){'+OUT('8')+'};',0,'8\n',''),
 'read_late_uninitialized': (IO+READ+BASE+'val empty<-make local i64;{{'+CALL+'q=empty;}};',2,'','cannot prove initialization'),
 'read_body_uninitialized': (NPE+'func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void throws NullPointerException where {owner O}{b;}'+BASE+'val empty<-make local i64;try{ {{'+CALL+'q=empty;}};{}}catch(e:const NullPointerException){};',2,'','cannot prove initialization'),
 'pending_return': (IO+REF+'proc run()->i64{'+BASE+'defer{'+OUT('p')+OUT('q')+'};'+CALL+'return p;}'+OUT('run()'),0,'7\n1\n1\n',''),
 'recover_entry_failure': (IO+NPE+READ+'proc run()->i64{'+BASE+'defer catch(e:const NullPointerException){'+OUT('8')+'};'+CALL+'p=null;return 3;}'+OUT('run()'),0,'8\n3\n',''),
 'pending_error': (IO+NPE+PTR+BASE+'try{ {{'+CALL+'ptr absent:i64=null in here;absent;}};{}}catch(e:const NullPointerException){'+OUT('p')+OUT('q')+'};',0,'7\n1\n',''),
 'failed_operand': (IO+NPE+PTR+BASE+'ptr absent:i64=null in here;try{ {{defer (<=>)(p,{absent;q});}};{}}catch(e:const NullPointerException){'+OUT('p')+OUT('q')+'};',0,'1\n7\n',''),
 'ended_slot': (PTR+BASE+'defer (<=>)(({{ptr r=x;r}}),q);',2,'','lexical scope has ended'),
 'fixed_slot': (PTR+'val x<-1,y<-7;const ptr p=x;ptr q=y;'+CALL,2,'','const reference binding'),
 'ref_permission': (PTR+'val x<-1,y<-7;ref p=x,q=y;'+CALL,0,'',''),
 'late_tls': (IO+'symbol @amount:i64;proc (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void where {owner O}{'+OUT('@amount')+'}'+BASE+'effect @amount<-1;{{'+CALL+'effect @amount<-7;}};',0,'7\n',''),
}
for name, body, result, output, diagnostic in (
 ('write_then_rebind','a<-3;a=b;', 'p;{}', '', 'before definite initialization'),
 ('entry_write','a<-3;a=b;',OUT('z'),'3\n',''),
 ('rebind_then_write','a=b;a<-3;',OUT('p'),'3\n',''),
):
    source=IO+NPE+'func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void throws NullPointerException where {owner O}{'+body+'}'
    source+='val x<-1,y<-make local i64,z<-make local i64;ptr p=x,q=y;try{ {{'+CALL+'p=z;}};'+result+'}catch(e:const NullPointerException){};'
    runner.CASES[name]=(source,2 if diagnostic else 0,output,diagnostic)
for order in ('method_first','builtin_first'):
    calls = 'run(1,7);run(false,true);' if order=='method_first' else 'run(false,true);run(1,7);'
    source=IO+READ+'proc run(a:T,b:T)->void where T{val x<-a,y<-b;ptr p=x,q=y;{{'+CALL+'}};}'+calls
    runner.CASES['mixed_context_'+order]=(source,0,'1\n7\n','')
runner.CASES.update({
 'method_failure_after_rebind': (IO+NPE+'func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void throws NullPointerException where {owner O}{a=null;b;}'+BASE+'try{ {{'+CALL+'q=null;}};{}}catch(e:const NullPointerException){'+OUT('(p is null)')+'};',0,'true\n',''),
 'method_ref_recursion': (IO+'func (<=>)(ref a:i64 in O,ref b:i64 in O)->void where {owner O}{if(a<b){intrinsic (<=>)(a,b);defer (<=>)(a,b);}else{};}'+BASE+'{{'+CALL+'}};'+OUT('p')+OUT('q'),0,'7\n1\n',''),
 'const_targets': (IO+REF.replace('a:i64','a:const i64').replace('b:i64','b:const i64')+'val x:const i64<-1,y:const i64<-7;ptr p=x,q=y;{{'+CALL+'}};'+OUT('p'),0,'7\n',''),
 'separate_owners': (IO+'func (<=>)(ptr a:i64 in O,ptr b:bool in P)->void where {owner O,owner P}{a=null;b=null;}val x<-1;ptr p=x;{{val y<-true;ptr q=y;{{'+CALL+'}};'+OUT('(q is null)')+'}};'+OUT('(p is null)'),0,'true\ntrue\n',''),
 'different_owner_rebind_rejected': ('func (<=>)(ptr a:i64 in O,ptr b:i64 in P)->void where {owner O,owner P}{a=b;}val x<-1;ptr p=x;{{val y<-7;ptr q=y;'+CALL+'}};',2,'','initializer-fixed owner'),
 'read_cleanup_initialize': (IO+READ+BASE+'val empty<-make local i64;{{'+CALL+'defer (<-)(empty,9);q=empty;}};',0,'1\n9\n',''),
 'body_read_cleanup_initialize': (IO+NPE+'proc (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void throws NullPointerException where {owner O}{'+OUT('b')+'}'+BASE+'val empty<-make local i64;try{ {{'+CALL+'defer (<-)(empty,9);q=empty;}};{}}catch(e:const NullPointerException){};',0,'9\n',''),
})
runner.CASES['failed_initializer_registration'] = (IO+READ+BASE+'val empty<-make local i64;{{'+CALL+'q=empty;defer (<-)(empty,9);}};',2,'','cannot prove initialization')
errors = NPE+'using "std:exceptions" MissingReturnError;using "std:exceptions" Exception;func missing()->i64{}func missing_error()->const MissingReturnError{try{missing();}catch(e:const MissingReturnError){return e;};}func null_error()->const NullPointerException{ptr p:i64=null in here;try{p;{}}catch(e:const NullPointerException){return e;};}'
runner.CASES['exception_targets'] = (IO+errors+REF.replace('a:i64','a:Exception').replace('b:i64','b:Exception')+'val x:Exception<-missing_error(),y:Exception<-null_error();ptr p=x,q=y;{{'+CALL+'}};'+OUT('p.category==y.category')+OUT('q.category==x.category'),0,'true\ntrue\n','[-Wmissing-return]')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
