#!/usr/bin/env python3
"""Retained binding slots exchange images without copying or losing failed records."""
from pathlib import Path
from native_test_support import run_native
import deferred_assignment_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
proc swap_images(flag:bool)->void{
 func f=wrap(base);func g=wrap(other);func h=wrap(wrap(base));
 ptr p=f,q=g,r=h;
 {{defer intrinsic (<=>)((if(flag){p}else{q}),r);observe(9010u64);}};
 observe(9011u64);
 observe(p(1u64));observe(q(1u64));observe(r(1u64));
 observe(9050u64);
 try{defer (<=>)(p,q);}catch(e:const OutOfMemoryError){observe(9051u64);};
 observe(p(1u64));observe(q(1u64));observe(r(1u64));
}
swap_images(true);observe(9000u64);
swap_images(false);observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==6', 'checkpoints==8').replace('record_failures==2', 'record_failures==4'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['3','12','2','3','12','2','2','3','12','2','3','12'])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
