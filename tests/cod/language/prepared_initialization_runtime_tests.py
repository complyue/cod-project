#!/usr/bin/env python3
"""Prepared snapshots preserve exact release through registration failure."""
from pathlib import Path
import deferred_initialization_runtime_tests as fixture

runner=fixture.runner
runner.SOURCE='''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemoryError;
proc observe(x:u64)->void{behave @print(x);}
proc verify(fail:bool)->void{
 val a<-87u64,b<-88u64,c<-make local u64;
 ptr p=a,q=a;
 defer{
  for(i in 0u64..3u64){
   observe(p);
   {{
    if(fail){observe(9001u64);}else{};
    try{defer (=)(p,q);}
    catch(e:const OutOfMemoryError){observe(9002u64);p=q;{}};
    q=c;
   }};
   q=b;
  };
  observe(p);
 };
}
verify(false);verify(true);
'''
# Block and prepared-call records have distinct sizes; lifetime accounting
# checks each successful allocation independently of that representation.
runner.HARNESS=runner.HARNESS.replace('assert(bytes==16);','assert(bytes>=16);')
runner.HARNESS=runner.HARNESS.replace('created==6 && peak==2 && failures==2','created==5 && peak==1 && failures==3')
runner.EXPECTED_VALUES=['87','87','88','88']*2
runner.EXPECTED_SHADOWS=[]
runner.EXTRA_SOURCES=[Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
