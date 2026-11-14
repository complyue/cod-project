#!/usr/bin/env python3
"""Anonymous query signatures do not allocate or retain callable payloads."""
from pathlib import Path
import contextual_query_runtime_tests as fixture
runner=fixture.runner
runner.SOURCE+='''
proc anonymous_queries(f:func(u64)->const u64)->void{
 observe(9010u64);
 val x=make local typeof(query_apply(func [f](x:u64)->const u64{return f(x);},1u64)){34u64};
 val y=make local typeof(query_apply(func(x:T)->const T where T{return x+x;},true)){true};
 val z=make local typeof((proc [f]()->const u64{observe(99u64);return f(1u64);})()){35u64};
 observe(9011u64);if(y){observe(x);}else{};observe(z);
}
{{func f=wrap(base);anonymous_queries(f);observe(f(1u64));}};
observe(9000u64);
'''
runner.HARNESS=runner.HARNESS.replace('checkpoints==13','checkpoints==14')
runner.EXPECTED_VALUES+=['34','35','2']
runner.EXTRA_SOURCES+=[Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
 raise SystemExit(runner.main())
