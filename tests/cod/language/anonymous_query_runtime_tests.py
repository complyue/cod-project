#!/usr/bin/env python3
"""Anonymous query signatures do not allocate or retain callable payloads."""
from pathlib import Path
from native_test_support import run_native
import contextual_query_runtime_tests as fixture
FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
proc anonymous_queries(f:func(u64)->const u64)->void{
 observe(9010u64);
 val x<-make local typeof(query_apply(func [f](x:u64)->const u64{return f(x);},1u64)){34u64};
 val y<-make local typeof(query_apply(func(x:T)->const T where T{return x+x;},true)){true};
 val z<-make local typeof((proc [f]()->const u64{observe(99u64);return f(1u64);})()){35u64};
 observe(9011u64);if(y){observe(x);}else{};observe(z);
}
{{func f=wrap(base);anonymous_queries(f);observe(f(1u64));}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==13','checkpoints==14'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['34','35','2'])
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__=='__main__':
 raise SystemExit(run_native(FIXTURE))
