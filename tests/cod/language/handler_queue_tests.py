#!/usr/bin/env python3
"""Handler registrations retain their incoming completion and cleanup queue."""
from pathlib import Path
import handler_payload_tests as fixture

runner=fixture.runner
safe,checked=fixture.safe,fixture.checked
BASE=fixture.BASE
runner.CASES={}
safe('normal_body_queue',BASE+'payload<-failure;{{defer{defer set(payload,failure);}catch(e:const MissingReturnError){defer set(payload,item);};}};')
safe('caught_queue',BASE+'{{defer{defer set(payload,item);}catch(e:const MissingReturnError){defer set(payload,e);};throw failure;}};')
safe('catch_partition',BASE+'{{defer catch(e:const MissingReturnError){defer set(payload,e);}catch(e){defer set(payload,item);};throw failure;}};')
safe('catch_both_branches',BASE+'{{defer catch(e:const MissingReturnError){if(true){defer set(payload,e);}else{defer (<-)(payload,e);};};throw failure;}};')
safe('nested_intrinsic',BASE+'{{defer catch(e:const MissingReturnError){defer intrinsic (<-)(payload,e);};throw failure;}};')
safe('nested_layers',BASE+'{{defer catch(e:const MissingReturnError){defer{defer{defer set(payload,e);};};};throw failure;}};')
safe('nested_guarded',BASE+'{{defer catch(e:const MissingReturnError){defer{}finally{defer set(payload,e);};};throw failure;}};')
safe('nested_failure_recovered',BASE+'{{defer catch(e:const MissingReturnError){defer catch(next){defer set(payload,next);};defer{throw e;};};throw failure;}};')
safe('catch_before_finally_registration',BASE+'{{defer catch(e:const MissingReturnError){defer set(payload,e);}finally{defer set(payload,item);};throw failure;}};')
safe('catch_queue_after_finally_write',BASE+'{{defer catch(e:const MissingReturnError){defer set(payload,e);}finally{payload<-item;{}};throw failure;}};')
safe('finally_registration',BASE+'{{defer catch(e:const MissingReturnError){}finally{defer set(payload,failure);};throw failure;}};')
safe('handler_swap',BASE+'item<-failure;{{defer catch(e:const MissingReturnError){defer (<->)(payload,item);};throw failure;}};')
safe('handler_rebind',BASE+'item<-failure;ptr selected=payload;{{defer catch(e:const MissingReturnError){defer (=)(selected,item);};throw failure;}};',target='selected')
safe('handler_loop',BASE+'{{defer catch(e:const MissingReturnError){for(i in 0u64..3u64){defer set(payload,e);};};throw failure;}};')
safe('handler_loop_else',BASE+'{{defer catch(e:const MissingReturnError){for(i in 0u64..0u64){}else{defer set(payload,e);};};throw failure;}};')
safe('body_and_handler_loops',BASE+'{{defer{for(i in 0u64..3u64){defer set(payload,item);};}catch(e:const MissingReturnError){for(i in 0u64..3u64){defer set(payload,e);};};throw failure;}};')
safe('handler_then_merged_loop',BASE+'{{defer catch(e:const MissingReturnError){defer set(payload,e);}finally{for(i in 0u64..3u64){defer set(item,failure);};};throw failure;}};')
safe('no_failure_skips_handler',BASE+'payload<-failure;{{defer catch(e){defer set(payload,item);};}};')
safe('handler_return_after_queue',BASE+'recover(payload,failure);','func recover(ref target:Exception in O,value:MissingReturnError)->void where {owner O}{defer catch(e:const MissingReturnError){defer set(target,e);return;};throw value;}')
safe('borrowed_return_after_queue',BASE+'ptr selected=recover(payload,failure);','func recover(ref target:Exception in O,value:MissingReturnError)->ref Exception in O where {owner O}{defer catch(e:const MissingReturnError){defer set(target,e);return target;};throw value;}',target='selected')
checked('no_failure_no_write',BASE+'{{defer catch(e){defer set(payload,failure);};}};')
checked('other_handler_queue',BASE+'try{ {{defer catch(e:const MissingReturnError){defer set(payload,failure);}catch(e){defer set(payload,e);};throw item;}}; }catch(e:const Exception){};')
checked('body_queue_still_runs',BASE+'{{defer{defer set(payload,item);}catch(e){defer set(payload,failure);};}};')
checked('registration_order',BASE+'{{defer catch(e:const MissingReturnError){defer set(payload,item);}finally{defer set(payload,failure);};throw failure;}};')
checked('zero_loop',BASE+'{{defer catch(e:const MissingReturnError){for(i in 0u64..0u64){defer set(payload,e);};};throw failure;}};')
checked('else_skipped',BASE+'{{defer catch(e:const MissingReturnError){for(i in 0u64..3u64){}else{defer set(payload,e);};};throw failure;}};')
checked('optional_registration',BASE+'{{defer catch(e:const MissingReturnError){if(false){defer set(payload,e);}else{};};throw failure;}};')
safe('ordinary_try_finally',BASE+'{{try{defer set(payload,failure);}finally{defer set(item,failure);};}};')
safe('ordinary_catch_finally',BASE+'{{try{throw failure;}catch(e:const MissingReturnError){defer set(payload,e);}finally{defer set(item,failure);};}};')
safe('empty_finally_prefix',BASE+'{{try{defer set(payload,failure);}finally{};}};')
safe('loop_prefix_and_suffix',BASE+'{{try{for(i in 0u64..3u64){defer set(payload,failure);};}finally{for(j in 0u64..3u64){defer set(item,failure);};};}};')
safe('finalizer_inner_scope',BASE+'{{try{defer set(payload,failure);}finally{ {{defer set(item,failure);}};};}};')
safe('rethrow_keeps_queued_writes',BASE+'try{ {{try{defer (<-)(payload,failure);throw failure;}finally{defer (<-)(item,failure);};}}; }catch(e:const MissingReturnError){};')
checked('rethrow_callee_failure_envelope',BASE+'try{ {{try{defer set(payload,failure);throw failure;}finally{defer set(item,failure);};}}; }catch(e:const MissingReturnError){};',output='5\n')
checked('conditional_prefix',BASE+'{{try{if(false){defer set(payload,failure);}else{};}finally{defer set(item,failure);};}};')
checked('finalizer_masks_registration_failure',BASE+'try{ {{try{defer set(payload,failure);}finally{throw failure;};}}; }catch(e:const MissingReturnError){};',output='5\n')
checked('catch_all_recovers_registration_failure',BASE+'payload<-failure;{{defer{defer set(payload,failure);}catch(e){defer set(payload,item);};}};',output='5\n')
checked('finally_return_recovers_failed_registration',BASE+'ptr selected=recover(payload,failure);','func recover(ref target:Exception in O,value:MissingReturnError)->ref Exception in O where {owner O}{defer catch(e:const MissingReturnError){defer set(target,e);}finally{return target;};throw value;}',target='selected',output='5\n')
INIT='using "std:exceptions" OutOfMemoryError;'+fixture.fixture.IO+fixture.fixture.ERROR
runner.CASES['handler_initializes']=(INIT+'val failure<-unchecked(),value<-make local u64;{{defer catch(e:const MissingReturnError){defer{value<-19u64;{}};};throw failure;}};behave @print(value);',0,'19\n',fixture.fixture.WARN)
runner.CASES['failed_registration_no_initialization']=(INIT+'val failure<-unchecked(),value<-make local u64;try{ {{defer catch(e:const MissingReturnError){defer{value<-19u64;{}};};throw failure;}}; }catch(e:const OutOfMemoryError){};behave @print(value);',2,'','before definite initialization')
runner.CASE_ARGUMENTS={}
runner.CASE_SHADOWS={name:[] for name in runner.CASES}
runner.EXTRA_SOURCES=[Path(__file__),Path(fixture.__file__),Path(fixture.fixture.__file__),Path(__file__).with_name('exception_projection_tests.py')]

from language_test_support import expect_source_rejections
expect_source_rejections(runner, (
    'normal_body_queue',
    'caught_queue',
    'catch_partition',
    'catch_both_branches',
    'nested_intrinsic',
    'nested_layers',
    'nested_guarded',
    'nested_failure_recovered',
    'catch_before_finally_registration',
    'catch_queue_after_finally_write',
    'finally_registration',
    'handler_swap',
    'handler_rebind',
    'handler_loop',
    'handler_loop_else',
    'body_and_handler_loops',
    'handler_then_merged_loop',
    'no_failure_skips_handler',
    'handler_return_after_queue',
    'borrowed_return_after_queue',
    'no_failure_no_write',
    'no_failure_no_write_covered',
    'other_handler_queue',
    'other_handler_queue_covered',
    'body_queue_still_runs',
    'body_queue_still_runs_covered',
    'registration_order',
    'registration_order_covered',
    'zero_loop',
    'zero_loop_covered',
    'else_skipped',
    'else_skipped_covered',
    'optional_registration',
    'optional_registration_covered',
    'catch_all_recovers_registration_failure',
    'catch_all_recovers_registration_failure_covered',
    'finally_return_recovers_failed_registration',
    'finally_return_recovers_failed_registration_covered',
    'handler_initializes',
    'failed_registration_no_initialization',
), 'currently draining scope')

if __name__=='__main__':
    raise SystemExit(runner.main())
