#!/usr/bin/env python3
"""Deferred input partitions retain handler payload writes and identity paths."""
from pathlib import Path
import cleanup_payload_tests as fixture

runner=fixture.runner
safe,checked=fixture.safe,fixture.checked
BASE=fixture.BASE+'val failure<-unchecked();'
runner.CASES={}
safe('fallback',BASE+'{{defer catch(e:const MissingReturnError){payload<-e;{}};defer{throw failure;};}};')
safe('body_failure',BASE+'{{defer catch(e:const MissingReturnError){payload<-e;{}};throw failure;}};')
safe('root_catch',BASE+'{{defer catch(e){payload<-e;{}};throw failure;}};')
safe('skipped_body',BASE+'{{defer{payload<-checked();{}}catch(e:const MissingReturnError){payload<-e;{}};throw failure;}};')
safe('empty_finally',BASE+'{{defer catch(e:const MissingReturnError){payload<-e;{}}finally{};throw failure;}};')
safe('other_finally_target',BASE+'{{defer catch(e:const MissingReturnError){payload<-e;{}}finally{item<-checked();{}};throw failure;}};')
safe('finally_overrides',BASE+'{{defer catch(e:const MissingReturnError){payload<-checked();{}}finally{payload<-failure;{}};throw failure;}};')
safe('handler_call',BASE+'{{defer catch(e:const MissingReturnError){set(payload,e);};throw failure;}};')
safe('handler_prepared_call',BASE+'{{defer catch(e:const MissingReturnError){defer set(payload,e);};throw failure;}};',output='5\n')
safe('handler_prepared_transfer',BASE+'{{defer catch(e:const MissingReturnError){defer (<-)(payload,e);};throw failure;}};',output='5\n')
safe('handler_delayed_block',BASE+'{{defer catch(e:const MissingReturnError){defer{payload<-e;{}};};throw failure;}};',output='5\n')
safe('scoped_handler_prepared',BASE+'{{defer catch(e:const MissingReturnError){ {{defer set(payload,e);}};};throw failure;}};')
safe('scoped_handler_block',BASE+'{{defer catch(e:const MissingReturnError){ {{defer{payload<-e;{}};}};};throw failure;}};')
safe('nested_handler',BASE+'try{ {{defer catch(e:const MissingReturnError){defer catch(inner){payload<-e;{}};throw item;};throw failure;}}; }catch(e:const Exception){};')
safe('nested_scope',BASE+'try{ {{defer catch(e:const MissingReturnError){ {{defer catch(inner){payload<-e;{}};throw item;}};};throw failure;}}; }catch(e:const Exception){};')
safe('handler_partition',BASE+'{{defer catch(e:const MissingReturnError){payload<-e;{}}catch(e){payload<-checked();{}};throw failure;}};')
safe('handler_return',BASE+'recover(payload,failure);','func recover(ref target:Exception in O,value:MissingReturnError)->void where {owner O}{defer catch(e:const MissingReturnError){target<-e;return;};throw value;}')
safe('borrowed_return',BASE+'ptr selected=recover(payload,failure);','func recover(ref target:Exception in O,value:MissingReturnError)->ref Exception in O where {owner O}{defer catch(e:const MissingReturnError){target<-e;return target;};throw value;}',target='selected')
safe('recursive',BASE+'recover(payload,failure,12u64);','func recover(ref target:Exception in O,value:MissingReturnError,n:u64)->void where {owner O}{defer catch(e:const MissingReturnError){target<-e;{}};if(n==0u64){throw value;}else{recover(target,value,n-1u64);};}')
safe('later_action_reads',BASE+'{{defer{payload<-item;{}};defer catch(e:const MissingReturnError){item<-e;{}};throw failure;}};')
safe('no_failure_identity',BASE+'payload<-failure;{{defer catch(e:const MissingReturnError){payload<-checked();{}};}};')
checked('no_failure_keeps_checked',BASE+'{{defer catch(e:const MissingReturnError){payload<-e;{}};}};')
checked('body_runs_without_failure',BASE+'payload<-failure;{{defer{payload<-checked();{}}catch(e:const MissingReturnError){payload<-e;{}};}};')
checked('finally_keeps_checked',BASE+'{{defer catch(e:const MissingReturnError){payload<-e;{}}finally{payload<-checked();{}};throw failure;}};')
checked('later_action_overrides',BASE+'{{defer{payload<-checked();{}};defer catch(e:const MissingReturnError){payload<-e;{}};throw failure;}};')
checked('root_checked_input',BASE+'try{ {{defer catch(e){payload<-e;{}};throw item;}}; }catch(e:const Exception){};')
checked('other_handler',BASE+'try{ {{defer catch(e:const MissingReturnError){payload<-e;{}}catch(e){payload<-e;{}};throw item;}}; }catch(e:const Exception){};')
checked('branch_recovery_join',BASE+'{{defer catch(e:const MissingReturnError){if(false){payload<-e;{}}else{};};throw failure;}};')
runner.CASE_ARGUMENTS={}
runner.CASE_SHADOWS={name:[] for name in runner.CASES}
runner.EXTRA_SOURCES=[Path(__file__),Path(fixture.__file__),Path(__file__).with_name('exception_projection_tests.py')]
if __name__=='__main__':
    raise SystemExit(runner.main())
