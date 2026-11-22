#!/usr/bin/env python3
"""Caller-slot methods preserve complete callable targets and invocation evidence."""
from pathlib import Path
import foundation_tests as runner
from language_test_support import expect_source_rejections

IO = 'using symbol "std:io" @print;'
OUT = lambda value: 'behave @print('+value+');'
NPE = 'using "std:exceptions" NullPointerException;'
TYPE = 'func()->const u64'
VALUES = 'func one()->const u64{return 1u64;}func two()->const u64{return 2u64;}func three()->const u64{return 3u64;}'
BASE = VALUES+'func first=one;func second=two;func third=three;ptr p=first,q=second,r=third;'
def method(body, kind='func', mode='ref', throws=''):
    completion = 'return;' if kind == 'proc' else '{}'
    return kind+' (<=>)('+mode+' left:'+TYPE+' in O,'+mode+' right:'+TYPE+' in O)->void '+throws+'where {owner O}{'+body+completion+'}'
SWAP = method('intrinsic (<=>)(left,right);')
OBSERVE = method(OUT('left()')+'intrinsic (<=>)(left,right);','proc')
READERS = NPE+'func write(ptr x:i64 in O)->void throws NullPointerException where {owner O}{x<-7:void;}func read(ptr x:i64 in O)->void throws NullPointerException where {owner O}{x;{}}'
PROVE = 'func (<=>)(ref left:T in O,ref right:T in O)->void throws NullPointerException where {T,owner O}{val raw<-make local i64;left(raw);raw;{}}'
HOLDERS = 'func writer=write;func reader=read;ptr p=writer,q=writer;'
runner.CASES = {
 'swap_complete_targets': (IO+SWAP+BASE+'p<=>q;'+OUT('p()')+OUT('q()')+OUT('p is second'),0,'2\n1\ntrue\n',''),
 'explicit_named_call': (IO+SWAP+BASE+'(<=>)(p,q);'+OUT('p()')+OUT('q()'),0,'2\n1\n',''),
 'reload_after_operands': (IO+OBSERVE+BASE+'p<=>{p=third;q};'+OUT('p()')+OUT('q()'),0,'3\n2\n3\n',''),
 'ordered_selection': (IO+OBSERVE+BASE+'{'+OUT('7')+'p}<=>{'+OUT('8')+'q};'+OUT('p()'),0,'7\n8\n1\n2\n',''),
 'deferred_late_target': (IO+OBSERVE+BASE+'{{defer (<=>)(p,q);p=third:void;}};'+OUT('p()')+OUT('q()'),0,'3\n2\n3\n',''),
 'deferred_alias_selection': (IO+SWAP+BASE+'ptr var selected->if(true){p}else{q};{{defer (<=>)(selected,r);p=second:void;}};'+OUT('p()')+OUT('q()')+OUT('r()'),0,'3\n2\n2\n',''),
 'deferred_method_in_method': (IO+method('defer intrinsic (<=>)(left,right);')+BASE+'p<=>q;'+OUT('p()')+OUT('q()'),0,'2\n1\n',''),
 'nested_deferred_method': (IO+method('if(left()<right()){defer (<=>)(left,right);intrinsic (<=>)(left,right);}else{};')+BASE+'p<=>q;'+OUT('p()')+OUT('q()'),0,'2\n1\n',''),
 'deferred_late_payload': (IO+OBSERVE+BASE+'{{defer (<=>)(p,q);first<-three:void;}};'+OUT('p()')+OUT('q()'),0,'3\n2\n3\n',''),
 'imported_named_alias': ({'ops.cod':SWAP+'export (<=>);','main.cod':IO+'using "ops.cod" (<=>) as swap;'+BASE+'swap(p,q);'+OUT('p()')+OUT('q()')},0,'2\n1\n',''),
 'mixed_deferred_queue': (IO+SWAP+BASE+'{{defer (<=>)(p,q);defer intrinsic (=)(p,r);}};'+OUT('p()')+OUT('q()'),0,'2\n3\n',''),
 'self_slot': (IO+OBSERVE+BASE+'p<=>p;'+OUT('p()'),0,'1\n1\n',''),
 'local_helper_bindings': (IO+'func helper(ref left:'+TYPE+' in O,ref right:'+TYPE+' in O)->void where {owner O}{intrinsic (<=>)(left,right);}'+method('helper(left,right);')+BASE+'p<=>q;'+OUT('p()')+OUT('q()'),0,'1\n2\n',''),
 'payload_copy': (IO+method('func saved=left;left<-right;right<-saved;')+BASE+'p<=>q;'+OUT('first()')+OUT('second()')+OUT('p is first'),0,'2\n1\ntrue\n',''),
 'rebind_then_copy': (IO+method('left=right;left<-three;')+BASE+'p<=>q;'+OUT('first()')+OUT('second()')+OUT('p()'),0,'1\n3\n3\n',''),
 'call_rebound_formal': (IO+method('left=right;'+OUT('left()'),'proc')+BASE+'p<=>q;'+OUT('p()'),0,'2\n2\n',''),
 'recursive_method': (IO+method('if(left()<right()){intrinsic (<=>)(left,right);left<=>right;}else{};')+BASE+'p<=>q;'+OUT('p()')+OUT('q()'),0,'2\n1\n',''),
 'generic_method': (IO+'func (<=>)(ref left:T in O,ref right:T in O)->void where {T,owner O}{intrinsic (<=>)(left,right);}'+BASE+'p<=>q;val x<-4,y<-7;ptr a=x,b=y;a<=>b;'+OUT('p()')+OUT('a'),0,'2\n7\n',''),
 'captured_complete_values': (IO+SWAP+'func wrap(value:'+TYPE+')->'+TYPE+'{return func[value]()->const u64{return value()+10u64;};}'+BASE+'first<-wrap(one);second<-wrap(wrap(two));p<=>q;'+OUT('p()')+OUT('q()'),0,'22\n11\n',''),
 'independent_capture_snapshot': (IO+SWAP+'func wrap(value:'+TYPE+')->'+TYPE+'{return func[value]()->const u64{return value();};}'+BASE+'func saved=wrap(first);p<=>q;first<-three;'+OUT('saved()')+OUT('p()')+OUT('q()'),0,'1\n2\n3\n',''),
 'nullable_targets': (IO+method('intrinsic (<=>)(left,right);',mode='ptr')+BASE+'p=null;p<=>q;'+OUT('p is second')+OUT('q is null'),0,'true\ntrue\n',''),
 'default_entry_null_check': (IO+NPE+method(OUT('99'),'proc','')+BASE+'try{p<=>{p=null;q};}catch(e:const NullPointerException){'+OUT('8')+'};'+OUT('q is second'),0,'8\ntrue\n',''),
 'exception_after_rebind': (IO+NPE+method('left=right;right=null;right();','func','ptr','throws NullPointerException ')+BASE+'try{p<=>q;}catch(e:const NullPointerException){'+OUT('p is second')+OUT('q is null')+'};',0,'true\ntrue\n',''),
 'procedure_holders': (IO+'proc one()->void{'+OUT('1')+'return;}proc two()->void{'+OUT('2')+'return;}proc (<=>)(ref left:proc()->void in O,ref right:proc()->void in O)->void where {owner O}{left();intrinsic (<=>)(left,right);return;}proc first=one;proc second=two;ptr p=first,q=second;p<=>q;p();',0,'1\n2\n',''),
 'function_cannot_call_procedure': ('proc target()->void{}func (<=>)(ref left:proc()->void in O,ref right:proc()->void in O)->void where {owner O}{left();}proc holder=target;ptr p=holder;p<=>p;',2,'','function cannot invoke a procedure'),
 'fixed_slot': (SWAP+VALUES+'func first=one;const ptr p=first;ptr q=first;p<=>q;',2,'','const reference binding'),
 'different_owners': (SWAP+VALUES+'func first=one;ptr p=first;{{func second=two;ptr q=second;p<=>q;}};',2,'','initializer-fixed owner'),
 'readonly_write': ('func (<=>)(ref left:const '+TYPE+' in O,ref right:const '+TYPE+' in O)->void where {owner O}{left<-right:void;}'+VALUES+'func first=one;ptr p:const '+TYPE+'=first;p<=>p;',2,'','readonly destination'),
 'assignment_result_has_no_slot': (SWAP+BASE+'(p=q)<=>r;',2,'','binding slot'),
 'writer_proof': (READERS+PROVE+HOLDERS+'try{p<=>q;}catch(e:const NullPointerException){};',0,'',''),
 'late_reader_requires_initialization': (READERS+PROVE+HOLDERS+'try{p<=>{p=reader;q};}catch(e:const NullPointerException){};',2,'',"initialization of 'raw'"),
 'deferred_reader_requires_initialization': (READERS+PROVE+HOLDERS+'try{ {{defer (<=>)(p,q);p=reader;{} }}; }catch(e:const NullPointerException){};',2,'',"initialization of 'raw'"),
}
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

expect_source_rejections(runner, (
    'function_cannot_call_procedure',
), 'procedure signatures cannot occupy ordinary value or reference storage')

if __name__ == '__main__':
    raise SystemExit(runner.main())
