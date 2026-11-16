#!/usr/bin/env python3
"""Cleanup composes payload writes with saved values and invocation-time reads."""
from pathlib import Path
import foundation_tests as runner
from exception_projection_tests import IO, ERROR, WARN, COVER

COPY='func set(ref target:Exception in O,incoming:Exception)->void where {owner O}{target<-incoming;}'
BORROW='func copy(ref target:Exception in O,ref incoming:const Exception in P)->void where {owner O,owner P}{target<-incoming;}'
BASE='val payload:Exception<-checked(),item:Exception<-checked();'
runner.CASES={}
def safe(name,body,helpers='',target='payload',output='5\n'):
    runner.CASES[name]=(IO+ERROR+COPY+BORROW+helpers+body+'try{throw '+target+';}catch(e:const MissingReturnError){behave @print(e.category);};',0,output,WARN)
def checked(name,body,helpers='',target='payload',output='4\n'):
    runner.CASES[name]=(ERROR+COPY+BORROW+helpers+body+'throw '+target+';',2,'',COVER)
    runner.CASES[name+'_covered']=(IO+ERROR+COPY+BORROW+helpers+body+'try{throw '+target+';}catch(e:const Exception){behave @print(e.category);};',0,output,WARN)

safe('block',BASE+'{{defer{payload<-unchecked();{}};}};')
safe('prepared',BASE+'{{defer set(payload,unchecked());}};')
safe('prepared_builtin',BASE+'{{defer (<-)(payload,unchecked());}};')
safe('prepared_intrinsic',BASE+'{{defer intrinsic (<-)(payload,unchecked());}};')
safe('copied_registration_value',BASE+'item<-unchecked();{{defer set(payload,item);item<-checked();}};')
safe('borrowed_invocation_value',BASE+'{{defer copy(payload,item);item<-unchecked();}};')
safe('borrowed_selected_address',BASE+'val other:Exception<-checked();ptr selected=item;{{defer copy(payload,selected);selected=other;item<-unchecked();}};')
checked('block_current_binding_join',BASE+'val other:Exception<-unchecked();ptr selected=item;{{defer{copy(payload,selected);};selected=other;}};',output='5\n')
safe('prepared_lifo',BASE+'{{defer set(payload,unchecked());defer set(payload,checked());}};')
safe('block_lifo',BASE+'{{defer{payload<-unchecked();{}};defer{payload<-checked();{}};}};')
safe('mixed_lifo',BASE+'{{defer copy(payload,item);defer{item<-unchecked();{}};}};')
safe('nested_scope',BASE+'{{defer{ {{defer set(payload,unchecked());}};};}};')
safe('nested_registration_snapshot',BASE+'{{defer{item<-unchecked();defer set(payload,item);item<-checked();};}};')
safe('nested_registration_borrow',BASE+'{{defer{defer copy(payload,item);item<-unchecked();};}};')
safe('conditional_register',BASE+'{{if(true){defer set(payload,unchecked());}else{defer{payload<-unchecked();{}};};}};')
safe('body_exception',BASE+'val failure<-unchecked();try{ {{defer set(payload,failure);throw failure;}}; }catch(e:const MissingReturnError){};')
safe('deferred_exception',BASE+'val failure<-unchecked();try{ {{defer set(payload,failure);defer{throw failure;};}}; }catch(e:const MissingReturnError){};')
checked('fallback_recovery_join',BASE+'val failure<-unchecked();{{defer catch(e:const MissingReturnError){payload<-e;{}};defer{throw failure;};}};',output='5\n')
safe('deferred_recovery',BASE+'val failure<-unchecked();{{defer{payload<-failure;{}}catch(e:const MissingReturnError){payload<-e;{}};defer{throw failure;};}};')
safe('prepared_exceptional_write',BASE+'val failure<-unchecked();try{ {{defer fail_after(payload,failure);}}; }catch(e:const MissingReturnError){};','func fail_after(ref target:Exception in O,value:MissingReturnError)->void where {owner O}{target<-value;throw value;}')
safe('finally_after_defer',BASE+'{{defer{try{}finally{payload<-unchecked();{}};};}};')
safe('borrowed_result_cleanup',BASE+'ptr selected=choose(payload);','func choose(ref target:Exception in O)->ref Exception in O where {owner O}{defer{target<-unchecked();{}};return target;}',target='selected')
safe('return_snapshot',BASE+'val saved<-choose(payload);','func choose(ref target:Exception in O)->const Exception where {owner O}{target<-unchecked();defer{target<-checked();{}};return target;}',target='saved')
safe('deferred_return_snapshot',BASE+'val saved<-choose(payload);','func choose(ref target:Exception in O)->const Exception where {owner O}{defer{target<-unchecked();defer{target<-checked();{}};return target;};return target;}',target='saved')
safe('recursive_prepared',BASE+'{{defer recursive(payload,unchecked(),10u64);}};','func recursive(ref target:Exception in O,value:Exception,n:u64)->void where {owner O}{if(n==0u64){target<-value;{}}else{recursive(target,value,n-1u64);};}')
safe('recursive_block',BASE+'recursive(payload,unchecked(),10u64);','func recursive(ref target:Exception in O,value:Exception,n:u64)->void where {owner O}{defer set(target,value);if(n!=0u64){recursive(target,value,n-1u64);}else{};}')
safe('loop_queue',BASE+'val failure<-unchecked();{{for(i in 0u64..3u64){defer set(payload,failure);};}};')
safe('callable_prepared',BASE+'func writer=set;{{defer writer(payload,unchecked());}};')
safe('symbolic_prepared',BASE+'{{defer behave @writer(payload,unchecked());}};','proc @writer(ref target:Exception in O,value:Exception)->void where {owner O}{target<-value;}effect @writer;')
safe('empty_cleanup_preserves',BASE+'payload<-unchecked();{{defer{};}};')
safe('registration_does_not_write',BASE+'payload<-unchecked();{{defer set(payload,checked());try{throw payload;}catch(e:const MissingReturnError){behave @print(8);};}};payload<-unchecked();',output='8\n5\n')
safe('prepared_swap',BASE+'item<-unchecked();{{defer (<->)(payload,item);}};')
safe('block_swap',BASE+'item<-unchecked();{{defer{payload<->item;};}};')
checked('copied_checked_snapshot',BASE+'{{defer set(payload,item);item<-unchecked();}};')
checked('borrowed_late_checked',BASE+'item<-unchecked();{{defer copy(payload,item);item<-checked();}};')
checked('later_cleanup_replaces',BASE+'{{defer{payload<-checked();{}};defer set(payload,unchecked());}};')
checked('conditional_identity',BASE+'{{if(false){defer set(payload,unchecked());}else{};}};')
checked('zero_iteration',BASE+'{{for(i in 0u64..0u64){defer set(payload,unchecked());};}};')
checked('future_target',BASE+'select_future(payload);','func select_future(ref output:Exception in O)->void where {owner O}{val first:Exception<-checked();ptr selected=first;defer{output<-first;{}};defer{selected<-unchecked();{}};val future:Exception<-checked();selected=future;}')
runner.CASE_ARGUMENTS={}
runner.CASE_SHADOWS={name:[] for name in runner.CASES}
runner.EXTRA_SOURCES=[Path(__file__),Path(__file__).with_name('exception_projection_tests.py')]
if __name__=='__main__':
    raise SystemExit(runner.main())
