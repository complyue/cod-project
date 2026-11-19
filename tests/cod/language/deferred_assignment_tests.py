#!/usr/bin/env python3
"""Prepared assignments retain operands and publish mutations during cleanup."""
from pathlib import Path
from itertools import product
import foundation_tests as runner
from language_test_support import observe_after_cleanup

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
ERRORS = NPE + '''using "std:exceptions" Exception;
using "std:exceptions" MissingReturnError;
func missing()->i64{}
func first()->const Exception{try{missing();{}}catch(e:const MissingReturnError){return e;};}
func second()->const Exception{ptr p:i64=null in here;try{p;{}}catch(e:const NullPointerException){return e;};}
'''
OUT = lambda value: 'behave @print('+value+');'

def assignment_program(setup, work, observation, declarations=''):
    body = observe_after_cleanup(work, observation)
    return IO+declarations+'proc run()->void{'+setup+body+'}run();'


runner.CASES = {}
for spelling in ('call', 'intrinsic'):
    def defer(op, left, right):
        return 'defer '+('intrinsic ' if spelling == 'intrinsic' else '')+'('+op+')('+left+','+right+');'
    cases = {
        'value_snapshot': (IO+'ref a:i64=1,b:i64=7;{{'+defer('<-','a','b')+'b<-9;'+OUT('a')+'}};'+OUT('a'),0,'1\n7\n',''),
        'source_target_snapshot': (assignment_program('val a<-1,b<-7,c<-9;ptr p=a,q=b;',
            defer('=','p','q')+'q=c;b<-8;'+OUT('p'),OUT('p')),0,'1\n8\n',''),
        'unrelated_binding_facts': (assignment_program('val a<-1,b<-7;ptr p=a,q=b;',
            defer('=','p','q')+'q=null;',OUT('p')+'try{'+OUT('q')+'}catch(e:const NullPointerException){'+OUT('-1')+'};',NPE),
            0,'7\n-1\n',''),
        'unrelated_initialization': (assignment_program('val a<-1,b<-7,c<-make local i64;ptr p=a,q=b;',
            defer('=','p','q')+'q=c;','q;{}'),2,'',"cannot prove initialization of 'c'"),
        'borrowed_callable_result': (assignment_program('val a<-1,b<-7;ptr p=a;func get=identity;',
            defer('=','p','get(b)')+'b<-9;'+OUT('p'),OUT('p'),
            'func identity(ref input:i64 in O)->ref i64 in O where {owner O}{return input;}'),0,'1\n9\n',''),
        'destination_snapshot': (assignment_program('val a<-1,b<-2;ptr p=a;',
            defer('<-','p','7')+'p=b;',OUT('a')+OUT('b')+OUT('p')),0,'7\n2\n2\n',''),
        'selected_slot': (assignment_program('val a<-1,b<-2,c<-7;ptr p=a,q=b;',
            defer('=','{if(true){p}else{q}}','c'),OUT('p')+OUT('q')),0,'7\n2\n',''),
        'alias_slot': (assignment_program('val a<-1,b<-7;ptr p=a;ptr var alias->p;',
            defer('=','alias','b'),OUT('p')),0,'7\n',''),
        'once_in_order': (IO+'ref a:i64=1;{{'+defer('<-','{'+OUT('1')+'a}','{'+OUT('2')+'7}')+OUT('3')+'}};'+OUT('a'),0,'1\n2\n3\n7\n',''),
        'lifo_writes': (IO+'ref a:i64=1;{{'+defer('<-','a','7')+defer('<-','a','9')+'}};'+OUT('a'),0,'7\n',''),
        'lifo_rebinds': (assignment_program('val a<-1,b<-7,c<-9;ptr p=a,q=b;',
            defer('=','p','q')+defer('=','q','c'),OUT('p')+OUT('q')),0,'7\n9\n',''),
        'late_block': (IO+'val a<-1;{{defer{'+OUT('a')+'};'+defer('<-','a','7')+'}};',0,'7\n',''),
        'initialize_at_cleanup': (assignment_program('val a<-make local i64;',defer('<-','a','7'),OUT('a')),0,'7\n',''),
        'initialize_then_rebind': (assignment_program('val a<-1,b<-make local i64;ptr p=a;',
            defer('=','p','b')+defer('<-','b','7'),OUT('p')),0,'7\n',''),
        'guarded_late_read': (IO+'proc run()->void{val a<-make local i64;val ready<-false;'
            'defer{if(ready){'+OUT('a')+'}else{};}finally{};'+defer('<-','a','7')+'ready<-true;}run();',0,'7\n',''),
        'registration_failure_read': (IO+'val a<-make local i64;defer{'+OUT('a')+'};'+defer('<-','a','7'),2,'','cannot prove initialization'),
        'caught_registration_failure': (IO+'using "std:exceptions" OutOfMemoryError;val a<-make local i64;try{ {{'+defer('<-','a','7')+'}};{}}catch(e:const OutOfMemoryError){};'+OUT('a'),2,'','before definite initialization'),
        'ref_source_uninitialized': ('val a<-1,b<-make local i64;ref p=a;'+defer('=','p','b'),2,'','before definite initialization'),
        'widening': (IO+'ref a:u64=1u64;{{'+defer('<-','a','7u8')+'}};'+OUT('a'),0,'7\n',''),
        'exception_category': (assignment_program('val dst:Exception<-first(),src:Exception<-second();',defer('<-','dst','src'),
            'try{throw dst;}catch(e:const MissingReturnError){'+OUT('1')+'}catch(e:const NullPointerException){'
            +OUT('7')+'}catch(e){'+OUT('99')+'};',ERRORS),0,'7\n','[-Wmissing-return]'),
        'clear': (assignment_program('val a<-1;ptr p=a;',defer('=','p','null'),OUT('(p is null)')),0,'true\n',''),
        'restore': (assignment_program('val a<-7;ptr p:i64=null in here;',defer('=','p','a'),OUT('p')),0,'7\n',''),
        'restore_observer_armed_before_registration': (IO+'proc run()->void{val a<-7;ptr p:i64=null in here;val ready<-true;'
            'defer{if(ready){'+OUT('p')+'}else{};};'+defer('=','p','a')+'}run();',2,'',"checked exception 'NullPointerException'"),
        'observer_disarmed_during_cleanup': (IO+'proc run()->void{ptr p:i64=null in here;val ready<-false;'
            'defer{if(ready){'+OUT('p')+'}else{};};'+defer('<-','ready','false')+'ready<-true;}run();',0,'',''),
        'skipped': (IO+'ref a:i64=1;{{if(false){'+defer('<-','a','7')+'}else{};}};'+OUT('a'),0,'1\n',''),
        'loop_records': (IO+'ref a:i64=9;{{for(i in 0..3){'+defer('<-','a','i')+'};'+OUT('a')+'}};'+OUT('a'),0,'9\n0\n',''),
        'pending_return': (IO+'proc run()->i64{val a<-1;defer{'+OUT('a')+'};'+defer('<-','a','7')+'return a;}'+OUT('run()'),0,'7\n1\n',''),
        'pending_failure': (IO+NPE+'ref a:i64=1;ptr p:i64=null in here;try{ {{'+defer('<-','a','7')+'p;}};{}}catch(e:const NullPointerException){'+OUT('a')+'};',0,'7\n',''),
        'failed_source': (IO+NPE+'ref a:i64=1,count:i64=0;ptr p:i64=null in here;try{ {{'+defer('<-','a','{count<-7;p}')+'}};{}}catch(e:const NullPointerException){'+OUT('a')+OUT('count')+'};',0,'1\n7\n',''),
        'null_destination_order': (IO+NPE+'ptr p:i64=null in here;try{ {{'+defer('<-','p','{'+OUT('99')+'7}')+'}};{}}catch(e:const NullPointerException){'+OUT('7')+'};',0,'7\n',''),
        'aborted_source': (IO+'proc run()->i64{val a<-1;defer{'+OUT('a')+'};'+defer('<-','a','{return 7;}')+'return 0;}'+OUT('run()'),0,'1\n7\n',''),
        'recursive_generic': (IO+'func fill(ref a:T in O,v:T,n:u64)->void where {T,owner O}{'+defer('<-','a','v')+'if(n>0u64){fill(a,v,n-1u64);}else{};}val x<-1,y<-2u64;fill(x,7,30u64);fill(y,8u64,40u64);'+OUT('x')+OUT('y'),0,'7\n8\n',''),
        'not_initialized_at_registration': ('val a<-make local i64;{{'+defer('<-','a','7')+'a;}};',2,'',"cannot prove initialization of 'a'"),
        'readonly': ('val a:const i64<-1;'+defer('<-','a','7'),2,'','readonly destination'),
        'fixed': ('val a<-1,b<-7;const ptr p=a;'+defer('=','p','b'),2,'','const reference binding'),
        'owner_mismatch': ('val a<-1;ptr p=a;{{val b<-7;'+defer('=','p','b')+'}};',2,'','initializer-fixed owner'),
        'bad_source_type': ('val a<-1;'+defer('<-','a','true'),2,'','source does not satisfy destination type'),
        'readonly_rebind': ('val a<-1;val b:const i64<-7;ptr p=a;'+defer('=','p','b'),2,'','readonly source cannot rebind'),
        'ended_binding': ('ref a:i64=1,b:i64=7;'+defer('=','{{ptr p=a;return p;}}','b'),2,'','storage destination'),
        'source_uninitialized': ('val a<-1,b<-make local i64;'+defer('<-','a','b'),2,'','before definite initialization'),
    }
    for name, case in cases.items(): runner.CASES[spelling+'_'+name] = case
    # Rebinding changes slots; no action in this model initializes a payload.
    for number, (destination, initialized, observer) in enumerate(product(
            ('p', 'q', 'destination'), (False, True), ('p', 'q', 'r', 'c'))):
        cells = {'a': 1, 'b': 7, 'c': 9 if initialized else None}
        bindings = {'p': 'a', 'q': 'b', 'r': 'b'}
        saved_slot = 'p' if destination == 'destination' else destination
        saved_target = bindings['q']
        bindings.update(q='c', r='c')
        bindings[saved_slot] = saved_target
        value = cells[bindings.get(observer, observer)]
        setup = ('val a<-1,b<-7,c<-'+('9' if initialized else 'make local i64')+
                 ';ptr p=a,q=b,r=q;ptr var destination->p;')
        work = defer('=', destination, 'q')+'q=c;r=c;'
        source = assignment_program(setup,work,OUT(observer))
        runner.CASES[spelling+'_initialization_model_'+str(number)] = (
            source, 2 if value is None else 0, '' if value is None else str(value)+'\n',
            "cannot prove initialization of 'c'" if value is None else '')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__), Path(__file__).with_name('language_test_support.py')]
if __name__ == '__main__':
    raise SystemExit(runner.main())
