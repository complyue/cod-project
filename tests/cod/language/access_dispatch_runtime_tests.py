#!/usr/bin/env python3
"""Access-selected calls retain complete values and failure-safe preparation."""
from pathlib import Path
import nullable_flow_runtime_tests as fixture
runner=fixture.runner
runner.SOURCE += '''
proc visit(ref f:func(u64)->const u64 in O)->void where {owner O}{f<-wrap(base);}
proc visit(ref f:const (func(u64)->const u64) in O)->void where {owner O}{observe(f(1u64));}
proc consume_access(f:func(u64)->const u64)->void{observe(9999u64);}
proc consume_access(f:const (func(u64)->const u64))->void{observe(9999u64);}
func read_holder(ref f:func(u64)->const u64 in O)->ref const (func(u64)->const u64) in O where {owner O}{return f;}
func read_access(ref f:func(u64)->const u64 in O)->const u64 where {owner O}{return f(0u64)+100u64;}
func read_access(ref f:const (func(u64)->const u64) in O)->const u64 where {owner O}{return f(0u64)+200u64;}
func walk_access(select:F,ref f:func(u64)->const u64 in O,n:u64)->const u64 where {F,owner O}{
 if(n==0u64){return read_access(select(f));}else{return walk_access(select,f,n-1u64);};
}
{{
 func f=wrap(wrap(other));ptr observed:const (func(u64)->const u64)=f;
 visit(observed);visit(f);visit(observed);
 f<-wrap(other);observe(9020u64);try{visit(f);}catch(e:const OutOfMemory){observe(9021u64);};visit(observed);
 observe(9030u64);try{consume_access(observed);}catch(e:const OutOfMemory){observe(9031u64);};visit(observed);
 observe(9050u64);try{defer visit(observed);}catch(e:const OutOfMemory){observe(9051u64);};
 {{defer visit(observed);f<-wrap(base);}};
 func writable=borrow_holder;func readonly=read_holder;
 observe(walk_access(writable,f,40u64));observe(walk_access(readonly,f,40u64));
 observe(9010u64);val total<-0u64;
 for(i in 0u64..10000u64){total<-walk_access(readonly,f,0u64)+total;};
 observe(9011u64);observe(total);
}};
observe(9000u64);
'''
runner.HARNESS=runner.HARNESS.replace('checkpoints==8','checkpoints==9').replace('creation_failures==2','creation_failures==3').replace('copy_failures==7','copy_failures==8').replace('record_failures==4','record_failures==5')
runner.EXPECTED_VALUES += ['13','2','12','12','2','101','201','2010000']
runner.EXTRA_SOURCES += [Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
