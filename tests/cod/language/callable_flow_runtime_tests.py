#!/usr/bin/env python3
"""Callable-flow proofs preserve nested images, native recursion and cleanup."""
from pathlib import Path
import borrowed_callable_runtime_tests as fixture
runner=fixture.runner
runner.SOURCE += '''
using "std:exceptions" NullPointerException;
proc set_target(ptr x:u64 in O)->void throws NullPointerException where {owner O}{x<-7u64;}
proc ignore_target(ptr x:u64 in O)->void throws NullPointerException where {owner O}{}
func wrap_writer(f:F)->const F where F{
 return proc[f](ptr x:u64 in O)->void throws NullPointerException where {owner O}{f(x);};
}
proc walk_writer(f:F,ptr x:u64 in O,n:u64)->void throws NullPointerException where {F,owner O}{
 if(n==0u64){f(x);}else{walk_writer(f,x,n-1u64);};
}
{{
 proc unused=ignore_target;proc selected=set_target;
 for(i in 0u64..80u64){{selected<-wrap_writer(selected);}};
 val x=make local u64;
 try{walk_writer(selected,x,40u64);observe(x);}catch(e:const NullPointerException){observe(9999u64);};
 observe(9020u64);try{selected<-wrap_writer(selected);{}}catch(e:const OutOfMemory){observe(9021u64);};
 proc other=wrap_writer(set_target);
 observe(9030u64);try{selected<-other;{}}catch(e:const OutOfMemory){observe(9031u64);};
 observe(9050u64);try{0;{{defer selected(x);}};}catch(e:const OutOfMemory){observe(9051u64);}catch(e:const NullPointerException){observe(9999u64);};
 try{0;{{defer selected(x);}};observe(x);}catch(e:const NullPointerException){observe(9999u64);};
 observe(9010u64);
 try{for(i in 0u64..10000u64){selected(x);};observe(x);}catch(e:const NullPointerException){observe(9999u64);};
 observe(9011u64);
}};
observe(9000u64);
'''
runner.HARNESS=runner.HARNESS.replace('checkpoints==6','checkpoints==7').replace('creation_failures==1','creation_failures==2').replace('copy_failures==5','copy_failures==6').replace('record_failures==2','record_failures==3')
runner.EXPECTED_VALUES += ['7','7','7']
runner.EXTRA_SOURCES += [Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
