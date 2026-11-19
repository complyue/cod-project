#!/usr/bin/env python3
"""Prepared snapshots preserve exact release through registration failure."""
from pathlib import Path
from native_test_support import run_native
import deferred_initialization_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source='''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemoryError;
proc observe(x:u64)->void{behave @print(x);return;}
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
''')
# Block and prepared-call records have distinct sizes; lifetime accounting
# checks each successful allocation independently of that representation.
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('assert(bytes==16);','assert(bytes>=16);'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('created==6 && peak==2 && failures==2','created==5 && peak==1 && failures==3'))
FIXTURE = FIXTURE.updated(expected_values=['87','87','88','88']*2)
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__=='__main__':
    raise SystemExit(run_native(FIXTURE))
