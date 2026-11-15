#!/usr/bin/env python3
"""Callable signature queries allocate no images and require no executable bodies."""
from pathlib import Path
import declaration_query_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
func signature_only(input:T)->const T where T;
proc procedure_signature_only(input:T)->const T where T;
proc signature_queries(callback:func(u64)->const u64)->void{
 observe(9010u64);
 val slot<-make local u64;
 val first<-make local typeof(declared_query(signature_only,slot)){36u64};
 val second<-make local typeof(declared_proc(procedure_signature_only,3u64)){37u64};
 val third<-make local typeof(declared_query(signature_only,declared_query(signature_only,true))){true};
 observe(9011u64);
 if(third){observe(first);observe(second);}else{};
 observe(callback(1u64));
}
signature_queries(wrap(base));observe(9000u64);
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==14', 'checkpoints==15')
runner.EXPECTED_VALUES += ['36', '37', '2']
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
