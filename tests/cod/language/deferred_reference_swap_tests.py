#!/usr/bin/env python3
"""Deferred binding exchange follows invocation-time targets and validity proofs."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
OUT = lambda value: 'behave @print('+value+');'
BASE = 'val a<-1,b<-7,c<-9;ptr p=a,q=b;'
runner.CASES = {}
for spelling in ('call', 'intrinsic'):
    def swap(left='p', right='q'):
        return 'defer '+('intrinsic ' if spelling == 'intrinsic' else '')+'(<=>)('+left+','+right+');'
    cases = {
        'current_targets': (IO+BASE+'{{'+swap()+'q=c;}};'+OUT('p')+OUT('q'),0,'9\n1\n',''),
        'no_early_exchange': (IO+BASE+'{{'+swap()+OUT('p')+OUT('q')+'}};'+OUT('p')+OUT('q'),0,'1\n7\n7\n1\n',''),
        'selected_slot': (IO+BASE+'ptr r=c;val flag<-true;{{'+swap('{if(flag){p}else{q}}','r')+'flag<-false;}};'+OUT('p')+OUT('q')+OUT('r'),0,'9\n7\n1\n',''),
        'selected_alias': (IO+BASE+'ptr r=c;val flag<-true;ptr var chosen->{if(flag){p}else{q}};{{'+swap('chosen','r')+'flag<-false;p=b;}};'+OUT('p')+OUT('q')+OUT('r'),0,'9\n7\n7\n',''),
        'temporary_alias': (IO+BASE+'ptr r=c;{{'+swap('({{ptr var chosen->{if(true){p}else{q}};chosen}})','r')+'}};'+OUT('p')+OUT('q')+OUT('r'),0,'9\n7\n1\n',''),
        'same_slot': (IO+BASE+'{{'+swap('p','p')+'p=c;}};'+OUT('p')+OUT('q'),0,'9\n7\n',''),
        'same_slot_alias': (IO+BASE+'ptr var alias->p;{{'+swap('alias','p')+'p=c;}};'+OUT('p'),0,'9\n',''),
        'nullable_slots': (IO+BASE+'{{'+swap()+'q=null;}};'+OUT('(p is null)')+OUT('q'),0,'true\n1\n',''),
        'null_at_registration': (IO+'val a<-7;ptr p:i64=null in here,q=a;{{'+swap()+'}};'+OUT('p')+OUT('(q is null)'),0,'7\ntrue\n',''),
        'once_in_order': (IO+BASE+'{{'+swap('{'+OUT('1')+'p}','{'+OUT('2')+'q}')+OUT('3')+'}};'+OUT('p')+OUT('q'),0,'1\n2\n3\n7\n1\n',''),
        'operand_rebind': (IO+BASE+'{{'+swap('p','{p=c;q}')+'}};'+OUT('p')+OUT('q'),0,'7\n9\n',''),
        'operand_cleanup': (IO+BASE+'{{'+swap('({{defer{p=c;};p}})','q')+'}};'+OUT('p')+OUT('q'),0,'7\n9\n',''),
        'queued_rebind': (IO+BASE+'{{'+swap()+'defer (=)(q,c);}};'+OUT('p')+OUT('q'),0,'9\n1\n',''),
        'later_rebind': (IO+BASE+'{{defer (=)(q,c);'+swap()+'}};'+OUT('p')+OUT('q'),0,'7\n9\n',''),
        'twice': (IO+BASE+'{{'+swap()+swap()+'}};'+OUT('p')+OUT('q'),0,'1\n7\n',''),
        'loop_queue': (IO+BASE+'{{for(i in 0..3){'+swap()+'};}};'+OUT('p')+OUT('q'),0,'7\n1\n',''),
        'late_observer': (IO+BASE+'{{defer{'+OUT('p')+OUT('q')+'};'+swap()+'q=c;}};',0,'9\n1\n',''),
        'ref_pair': (IO+'val a<-1,b<-7,c<-9;ref p=a,q=b;{{'+swap()+'q=c;}};'+OUT('p')+OUT('q'),0,'9\n1\n',''),
        'ref_ptr_valid': (IO+'val a<-1,b<-7,c<-9;ref p=a;ptr q=b;{{'+swap()+'q=c;}};'+OUT('p')+OUT('q'),0,'9\n1\n',''),
        'ref_ptr_restored': (IO+'val a<-1,b<-7,c<-9;ref p=a;ptr q=b;{{'+swap()+'q=null;q=c;}};'+OUT('p')+OUT('q'),0,'9\n1\n',''),
        'ref_ptr_cleanup_restore': (IO+'val a<-1,b<-7,c<-9;ref p=a;ptr q=b;{{'+swap()+'defer (=)(q,c);q=null;}};'+OUT('p')+OUT('q'),0,'9\n1\n',''),
        'ref_ptr_cleanup_clear_restore': (IO+'val a<-1,b<-7,c<-9;ref p=a;ptr q=b;{{'+swap()+'defer (=)(q,c);defer (=)(q,null);}};'+OUT('p')+OUT('q'),0,'9\n1\n',''),
        'ref_ptr_cleanup_null': ('val a<-1,b<-7;ref p=a;ptr q=b;{{'+swap()+'defer (=)(q,null);}};',2,'','cannot prove a non-null target'),
        'conditional_nullable_queue': (IO+'proc run(flag:bool)->void{val a<-1,b<-7;ref p=a;ptr q=if(flag){b}else{null};{{if(!(q is null)){'+swap()+'}else{};}};'+OUT('p')+'}run(true);run(false);',0,'7\n1\n',''),
        'conditional_guard_body_null': ('proc run(flag:bool)->void{val a<-1,b<-7;ref p=a;ptr q=if(flag){b}else{null};{{if(!(q is null)){'+swap()+'}else{};q=null;}};}run(true);',2,'','cannot prove a non-null target'),
        'conditional_guard_cleanup_null': ('proc run(flag:bool)->void{val a<-1,b<-7;ref p=a;ptr q=if(flag){b}else{null};{{if(!(q is null)){'+swap()+'defer (=)(q,null);}else{};}};}run(true);',2,'','cannot prove a non-null target'),
        'conditional_guard_swapped_null': ('proc run(flag:bool)->void{val a<-1,b<-7;ref p=a;ptr q=if(flag){b}else{null},r:i64=null in here;{{if(!(q is null)){'+swap()+swap('q','r')+'}else{};}};}run(true);',2,'','cannot prove a non-null target'),
        'conditional_guard_restored': (IO+'proc run(flag:bool)->void{val a<-1,b<-7,c<-9;ref p=a;ptr q=if(flag){b}else{null};{{if(!(q is null)){'+swap()+'defer (=)(q,c);q=null;{}}else{};}};'+OUT('p')+'}run(true);run(false);',0,'9\n1\n',''),
        'loop_guard_escapes_iteration': ('val a<-1,b<-7;ref p=a;ptr q=b;{{for(i in 0..2){q=if(i==0){b}else{null};if(!(q is null)){'+swap()+'}else{};};}};',2,'','cannot prove a non-null target'),
        'loop_guard_iteration_cleanup': (IO+'val a<-1,b<-7;ref p=a;ptr q=b;for(i in 0..2){ {{q=if(i==0){b}else{null};if(!(q is null)){'+swap()+'}else{};}};};'+OUT('p'),0,'7\n',''),
        'ref_ptr_cleanup_initialize': (IO+'val a<-1,b<-7,c<-make local i64;ref p=a;ptr q=b;{{'+swap()+'defer (<-)(c,9);q=c;}};'+OUT('p')+OUT('q'),0,'9\n1\n',''),
        'ref_ptr_late_null': ('val a<-1,b<-7;ref p=a;ptr q=b;{{'+swap()+'q=null;}};',2,'','cannot prove a non-null target'),
        'ref_ptr_late_uninitialized': ('val a<-1,b<-7,c<-make local i64;ref p=a;ptr q=b;{{'+swap()+'q=c;}};',2,'','cannot prove initialization'),
        'ref_ptr_failed_restore': ('using "std:exceptions" OutOfMemoryError;val a<-1,b<-7,c<-9;ref p=a;ptr q=b;{{'+swap()+'q=null;try{defer (=)(q,c);}catch(e:const OutOfMemoryError){};}};',2,'','cannot prove a non-null target'),
        'before_return': (IO+'proc run()->i64{'+BASE+'defer{'+OUT('p')+OUT('q')+'};'+swap()+'return p;}'+OUT('run()'),0,'7\n1\n1\n',''),
        'pending_failure': (IO+NPE+BASE+'ptr absent:i64=null in here;try{ {{'+swap()+'absent;}};{}}catch(e:const NullPointerException){'+OUT('p')+OUT('q')+'};',0,'7\n1\n',''),
        'failed_operand': (IO+NPE+BASE+'ptr absent:i64=null in here;try{ {{'+swap('p','{absent;q}')+'}};{}}catch(e:const NullPointerException){'+OUT('p')+OUT('q')+'};',0,'1\n7\n',''),
        'skipped': (IO+BASE+'{{if(false){'+swap()+'}else{};}};'+OUT('p')+OUT('q'),0,'1\n7\n',''),
        'fixed_slot': ('val a<-1,b<-7;const ptr p=a;ptr q=b;'+swap(),2,'','const reference binding'),
        'owner_mismatch': ('val a<-1;ptr p=a;{{val b<-7;ptr q=b;'+swap()+'}};',2,'','initializer-fixed owner'),
        'readonly_mismatch': ('val a<-1;val b:const i64<-7;ptr p=a,q=b;'+swap(),2,'','readonly source cannot rebind'),
        'exact_type': ('val a<-1,b<-7u64;ptr p=a,q=b;'+swap(),2,'','exact target type'),
        'value_not_slot': ('val a<-1,b<-7;'+swap('a','b'),2,'','val storage has no reference binding'),
        'ended_slot': ('val a<-1,b<-7;ptr q=b;'+swap('({{ptr p=a;p}})','q'),2,'','lexical scope has ended'),
    }
    for name, case in cases.items(): runner.CASES[spelling+'_'+name] = case
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
