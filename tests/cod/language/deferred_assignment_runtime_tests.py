#!/usr/bin/env python3
"""Prepared assignment images survive mutation and release on registration failure."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
func selected_image(ref input:func(u64)->const u64 in O)->ref (func(u64)->const u64) in O where {owner O}{return input;}
{{
 func f=wrap(base);func g=wrap(other);
 {{
  defer intrinsic (<-)(f,g);
  observe(f(1u64));g<-wrap(base);observe(g(1u64));
  observe(9010u64);
 }};
 observe(9011u64);observe(f(1u64));
 observe(9030u64);
 try{defer (<-)(f,g);}catch(e:const OutOfMemory){observe(9031u64);};
 observe(f(1u64));
 observe(9050u64);
 try{defer intrinsic (<-)(f,g);}catch(e:const OutOfMemory){observe(9051u64);};
 observe(f(1u64));
 ptr p=f,q=g;
 func select_image=selected_image;
 {{defer (=)((if(true){p}else{q}),select_image(g));}};
 observe(p(1u64));
}};
observe(9000u64);
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==5', 'checkpoints==6')
runner.HARNESS = runner.HARNESS.replace('copy_failures==3', 'copy_failures==4').replace('record_failures==1', 'record_failures==2')
runner.EXPECTED_VALUES += ['2', '2', '12', '12', '12', '2']
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
