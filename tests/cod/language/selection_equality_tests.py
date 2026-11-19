#!/usr/bin/env python3
"""Selected-reference equality survives observation, calls and boolean transport."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;using "std:exceptions" NullPointerException;'
LOCAL = 'val a<-make local u64,b<-make local u64,c<-make local u64;ptr p=if(flag){a}else{b};ptr q=if(other){a}else{b};'
CALLS = 'run(false,false);run(false,true);run(true,false);run(true,true);'
SAME = 'func same(ptr x:const u64 in O,ptr y:const u64 in O)->bool where {owner O}{return x is y;}'
RELAY = 'func relay(ptr x:u64 in O)->ptr u64 in O where {owner O}{return x;}'
WRITER = 'proc write_read(ptr x:u64 in O,ptr y:const u64 in O)->void throws NullPointerException where {owner O}{x<-7u64;behave @print(y);return;}'
runner.CASES = {}


def case(name, setup='', condition='p is q', work='p<-7u64;', read='q', definitions='',
         reject=False, output='7\n0\n0\n7\n', diagnostic='initialization', body=None, local=LOCAL):
    if body is None:
        body = 'if('+condition+'){'+work+'behave @print('+read+');}else{behave @print(0u64);};'
    source = IO+definitions+'proc run(flag:bool,other:bool)->void{'+local+'try{ '+setup+body+'}catch(e:const NullPointerException){behave @print(99u64);};return;}'+CALLS
    runner.CASES[name] = (source, 2 if reject else 0, '' if reject else output, diagnostic if reject else '')


case('direct')
case('reverse', condition='q is p')
case('negated_difference', condition='!(p is not q)')
case('explicit_call', condition='(is)(p,q)')
case('intrinsic_call', condition='intrinsic (is)(p,q)')
case('stored', setup='val equal<-p is q;', condition='equal')
case('copied_boolean', setup='val equal<-p is q;val saved<-equal;equal<-false;', condition='saved')
case('borrowed_boolean', setup='val equal<-p is q;ref saved:const bool=equal;', condition='saved')
case('changed_boolean', setup='val equal<-p is q;equal<-true;', condition='equal', reject=True)
case('disjoint_comparison', setup='ptr independent=c;', condition='p is independent', output='0\n'*4)
case('inequality_does_not_alias', condition='p is not q', reject=True)
case('disjunction_does_not_alias', condition='p is q||flag', reject=True)
case('conjunction', condition='p is q&&flag', output='0\n0\n0\n7\n')
case('reverse_conjunction', condition='flag&&p is q', output='0\n0\n0\n7\n')
case('stored_conjunction', setup='val equal<-p is q&&flag;', condition='equal', output='0\n0\n0\n7\n')
case('helper_conjunction', definitions=SAME, condition='same(p,q)&&flag', output='0\n0\n0\n7\n')
case('branch_evidence', condition='if(flag){p is q}else{q is p}')
case('transitive', setup='ptr third=if(flag){a}else{b};', condition='p is q&&q is third', read='third')
case('transitive_saved', setup='ptr third=if(flag){a}else{b};val first<-p is q;val second<-q is third;q=c;', condition='first&&second', read='third')
case('old_source', setup='ptr saved=p;val equal<-p is q;p=c;', condition='equal', work='saved<-7u64;')
case('old_target', setup='ptr saved=q;val equal<-p is q;q=c;', condition='equal', read='saved')
case('copy_after_observation', setup='val equal<-p is q;ptr saved=q;q=c;', condition='equal', read='saved')
case('new_source_is_independent', setup='val equal<-p is q;p=c;', condition='equal', reject=True)
case('new_target_is_independent', setup='val equal<-p is q;q=c;', condition='equal', reject=True)
case('ordered_operand_snapshot', setup='ptr saved=p;', condition='p is {p=c;q}', work='saved<-7u64;')
case('ordered_operand_changed_source', condition='p is {p=c;q}', reject=True)
case('weak_slot_change', setup='val equal<-p is q;var slot->if(flag){p}else{q};slot=c;', condition='equal', reject=True)
case('read_before_write', work='behave @print(q);p<-7u64;', reject=True)
case('failed_write', definitions='func fail()->u64 throws NullPointerException{ptr absent:u64=null in here;return absent;}', work='try{p<-fail();{}}catch(e:const NullPointerException){};', reject=True)
case('failed_registration', definitions='using "std:exceptions" OutOfMemoryError;', work='try{ {{defer{p<-7u64;};}};}catch(e:const OutOfMemoryError){};', reject=True)
case('ref_entry_requires_write', work='ref required=q;p<-7u64;', reject=True)
case('readonly_remains_readonly', setup='ptr readonly:const u64=p;', condition='readonly is q', work='readonly<-7u64;', reject=True, diagnostic='readonly')
case('helper', definitions=SAME, condition='same(p,q)')
case('generic_helper', definitions=SAME.replace('u64','T').replace('where {owner O}', 'where {T,owner O}'), condition='same(p,q)')
case('recursive_helper', definitions=SAME.replace(')->bool', ',n:u64)->bool').replace('return x is y;', 'if(n==0u64){return x is y;}else{return same(x,y,n-1u64);};'), condition='same(p,q,9u64)')
case('stored_helper', definitions=SAME, setup='val equal<-same(p,q);', condition='equal')
case('helper_alternative_without_equality', definitions=SAME+'func accept(ptr x:const u64 in O,ptr y:const u64 in O)->bool where {owner O}{return true;}', setup='func compare=if(flag){same}else{accept};', condition='compare(p,q)', reject=True)
case('callable_helper', definitions=SAME, setup='func compare=same;', condition='compare(p,q)')
case('captured_helper', definitions=SAME, setup='func compare[same](ptr x:const u64 in O,ptr y:const u64 in O)->bool where {owner O}{return same(x,y);};', condition='compare(p,q)')
case('helper_saved_before_rebind', definitions=SAME.replace('return x is y;', 'val equal<-x is y;x=y;return equal;'), condition='same(p,q)')
case('helper_compares_rebound_input', definitions=SAME.replace('return x is y;', 'x=y;return x is y;'), condition='same(p,q)', reject=True)
case('finally_keeps_comparison', definitions=SAME.replace('return x is y;', 'try{return x is y;}finally{x=y;{}};'), condition='same(p,q)')
case('finally_replaces_comparison', definitions=SAME.replace('return x is y;', 'try{return x is y;}finally{return true;}'), condition='same(p,q)', reject=True)
case('deferred_comparison', definitions=SAME.replace('return x is y;', 'defer{return x is y;};'), condition='same(p,q)')
case('returned_operand', definitions=RELAY, condition='relay(p) is q')
case('copied_call_arguments', definitions=WRITER, setup='q=p;', body='write_read(p,q);', output='7\n'*4)
case('returned_call_arguments', definitions=RELAY+WRITER, body='write_read(relay(p),p);', output='7\n'*4)
case('independent_call_arguments', definitions=WRITER, body='write_read(p,q);', reject=True)
case('call_reads_before_write', definitions=WRITER.replace('x<-7u64;behave @print(y);', 'behave @print(y);x<-7u64;'), setup='q=p;', body='write_read(p,q);', reject=True)
case('guarded_call_arguments', definitions=WRITER, work='write_read(p,q);', output='7\n7\n0\n0\n7\n7\n')
case('prepared_call_arguments', definitions=WRITER, setup='q=p;', body='{{defer write_read(p,q);}};', output='7\n'*4)
case('delayed_guard', body='{{defer{if(p is q){p<-7u64;behave @print(q);}else{behave @print(0u64);};};}};')
case('loop_guard', body='for(i in 0u64..2u64){if(p is q){p<-7u64;behave @print(q);}else{behave @print(0u64);};};', output='7\n7\n0\n0\n0\n0\n7\n7\n')

OUTPUT = 'func set(ref out:bool in B,ptr x:const u64 in O,ptr y:const u64 in O)->void where {owner B,owner O}{out<-x is y:void;}'
case('borrowed_output', definitions=OUTPUT, setup='val equal<-false;set(equal,p,q);', condition='equal')
case('deferred_borrowed_output', definitions=OUTPUT, setup='val equal<-false;{{defer set(equal,p,q);}};', condition='equal')
case('output_keeps_snapshot', definitions=OUTPUT, setup='val equal<-false;ptr saved=q;set(equal,p,q);q=c;', condition='equal', read='saved')
case('output_does_not_refine_new_target', definitions=OUTPUT, setup='val equal<-false;set(equal,p,q);q=c;', condition='equal', reject=True)
SELECT = 'func choose(ptr x:const u64 in O,ptr y:const u64 in O,ref fallback:const u64 in O)->ptr const u64 in O where {owner O}{if(x is y){return null in O;}else{return fallback;};}'
case('nullable_result_raw', definitions=SELECT, setup='val fallback<-9u64;', condition='choose(p,q,fallback) is null in here')
case('nullable_result_stored', definitions=SELECT, setup='val fallback<-9u64;ptr selected=choose(p,q,fallback);', condition='selected is null in here')
case('nullable_result_nonnull', definitions=SELECT.replace('return null in O;', 'return fallback;').replace('else{return fallback;}', 'else{return null in O;}'), setup='val fallback<-9u64;', condition='choose(p,q,fallback) is not null in here')
case('nullable_result_saved', definitions=SELECT, setup='val fallback<-9u64;ptr saved=q;val equal<-choose(p,q,fallback) is null in here;q=c;', condition='equal', read='saved')
case('nullable_result_changed_target', definitions=SELECT, setup='val fallback<-9u64;val equal<-choose(p,q,fallback) is null in here;q=c;', condition='equal', reject=True)
case('nullable_result_independent', definitions=SELECT.replace('x is y', 'true'), setup='val fallback<-9u64;', condition='choose(p,q,fallback) is null in here', reject=True)
for helper in (False, True):
    case('nullable'+('_helper' if helper else ''), definitions=SAME if helper else '', condition='same(p,q)' if helper else 'p is q',
         local='val a<-make local u64;ptr p=if(flag){a}else{null in here};ptr q=if(other){a}else{null in here};', output='99\n0\n0\n7\n')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
