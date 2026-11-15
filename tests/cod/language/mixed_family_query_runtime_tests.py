#!/usr/bin/env python3
"""Queries neither invoke partial methods nor construct declared callable operands."""
from pathlib import Path
import signature_query_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
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
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==15', 'checkpoints==16')
runner.EXPECTED_VALUES += ['38', '39', '2']
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
