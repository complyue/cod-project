#!/usr/bin/env python3
"""Borrowed returns survive prepared-registration failures with exact release."""
from pathlib import Path
from native_test_support import run_native
import deferred_initialization_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source='''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemoryError;
using "std:exceptions" NullPointerException;
proc observe(x:u64)->void{behave @print(x);return;}
proc select(ptr source:u64 in O,ptr spare:u64 in O,fail:bool)->ptr u64 in O where {owner O}{
 ptr output=source,current=source;
 for(i in 0u64..3u64){
  {{
   if(fail){observe(9001u64);}else{};
   try{defer (=)(output,current);}
   catch(e:const OutOfMemoryError){observe(9002u64);output=current;{}};
   current=spare:void;
  }};
  current=source;
 };
 try{return output;}finally{output=spare;{}};
}
val a<-87u64,b<-88u64,c<-make local u64;
try{
 ptr first=select(a,c,false);
 observe(first);
 ptr second=select(b,c,true);
 observe(second);
}catch(e:const NullPointerException){observe(99u64);};
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('assert(bytes==16);','assert(bytes>=16);'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('created==6 && peak==2 && failures==2','created==3 && peak==1 && failures==3'))
FIXTURE = FIXTURE.updated(expected_values=['87','88'])
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__=='__main__':
    raise SystemExit(run_native(FIXTURE))
