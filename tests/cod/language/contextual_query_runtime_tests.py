#!/usr/bin/env python3
"""Contextual signatures inspect queries while real calls preserve owned snapshots."""
from pathlib import Path
from native_test_support import run_native
import type_query_runtime_tests as fixture
FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func query_apply(f:func(T)->const T,x:T)->const T where T{return f(x);}
proc contextual_queries(f:func(u64)->const u64)->void{
 observe(9010u64);
 val x<-make local typeof(query_apply(unproved,true)){true};
 val y<-make local typeof(query_apply(f,3u64)){32u64};
 observe(9011u64);if(x){observe(y);}else{};
 return;
}
proc query_later()->const u64{observe(33u64);return 1u64;}
{{
 func f=wrap(base);contextual_queries(f);
 observe(9030u64);
 try{query_apply(f,query_later());{}}catch(e:const OutOfMemoryError){observe(9031u64);};
 observe(f(1u64));
}};
observe(query_apply(wrap(base),query_apply(wrap(other),1u64)));
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==12','checkpoints==13').replace('copy_failures==7 && record_failures','copy_failures==8 && record_failures'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['32','2','13'])
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__=='__main__':
 raise SystemExit(run_native(FIXTURE))
