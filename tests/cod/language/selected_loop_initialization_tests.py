#!/usr/bin/env python3
"""Selected initialization is inductive across loop entry and reaching backedges."""
from itertools import product
from pathlib import Path
import foundation_tests as runner

IO='using symbol "std:io" @print;using "std:exceptions" NullPointerException;using "std:exceptions" OutOfMemoryError;'
SETUP='val a<-7u64,b<-8u64,c<-make local u64;ptr selected=a;ptr observed=a;'
UPDATE='if(selected is a){selected=b;}else{c<-9u64;observed=c;};'
READ='behave @print(observed);'
runner.CASES={}


def case(name,body=READ+UPDATE,setup=SETUP,definitions='',output='7\n7\n9\n',reject=False,diagnostic='initialization',source=None):
    if source is None:
        source=IO+definitions+'proc run()->void{try{'+setup+'for(i in 0u64..3u64){ '+body+' };}catch(e:const NullPointerException){behave @print(99u64);};}run();'
    runner.CASES[name]=(source,2 if reject else 0,'' if reject else output,diagnostic if reject else '')


case('selected_backedge')
case('selected_current_iteration',body=UPDATE+READ,output='7\n9\n9\n')
case('missing_backedge_write',body=READ+UPDATE.replace('c<-9u64;',''),reject=True)
case('missing_current_write',body=UPDATE.replace('c<-9u64;','')+READ,reject=True)
case('wrong_backedge_write',body=READ+UPDATE.replace('c<-9u64;','b<-9u64;'),reject=True)
case('read_before_write',body=READ+UPDATE.replace('c<-9u64;observed=c;','observed=c;behave @print(observed);c<-9u64;'),reject=True)
case('initial_target_unwritten',setup=SETUP.replace('observed=a','observed=c'),reject=True)
case('late_initialization',setup=SETUP.replace('observed=a','observed=c'),body='c<-9u64;'+READ+UPDATE,output='9\n'*3)
case('unrelated_unwritten_cell',body=READ+UPDATE+'behave @print(c);',reject=True)
case('alias_rebinding',setup=SETUP+'var slot->observed;',body=READ+UPDATE.replace('observed=c','slot=c'))
case('alias_missing_write',setup=SETUP+'var slot->observed;',body=READ+UPDATE.replace('c<-9u64;','').replace('observed=c','slot=c'),reject=True)
case('saved_reference',setup=SETUP+'ptr saved=observed;',body='behave @print(saved);observed=c;',output='7\n'*3)
case('fresh_iteration',setup='',body='{{val x<-make local u64;if(i==0u64){x<-7u64;{}}else{};behave @print(x);}};',reject=True)
case('readonly',setup=SETUP.replace('ptr observed=a','ptr observed:const u64=a'),body=READ+'observed<-7u64;'+UPDATE,reject=True,diagnostic='readonly')
case('null_is_not_payload',body=READ+'observed=null;',output='7\n99\n')
case('null_is_not_ref',definitions='func use(ref x:const u64 in O)->void where {owner O}{}',body='use(observed);observed=null;',reject=True,diagnostic='proven non-null')
case('nested_loop',body='for(j in 0u64..2u64){'+READ+UPDATE+'};',output='7\n7\n9\n9\n9\n9\n')
case('nested_missing_write',body='for(j in 0u64..2u64){'+READ+UPDATE.replace('c<-9u64;','')+'};',reject=True)
case('exiting_path_has_no_backedge',body=READ+'if(i==1u64){observed=c;return;}else{};',output='7\n7\n')
case('exiting_read_still_checked',body=READ+'if(i==1u64){observed=c;behave @print(observed);return;}else{};',reject=True)
case('finally_backedge',body=READ+'try{}finally{'+UPDATE+'{}};')
case('finally_missing_write',body=READ+'try{}finally{'+UPDATE.replace('c<-9u64;','')+'{}};',reject=True)
case('cleanup_backedge',body=READ+'{{defer{'+UPDATE+'{}};}};')
case('cleanup_missing_write',body=READ+'{{defer{'+UPDATE.replace('c<-9u64;','')+'{}};}};',reject=True)
case('recovered_registration',body=READ+'{{try{defer{'+UPDATE+'{}};}catch(e:const OutOfMemoryError){'+UPDATE+'{}};}};')
case('registration_without_write',body=READ+'{{try{defer{c<-9u64;};}catch(e:const OutOfMemoryError){};}};observed=c;',reject=True)
FAIL='func fail()->u64 throws NullPointerException{ptr absent:u64=null in here;return absent;}'
case('failed_write',definitions=FAIL,body=READ+'try{c<-fail();{}}catch(failure:const NullPointerException){};observed=c;',reject=True)
case('recovered_write',definitions=FAIL,body=READ+'try{c<-fail();{}}catch(failure:const NullPointerException){c<-9u64;{}};observed=c;',output='7\n9\n9\n')
SET='proc set(ptr x:u64 in O)->void throws NullPointerException where {owner O}{x<-9u64;}'
case('callee_write',definitions=SET,body=READ+'set(c);observed=c;',output='7\n9\n9\n')
case('prepared_write',definitions=SET,body=READ+'{{defer set(c);}};observed=c;',output='7\n9\n9\n')
SWAP='func (<=>)(ptr x:u64 in O,ptr y:u64 in O)->void where {owner O}{intrinsic (<=>)(x,y);}'
case('caller_slot_backedge',definitions=SWAP,setup=SETUP+'ptr other=c;',body=READ+'c<-9u64;observed<=>other;',output='7\n9\n7\n')
case('caller_slot_missing_write',definitions=SWAP,setup=SETUP+'ptr other=c;',body=READ+'observed<=>other;',reject=True)
case('delayed_read_changed_target',body='{{defer{'+READ+'};observed=c;}};',reject=True)
case('delayed_read_written_target',body='{{defer{'+READ+'};c<-9u64;observed=c;}};',output='9\n'*3)
OBSERVE='proc observe(value:u64)->void{behave @print(value);}'
case('prepared_value_snapshot',definitions=OBSERVE,body='{{defer observe(observed);c<-9u64;observed=c;}};',output='7\n9\n9\n')
case('prepared_borrow_snapshot',definitions=OBSERVE.replace('value:u64','ref value:const u64 in O').replace('->void{','->void where {owner O}{'),body='{{defer observe(observed);c<-9u64;observed=c;}};',output='7\n9\n9\n')

case('dynamic_count',source=IO+'proc run(n:u64)->void{try{'+SETUP+'for(i in 0u64..n){'+READ+UPDATE+'}else{behave @print(0u64);};}catch(e:const NullPointerException){behave @print(99u64);};}run(0u64);run(1u64);run(3u64);',output='0\n7\n7\n7\n9\n')
case('fold',source=IO+'try{'+SETUP+'behave @print(fold(i in 0u64..3u64,total=0u64){'+UPDATE+'total+observed});}catch(e:const NullPointerException){behave @print(99u64);};',output='25\n')

for name,initial,write,after,reject in (
    ('conditional_cleanup','a','c<-9u64;','',False),
    ('conditional_cleanup_uninitialized_entry','c','c<-9u64;','',True),
    ('conditional_cleanup_wrong_write','a','b<-9u64;','',True),
    ('conditional_cleanup_changed_entry','a','c<-9u64;','observed=c;',True),
    ('conditional_cleanup_early_read','a','observed=c;behave @print(observed);c<-9u64;','',True),
):
    source=IO+'proc run(flag:bool)->void{try{'+SETUP.replace('observed=a','observed='+initial)+'{{defer{if(flag){'+write+'observed=c;{}}else{};};'+after+'}};'+READ+'}catch(e:const NullPointerException){behave @print(99u64);};}run(false);run(true);'
    case(name,source=source,reject=reject,output='7\n9\n')

# This interpreter models cells and binding assignments directly. Delayed
# propagation through p/q requires more than one pass to reject stale facts.
SCHEDULES=(
    ('read','write','p_b'),
    ('read','p_b'),
    ('p_b','read','write'),
    ('write','p_b','read'),
    ('read','p_q','q_b','write'),
    ('read','p_q','q_b'),
    ('read','swap','q_b','write'),
    ('read','swap','q_b'),
)
for number,(schedule,q_b,b_initialized) in enumerate(product(SCHEDULES,(False,True),(False,True))):
    cells={'a':7,'b':8 if b_initialized else None};bindings={'p':'a','q':'b' if q_b else 'a'}
    values=[]
    for _ in range(4):
        for action in schedule:
            if action=='read':values.append(cells[bindings['p']])
            elif action=='write':cells['b']=9
            elif action=='p_b':bindings['p']='b'
            elif action=='p_q':bindings['p']=bindings['q']
            elif action=='q_b':bindings['q']='b'
            else:bindings['p'],bindings['q']=bindings['q'],bindings['p']
    spelling={'read':'behave @print(p);','write':'b<-9u64;','p_b':'p=b;','p_q':'p=q;','q_b':'q=b;','swap':'intrinsic (<=>)(p,q);'}
    source=IO+'try{val a<-7u64,b<-'+('8u64' if b_initialized else 'make local u64')+';ptr p=a,q='+('b' if q_b else 'a')+';for(i in 0u64..4u64){'+''.join(spelling[x] for x in schedule)+'};}catch(e:const NullPointerException){behave @print(99u64);};'
    case('model_'+str(number),source=source,reject=None in values,output=''.join(str(x)+'\n' for x in values))
    deferred=source.replace('0u64..4u64){','0u64..4u64){ {{defer{').replace('};}catch(e:','{} };}}; };}catch(e:')
    case('deferred_model_'+str(number),source=deferred,reject=None in values,output=''.join(str(x)+'\n' for x in values))

for written in (False,True):
    bindings='ptr '+','.join('p'+str(i)+'=a' for i in range(8))+';'
    advance=''.join('p'+str(i)+'=p'+str(i+1)+';' for i in range(7))+'p7=b;'
    source=IO+'try{val a<-7u64,b<-make local u64;'+bindings+'for(i in 0u64..12u64){behave @print(p0);'+('b<-9u64;' if written else '')+advance+'};}catch(e:const NullPointerException){behave @print(99u64);};'
    case('long_pipeline_'+str(written),source=source,reject=not written,output='7\n'*8+'9\n'*4)

runner.CASE_ARGUMENTS={}
runner.CASE_SHADOWS={name:[] for name in runner.CASES}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
