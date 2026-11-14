#!/usr/bin/env python3
"""Owner-selected native calls preserve images, preparation failures and reuse."""
from pathlib import Path
import access_dispatch_runtime_tests as fixture
runner=fixture.runner
runner.SOURCE += '''
proc visit_owner(ref f:func(u64)->const u64 in O,ref g:func(u64)->const u64 in O)->void where {owner O}{f<-wrap(g);}
proc visit_owner(ref f:func(u64)->const u64 in O,ref g:func(u64)->const u64 in P)->void where {owner O,owner P}{observe(f(0u64));}
proc copy_owner(ref f:func(u64)->const u64 in O,ref g:func(u64)->const u64 in O)->void where {owner O}{func c=f;observe(c(0u64));}
proc copy_owner(ref f:func(u64)->const u64 in O,ref g:func(u64)->const u64 in P)->void where {owner O,owner P}{observe(9999u64);}
func owner_first(ref f:func(u64)->const u64 in O,ref g:func(u64)->const u64 in P)->ref (func(u64)->const u64) in O where {owner O,owner P}{return f;}
func owner_second(ref f:func(u64)->const u64 in O,ref g:func(u64)->const u64 in P)->ref (func(u64)->const u64) in P where {owner O,owner P}{return g;}
func owner_read(ref f:func(u64)->const u64 in O,ref g:func(u64)->const u64 in O)->const u64 where {owner O}{return g(0u64)+100u64;}
func owner_read(ref f:func(u64)->const u64 in O,ref g:func(u64)->const u64 in P)->const u64 where {owner O,owner P}{return g(0u64)+200u64;}
func walk_owner(select:F,ref f:func(u64)->const u64 in O,n:u64)->const u64 where {F,owner O}{
 if(n==0u64){func g=base;ptr p=select(f,g);return owner_read(f,p);}else{return walk_owner(select,f,n-1u64);};
}
{{
 func f=wrap(other);func g=base;
 visit_owner(f,g);observe(f(0u64));
 {{func h=other;visit_owner(f,h);}};
 f<-wrap(other);observe(9020u64);try{visit_owner(f,g);}catch(e:const OutOfMemory){observe(9021u64);};observe(f(0u64));
 observe(9030u64);try{copy_owner(f,g);}catch(e:const OutOfMemory){observe(9031u64);};observe(f(0u64));
 observe(9050u64);try{defer visit_owner(f,g);}catch(e:const OutOfMemory){observe(9051u64);};
 {{defer visit_owner(f,g);}};observe(f(0u64));
 func first=owner_first;func second=owner_second;
 observe(walk_owner(first,f,40u64));observe(walk_owner(second,f,40u64));
 observe(9010u64);val total=0u64;
 for(i in 0u64..10000u64){total<-walk_owner(second,f,0u64)+total;};
 observe(9011u64);observe(total);
}};
observe(9000u64);
'''
runner.HARNESS=runner.HARNESS.replace('checkpoints==9','checkpoints==10').replace('creation_failures==3','creation_failures==4').replace('copy_failures==8','copy_failures==9').replace('record_failures==5','record_failures==6')
runner.EXPECTED_VALUES += ['1','1','11','11','1','101','200','2000000']
runner.EXTRA_SOURCES += [Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
