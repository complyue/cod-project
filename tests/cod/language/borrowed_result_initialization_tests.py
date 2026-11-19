#!/usr/bin/env python3
"""Borrowed returns preserve sufficient initialization of their saved targets."""
from itertools import product
from pathlib import Path
import foundation_tests as runner

IO='using symbol "std:io" @print;using "std:exceptions" NullPointerException;using "std:exceptions" OutOfMemoryError;'
HEAD='func select(ptr source:u64 in O,ptr spare:u64 in O)->ptr u64 in O throws NullPointerException where {owner O}'
LOOP='ptr output=source,current=source;for(i in 0u64..3u64){ {{defer (=)(output,current);current=spare:void;}};current=source;};'
SELECT=HEAD+'{'+LOOP+'return output;}'
SETUP='val a<-7u64,b<-8u64,c<-make local u64;'
runner.CASES={}


def case(name,definition=SELECT,call='select(a,c)',setup=SETUP,after='',output='7\n',reject=False,diagnostic='initialization',receive='ptr result=',read='behave @print(result);'):
    source=IO+definition+'proc run()->void{'+setup+'try{ '+receive+call+';'+after+read+'}catch(e:const NullPointerException){behave @print(99u64);};return;}run();'
    runner.CASES[name]=(source,2 if reject else 0,'' if reject else output,diagnostic if reject else '')


case('returned_selection')
case('uninitialized_source',call='select(c,a)',reject=True)
case('both_initialized',call='select(a,b)')
case('same_actual_target',call='select(a,a)')
case('same_uninitialized_target',call='select(c,c)',reject=True)
case('null_source',setup=SETUP+'ptr absent:u64=null in here;',call='select(absent,c)',output='99\n')
case('null_spare',setup=SETUP+'ptr absent:u64=null in here;',call='select(a,absent)')
case('value_receiver',receive='val result<-')
case('copied_target',after='a<-9u64;',output='9\n')
case('independent_value',receive='val result<-',after='a<-9u64;')
case('source_slot_changed',setup=SETUP+'ptr input=a;',call='select(input,c)',after='input=c;')
case('late_slot_change_cannot_initialize',setup=SETUP+'ptr input=c;',call='select(input,a)',after='input=a;',reject=True)
case('late_payload_write',call='select(c,a)',after='c<-9u64;',output='9\n')
case('unrelated_payload_write',call='select(c,a)',after='b<-9u64;',reject=True)
case('missing_loop_restore', definition=HEAD+
     '{ptr output=source,current=source;for(i in 0u64..3u64){output=current;current=spare;};return output;}',
     reject=True)
case('intrinsic_rebind',definition=SELECT.replace('defer (=)','defer intrinsic (=)'))
case('saved_return_finally_rebind',definition=SELECT.replace('return output;','try{return output;}finally{output=spare;{}};'))
case('saved_return_finally_input_rebind',definition=SELECT.replace('return output;','try{return output;}finally{source=spare;{}};'))
case('bad_saved_return_finally_input_rebind',definition=SELECT.replace('return output;','try{return output;}finally{source=spare;{}};'),call='select(c,a)',reject=True)
case('finally_write_saved_target',definition=SELECT.replace('return output;','try{return output;}finally{source<-9u64;{}};'),call='select(c,a)',output='9\n')
case('finally_write_other_target',definition=SELECT.replace('return output;','try{return output;}finally{source=spare;source<-9u64;{}};'),call='select(c,a)',reject=True)
case('finally_return_replaces_bad',definition=SELECT.replace('return output;','try{return spare;}finally{return output;};'))
case('finally_return_replaces_good',definition=SELECT.replace('return output;','try{return output;}finally{return spare;};'),reject=True)
case('defer_return_replaces_bad',definition=SELECT.replace('return output;','defer{return output;};return spare;'))
case('defer_return_replaces_good',definition=SELECT.replace('return output;','defer{return spare;};return output;'),reject=True)
case('defer_late_output_selection',definition=SELECT.replace('return output;','defer{return output;};output=spare;return source;'),reject=True)
case('defer_return_then_rebind',definition=SELECT.replace('return output;','defer{output=spare;};defer{return output;};return spare;'))
case('defer_return_then_write',definition=SELECT.replace('return output;','defer{source<-9u64;};defer{return output;};return spare;'),call='select(c,a)',output='9\n')
case('nested_defer_return',definition=SELECT.replace('return output;','defer{defer{return output;};};return spare;'))
case('nested_defer_bad_return',definition=SELECT.replace('return output;','defer{defer{return spare;};};return output;'),reject=True)
case('generic',definition=SELECT.replace('u64 in O','T in O').replace('where {owner O}','where {T,owner O}'))
case('generic_bad',definition=SELECT.replace('u64 in O','T in O').replace('where {owner O}','where {T,owner O}'),call='select(c,a)',reject=True)
RECURSIVE=HEAD.replace('spare:u64 in O)','spare:u64 in O,n:u64)')+'{if(n==0u64){'+LOOP+'return output;}else{return select(source,spare,n-1u64);};}'
case('recursive',definition=RECURSIVE,call='select(a,c,3u64)')
case('recursive_bad',definition=RECURSIVE,call='select(c,a,3u64)',reject=True)
FORWARD='func forward(ptr source:u64 in O,ptr spare:u64 in O)->ptr u64 in O throws NullPointerException where {owner O}{return select(source,spare);}'
case('transitive',definition=SELECT+FORWARD,call='forward(a,c)')
case('transitive_bad',definition=SELECT+FORWARD,call='forward(c,a)',reject=True)
case('callable',setup=SETUP+'func chosen=select;',call='chosen(a,c)')
case('callable_bad',setup=SETUP+'func chosen=select;',call='chosen(c,a)',reject=True)
DISPATCH='func dispatch(f:F,ptr source:u64 in O,ptr spare:u64 in O)->ptr u64 in O throws NullPointerException where {F,owner O}{return f(source,spare);}'
case('callable_forward',definition=SELECT+DISPATCH,call='dispatch(select,a,c)')
case('callable_forward_bad',definition=SELECT+DISPATCH,call='dispatch(select,c,a)',reject=True)
FLAG=HEAD.replace('spare:u64 in O)','spare:u64 in O,flag:bool)')
case('alternative_good',definition=FLAG+'{'+LOOP+'if(flag){return output;}else{return source;};}',call='select(a,c,true)')
case('alternative_bad',definition=FLAG+'{'+LOOP+'if(flag){return output;}else{return spare;};}',call='select(a,c,true)',reject=True)
case('conditional_cleanup_bad',definition=FLAG+'{'+LOOP+'defer{if(flag){return spare;}else{{}};};return output;}',call='select(a,c,false)',reject=True)
case('conditional_cleanup_good',definition=FLAG+'{'+LOOP+'defer{if(flag){return source;}else{{}};};return output;}',call='select(a,c,true)')
runner.CASES['declared_failure_retained']=(IO+SELECT+SETUP+'ptr result=select(a,c);',2,'',"checked exception 'NullPointerException'")
case('readonly_preserved',definition=SELECT.replace('->ptr u64','->ptr const u64'),read='result<-9u64;',reject=True,diagnostic='readonly')

REF=HEAD.replace('->ptr u64','->ref u64').replace('ptr spare','ref spare')+'{if(source is null){return spare;}else{};defer{source<-9u64;};return source;}'
case('ref_return_read_before_cleanup',definition=REF,call='select(c,a)',reject=True)
case('ref_return_then_write',definition=REF,call='select(a,b)',output='9\n')

case('later_operand_rebind',setup=SETUP+'ptr input=a;',call='select(input,{input=c;c})')
case('later_operand_bad_snapshot',setup=SETUP+'ptr input=c;',call='select(input,{input=a;a})',reject=True)
USE=('proc ptr_use(ptr selected:u64 in O)->void where {owner O}'
     '{if(selected is not null){behave @print(selected);}else{};return;}')
DEFERRED_SELECTION=SELECT.replace(' throws NullPointerException','')+USE
DEFERRED_TARGETS=SETUP+'ptr initialized=a,unwritten=c;'
case('helper_in_deferred_frame', definition=DEFERRED_SELECTION, setup=DEFERRED_TARGETS,
     receive='', call='{{defer{ptr_use(select(initialized,unwritten));};}}', read='{}')
case('bad_helper_in_deferred_frame', definition=DEFERRED_SELECTION, setup=DEFERRED_TARGETS,
     receive='', call='{{defer{ptr_use(select(unwritten,initialized));};}}', read='{}', reject=True)
case('expired_local_owner',definition=SELECT+FORWARD.replace('return select(source,spare);','val local<-7u64;return select(local,local);'),call='forward(a,c)',reject=True,diagnostic='declared external owner')

# Interpret normal execution and every failed-registration prefix. An earlier
# registered return can recover a later registration failure, so its result
# must also be initialized even when the successful full queue is safe.
for number,(source_initialized,spare_initialized,return_first,write_saved) in enumerate(product((False,True),repeat=4)):
    initial={'source':7 if source_initialized else None,'spare':8 if spare_initialized else None}
    actions=[('return',None),('rebind','spare')]
    if not return_first:actions.reverse()
    if write_saved:actions.insert(0,('write','source'))
    values=[]
    for prefix in range(len(actions)+1):
        cells=initial.copy();output='source'
        pending='source' if prefix==len(actions) else None
        for kind,target in reversed(actions[:prefix]):
            if kind=='return':pending=output
            elif kind=='rebind':output=target
            else:cells[target]=9
        if pending is not None:values.append(cells[pending])
    registered=['defer{return output;};','defer{output=spare;};']
    if not return_first:registered.reverse()
    definition=HEAD+'{'+LOOP+('defer{source<-9u64;};' if write_saved else '')+''.join(registered)+'return output;}'
    setup='val a<-'+('7u64' if source_initialized else 'make local u64')+',c<-'+('8u64' if spare_initialized else 'make local u64')+';'
    case('return_model_'+str(number),definition=definition,setup=setup,reject=None in values,output=str(values[-1])+'\n')

runner.CASE_ARGUMENTS={}
runner.CASE_SHADOWS={name:[] for name in runner.CASES}
runner.EXTRA_SOURCES=[Path(__file__)]

from language_test_support import expect_source_rejections
expect_source_rejections(runner, (
    'nested_defer_return',
    'nested_defer_bad_return',
), 'currently draining scope')

if __name__=='__main__':
    raise SystemExit(runner.main())
