#!/usr/bin/env python3
"""Type-only callable and caught-value references need no runtime payload work."""
from pathlib import Path
import selected_slot_runtime_tests as fixture
runner=fixture.runner
runner.SOURCE+='''
func unproved(x:T)->const T where T{return x+x;}
proc type_queries(f:func(u64)->const u64)->void{
 observe(9010u64);
 val x<-make local typeof(f(3u64)){29u64};
 val y<-make local typeof(unproved(true)){true};
 observe(9011u64);
 observe(x);if(y){observe(30u64);}else{};
}
proc query_catch()->void{
 try{throw bound_error();}catch(e:const MissingReturnException){
  observe(9080u64);
  defer{{val x<-make local typeof(e.category){31u64};observe(x);}};
  observe(9081u64);
 };
}
type_queries(wrap(base));query_catch();observe(9000u64);
'''
runner.HARNESS=runner.HARNESS.replace('checkpoints==11','checkpoints==12')
runner.HARNESS=runner.HARNESS.replace('static bool deny_create','static bool query_record=false;\nstatic unsigned query_records=0;\nstatic bool deny_create')
runner.HARNESS=runner.HARNESS.replace('  if(deny_record){++record_failures;','  if(query_record){assert(size==16);++query_records;}\n  if(deny_record){++record_failures;')
runner.HARNESS=runner.HARNESS.replace('  if(value==9000)','  if(value==9080){assert(!query_record && !query_records);query_record=true;return;}\n  if(value==9081){assert(query_record && query_records==1);query_record=false;return;}\n  if(value==9000)')
runner.EXPECTED_VALUES+=['29','30','31']
runner.EXTRA_SOURCES+=[Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
