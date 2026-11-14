#!/usr/bin/env python3
"""Contextual preparation and lexical recursion retain native image ownership."""
from pathlib import Path
import owner_dispatch_runtime_tests as fixture
runner=fixture.runner
runner.SOURCE += '''
func identity_prepared(x:T)->const T where T{return x;}
func apply_prepared(f:func(T)->const T,x:T)->const T where T{return f(x);}
func missing_prepared()->const (func(u64)->const u64){}
proc show_prepared(f:func(T)->const T,x:T)->void where T{observe(f(x)(1u64));}
{{
 func f=wrap(base);func g=wrap(other);
 proc recur[f](ref p:func(u64)->const u64 in O,n:u64)->void where {owner O}{
  if(n==0u64){p<-wrap(f);{}}else{recur(p,n-1u64);};
 };
 func read(ref p:func(u64)->const u64 in O,n:u64)->const u64 where {owner O}{
  if(n==0u64){return p(1u64);}else{return read(p,n-1u64);};
 };
 recur(g,40u64);observe(g(1u64));
 func copied=apply_prepared(identity_prepared,g);observe(copied(1u64));
 observe(9020u64);try{recur(g,0u64);}catch(e:const OutOfMemory){observe(9021u64);};observe(g(1u64));
 observe(9030u64);try{proc c=recur;}catch(e:const OutOfMemory){observe(9031u64);};
 observe(9050u64);try{defer recur(g,2u64);}catch(e:const OutOfMemory){observe(9051u64);};
 try{apply_prepared(identity_prepared,missing_prepared());{}}catch(e:const MissingReturnException){observe(320u64);};
 observe(9010u64);val total=0u64;
 for(i in 0u64..10000u64){total<-read(g,0u64)+total;};
 observe(9011u64);observe(total);
 defer recur(g,2u64);defer show_prepared(identity_prepared,g);g<-wrap(other);observe(g(1u64));
}};
observe(9000u64);
'''
runner.HARNESS=runner.HARNESS.replace('checkpoints==10','checkpoints==11').replace('creation_failures==4','creation_failures==5').replace('copy_failures==9','copy_failures==10').replace('record_failures==6','record_failures==7')
runner.EXPECTED_VALUES += ['3','3','3','320','30000','12','3']
runner.EXTRA_SOURCES += [Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
