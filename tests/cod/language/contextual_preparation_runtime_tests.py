#!/usr/bin/env python3
"""Context-selected copies and borrows preserve native ownership and cleanup."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
func choose_mode(tag:bool,ref f:func(u64)->const u64 in O,x:u64)->const u64 where {owner O}{return f(x);}
func choose_mode(tag:i64,f:func(u64)->const u64,x:u64)->const u64{return f(x);}
func inspect(tag:T,ref f:func(u64)->const u64 in O,x:u64)->const u64 where {T,owner O}{return choose_mode(tag,f,x);}
proc probe(tag:T,ref f:func(u64)->const u64 in O)->const u64 where {T,owner O}{return choose_mode(tag,f,{{observe(9100u64);1u64}});}
proc report_mode(tag:bool,ref f:func(u64)->const u64 in O,x:u64)->void where {owner O}{observe(f(x));}
proc report_mode(tag:i64,f:func(u64)->const u64,x:u64)->void{observe(f(x));}
proc later(tag:T,ref f:func(u64)->const u64 in O)->void where {T,owner O}{defer report_mode(tag,f,23u64);f<-wrap(other);}
{{
  func f=wrap(base);
  observe(inspect(false,f,20u64));observe(inspect(0,f,21u64));observe(probe(false,f));
  observe(9010u64);val sum=0u64;
  for(i in 0u64..10000u64){sum<-sum+inspect(false,f,i);};
  observe(9011u64);observe(sum);
  observe(9030u64);
  try{probe(0,f);{}}catch(e:const OutOfMemory){observe(9031u64);};
}};
observe(9000u64);
{{
  func f=wrap(base);later(false,f);f<-wrap(base);later(0,f);
  observe(9050u64);
  try{later(false,f);}catch(e:const OutOfMemory){observe(9051u64);};
}};
observe(9000u64);
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==5', 'checkpoints==7').replace('copy_failures==3', 'copy_failures==4').replace('record_failures==1', 'record_failures==2')
runner.EXPECTED_VALUES += ['21', '22', '9100', '2', '50005000', '34', '24']
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
