#!/usr/bin/env python3
"""Aborted storage operations preserve complete images, targets and cleanup."""
from pathlib import Path
import named_completion_runtime_tests as fixture
runner=fixture.runner
runner.SOURCE += '''
proc stop_storage(ref f:func(u64)->const u64 in O)->const u64 where {owner O}{
 func copied=f;defer observe(41u64);
 func unused:func(u64)->const u64={{return copied(1u64);}};
 observe(9999u64);return 0u64;
}
func image_storage(f:func(u64)->const u64)->const (func(u64)->const u64){
 val unused<-make local u64 {return wrap(f);};return base;
}
proc rebind_storage(ref f:func(u64)->const u64 in O)->const u64 where {owner O}{
 ptr p=f;defer{observe(p(1u64));};p={{return 42u64;}};return 0u64;
}
proc swap_storage(ref f:func(u64)->const u64 in O,ref g:func(u64)->const u64 in O)->const u64 where {owner O}{
 ptr p=f,q=g;defer{observe(p(1u64));observe(q(1u64));};p<=>{{return 43u64;}};return 0u64;
}
func no_storage_copy()->const u64{
 func unused:func(u64)->const u64={{return 44u64;}};return 0u64;
}
{{
 func f=wrap(base);func g=wrap(other);
 observe(stop_storage(f));func result=image_storage(f);observe(result(1u64));
 observe(rebind_storage(f));observe(swap_storage(f,g));
 observe(9030u64);try{stop_storage(f);{}}catch(e:const OutOfMemory){observe(9031u64);};
 observe(9030u64);observe(no_storage_copy());observe(9061u64);
}};
observe(9000u64);
'''
runner.HARNESS=runner.HARNESS.replace('checkpoints==7','checkpoints==8').replace('copy_failures==4','copy_failures==5')
runner.HARNESS=runner.HARNESS.replace('  assert(value!=9999);','  if(value==9061){assert(deny_copy && copy_failures==5 && images.size()==live_baseline);deny_copy=false;return;}\n  assert(value!=9999);')
runner.EXPECTED_VALUES += ['41','2','3','2','42','2','12','43','44']
runner.EXTRA_SOURCES += [Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
