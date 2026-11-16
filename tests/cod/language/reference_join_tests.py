#!/usr/bin/env python3
"""Nullable reference joins preserve each incoming path's evidence."""
from pathlib import Path
import selection_equality_tests as fixture

runner = fixture.runner
runner.CASES = {}
CHOICE = 'if(p is q){null in here}else{a}'
SELECT = 'ptr selected='+CHOICE+';'


def case(name, setup=SELECT, condition='selected is null', **kwargs):
    fixture.case(name, setup=setup, condition=condition, **kwargs)


case('conditional')
case('raw_conditional', setup='', condition='('+CHOICE+') is null')
case('nonnull_result', setup='ptr selected=if(p is q){a}else{null in here};', condition='selected is not null')
case('negated_null', condition='!(selected is not null)')
case('independent_condition', setup='ptr selected=if(flag){null in here}else{a};', reject=True)
case('both_null', setup='ptr selected:u64=if(p is q){null in here}else{null in here};', reject=True)
case('both_nonnull', setup='ptr selected=if(p is q){a}else{b};', condition='selected is not null', reject=True)
case('copied_result', setup=SELECT+'ptr saved=selected;selected=a;', condition='saved is null')
case('saved_boolean', setup=SELECT+'val ready<-selected is null;selected=a;', condition='ready')
case('replaced_boolean', setup=SELECT+'val ready<-selected is null;ready<-true;', condition='ready', reject=True)
case('replaced_result', setup=SELECT+'selected=null;', reject=True)
case('weak_result_update', setup=SELECT+'ptr alternate=selected;var slot->if(flag){selected}else{alternate};slot=null;', reject=True)
case('old_target', setup='ptr saved=q;'+SELECT+'q=c;', read='saved')
case('changed_target', setup=SELECT+'q=c;', reject=True)
case('old_source', setup='ptr saved=p;'+SELECT+'p=c;', work='saved<-7u64;')
case('changed_source', setup=SELECT+'p=c;', reject=True)
case('conjunction', condition='selected is null&&flag', output='0\n0\n0\n7\n')
case('reverse_conjunction', condition='flag&&selected is null', output='0\n0\n0\n7\n')
case('disjunction', condition='selected is null||flag', reject=True)
case('nested_selection', setup='ptr selected=if(flag){'+CHOICE+'}else{if(q is p){null in here}else{b}};')
case('nested_independent_selection', setup='ptr selected=if(flag){'+CHOICE+'}else{null in here};', reject=True)
case('scoped_result', setup='ptr absent:u64=null in here;ptr selected={{if(p is q){absent}else{a}}};')
case('finally_result', setup='ptr selected=try{'+CHOICE+'}finally{{}};')
case('finally_keeps_old_target', setup='ptr saved=q;ptr selected=try{'+CHOICE+'}finally{q=c;{}};', read='saved')
case('finally_changed_target', setup='ptr selected=try{'+CHOICE+'}finally{q=c;{}};', reject=True)
HANDLED = 'ptr absent:u64=null in here;ptr selected=try{if(p is q){absent;{}}else{};a}catch(e:const NullPointerException){absent};'
case('handler_result', setup=HANDLED)
case('handler_saved_target', setup='ptr saved=q;'+HANDLED+'q=c;', read='saved')
case('handler_changed_target', setup=HANDLED+'q=c;', reject=True)
case('handler_independent_failure', setup=HANDLED.replace('if(p is q)', 'if(flag)'), reject=True)
case('handler_nonnull_result', setup=HANDLED.replace(';a}catch', ';absent}catch').replace('Exception){absent};', 'Exception){a};'), condition='selected is not null')
case('deferred_guard', setup=SELECT, body='{{defer{if(selected is null){p<-7u64;behave @print(q);}else{behave @print(0u64);};};}};')
case('deferred_result_change', setup=SELECT, body='{{defer{if(selected is null){p<-7u64;behave @print(q);}else{behave @print(0u64);};};selected=null;{} }};', reject=True)
DEFERRED = '{{defer{if(selected is null){p<-7u64;behave @print(q);}else{behave @print(0u64);};};AFTER{} }};'
case('deferred_changed_target', body=DEFERRED.replace('AFTER', 'q=c;'), reject=True)
case('deferred_changed_source', body=DEFERRED.replace('AFTER', 'p=c;'), reject=True)
case('deferred_other_action_changes_target', body=DEFERRED.replace('AFTER', 'defer{q=c;{}};'), reject=True)
case('deferred_saved_target', setup='ptr saved=q;'+SELECT, body=DEFERRED.replace('AFTER', 'q=c;').replace('@print(q)', '@print(saved)'))
case('deferred_no_write', body=DEFERRED.replace('AFTER', '').replace('p<-7u64;', ''), reject=True)
case('deferred_raw_join', setup='ptr absent:u64=null in here;', body=DEFERRED.replace('AFTER', '').replace('selected is null', '(if(p is q){absent}else{a}) is null'))
case('deferred_rebinds_to_source', body=DEFERRED.replace('AFTER', 'q=p;'))
case('deferred_rebinds_to_target', body=DEFERRED.replace('AFTER', 'p=q;'))
case('deferred_rebinds_both', body=DEFERRED.replace('AFTER', 'p=c;q=c;'))
case('deferred_weak_change', setup=SELECT+'var slot->if(flag){p}else{q};', body=DEFERRED.replace('AFTER', 'slot=c;'), reject=True)
case('deferred_rebind_before_write', body=DEFERRED.replace('AFTER', '').replace('p<-7u64;', 'p=c;p<-7u64;'), reject=True)
case('deferred_failed_write', definitions='func fail()->u64 throws NullPointerException{ptr absent:u64=null in here;return absent;}', body=DEFERRED.replace('AFTER', '').replace('p<-7u64;', 'try{p<-fail();{}}catch(e:const NullPointerException){};'), reject=True)
case('prior_action_registration_can_fail', body=DEFERRED.replace('p<-7u64;', '').replace('AFTER', 'defer{p<-7u64;};'), reject=True)
case('prior_action_handles_registration_failure', definitions='using "std:exceptions" OutOfMemoryError;', body=DEFERRED.replace('p<-7u64;', '').replace('};AFTER', '}catch(allocation:const OutOfMemoryError){};AFTER').replace('AFTER', 'defer{p<-7u64;};'))
case('failed_registration', definitions='using "std:exceptions" OutOfMemoryError;', work='try{ {{defer{p<-7u64;};}};}catch(e:const OutOfMemoryError){};', reject=True)
case('loop_guard', setup=SELECT, body='for(i in 0u64..2u64){if(selected is null){p<-7u64;behave @print(q);}else{behave @print(0u64);};};', output='7\n7\n0\n0\n0\n0\n7\n7\n')
case('loop_changes_result', setup=SELECT+'for(i in 0u64..2u64){selected=null;};', reject=True)
case('read_before_write', work='behave @print(q);p<-7u64;', reject=True)
case('ref_entry_requires_write', work='ref required=q;p<-7u64;', reject=True)
case('readonly', setup=SELECT+'ptr readonly:const u64=p;', work='readonly<-7u64;', reject=True, diagnostic='readonly')

JOIN = 'func choose(ptr x:const u64 in O,ptr y:const u64 in O,ref fallback:const u64 in O)->ptr const u64 in O where {owner O}{return if(x is y){null in O}else{fallback};}'
case('returned_conditional', definitions=JOIN, setup='val fallback<-9u64;ptr selected=choose(p,q,fallback);')
case('raw_returned_conditional', definitions=JOIN, setup='val fallback<-9u64;', condition='choose(p,q,fallback) is null')
case('generic_result', definitions=JOIN.replace('u64','T').replace('where {owner O}', 'where {T,owner O}'), setup='val fallback<-9u64;ptr selected=choose(p,q,fallback);')
case('recursive_result', definitions=JOIN.replace(')->ptr', ',n:u64)->ptr').replace('return if', 'if(n>0u64){return choose(x,y,fallback,n-1u64);}else{};return if'), setup='val fallback<-9u64;ptr selected=choose(p,q,fallback,8u64);')
case('generic_recursive_result', definitions=JOIN.replace('u64','T').replace('where {owner O}', 'where {T,owner O}').replace(')->ptr', ',n:u64)->ptr').replace('return if', 'if(n>0u64){return choose(x,y,fallback,n-1u64);}else{};return if'), setup='val fallback<-9u64;ptr selected=choose(p,q,fallback,8u64);')
MUTUAL = JOIN.replace(')->ptr', ',n:u64)->ptr')
case('mutual_result', definitions=MUTUAL.replace('return if', 'if(n>0u64){return forward(x,y,fallback,n-1u64);}else{};return if')+MUTUAL.replace('choose(', 'forward(').replace('return if(x is y){null in O}else{fallback};', 'return choose(x,y,fallback,n);'), setup='val fallback<-9u64;ptr selected=choose(p,q,fallback,8u64);')
case('recursive_independent_result', definitions=JOIN.replace(')->ptr', ',n:u64)->ptr').replace('return if', 'if(n>0u64){return choose(x,y,fallback,n-1u64);}else{};return if').replace('x is y', 'true'), setup='val fallback<-9u64;ptr selected=choose(p,q,fallback,8u64);', reject=True)
case('callable_result', definitions=JOIN, setup='val fallback<-9u64;func call=choose;ptr selected=call(p,q,fallback);')
case('declared_failure', definitions='using "std:exceptions" Exception;'+JOIN.replace('in O where', 'in O throws Exception where'), setup='val fallback<-9u64;ptr selected=choose(p,q,fallback);', reject=True, diagnostic='declared checked exception')
case('borrowed_boolean_output', definitions='func absent(ref ready:bool in B,ptr candidate:const u64 in O)->void where {owner B,owner O}{ready<-candidate is null;}', setup=SELECT+'val ready<-false;absent(ready,selected);', condition='ready')

runner.CASES['empty_recursive_result_keeps_declared_failure'] = (
    'using "std:exceptions" Exception;'
    'func spin(ptr x:const u64 in O)->ptr const u64 in O throws Exception where {owner O}{return spin(x);}'
    'proc unhandled(ptr x:const u64 in O)->void where {owner O}{ptr saved=spin(x);}'
    'val item<-1u64;if(false){unhandled(item);}else{};',
    2, '', 'declared checked exception')

for name, write, after, rejected in (
    ('boolean_write_path', 'target<-7u64;', '', False),
    ('boolean_no_write', '', '', True),
    ('boolean_changed_input', 'target<-7u64;', 'flag<-false;', False),
    ('boolean_replaced_selection', 'target<-7u64;', 'selected=null;', True),
):
    source = fixture.IO+'proc run(flag:bool)->void{val target<-make local u64;'
    source += 'ptr selected=if(flag){'+write+'null in here}else{target};'+after
    source += 'if(selected is null){behave @print(target);}else{behave @print(0u64);};}run(false);run(true);'
    runner.CASES[name] = (source, 2 if rejected else 0, '' if rejected else '0\n7\n', 'initialization' if rejected else '')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
