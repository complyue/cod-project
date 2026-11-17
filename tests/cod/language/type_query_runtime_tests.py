#!/usr/bin/env python3
"""Type-only callable and caught-value references need no runtime payload work."""
from pathlib import Path
from native_test_support import run_native
import selected_slot_runtime_tests as fixture
FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func unproved(x:T)->const T where T{return x+x;}
proc type_queries(f:func(u64)->const u64)->void{
 observe(9010u64);
 val x<-make local typeof(f(3u64)){29u64};
 val y<-make local typeof(unproved(true)){true};
 observe(9011u64);
 observe(x);if(y){observe(30u64);}else{};
}
proc query_catch()->void{
 try{throw bound_error();}catch(e:const MissingReturnError){
  observe(9080u64);
  defer{{val x<-make local typeof(e.category){31u64};observe(x);}};
  observe(9081u64);
 };
}
type_queries(wrap(base));query_catch();observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==11','checkpoints==12'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('static bool deny_create','static bool query_record=false;\nstatic unsigned query_records=0;\nstatic bool deny_create'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('  if(deny_record){++record_failures;','  if(query_record){assert(size==16);++query_records;}\n  if(deny_record){++record_failures;'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('  if(value==9000)','  if(value==9080){assert(!query_record && !query_records);query_record=true;return;}\n  if(value==9081){assert(query_record && query_records==1);query_record=false;return;}\n  if(value==9000)'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['29','30','31'])
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__=='__main__':
    raise SystemExit(run_native(FIXTURE))
