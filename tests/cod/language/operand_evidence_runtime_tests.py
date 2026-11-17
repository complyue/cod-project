#!/usr/bin/env python3
"""Later type evidence and operator fallback preserve complete-value ownership."""
from pathlib import Path
from native_test_support import run_native
import contextual_preparation_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
for name in ('choose_mode', 'report_mode'):
    FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(name+'(tag:bool,ref f:func(u64)->const u64 in O,', name+'(ref f:func(u64)->const u64 in O,tag:bool,'))
    FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(name+'(tag:i64,f:func(u64)->const u64,', name+'(f:func(u64)->const u64,tag:i64,'))
    FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(name+'(tag,f,', name+'(f,tag,'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('choose_mode(f,tag,{{observe(9100u64);', 'choose_mode(f,{observe(9101u64);tag},{{observe(9100u64);'))
FIXTURE = FIXTURE.updated(expected_values=(*FIXTURE.expected_values[:FIXTURE.expected_values.index('9100')], '9101', *FIXTURE.expected_values[FIXTURE.expected_values.index('9100'):]))
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func (+)(ref f:func(u64)->const u64 in O,x:u64)->const u64 where {owner O}{return f(x);}
func add(ref f:T in O,x:u64)->const u64 where {T,owner O}{return f+x;}
{{
  func f=wrap(base);val n<-1u64;
  observe(add(f,24u64));observe(add(n,24u64));
  observe(9010u64);val sum<-0u64;
  for(i in 0u64..10000u64){sum<-sum+add(f,i);};
  observe(9011u64);observe(sum);
  observe(9050u64);
  try{later(0,f);}catch(e:const OutOfMemoryError){observe(9051u64);};
}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==7', 'checkpoints==8').replace('record_failures==2', 'record_failures==3'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['25', '25', '50005000'])
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
