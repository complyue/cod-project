#!/usr/bin/env python3
"""Prepared assignments retain operands and publish mutations during cleanup."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
ERRORS = NPE + '''using "std:exceptions" Exception;
using "std:exceptions" MissingReturnError;
func missing()->i64{}
func first()->const Exception{try{missing();{}}catch(e:const MissingReturnError){return e;};}
func second()->const Exception{ptr p:i64=null in here;try{p;{}}catch(e:const NullPointerException){return e;};}
'''
OUT = lambda value: 'behave @print('+value+');'
runner.CASES = {}
for spelling in ('call', 'intrinsic'):
    def defer(op, left, right):
        return 'defer '+('intrinsic ' if spelling == 'intrinsic' else '')+'('+op+')('+left+','+right+');'
    cases = {
        'value_snapshot': (IO+'val a<-1,b<-7;{{'+defer('<-','a','b')+'b<-9;'+OUT('a')+'}};'+OUT('a'),0,'1\n7\n',''),
        'source_target_snapshot': (IO+'val a<-1,b<-7,c<-9;ptr p=a,q=b;{{'+defer('=','p','q')+'q=c;b<-8;'+OUT('p')+'}};'+OUT('p'),0,'1\n8\n',''),
        'unrelated_binding_facts': (IO+NPE+'val a<-1,b<-7;ptr p=a,q=b;{{'+defer('=','p','q')+'q=null;}};'+OUT('p')+'try{'+OUT('q')+'}catch(e:const NullPointerException){'+OUT('-1')+'};',0,'7\n-1\n',''),
        'unrelated_initialization': ('val a<-1,b<-7,c<-make local i64;ptr p=a,q=b;{{'+defer('=','p','q')+'q=c;}};q;',2,'','before definite initialization'),
        'borrowed_callable_result': (IO+'func identity(ref input:i64 in O)->ref i64 in O where {owner O}{return input;}val a<-1,b<-7;ptr p=a;func get=identity;{{'+defer('=','p','get(b)')+'b<-9;'+OUT('p')+'}};'+OUT('p'),0,'1\n9\n',''),
        'destination_snapshot': (IO+'val a<-1,b<-2;ptr p=a;{{'+defer('<-','p','7')+'p=b;}};'+OUT('a')+OUT('b')+OUT('p'),0,'7\n2\n2\n',''),
        'selected_slot': (IO+'val a<-1,b<-2,c<-7;ptr p=a,q=b;{{'+defer('=','(if(true){p}else{q})','c')+'}};'+OUT('p')+OUT('q'),0,'7\n2\n',''),
        'alias_slot': (IO+'val a<-1,b<-7;ptr p=a;ptr var alias->p;{{'+defer('=','alias','b')+'}};'+OUT('p'),0,'7\n',''),
        'once_in_order': (IO+'val a<-1;{{'+defer('<-','{'+OUT('1')+'a}','{'+OUT('2')+'7}')+OUT('3')+'}};'+OUT('a'),0,'1\n2\n3\n7\n',''),
        'lifo_writes': (IO+'val a<-1;{{'+defer('<-','a','7')+defer('<-','a','9')+'}};'+OUT('a'),0,'7\n',''),
        'lifo_rebinds': (IO+'val a<-1,b<-7,c<-9;ptr p=a,q=b;{{'+defer('=','p','q')+defer('=','q','c')+'}};'+OUT('p')+OUT('q'),0,'7\n9\n',''),
        'late_block': (IO+'val a<-1;{{defer{'+OUT('a')+'};'+defer('<-','a','7')+'}};',0,'7\n',''),
        'initialize_at_cleanup': (IO+'val a<-make local i64;{{'+defer('<-','a','7')+'}};'+OUT('a'),0,'7\n',''),
        'initialize_then_rebind': (IO+'val a<-1,b<-make local i64;ptr p=a;{{'+defer('=','p','b')+defer('<-','b','7')+'}};'+OUT('p'),0,'7\n',''),
        'guarded_late_read': (IO+'val a<-make local i64;{{defer{'+OUT('a')+'}finally{};'+defer('<-','a','7')+'}};',0,'7\n',''),
        'registration_failure_read': (IO+'val a<-make local i64;{{defer{'+OUT('a')+'};'+defer('<-','a','7')+'}};',2,'','cannot prove initialization'),
        'caught_registration_failure': (IO+'using "std:exceptions" OutOfMemoryError;val a<-make local i64;try{ {{'+defer('<-','a','7')+'}};{}}catch(e:const OutOfMemoryError){};'+OUT('a'),2,'','before definite initialization'),
        'ref_source_uninitialized': ('val a<-1,b<-make local i64;ref p=a;'+defer('=','p','b'),2,'','before definite initialization'),
        'widening': (IO+'val a<-1u64;{{'+defer('<-','a','7u8')+'}};'+OUT('a'),0,'7\n',''),
        'exception_category': (IO+ERRORS+'val dst:Exception<-first(),src:Exception<-second();{{'+defer('<-','dst','src')+'}};try{throw dst;}catch(e:const MissingReturnError){'+OUT('1')+'}catch(e:const NullPointerException){'+OUT('7')+'}catch(e){'+OUT('99')+'};',0,'7\n','[-Wmissing-return]'),
        'clear': (IO+'val a<-1;ptr p=a;{{'+defer('=','p','null')+'}};'+OUT('(p is null)'),0,'true\n',''),
        'restore': (IO+'val a<-7;ptr p:i64=null in here;{{'+defer('=','p','a')+'}};'+OUT('p'),0,'7\n',''),
        'skipped': (IO+'val a<-1;{{if(false){'+defer('<-','a','7')+'}else{};}};'+OUT('a'),0,'1\n',''),
        'loop_records': (IO+'val a<-9;{{for(i in 0..3){'+defer('<-','a','i')+'};'+OUT('a')+'}};'+OUT('a'),0,'9\n0\n',''),
        'pending_return': (IO+'proc run()->i64{val a<-1;defer{'+OUT('a')+'};'+defer('<-','a','7')+'return a;}'+OUT('run()'),0,'7\n1\n',''),
        'pending_failure': (IO+NPE+'val a<-1;ptr p:i64=null in here;try{ {{'+defer('<-','a','7')+'p;}};{}}catch(e:const NullPointerException){'+OUT('a')+'};',0,'7\n',''),
        'failed_source': (IO+NPE+'val a<-1,count<-0;ptr p:i64=null in here;try{ {{'+defer('<-','a','{count<-7;p}')+'}};{}}catch(e:const NullPointerException){'+OUT('a')+OUT('count')+'};',0,'1\n7\n',''),
        'null_destination_order': (IO+NPE+'ptr p:i64=null in here;try{ {{'+defer('<-','p','{'+OUT('99')+'7}')+'}};{}}catch(e:const NullPointerException){'+OUT('7')+'};',0,'7\n',''),
        'aborted_source': (IO+'proc run()->i64{val a<-1;defer{'+OUT('a')+'};'+defer('<-','a','{{return 7;}}')+'return 0;}'+OUT('run()'),0,'1\n7\n',''),
        'recursive_generic': (IO+'func fill(ref a:T in O,v:T,n:u64)->void where {T,owner O}{'+defer('<-','a','v')+'if(n>0u64){fill(a,v,n-1u64);}else{};}val x<-1,y<-2u64;fill(x,7,30u64);fill(y,8u64,40u64);'+OUT('x')+OUT('y'),0,'7\n8\n',''),
        'not_initialized_at_registration': ('val a<-make local i64;{{'+defer('<-','a','7')+'a;}};',2,'','before definite initialization'),
        'readonly': ('val a:const i64<-1;'+defer('<-','a','7'),2,'','readonly destination'),
        'fixed': ('val a<-1,b<-7;const ptr p=a;'+defer('=','p','b'),2,'','const reference binding'),
        'owner_mismatch': ('val a<-1;ptr p=a;{{val b<-7;'+defer('=','p','b')+'}};',2,'','initializer-fixed owner'),
        'bad_source_type': ('val a<-1;'+defer('<-','a','true'),2,'','source does not satisfy destination type'),
        'readonly_rebind': ('val a<-1;val b:const i64<-7;ptr p=a;'+defer('=','p','b'),2,'','readonly source cannot rebind'),
        'ended_binding': ('val a<-1,b<-7;'+defer('=','({{ptr p=a;p}})','b'),2,'','lexical scope has ended'),
        'source_uninitialized': ('val a<-1,b<-make local i64;'+defer('<-','a','b'),2,'','before definite initialization'),
    }
    for name, case in cases.items(): runner.CASES[spelling+'_'+name] = case
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
