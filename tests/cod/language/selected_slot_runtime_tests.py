#!/usr/bin/env python3
"""Selected reference slots allocate nothing and preserve image ownership on failure."""
from pathlib import Path
from native_test_support import run_native
import contextual_capture_runtime_tests as fixture
FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])

FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
proc selected_images(flag:bool)->void{
 func f=wrap(base);func g=wrap(other);ptr a=f,b=g,c=f;
 observe(9010u64);
 (if(flag){a}else{b})<=>c;
 (if(flag){b}else{a})=c;
 observe(9011u64);
 observe(a(1u64));observe(b(1u64));observe(c(1u64));
 observe(9030u64);
 try{((if(flag){a}else{b})=g)<-f;{}}catch(e:const OutOfMemoryError){observe(9031u64);};
 observe(a(1u64));observe(b(1u64));observe(c(1u64));
}
func selected_return(flag:bool)->const (func(u64)->const u64){
 func f=wrap(base);func g=wrap(other);ptr a=f,b=g;
 defer{(if(flag){a}else{b})=g;};return a;
}
selected_images(true);selected_images(false);
observe(selected_return(true)(1u64));observe(selected_return(false)(1u64));
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==10', 'checkpoints==11'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('copy_failures==5 && record_failures', 'copy_failures==7 && record_failures'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['2', '2', '2', '12', '2', '2', '12', '2', '12', '12', '12', '12', '2', '2'])
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
