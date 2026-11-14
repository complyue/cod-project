#!/usr/bin/env python3
"""Selected reference slots allocate nothing and preserve image ownership on failure."""
from pathlib import Path
import contextual_capture_runtime_tests as fixture
runner = fixture.runner

runner.SOURCE += '''
proc selected_images(flag:bool)->void{
 func f=wrap(base);func g=wrap(other);ptr a=f,b=g,c=f;
 observe(9010u64);
 (if(flag){a}else{b})<=>c;
 (if(flag){b}else{a})=c;
 observe(9011u64);
 observe(a(1u64));observe(b(1u64));observe(c(1u64));
 observe(9030u64);
 try{((if(flag){a}else{b})=g)<-f;{}}catch(e:const OutOfMemory){observe(9031u64);};
 observe(a(1u64));observe(b(1u64));observe(c(1u64));
}
func selected_return(flag:bool)->const (func(u64)->const u64){
 func f=wrap(base);func g=wrap(other);ptr a=f,b=g;
 defer{(if(flag){a}else{b})=g;};return a;
}
selected_images(true);selected_images(false);
observe(selected_return(true)(1u64));observe(selected_return(false)(1u64));
observe(9000u64);
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==10', 'checkpoints==11')
runner.HARNESS = runner.HARNESS.replace('copy_failures==5 && record_failures', 'copy_failures==7 && record_failures')
runner.EXPECTED_VALUES += ['2', '2', '2', '12', '2', '2', '12', '2', '12', '12', '12', '12', '2', '2']
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
