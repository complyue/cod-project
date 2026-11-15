#!/usr/bin/env python3
"""Inspect declared contextual calls without creating or copying callable images."""
from pathlib import Path
import contextual_query_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
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
}
declared_queries(wrap(base));observe(9000u64);
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==13', 'checkpoints==14')
runner.EXPECTED_VALUES += ['34', '35']
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
