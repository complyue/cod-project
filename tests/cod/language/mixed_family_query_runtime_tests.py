#!/usr/bin/env python3
"""Queries neither invoke partial methods nor construct declared callable operands."""
from pathlib import Path
from native_test_support import run_native
import signature_query_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
proc mixed_target(input:T)->const T where T;
proc mixed_target(input:bool)->bool{observe(99u64);return input;}
proc mixed_queries(callback:func(u64)->const u64)->void{
 observe(9010u64);
 val slot<-make local u64;
 val first<-make local typeof(mixed_target(slot)){38u64};
 val second<-make local typeof(declared_proc(mixed_target,3u64)){39u64};
 val third<-make local typeof(mixed_target(true)){true};
 observe(9011u64);
 if(third){observe(first);observe(second);}else{};
 observe(callback(1u64));
}
mixed_queries(wrap(base));observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==15', 'checkpoints==16'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['38', '39', '2'])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
