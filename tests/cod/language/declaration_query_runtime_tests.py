#!/usr/bin/env python3
"""Inspect declared contextual calls without creating or copying callable images."""
from pathlib import Path
from native_test_support import run_native
import contextual_query_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func declared_query(callback:func(T)->const T,input:T)->const T where T;
func declared_many(callback:func(T)->const T,inputs:T...)->const T where T;
proc declared_proc(callback:proc(T)->const T,input:T)->const T where T;
proc query_effect(input:T)->const T where T{observe(99u64);return input;}
proc declared_queries(callback:func(u64)->const u64)->void{
 observe(9010u64);
 val first<-make local typeof(declared_query(unproved,true)){true};
 val second<-make local typeof(declared_query(callback,3u64)){34u64};
 val third<-make local typeof(declared_many(unproved,true,false)){true};
 val fourth<-make local typeof(declared_proc(query_effect,3u64)){35u64};
 observe(9011u64);
 if(first&&third){observe(second);observe(fourth);}else{};
 return;
}
declared_queries(wrap(base));observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==13', 'checkpoints==14'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['34', '35'])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
