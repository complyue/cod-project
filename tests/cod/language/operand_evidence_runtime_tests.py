#!/usr/bin/env python3
"""Later type evidence and operator fallback preserve complete-value ownership."""
from pathlib import Path
import contextual_preparation_runtime_tests as fixture

runner = fixture.runner
for name in ('choose_mode', 'report_mode'):
    runner.SOURCE = runner.SOURCE.replace(name+'(tag:bool,ref f:func(u64)->const u64 in O,', name+'(ref f:func(u64)->const u64 in O,tag:bool,')
    runner.SOURCE = runner.SOURCE.replace(name+'(tag:i64,f:func(u64)->const u64,', name+'(f:func(u64)->const u64,tag:i64,')
    runner.SOURCE = runner.SOURCE.replace(name+'(tag,f,', name+'(f,tag,')
runner.SOURCE = runner.SOURCE.replace('choose_mode(f,tag,{{observe(9100u64);', 'choose_mode(f,{observe(9101u64);tag},{{observe(9100u64);')
runner.EXPECTED_VALUES.insert(runner.EXPECTED_VALUES.index('9100'), '9101')
runner.SOURCE += '''
func (+)(ref f:func(u64)->const u64 in O,x:u64)->const u64 where {owner O}{return f(x);}
func add(ref f:T in O,x:u64)->const u64 where {T,owner O}{return f+x;}
{{
  func f=wrap(base);val n=1u64;
  observe(add(f,24u64));observe(add(n,24u64));
  observe(9010u64);val sum=0u64;
  for(i in 0u64..10000u64){sum<-sum+add(f,i);};
  observe(9011u64);observe(sum);
  observe(9050u64);
  try{later(0,f);}catch(e:const OutOfMemory){observe(9051u64);};
}};
observe(9000u64);
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==7', 'checkpoints==8').replace('record_failures==2', 'record_failures==3')
runner.EXPECTED_VALUES += ['25', '25', '50005000']
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
