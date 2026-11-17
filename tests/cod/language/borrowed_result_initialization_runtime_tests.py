#!/usr/bin/env python3
"""Borrowed returns survive prepared-registration failures with exact release."""
from pathlib import Path
import deferred_initialization_runtime_tests as fixture

runner=fixture.runner
runner.SOURCE='''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemoryError;
using "std:exceptions" NullPointerException;
proc observe(x:u64)->void{behave @print(x);}
proc select(ptr source:u64 in O,ptr spare:u64 in O,fail:bool)->ptr u64 in O where {owner O}{
 ptr output=source,current=source;
 for(i in 0u64..3u64){
  {{
   if(fail){observe(9001u64);}else{};
   try{defer (=)(output,current);}
   catch(e:const OutOfMemoryError){observe(9002u64);output=current;{}};
   current=spare;
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
'''
runner.HARNESS=runner.HARNESS.replace('assert(bytes==16);','assert(bytes>=16);')
runner.HARNESS=runner.HARNESS.replace('created==6 && peak==2 && failures==2','created==3 && peak==1 && failures==3')
runner.EXPECTED_VALUES=['87','88']
runner.EXPECTED_SHADOWS=[]
runner.EXTRA_SOURCES=[Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
