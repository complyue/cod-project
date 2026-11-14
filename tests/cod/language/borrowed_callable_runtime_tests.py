#!/usr/bin/env python3
"""Borrowed callable views preserve image ownership and allocation-free access."""
from pathlib import Path
import captured_callable_runtime_tests as fixture
runner=fixture.runner
runner.SOURCE += '''
func borrow_holder(ref f:func(u64)->const u64 in O)->ref (func(u64)->const u64) in O where {owner O}{return f;}
proc replace_borrowed(ref f:func(u64)->const u64 in O,ref g:func(u64)->const u64 in P)->void where {owner O,owner P}{f<-g;}
func walk_borrow(select:F,ref f:func(u64)->const u64 in O,n:u64)->ref (func(u64)->const u64) in O where {F,owner O}{
 if(n==0u64){return select(f);}else{return walk_borrow(select,f,n-1u64);};
}
{{
 func f=wrap(base);func g=wrap(other);func select=borrow_holder;
 ptr p=select(f);observe(p(1u64));
 proc writer=replace_borrowed;writer(f,g);observe(p(1u64));f<-wrap(base);
 observe(9030u64);try{writer(f,g);}catch(e:const OutOfMemory){observe(9031u64);};observe(p(2u64));
 func captured=func[select](ref h:func(u64)->const u64 in O)->ref (func(u64)->const u64) in O where {owner O}{return select(h);};
 ptr q=captured(f);q<-g;observe(f(3u64));
 observe(9030u64);try{captured({observe(9999u64);f});{}}catch(e:const OutOfMemory){observe(9031u64);};
 observe(9050u64);try{defer writer(f,g);}catch(e:const OutOfMemory){observe(9051u64);};
 f<-wrap(base);{{defer writer(f,g);f<-wrap(wrap(base));}};observe(f(4u64));
 observe(9010u64);val total=0u64;for(i in 0u64..10000u64){{ptr live=select(f);total<-total+live(i);}};observe(9011u64);observe(total);
 observe(walk_borrow(select,f,40u64)(5u64));
}};
observe(9000u64);
'''
runner.HARNESS=runner.HARNESS.replace('checkpoints==5','checkpoints==6').replace('copy_failures==3','copy_failures==5').replace('record_failures==1','record_failures==2')
runner.EXPECTED_VALUES += ['2','12','3','14','15','50105000','16']
runner.EXTRA_SOURCES += [Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
