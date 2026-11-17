#!/usr/bin/env python3
"""Prepared selections retain initialization from their actual capture frames."""
from itertools import product
from pathlib import Path
import foundation_tests as runner

IO='using symbol "std:io" @print;using "std:exceptions" NullPointerException;using "std:exceptions" OutOfMemoryError;'
SETUP='val a<-7u64,b<-8u64,c<-make local u64;ptr p=a,q=a;'
COPY='{{defer (=)(p,q);q=c;}};q=a;'
READ='behave @print(p);'
runner.CASES={}


def case(name,body=READ+COPY,setup=SETUP,after='',before='',definitions='',output='7\n'*4,reject=False,diagnostic='initialization',action=None):
    if action is None:action='defer{for(i in 0u64..4u64){ '+body+' };};'
    source=IO+definitions+'proc run()->void{'+setup+'try{ {{'+before+action+after+'{} }}; }catch(e:const NullPointerException){behave @print(99u64);};}run();'
    runner.CASES[name]=(source,2 if reject else 0,'' if reject else output,diagnostic if reject else '')


case('captured_entry')
case('read_after_invocation',body=COPY+READ)
case('changed_capture_entry',after='q=c;',reject=True)
case('written_capture_entry',after='c<-9u64;q=c;',output='7\n9\n7\n7\n')
case('later_action_changes_capture',after='defer{q=c;};',reject=True)
case('later_action_writes_capture',after='defer{c<-9u64;q=c;};',output='7\n9\n7\n7\n')
case('late_binding_does_not_initialize',body=READ+COPY.replace('q=a;','q=b;'),after='q=c;',reject=True)
case('two_prepared_snapshots',body=READ+'{{defer (=)(p,q);q=b;defer (=)(p,q);q=c;}};q=a;')
case('two_snapshots_bad_first',body=READ+'{{defer (=)(p,q);q=b;defer (=)(p,q);q=a;}};q=a;',after='q=c;',reject=True)
case('nested_action',body=READ+'{{defer{ '+COPY+' };}};')
case('nested_action_bad_entry',body=READ+'{{defer{ '+COPY+' };}};',after='q=c;',reject=True)
case('nested_queue',body=READ+'{{defer{defer (=)(p,q);q=c;};}};q=a;')
case('nested_queue_bad_entry',body=READ+'{{defer{defer (=)(p,q);q=a;};}};q=a;',after='q=c;',reject=True)
case('deep_action',action='defer{defer{for(i in 0u64..4u64){'+READ+COPY+'};};};')
case('finally',action='defer{}finally{for(i in 0u64..4u64){'+READ+COPY+'};};')
case('weak_destination',setup=SETUP+'ptr other=a;',body=READ+'behave @print(other);{{defer (=)((if(i==0u64){p}else{other}),q);q=c;}};q=a;',output='7\n'*8)
case('weak_bad_entry',setup=SETUP+'ptr other=a;',body=READ+'behave @print(other);{{defer (=)((if(i==0u64){p}else{other}),q);q=a;}};',after='q=c;',reject=True)
case('alias_destination',setup=SETUP+'var slot->p;',body=READ+COPY.replace('(p,q)','(slot,q)'))
case('intrinsic',body=READ+COPY.replace('defer (=)','defer intrinsic (=)'))
case('recovered_registration',body=READ+'{{try{defer (=)(p,q);}catch(e:const OutOfMemoryError){p=q;{}};q=c;}};q=a;')
case('recovered_registration_bad_entry',body=READ+'{{try{defer (=)(p,q);}catch(e:const OutOfMemoryError){p=q;{}};q=a;}};',after='q=c;',reject=True)
case('unrelated_uninitialized_cell',body=READ+COPY+'behave @print(c);',reject=True)
case('null_snapshot',after='q=null;',output='7\n99\n')
case('call_after_invocation',definitions='proc observe(x:u64)->void{behave @print(x);}',body=COPY+'observe(p);')

CALL_BODY='ptr output=source,current=source;for(i in 0u64..3u64){ {{defer (=)(output,current);current=spare;}};current=source;behave @print(output);};'
CALL_HEAD='proc invoke(ptr source:u64 in O,ptr spare:u64 in O)->void throws NullPointerException where {owner O}'
INVOKE=CALL_HEAD+'{'+CALL_BODY+'}'
GENERIC=INVOKE.replace('source:u64','source:T').replace('spare:u64','spare:T').replace('where {owner O}','where {T,owner O}')
RECURSIVE=CALL_HEAD.replace('spare:u64 in O)','spare:u64 in O,n:u64)')+'{if(n==0u64){'+CALL_BODY+'}else{invoke(source,spare,n-1u64);};}'
DISPATCH='proc dispatch(f:F,ptr source:u64 in O,ptr spare:u64 in O)->void throws NullPointerException where {F,owner O}{f(source,spare);}'
case('call_entry',definitions=INVOKE,action='invoke(a,c);',output='7\n'*3)
case('call_bad_entry',definitions=INVOKE,action='invoke(c,a);',reject=True)
case('call_generic',definitions=GENERIC,action='invoke(a,c);',output='7\n'*3)
case('call_generic_bad',definitions=GENERIC,action='invoke(c,a);',reject=True)
case('call_recursive',definitions=RECURSIVE,action='invoke(a,c,3u64);',output='7\n'*3)
case('call_recursive_bad',definitions=RECURSIVE,action='invoke(c,a,3u64);',reject=True)
case('call_callable',definitions=INVOKE,setup=SETUP+'proc callable=invoke;',action='callable(a,c);',output='7\n'*3)
case('call_callable_bad',definitions=INVOKE,setup=SETUP+'proc callable=invoke;',action='callable(c,a);',reject=True)
case('call_prepared',definitions=INVOKE,action='defer invoke(a,c);',output='7\n'*3)
case('call_saved_before_rebind',definitions=INVOKE,action='defer invoke(q,c);',after='q=c;',output='7\n'*3)
case('call_saved_uninitialized',definitions=INVOKE,setup=SETUP.replace('q=a','q=c'),action='defer invoke(q,c);',after='q=a;',reject=True)
case('call_later_payload_write',definitions=INVOKE,setup=SETUP.replace('q=a','q=c'),action='defer invoke(q,c);',after='c<-9u64;q=a;',output='9\n'*3)
case('call_transitive',definitions=INVOKE+DISPATCH,setup=SETUP+'proc callable=invoke;',action='dispatch(callable,a,c);',output='7\n'*3)
case('call_transitive_bad',definitions=INVOKE+DISPATCH,setup=SETUP+'proc callable=invoke;',action='dispatch(callable,c,a);',reject=True)
case('call_missing_restore',definitions=INVOKE.replace('current=source;behave','current=spare;behave'),action='invoke(a,c);',reject=True)
case('call_deferred_frame',definitions=INVOKE,action='defer{invoke(q,c);};',output='7\n'*3)
case('call_late_block_rebind',definitions=INVOKE,action='defer{invoke(q,c);};',after='q=c;',reject=True)

# Interpret the saved reference and the reverse registration queue directly.
# Repeated source nodes represent separate dynamic captures, including the
# first uninitialized capture followed by initialized later captures.
for number,(initial,next_target,b_initialized,last) in enumerate(product(('a','b'),('a','b'),(False,True),('a','c'))):
    cells={'a':7,'b':8 if b_initialized else None,'c':None}
    bindings={'p':'a','q':initial}
    queue=[]
    for _ in range(3):
        queue.append(('read',None));queue.append(('assign',bindings['q']))
        bindings['q']=next_target
    bindings['q']=last
    values=[]
    for operation,target in reversed(queue):
        if operation=='assign':bindings['p']=target
        else:values.append(cells[bindings['p']])
    setup=SETUP.replace('b<-8u64','b<-'+('8u64' if b_initialized else 'make local u64')).replace('q=a','q='+initial)
    action='defer{for(i in 0u64..3u64){defer{'+READ+'};defer (=)(p,q);q='+next_target+';};q='+last+';};'
    case('model_'+str(number),setup=setup,action=action,reject=None in values,output=''.join(str(value)+'\n' for value in values))

runner.CASE_ARGUMENTS={}
runner.CASE_SHADOWS={name:[] for name in runner.CASES}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
