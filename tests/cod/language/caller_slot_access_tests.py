#!/usr/bin/env python3
"""Caller binding methods preserve physical target access through readonly aliases."""
from itertools import product
from pathlib import Path
import foundation_tests as runner

IO='using symbol "std:io" @print;using "std:exceptions" NullPointerException;'
HEAD='func (<=>)(ptr left:const i64 in O,ptr right:const i64 in O)->void where {owner O}'
BASE='val x<-1,y<-7,z<-9;ptr p=x,q=y;ptr var ro:const->p;ptr var other:const->q;'
OUT='behave @print(p);behave @print(q);'
runner.CASES={}


def case(name,body='',setup=BASE,call='ro<=>other;',after=OUT,output='1\n7\n',reject=False,definition=None,diagnostic='permission preservation'):
    source=IO+(HEAD+'{ '+body+'{};}' if definition is None else definition)+setup+'try{ '+call+after+'}catch(e:const NullPointerException){behave @print(99);};'
    runner.CASES[name]=(source,2 if reject else 0,'' if reject else output,diagnostic if reject else '')


case('readonly_alias_noop')
case('readonly_alias_swap',body='intrinsic (<=>)(left,right);',output='7\n1\n')
case('readonly_alias_rebind',body='left=right;',output='7\n7\n')
case('saved_target_restore',body='ptr saved=left;left=right;left=saved;')
case('parameter_alias_write',body='ptr var saved:const->left;saved=right;',output='7\n7\n')
case('same_physical_slot',body='intrinsic (<=>)(left,right);',call='ro<=>ro;')
case('readonly_physical_source_noop',setup=BASE.replace('ptr p=x,q=y;','ptr p=x;ptr q:const i64=y;'))
case('readonly_physical_source_rebind',body='left=right;',setup=BASE.replace('ptr p=x,q=y;','ptr p=x;ptr q:const i64=y;'),reject=True)
case('readonly_physical_source_swap',body='intrinsic (<=>)(left,right);',setup=BASE.replace('ptr p=x,q=y;','ptr p=x;ptr q:const i64=y;'),reject=True)
case('restoration_cannot_hide_bad_write',body='ptr saved=left;left=right;left=saved;',setup=BASE.replace('ptr p=x,q=y;','ptr p=x;ptr q:const i64=y;'),reject=True)
runner.CASES['prepared_keeps_binding_slots'] = (
    IO+HEAD+'{intrinsic (<=>)(left,right);}proc run()->void{'+BASE+
    'defer{'+OUT+'};defer (<=>)(ro,other);p=z;return;}run();',0,'7\n9\n','')
runner.CASES['prepared_bad_source'] = (
    IO+HEAD+'{left=right:void;}proc run()->void{'+BASE.replace('ptr p=x,q=y;','ptr p=x;ptr q:const i64=y;')+
    'defer (<=>)(ro,other);}run();',2,'','permission preservation')
case('deferred_action_swap',body='defer intrinsic (<=>)(left,right);',output='7\n1\n')
case('deferred_bad_write_restored',body='ptr saved=left;defer intrinsic (=)(left,saved);left=right;',setup=BASE.replace('ptr p=x,q=y;','ptr p=x;ptr q:const i64=y;'),reject=True)
case('delayed_body_swap',body='defer{intrinsic (<=>)(left,right);};',output='7\n1\n')
case('readonly_alias_cannot_write_payload',body='left<-3;',reject=True,diagnostic='readonly destination')
case('fixed_alias',setup=BASE.replace('ptr var ro:const->p;','const ptr var ro:const->p;'),reject=True,diagnostic='const reference binding')
case('fixed_physical_binding',setup=BASE.replace('ptr p=x,q=y;','const ptr p=x;ptr q=y;'),reject=True,diagnostic='const reference binding')
case('missing_initialization_unchanged',setup=BASE.replace('x<-1','x<-make local i64'),reject=True,diagnostic='initialization')
case('owner_mismatch', setup='val x<-1;ptr p:const i64=x;',
     call='{{val y<-7;ptr q:const i64=y;ptr var ro:const->p;ptr var other:const->q;ro<=>other;}};',
     after='{}', reject=True, diagnostic='owner')
GENERIC=HEAD.replace('i64','T').replace('where {owner O}','where {T,owner O}')
case('generic_alias_swap',definition=GENERIC+'{intrinsic (<=>)(left,right);}',output='7\n1\n')
case('null_input',body='intrinsic (<=>)(left,right);',setup=BASE+'q=null;',output='99\n')
case('selected_destination',body='intrinsic (<=>)(left,right);',call=' { if(x==1){ro}else{other} } <=>other;',output='7\n1\n')
case('selected_bad_source',body='left=right;',setup=BASE.replace('ptr p=x,q=y;','ptr p=x;ptr q:const i64=y;'),call='ro<=> { if(x==1){ro}else{other} } ;',reject=True)
case('finally_restoration_bad_source',definition=HEAD.replace(' where',' throws NullPointerException where')+'{ptr saved=left;try{left=right;left;{}}finally{left=saved;{}};}',setup=BASE.replace('ptr p=x,q=y;','ptr p=x;ptr q:const i64=y;'),reject=True)
CALLABLE='func()->const u64'
case('callable_payload_targets',definition=HEAD.replace('i64',CALLABLE)+'{intrinsic (<=>)(left,right);}',setup='func one()->const u64{return 1u64;}func two()->const u64{return 7u64;}func first=one;func second=two;ptr p=first,q=second;ptr var ro:const->p;ptr var other:const->q;',after='behave @print(p());behave @print(q());',output='7\n1\n')
case('explicit_alias_call',body='intrinsic (<=>)(left,right);',call='(<=>)(ro,other);',output='7\n1\n')
case('signature_access_stays_exact',body='left=right;',call='(<=>)(p,q);',output='7\n1\n')
# Separate same-type methods create an actual method-summary dependency while
# retaining the caller-binding convention for each invocation.
INNER=HEAD.replace('i64','u64')+'{intrinsic (<=>)(left,right);}'
WRAPPER='func bridge(ptr a:const u64 in O,ptr b:const u64 in O)->void where {owner O}{a<=>b;}'
case('ordinary_helper_local_slots',definition=INNER+WRAPPER,setup=BASE.replace('1,','1u64,').replace('7,','7u64,').replace('9;','9u64;'),call='bridge(ro,other);')
for body_name,body in [('swap','intrinsic (<=>)(left,right);'),('restore','ptr saved=left;left=right;left=saved:void;')]:
    for readonly in (False,True):
        setup=BASE if not readonly else BASE.replace('ptr p=x,q=y;','ptr p=x;ptr q:const i64=y;')
        main=IO+'using "./inner.cod" (<=>) as inner;'+HEAD+'{inner(left,right);}'+setup+'try{ro<=>other;'+OUT+'}catch(e:const NullPointerException){behave @print(99);};'
        runner.CASES['nested_'+body_name+('_bad' if readonly else '')]=({'inner.cod':HEAD+'{'+body+'}export (<=>);','main.cod':main},2 if readonly else 0,'' if readonly else ('7\n1\n' if body_name=='swap' else '1\n7\n'),'permission preservation' if readonly else '')
RECURSIVE=HEAD.replace(' where',' throws NullPointerException where')+'{if(left==0){intrinsic (<=>)(left,right);}else{(<=>)(right,left);};}'
case('recursive_swap',definition=RECURSIVE,setup=BASE.replace('y<-7','y<-0'),output='0\n1\n')
case('recursive_bad_source',definition=RECURSIVE,setup=BASE.replace('y<-7','y<-0').replace('ptr p=x,q=y;','ptr p=x;ptr q:const i64=y;'),reject=True)

# Each parameter may retain its own target or acquire the other. Compute the
# physical access obligation over every intermediate selection independently.
for number,(p_readonly,q_readonly,first,restore) in enumerate(product((False,True),repeat=4)):
    access=[p_readonly,q_readonly]
    history=[{0},{1}]
    body='ptr saved=left;'
    if first:
        history[0].add(1);body+='left=right;'
    else:
        history[1].add(0);body+='right=left;'
    if restore:
        history[0].add(0);body+='left=saved;'
    safe=all(access[d] or all(not access[s] for s in sources) for d,sources in enumerate(history))
    targets=[0,1]
    if first:targets[0]=targets[1]
    else:targets[1]=targets[0]
    if restore:targets[0]=0
    setup='val x<-1,y<-7;ptr p'+(':const i64' if p_readonly else '')+'=x;ptr q'+(':const i64' if q_readonly else '')+'=y;ptr var ro:const->p;ptr var other:const->q;'
    values=[1,7]
    case('access_model_'+str(number),body=body,setup=setup,reject=not safe,output=''.join(str(values[t])+'\n' for t in targets))

READONLY_LEFT = BASE.replace('ptr p=x,q=y;', 'ptr p:const i64=x;ptr q=y;')
RESTORE_LEFT = '{ptr saved=left;left=right;left=saved:void;}'
case('saved_target_cannot_restore_into_writable_slot', body='ptr saved=left;left=right;right=saved;',
     setup=READONLY_LEFT, reject=True)
case('distinct_slots_with_same_target', definition=HEAD+RESTORE_LEFT,
     setup=READONLY_LEFT.replace('ptr q=y;', 'ptr q=x;'), output='1\n1\n')
runner.CASES['nested_restore_distinct_mixed_access'] = ({
    'inner.cod': HEAD+RESTORE_LEFT+'export (<=>);',
    'main.cod': IO+'using "./inner.cod" (<=>) as inner;'+HEAD+'{inner(left,right);}'+READONLY_LEFT+
                'try{ro<=>other;'+OUT+'}catch(e:const NullPointerException){behave @print(99);};',
}, 0, '1\n7\n', '')
runner.CASES['prepared_restore_distinct_mixed_access'] = (
    IO+HEAD+'{ptr saved=left;defer intrinsic (=)(left,saved);left=right:void;}'+'proc run()->void{'+READONLY_LEFT+
    'defer{'+OUT+'};defer (<=>)(ro,other);p=z;return;}run();', 0, '9\n7\n', '')

runner.CASE_ARGUMENTS={}
runner.CASE_SHADOWS={name:[] for name in runner.CASES}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
