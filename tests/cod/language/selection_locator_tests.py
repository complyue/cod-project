#!/usr/bin/env python3
"""Observed reference identities agree with known target cells and borrowed aliases."""
from itertools import product
from pathlib import Path
import foundation_tests as runner

IO='using symbol "std:io" @print;using "std:exceptions" NullPointerException;'
LOCAL='val a<-make local u64,b<-make local u64,c<-make local u64;ptr selected=if(flag){a}else{b};'
CALLS='run(false,false);run(false,true);run(true,false);run(true,true);'
SAME='func same(ptr x:const u64 in O,ptr y:const u64 in O)->bool where {owner O}{return x is y;}'
RELAY='func relay(ptr x:u64 in O)->ptr u64 in O where {owner O}{return x;}'
WRITE='if(CONDITION){ FIRST;{}}else{ SECOND;{}};'
runner.CASES={}


def case(name,condition='selected is a',first='a<-7u64',second='b<-8u64',setup='',after='',read='selected',definitions='',reject=False,output='8\n8\n7\n7\n',local=LOCAL,body=None,diagnostic='initialization'):
    if body is None:
        body=WRITE.replace('CONDITION',condition).replace('FIRST',first).replace('SECOND',second)+after+'behave @print('+read+');'
    source=IO+definitions+'proc run(flag:bool,other:bool)->void{'+local+'try{ '+setup+body+'}catch(e:const NullPointerException){behave @print(99u64);};return;}'+CALLS
    runner.CASES[name]=(source,2 if reject else 0,'' if reject else output,diagnostic if reject else '')


case('direct')
case('reverse',condition='a is selected')
case('difference',condition='selected is not a',first='b<-8u64',second='a<-7u64')
case('negation',condition='!(selected is not a)')
case('explicit',condition='(is)(selected,a)')
case('intrinsic',condition='intrinsic (is)(selected,a)')
case('saved_guard',setup='val equal<-selected is a;',condition='equal')
case('copied_guard',setup='val equal<-selected is a,saved<-equal;equal<-false;',condition='saved')
case('changed_guard',setup='val equal<-selected is a;equal<-true;',condition='equal',reject=True)
case('saved_selection',setup='ptr saved=selected;',after='selected=c;',read='saved')
case('changed_selection',after='selected=c;',reject=True)
case('ordered_snapshot',setup='ptr saved=selected;',condition='selected is {selected=c;a}',read='saved')
case('ordered_changed_read',condition='selected is {selected=c;a}',reject=True)
case('changed_anchor',setup='ptr anchor=a;val equal<-selected is anchor;anchor=c;',condition='equal',first='anchor<-7u64',reject=True)
case('saved_anchor',setup='ptr anchor=a;val equal<-selected is anchor;anchor=c;',condition='equal')
case('missing_first',first='{}',reject=True)
case('missing_second',second='{}',reject=True)
case('wrong_first',first='c<-7u64',reject=True)
case('wrong_second',second='c<-8u64',reject=True)
case('independent_selection',setup='ptr independent=if(other){a}else{b};',condition='independent is a',reject=True)
case('same_bounds_are_not_equal',setup='ptr independent=if(other){a}else{b};',condition='selected is independent',reject=True)
case('weak_rebinding',setup='var slot->selected;',after='slot=c;',reject=True)
case('read_before_write',setup='behave @print(selected);',reject=True)
case('ref_entry',setup='ref required=selected;',reject=True)
case('readonly_write',setup='ptr frozen:const u64=a;',first='frozen<-7u64',reject=True,diagnostic='readonly')
case('readonly_comparison',setup='ptr frozen:const u64=a;',condition='selected is frozen')
case('nullable_selection',local=LOCAL.replace('else{b}', 'else{null in here}'),output='99\n99\n7\n7\n')
case('nullable_anchor',setup='ptr anchor=if(other){a}else{null in here};',condition='selected is anchor',reject=True)

MATCHED_SELECTION='ptr matched=if(flag){a}else{b};'
WRITE_READ_SELECTION='selected<-7u64;behave @print(matched);'
case('repeated_conditional_selection',setup=MATCHED_SELECTION,body=WRITE_READ_SELECTION,output='7\n'*4)
case('negated_conditional_selection',setup='ptr matched=if(!flag){b}else{a};',
     body=WRITE_READ_SELECTION,output='7\n'*4)
case('copied_condition',setup='val saved<-flag;ptr matched=if(saved){a}else{b};',
     body=WRITE_READ_SELECTION,output='7\n'*4)
case('conditional_selection_in_full_scope',setup=MATCHED_SELECTION,
     body='{{'+WRITE_READ_SELECTION+'}};',output='7\n'*4)
case('conditional_selection_in_cleanup',setup=MATCHED_SELECTION,
     body='{{defer{behave @print(matched);};selected<-7u64:void;}};',output='7\n'*4)
case('conditionally_selected_other_input',setup='ptr matched=if(other){a}else{b};',
     body=WRITE_READ_SELECTION,reject=True)
case('opposite_conditional_selection',setup='ptr matched=if(flag){b}else{a};',
     body=WRITE_READ_SELECTION,reject=True)
case('overwritten_condition',setup='val saved<-flag;saved<-!saved;ptr matched=if(saved){a}else{b};',
     body=WRITE_READ_SELECTION,reject=True)
case('rebound_conditional_arm',local=LOCAL.replace('ptr selected=if(flag){a}', 'ptr arm=a;ptr selected=if(flag){arm}'),
     setup='arm=c;ptr matched=if(flag){arm}else{b};',body=WRITE_READ_SELECTION,reject=True)
case('conditional_selection_across_iterations',setup='ptr arm=a;ptr prior=a;ptr current=a;',
     body='for(i in 0u64..2u64){current=if(flag){arm}else{b};'
          'if(i==0u64){prior=current;arm=c;{}}else{current<-7u64;behave @print(prior);};};',reject=True)

DECIDE='func decide(n:u64)->bool{return n==0u64;}'
LOCAL_GATE=LOCAL.replace('ptr selected=if(flag)', 'val gate<-decide(if(flag){0u64}else{1u64});ptr selected=if(gate)')
MATCHED_GATE=MATCHED_SELECTION.replace('if(flag)', 'if(gate)')
case('stored_boolean_decision',definitions=DECIDE,local=LOCAL_GATE,setup=MATCHED_GATE,
     body=WRITE_READ_SELECTION,output='7\n'*4)
case('stored_boolean_in_full_scope',definitions=DECIDE,local=LOCAL_GATE,setup=MATCHED_GATE,
     body='{{'+WRITE_READ_SELECTION+'}};',output='7\n'*4)
case('stored_boolean_in_cleanup',definitions=DECIDE,local=LOCAL_GATE,setup=MATCHED_GATE,
     body='{{defer{behave @print(matched);};selected<-7u64:void;}};',output='7\n'*4)
case('stored_boolean_copy_survives_replacement',definitions=DECIDE,local=LOCAL_GATE,
     setup='val saved<-gate;gate<-!gate;'+MATCHED_GATE.replace('if(gate)', 'if(saved)'),
     body=WRITE_READ_SELECTION,output='7\n'*4)
case('negated_stored_boolean',definitions=DECIDE,local=LOCAL_GATE,setup='ptr matched=if(!gate){b}else{a};',
     body=WRITE_READ_SELECTION,output='7\n'*4)
case('stored_negation',definitions=DECIDE,local=LOCAL_GATE.replace('gate<-decide', 'gate<-!decide'),
     setup='ptr matched=if(!gate){b}else{a};',body=WRITE_READ_SELECTION,output='7\n'*4)
case('stored_boolean_replacement',definitions=DECIDE,local=LOCAL_GATE,setup='gate<-!gate;'+MATCHED_GATE,
     body=WRITE_READ_SELECTION,reject=True)
case('stored_boolean_reference_write',definitions=DECIDE,local=LOCAL_GATE,
     setup='ref writable:bool=gate;writable<-!writable;'+MATCHED_GATE,
     body=WRITE_READ_SELECTION,reject=True)
FLIP='proc flip(ref target:bool in O)->void where {owner O}{target<-!target;return;}'
case('stored_boolean_call_write',definitions=DECIDE+FLIP,local=LOCAL_GATE,setup='flip(gate);'+MATCHED_GATE,
     body=WRITE_READ_SELECTION,reject=True)
case('stored_boolean_replacement_in_loop',definitions=DECIDE,local=LOCAL_GATE,
     setup='ptr prior=a;ptr current=a;',
     body='for(i in 0u64..2u64){current=if(gate){a}else{b};'
          'if(i==0u64){prior=current;gate<-!gate;{}}else{current<-7u64;behave @print(prior);};};',reject=True)

for name,condition,reject in (
    ('finite_bounds','selected is not a&&selected is not b',False),
    ('disjoint','selected is c',False),
    ('distinct_cells','a is b',False),
    ('same_cell','a is not a',False),
    ('possible_member','selected is a',True),
    ('disjunction','selected is c||flag',True),
):
    case(name,body='if('+condition+'){behave @print(c);}else{behave @print(0u64);};',reject=reject,output='0\n'*4)

for name,definition,condition,setup in (
    ('helper',SAME,'same(selected,a)',''),
    ('generic_helper',SAME.replace('u64','T').replace('where {owner O}','where {T,owner O}'),'same(selected,a)',''),
    ('recursive_helper',SAME.replace(')->bool',',n:u64)->bool').replace('return x is y;','if(n==0u64){return x is y;}else{return same(x,y,n-1u64);};'),'same(selected,a,8u64)',''),
    ('callable_helper',SAME,'compare(selected,a)','func compare=same;'),
    ('captured_helper',SAME,'compare(selected,a)','func compare[same](ptr x:const u64 in O,ptr y:const u64 in O)->bool where {owner O}{return same(x,y);};'),
    ('returned_operand',RELAY,'relay(selected) is a',''),
    ('borrowed_boolean',SAME,'equal','val value<-same(selected,a);ref equal:const bool=value;'),
):
    case(name,definitions=definition,condition=condition,setup=setup)

STORE='proc compare(ptr output:bool in B,ptr left:const u64 in O,ptr right:const u64 in O)->void throws NullPointerException where {owner B,owner O}{output<-left is right;return;}'
case('boolean_writeback',definitions=STORE,setup='val equal<-false;compare(equal,selected,a);',condition='equal')
case('changed_boolean_writeback',definitions=STORE,setup='val equal<-false;compare(equal,selected,a);equal<-true;',condition='equal',reject=True)
FILL='proc fill(ptr selected:u64 in O,ptr first:u64 in O,ptr second:u64 in O)->void throws NullPointerException where {owner O}{if(selected is first){first<-7u64;{}}else{second<-8u64;{}};return;}'
case('callee_writes',definitions=FILL,body='fill(selected,a,b);behave @print(selected);')
case('callee_read',definitions=FILL.replace('proc fill','func fill').replace('->void','->u64').replace('return;}', 'return selected;}'),body='behave @print(fill(selected,a,b));')
case('prepared_call',definitions=FILL,body='{{defer fill(selected,a,b);}};behave @print(selected);')
case('prepared_snapshot',definitions=FILL,setup='ptr saved=selected;',body='{{defer fill(selected,a,b);selected=c:void;}};behave @print(saved);')
case('prepared_changed_read',definitions=FILL,body='{{defer fill(selected,a,b);selected=c:void;}};behave @print(selected);',reject=True)

WORK=WRITE.replace('CONDITION','selected is a').replace('FIRST','a<-7u64').replace('SECOND','b<-8u64')
case('deferred',body='{{defer{'+WORK+'};}};behave @print(selected);')
case('nested_deferred',body='{{defer{defer{'+WORK+'};};}};behave @print(selected);')
case('finally',body='try{}finally{'+WORK+'};behave @print(selected);')
case('failed_registration',body='try{ {{defer{'+WORK+'};}}; }catch(failure){};behave @print(selected);',reject=True)
FAIL='func fail()->u64 throws NullPointerException{ptr absent:u64=null in here;return absent;}'
case('failed_write',definitions=FAIL,body='try{'+WORK.replace('7u64','fail()')+'}catch(failure:const NullPointerException){};behave @print(selected);',reject=True)
case('iteration_scope',local='',body='for(i in 0u64..2u64){ {{'+LOCAL+WORK+'behave @print(selected);}};};',output='8\n'*4+'7\n'*4)
case('fresh_iteration',local='',body='for(i in 0u64..2u64){ {{'+LOCAL+'if(i==0u64){'+WORK+'}else{};behave @print(selected);}};};',reject=True)

PERSISTENT='val a<-7u64,b<-8u64,c<-make local u64;ptr selected=a;ptr observed=a;'
case('later_iteration_read',local=PERSISTENT,body='for(i in 0u64..3u64){behave @print(observed);if(selected is a){selected=b;}else{observed=c;};};',reject=True)
case('current_iteration_read',local=PERSISTENT,body='for(i in 0u64..2u64){if(selected is a){selected=b;}else{observed=c;};behave @print(observed);};',reject=True)
case('rebound_loop_observation',local=PERSISTENT,body='for(i in 0u64..3u64){behave @print(selected);if(selected is a){selected=b;}else{selected=a;};};',output='7\n8\n7\n'*4)

CHOOSE='func choose(ptr first:const u64 in O,ptr second:const u64 in O)->u64 where {owner O}{if(first is second){return 7u64;}else{return 8u64;};}'
case('borrowed_inputs_may_alias',definitions=CHOOSE,body='behave @print(choose(a,a));behave @print(choose(a,b));',output='7\n8\n'*4)

# Model the actual selected cell and the branch's written cell, independently
# of the compiler's identity, bound and initialization representations.
for anchor,yes,no in product(range(3),repeat=3):
    names=('a','b','c');values=[]
    for selected in (1,1,0,0):
        equal=selected==anchor;written=yes if equal else no
        values.append((7 if equal else 8) if written==selected else None)
    safe=None not in values
    case('cell_model_'+str(anchor)+str(yes)+str(no),condition='selected is '+names[anchor],first=names[yes]+'<-7u64',second=names[no]+'<-8u64',reject=not safe,output=''.join(str(v)+'\n' for v in values))

runner.CASE_ARGUMENTS={}
runner.CASE_SHADOWS={name:[] for name in runner.CASES}
runner.EXTRA_SOURCES=[Path(__file__)]

from language_test_support import expect_source_rejections
expect_source_rejections(runner, (
    'nested_deferred',
), 'currently draining scope')

if __name__=='__main__':
    raise SystemExit(runner.main())
