#!/usr/bin/env python3
"""Deferred loops retain sufficient invocation-entry initialization conditions."""
from pathlib import Path
import selected_loop_initialization_tests as fixture

runner=fixture.runner
MODELS={name:value for name,value in runner.CASES.items() if name.startswith('model_')}
IO,SETUP,READ,UPDATE=fixture.IO,fixture.SETUP,fixture.READ,fixture.UPDATE
runner.CASES={}


def case(name,body=READ+UPDATE,setup=SETUP,before='',after='',definitions='',output='7\n7\n9\n',reject=False,diagnostic='initialization',loop=None,action=None):
    if loop is None:loop='for(i in 0u64..3u64){ '+body+' };'
    if action is None:action='defer{ '+loop+' };'
    source=IO+definitions+'proc run()->void{'+setup+'try{ {{'+before+action+after+'{} }}; }catch(e:const NullPointerException){behave @print(99u64);};}run();'
    runner.CASES[name]=(source,2 if reject else 0,'' if reject else output,diagnostic if reject else '')


case('invocation_entry')
case('current_iteration',body=UPDATE+READ,output='7\n9\n9\n')
case('missing_backedge_write',body=READ+UPDATE.replace('c<-9u64;',''),reject=True)
case('missing_current_write',body=UPDATE.replace('c<-9u64;','')+READ,reject=True)
case('wrong_backedge_write',body=READ+UPDATE.replace('c<-9u64;','b<-9u64;'),reject=True)
case('read_before_write',body=READ+UPDATE.replace('c<-9u64;observed=c;','observed=c;behave @print(observed);c<-9u64;'),reject=True)
case('uninitialized_entry',setup=SETUP.replace('observed=a','observed=c'),reject=True)
case('changed_after_registration',after='observed=c;',reject=True)
case('written_after_registration',after='c<-9u64;observed=c;',output='9\n'*3)
case('different_initialized_entry',after='observed=b;',output='8\n8\n9\n')
case('changed_phase',after='selected=b;',output='7\n9\n9\n')
case('later_action_changes_entry',after='defer{observed=c;};',reject=True)
case('later_action_writes_entry',after='defer{c<-9u64;observed=c;};',output='9\n'*3)
case('earlier_action_changes_later',before='defer{observed=c;};')
case('empty_loop',loop='for(i in 0u64..0u64){ '+READ+UPDATE+' };',after='observed=c;',output='')
case('single_iteration',loop='for(i in 0u64..1u64){ '+READ+'observed=c; };',output='7\n')
case('repeated_invalid_target',body=READ+'observed=c;',reject=True)
case('nested_loop',body='for(j in 0u64..2u64){'+READ+UPDATE+'};',output='7\n7\n9\n9\n9\n9\n')
case('nested_missing_write',body='for(j in 0u64..2u64){'+READ+UPDATE.replace('c<-9u64;','')+'};',reject=True)
case('nested_action',action='defer{defer{for(i in 0u64..3u64){'+READ+UPDATE+'};};};')
case('finally_loop',action='defer{}finally{for(i in 0u64..3u64){'+READ+UPDATE+'};};')
case('iteration_cleanup',body=READ+'{{defer{'+UPDATE+'{}};}};')
case('iteration_cleanup_missing_write',body=READ+'{{defer{'+UPDATE.replace('c<-9u64;','')+'{}};}};',reject=True)
case('unrelated_cell_unwritten',body=READ+UPDATE+'behave @print(c);',reject=True)
case('alias_rebinding',setup=SETUP+'var slot->observed;',body=READ+UPDATE.replace('observed=c','slot=c'))
case('alias_missing_write',setup=SETUP+'var slot->observed;',body=READ+UPDATE.replace('c<-9u64;','').replace('observed=c','slot=c'),reject=True)
case('fresh_iteration',body='{{val x<-make local u64;if(i==0u64){x<-7u64;{}}else{};behave @print(x);}};',reject=True)
case('readonly',setup=SETUP.replace('ptr observed=a','ptr observed:const u64=a'),body=READ+'observed<-7u64;'+UPDATE,reject=True,diagnostic='readonly')
case('null_entry',after='observed=null;',output='99\n')
case('null_is_not_ref',definitions='proc use(ref x:const u64 in O)->void where {owner O}{}',body='use(observed);'+UPDATE,after='observed=null;',reject=True,diagnostic='proven non-null')
FAIL='func fail()->u64 throws NullPointerException{ptr absent:u64=null in here;return absent;}'
case('pending_failure',definitions=FAIL,after='fail();',output='7\n7\n9\n99\n')
case('handler_loop',definitions=FAIL,after='fail();',action='defer catch(incoming){for(i in 0u64..3u64){'+READ+UPDATE+'};return;};')
case('failed_write',definitions=FAIL,body=READ+'try{c<-fail();{}}catch(failure:const NullPointerException){};observed=c;',reject=True)
case('recovered_write',definitions=FAIL,body=READ+'try{c<-fail();{}}catch(failure:const NullPointerException){c<-9u64;{}};observed=c;',output='7\n9\n9\n')
case('fold',loop='behave @print(fold(i in 0u64..3u64,total=0u64){'+UPDATE+'total+observed});',output='25\n')
OBSERVE='proc observe(value:u64)->void{behave @print(value);}'
case('prepared_value_snapshot',definitions=OBSERVE,body='{{defer observe(observed);c<-9u64;observed=c;}};',output='7\n9\n9\n')
case('prepared_borrow_snapshot',definitions=OBSERVE.replace('value:u64','ref value:const u64 in O').replace('->void{','->void where {owner O}{'),body='{{defer observe(observed);c<-9u64;observed=c;}};',output='7\n9\n9\n')

COPY_SETUP='val a<-7u64,b<-8u64,c<-make local u64;ptr phase=a,p=a,q=a;'
COPY_UPDATE='if(phase is a){phase=b;}else{c<-9u64;q=c;};'


def copied(name,copy='p=q;',update=COPY_UPDATE,after='',setup=COPY_SETUP,definitions='',reject=False,output='7\n7\n7\n9\n',read='behave @print(p);'):
    case('copy_'+name,setup=setup,definitions=definitions,after=after,
         loop='for(i in 0u64..4u64){'+read+copy+update+'};',reject=reject,output=output)


copied('propagated')
copied('missing_write',update=COPY_UPDATE.replace('c<-9u64;',''),reject=True)
copied('wrong_write',update=COPY_UPDATE.replace('c<-9u64;','b<-9u64;'),reject=True)
copied('changed_p',after='p=c;',reject=True)
copied('changed_q',after='q=c;',reject=True)
copied('written_q',after='c<-9u64;q=c;',output='7\n9\n9\n9\n')
copied('saved_before_rebinding',copy='p=q;q=c;',update='q=a;',output='7\n'*4)
copied('saved_unwritten',copy='p=q;q=a;',update='',after='q=c;',reject=True)
copied('conditional',copy='p=if(i==0u64){q}else{q};')
copied('conditional_unknown',copy='p=if(i==0u64){q}else{c};',reject=True)
copied('handler_join',copy='p=try{q}catch(e:const NullPointerException){q};')
copied('slot_join',setup=COPY_SETUP+'ptr other=a;',copy='(if(i==0u64){p}else{other})=q;',read='behave @print(p);behave @print(other);',output='7\n'*7+'9\n')
copied('slot_join_missing_write',setup=COPY_SETUP+'ptr other=a;',copy='(if(i==0u64){p}else{other})=q;',read='behave @print(p);behave @print(other);',update=COPY_UPDATE.replace('c<-9u64;',''),reject=True)
copied('borrowed_result',definitions='func choose(ptr x:u64 in O)->ptr u64 in O where {owner O}{return x;}',copy='p=choose(q);')
copied('borrowed_result_missing_write',definitions='func choose(ptr x:u64 in O)->ptr u64 in O where {owner O}{return x;}',copy='p=choose(q);',update=COPY_UPDATE.replace('c<-9u64;',''),reject=True)
# This safe case needs conditional registration-frame evidence transport.
# Keep its conservative diagnostic distinct from the invalid snapshot controls.
copied('prepared_conditional_unproved',copy='{{defer (=)(p,q);q=c;}};',update='q=a;',reject=True)
copied('prepared_readiness',setup=COPY_SETUP.replace('make local u64','9u64'),copy='{{defer (=)(p,q);q=c;}};',update='q=a;',output='7\n'*4)
copied('prepared_changed_entry',copy='{{defer (=)(p,q);q=a;}};',update='',after='q=c;',reject=True)
copied('prepared_written_entry',copy='{{defer (=)(p,q);q=a;}};',update='',after='c<-9u64;q=c;',output='7\n9\n7\n7\n')
copied('prepared_weak_changed_entry',setup=COPY_SETUP+'ptr other=a;',copy='{{defer (=)((if(i==0u64){p}else{other}),q);q=a;}};',update='',after='q=c;',reject=True)

# Reuse the independent cell/binding interpreter's exact source and outcomes,
# moving the complete loop into an invocation-time deferred action.
for name,(source,code,output,diagnostic) in MODELS.items():
    source=source.replace('for(i in 0u64..4u64){','{{defer{for(i in 0u64..4u64){').replace('};}catch(e:','};};}};}catch(e:')
    runner.CASES[name]=(source,code,output,diagnostic)

runner.CASE_ARGUMENTS={}
runner.CASE_SHADOWS={name:[] for name in runner.CASES}
runner.EXTRA_SOURCES=[Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
