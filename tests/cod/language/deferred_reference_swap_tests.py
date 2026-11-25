#!/usr/bin/env python3
"""Deferred binding exchange follows invocation-time targets and validity proofs."""
from pathlib import Path
import foundation_tests as runner
from language_test_support import observe_after_cleanup

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
OUT = lambda value: 'behave @print('+value+');'
BASE = 'val a<-1,b<-7,c<-9;ptr p=a,q=b;'
PAIR = OUT('p')+OUT('q')
TRIPLE = PAIR+OUT('r')
REF_PTR = 'val a<-1,b<-7,c<-9;ref p=a;ptr q=b;'
NULLABLE = 'val a<-1,b<-7;ref p=a;ptr q=if(flag){b}else{null};'

def exchange_program(setup, work, observation=PAIR, declarations=''):
    body = observe_after_cleanup(work, observation)
    return IO+declarations+'proc run()->void{'+setup+body+'return;}run();'


def flagged_program(setup, work, observation):
    body = observe_after_cleanup(work, observation)
    return IO+'proc run(flag:bool)->void{'+setup+body+'return;}run(true);run(false);'


SELECT_AFTER_CLEANUP = ('proc select(ref first:i64 in O,ref next:i64 in O)->ref i64 in O where {owner O}'
                        '{defer{return next;};return first;}')
ABSENT = 'ptr absent:i64=null in here;'
CHECKED_RUN = 'proc run()->void throws NullPointerException{'
CATCH_RUN = 'return;}try{run();}catch(e:const NullPointerException){};'
runner.CASES = {}
for spelling in ('call', 'intrinsic'):
    def swap(left='p', right='q'):
        return 'defer '+('intrinsic ' if spelling == 'intrinsic' else '')+'(<=>)('+left+','+right+');'
    cases = {
        'current_targets': (exchange_program(BASE,swap()+'q=c;'),0,'9\n1\n',''),
        'no_early_exchange': (exchange_program(BASE,swap()+PAIR),0,'1\n7\n7\n1\n',''),
        'selected_slot': (exchange_program(BASE+'ptr r=c;val flag<-true;',
            swap('{if(flag){p}else{q}}','r')+'flag<-false;',TRIPLE),0,'9\n7\n1\n',''),
        'selected_alias': (exchange_program(BASE+'ptr r=c;val flag<-true;ptr var chosen->{if(flag){p}else{q}};',
            swap('chosen','r')+'flag<-false;p=b;',TRIPLE),0,'9\n7\n7\n',''),
        'temporary_alias': (exchange_program(BASE+'ptr r=c;',
            swap('{ptr var chosen->{if(true){p}else{q}};chosen}','r'),TRIPLE),0,'9\n7\n1\n',''),
        'same_slot': (exchange_program(BASE,swap('p','p')+'p=c;'),0,'9\n7\n',''),
        'same_slot_alias': (exchange_program(BASE+'ptr var alias->p;',swap('alias','p')+'p=c;',OUT('p')),0,'9\n',''),
        'nullable_slots': (exchange_program(BASE,swap()+'q=null;',OUT('(p is null)')+OUT('q')),0,'true\n1\n',''),
        'null_at_registration': (exchange_program('val a<-7;ptr p:i64=null in here,q=a;',swap(),
            OUT('p')+OUT('(q is null)')),0,'7\ntrue\n',''),
        'once_in_order': (exchange_program(BASE,swap('{'+OUT('1')+'p}','{'+OUT('2')+'q}')+OUT('3')),0,'1\n2\n3\n7\n1\n',''),
        'operand_rebind': (exchange_program(BASE,swap('p','{p=c;q}')),0,'7\n9\n',''),
        'operand_cleanup': (exchange_program(BASE,swap('{p=select(p,c);p}','q'),declarations=SELECT_AFTER_CLEANUP),0,'7\n9\n',''),
        'queued_rebind': (exchange_program(BASE,swap()+'defer (=)(q,c);'),0,'9\n1\n',''),
        'later_rebind': (exchange_program(BASE,'defer (=)(q,c);'+swap()),0,'7\n9\n',''),
        'twice': (exchange_program(BASE,swap()+swap()),0,'1\n7\n',''),
        'loop_queue': (exchange_program(BASE,'for(i in 0..3){'+swap()+'};'),0,'7\n1\n',''),
        'late_observer': (IO+'proc run()->void{'+BASE+'defer{'+PAIR+'};'+swap()+'q=c;return;}run();',0,'9\n1\n',''),
        'ref_pair': (exchange_program('val a<-1,b<-7,c<-9;ref p=a,q=b;',swap()+'q=c;'),0,'9\n1\n',''),
        'ref_ptr_valid': (exchange_program(REF_PTR,swap()+'q=c;'),0,'9\n1\n',''),
        'ref_ptr_restored': (exchange_program(REF_PTR,swap()+'q=null;q=c;'),0,'9\n1\n',''),
        'ref_ptr_cleanup_restore': (exchange_program(REF_PTR,swap()+'defer (=)(q,c);q=null;'),0,'9\n1\n',''),
        'ref_ptr_cleanup_clear_restore': (exchange_program(REF_PTR,swap()+'defer (=)(q,c);defer (=)(q,null);'),0,'9\n1\n',''),
        'conditional_nullable_queue': (flagged_program(NULLABLE,'if(!(q is null)){'+swap()+'}else{};',OUT('p')),0,'7\n1\n',''),
        'conditional_guard_restored': (flagged_program('val c<-9;'+NULLABLE,
            'if(!(q is null)){'+swap()+'defer (=)(q,c);q=null;{}}else{};',OUT('p')),0,'9\n1\n',''),
        'loop_guard_iteration_cleanup': (IO+
            'func step(ref left:i64 in O,ref right:i64 in O,flag:bool)->ref i64 in O where {owner O}'
            '{ref p=left;ptr q=if(flag){right}else{null in O};defer{return p;};'
            'if(!(q is null)){'+swap()+'}else{};return p;}'
            'val a<-1,b<-7;ref selected=a;for(i in 0..2){selected=step(selected,b,i==0);};'+OUT('selected'),0,'7\n',''),
        'ref_ptr_cleanup_initialize': (exchange_program('val a<-1,b<-7,c<-make local i64;ref p=a;ptr q=b;',
            swap()+'defer (<-)(c,9);q=c;'),0,'9\n1\n',''),
        'ref_ptr_late_uninitialized': ('val a<-1,b<-7,c<-make local i64;ref p=a;ptr q=b;'+swap()+'q=c;',2,'','cannot prove initialization'),
        'before_return': (IO+'proc run()->i64{'+BASE+'defer{'+OUT('p')+OUT('q')+'};'+swap()+'return p;}'+OUT('run()'),0,'7\n1\n1\n',''),
        'pending_failure': (IO+NPE+CHECKED_RUN+BASE+ABSENT+observe_after_cleanup(swap(),PAIR)+'absent;'+CATCH_RUN,0,'7\n1\n',''),
        'failed_operand': (IO+NPE+CHECKED_RUN+BASE+ABSENT+'defer{'+PAIR+'};'+swap('p','{absent;q}')+CATCH_RUN,0,'1\n7\n',''),
        'skipped': (exchange_program(BASE,'if(false){'+swap()+'}else{};'),0,'1\n7\n',''),
        'fixed_slot': ('val a<-1,b<-7;const ptr p=a;ptr q=b;'+swap(),2,'','const reference binding'),
        'owner_mismatch': ('val a<-1;ptr p=a;{{val b<-7;ptr q=b;'+swap()+'}};',2,'','initializer-fixed owner'),
        'readonly_mismatch': ('val a<-1;val b:const i64<-7;ptr p=a,q=b;'+swap(),2,'','readonly source cannot rebind'),
        'exact_type': ('val a<-1,b<-7u64;ptr p=a,q=b;'+swap(),2,'','exact target type'),
        'value_not_slot': ('val a<-1,b<-7;'+swap('a','b'),2,'','val storage has no reference binding'),
        'ended_slot': ('ref a:i64=1,b:i64=7;ptr q=b;'+swap('{{ptr p=a;return p;}}','q'),2,'','storage destination'),
    }
    for name, case in cases.items(): runner.CASES[spelling+'_'+name] = case
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__), Path(__file__).with_name('language_test_support.py')]
if __name__ == '__main__':
    raise SystemExit(runner.main())
