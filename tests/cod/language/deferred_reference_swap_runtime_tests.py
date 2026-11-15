#!/usr/bin/env python3
"""Retained binding slots exchange images without copying or losing failed records."""
from pathlib import Path
import deferred_assignment_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
proc swap_images(flag:bool)->void{
 func f=wrap(base);func g=wrap(other);func h=wrap(wrap(base));
 ptr p=f,q=g,r=h;
 {{defer intrinsic (<=>)((if(flag){p}else{q}),r);observe(9010u64);}};
 observe(9011u64);
 observe(p(1u64));observe(q(1u64));observe(r(1u64));
 observe(9050u64);
 try{defer (<=>)(p,q);}catch(e:const OutOfMemory){observe(9051u64);};
 observe(p(1u64));observe(q(1u64));observe(r(1u64));
}
swap_images(true);observe(9000u64);
swap_images(false);observe(9000u64);
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==6', 'checkpoints==8').replace('record_failures==2', 'record_failures==4')
runner.EXPECTED_VALUES += ['3','12','2','3','12','2','2','3','12','2','3','12']
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
