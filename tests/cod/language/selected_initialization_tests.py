#!/usr/bin/env python3
"""Selected writes retain conditional initialization across cleanup alternatives."""
from pathlib import Path
import selection_equality_tests as fixture

runner = fixture.runner
runner.CASES = {}
SELECT = 'ptr selected=if(p is q){null in here}else{a};'
OOM = 'using "std:exceptions" OutOfMemoryError;'
BODY = '{{defer{if(GUARD){behave @print(READ);}else{behave @print(0u64);};};try{defer{WRITE};}catch(registration:const OutOfMemoryError){RECOVER{}};AFTER{} }};'


def case(name, write='p<-7u64;', recover='p<-7u64;', after='', read='q', guard='selected is null', setup=SELECT, definitions='', **kwargs):
    body = BODY.replace('GUARD',guard).replace('READ',read).replace('WRITE',write).replace('RECOVER',recover).replace('AFTER',after)
    fixture.case(name, setup=setup, definitions=OOM+definitions, body=body, **kwargs)


case('protected_registration')
case('protected_conditional_write', write='if(selected is null){p<-7u64;{}}else{};')
case('protected_conditional_recovery', recover='if(selected is null){p<-7u64;{}}else{};')
case('protected_both_conditional', write='if(selected is null){p<-7u64;{}}else{};', recover='if(selected is null){p<-7u64;{}}else{};')
case('direct_identity_guard', setup='', guard='p is q')
case('saved_guard', setup=SELECT+'val equal<-selected is null;', guard='equal')
case('saved_target', setup='ptr saved=q;'+SELECT, read='saved', after='q=c;')
case('changed_target', after='q=c;', reject=True)
case('changed_source', after='p=c;', reject=True)
case('changed_condition', after='selected=null;', reject=True)
case('recovery_no_write', recover='', reject=True)
case('recovery_other_target', recover='c<-7u64;', reject=True)
case('recovery_partial_write', recover='if(flag){p<-7u64;{}}else{};', reject=True)
case('action_no_write', write='', reject=True)
case('action_other_target', write='c<-7u64;', reject=True)
case('action_partial_write', write='if(flag){p<-7u64;{}}else{};', reject=True)
case('read_before_action_write', write='behave @print(q);p<-7u64;', reject=True)
case('read_before_recovery_write', recover='behave @print(q);p<-7u64;', reject=True)
case('weak_target_change', setup=SELECT+'var slot->if(flag){q}else{p};', after='slot=c;', reject=True)
case('unprotected_registration', recover='throw registration;', reject=True)
FAIL = 'func fail()->u64 throws NullPointerException{ptr absent:u64=null in here;return absent;}'
case('failed_action_write', definitions=FAIL, write='try{p<-fail();{}}catch(failure:const NullPointerException){};', reject=True)
case('failed_recovery_write', definitions=FAIL, recover='try{p<-fail();{}}catch(failure:const NullPointerException){};', reject=True)
SET = 'func set(ptr x:u64 in O)->void throws NullPointerException where {owner O}{x<-7u64;}'
case('call_writes', definitions=SET, write='try{set(p);}catch(failure){p<-7u64;{}};', recover='try{set(p);}catch(failure){p<-7u64;{}};')
case('generic_call_writes', definitions=SET.replace('u64','T').replace('x<-7T;','x<-value;').replace('in O)->','in O,value:T)->').replace('where {owner O}', 'where {T,owner O}'), write='try{set(p,7u64);}catch(failure){p<-7u64;{}};', recover='try{set(p,7u64);}catch(failure){p<-7u64;{}};')
case('conditional_call_writes', definitions=SET.replace('in O)->', 'in O,flag:bool)->').replace('x<-7u64;', 'if(flag){x<-7u64;{}}else{};'), write='try{set(p,selected is null);}catch(failure){p<-7u64;{}};', recover='try{set(p,selected is null);}catch(failure){p<-7u64;{}};')
case('callable_writes', definitions=SET, setup=SELECT+'func setter=set;', write='try{setter(p);}catch(failure){p<-7u64;{}};', recover='try{setter(p);}catch(failure){p<-7u64;{}};')

# A failed call may terminate without a catchable category. A guarded reader
# runs only on normal incoming completion; explicit recovery still writes.
for name in ('call_writes','generic_call_writes','conditional_call_writes','callable_writes'):
    source,code,output,diagnostic=runner.CASES[name]
    source=source.replace('};};try{defer', '};}catch(incoming){};try{defer',1)
    runner.CASES[name]=(source,code,output,diagnostic)

case('recovery_boolean_replaced', recover='if(flag){p<-7u64;{}}else{};flag<-true;', guard='selected is null&&flag', reject=True)
case('action_boolean_replaced', write='if(flag){p<-7u64;{}}else{};flag<-true;', guard='selected is null&&flag', reject=True)
case('saved_write_condition', setup=SELECT+'val gate<-flag;', guard='selected is null&&gate', write='if(gate){p<-7u64;{}}else{};', recover='if(gate){p<-7u64;{}}else{};', after='flag<-false;', output='0\n0\n0\n7\n')

case('saved_source', setup='ptr saved=p;'+SELECT, write='saved<-7u64;', recover='saved<-7u64;', after='p=c;')
case('saved_rhs_target', setup='ptr saved=p;'+SELECT, write='saved<-{p=c;7u64};', recover='saved<-{p=c;7u64};')
case('rhs_changes_read_target', write='p<-{q=c;7u64};', recover='p<-{q=c;7u64};', reject=True)
case('retained_guard', setup=SELECT+'val saved<-selected is null;', guard='saved', after='selected=null;')
case('unconditional_reader', guard='true', reject=True)
case('independent_guard', setup='ptr selected=if(flag){null in here}else{a};', reject=True)
case('readonly_write', setup=SELECT+'ptr frozen:const u64=p;', write='frozen<-7u64;', reject=True, diagnostic='readonly')

for name in ('protected_registration', 'protected_both_conditional', 'saved_target', 'recovery_no_write', 'action_no_write'):
    source, code, output, diagnostic = runner.CASES[name]
    source=source.replace('{{defer', 'for(i in 0u64..2u64){ {{defer',1)
    source=source.replace('{} }};}catch(e:', '{} }};};}catch(e:')
    runner.CASES['loop_'+name]=(source,code,'' if code else ''.join(line+'\n'+line+'\n' for line in output.splitlines()),diagnostic)

for name, after, rejected in (('prepared_target','',False),('prepared_saved_target','p=c;',False),('prepared_changed_reader','q=c;',True)):
    body=BODY.replace('GUARD','selected is null').replace('READ','q').replace('defer{WRITE}', 'defer (<-)(p,7u64)').replace('RECOVER','p<-7u64;').replace('AFTER',after)
    fixture.case(name,setup=SELECT,definitions=OOM,body=body,reject=rejected)

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
