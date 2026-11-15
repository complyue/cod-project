#!/usr/bin/env python3
"""Prepared reference intrinsics preserve image ownership and registration cleanup."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
{{
 func f=wrap(base);func g=wrap(other);ptr p=f,q=g;
 observe(9010u64);
 {{defer (is)(p,null);defer intrinsic (is)(q,null);p<=>q;}};
 observe(9011u64);observe(p(1u64));observe(q(1u64));
 observe(9050u64);
 try{defer (is)(p,null);}catch(e:const OutOfMemoryError){observe(9051u64);};
 observe(p(2u64));observe(q(2u64));
 val uninitialized<-make local u64;
 {{defer (is)(uninitialized,null);defer (is)(p,null);defer (is)(q,null);}};
}};
observe(9000u64);
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==5', 'checkpoints==6').replace('record_failures==1', 'record_failures==2')
runner.EXPECTED_VALUES += ['12', '2', '13', '3']
runner.EXPECTED_SHADOWS = []
runner.EXTRA_SOURCES += [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
