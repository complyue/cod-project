#!/usr/bin/env python3
"""Conditional borrowed returns use shared, depth-independent native bodies."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE='''
using symbol "std:io" @print;
using "std:exceptions" NullPointerException;
func step(ptr source:T in O,ptr spare:T in O,n:u64)->ptr T in O where {T,owner O}{
 if(n==0u64){
  ptr output=source,current=source;
  for(i in 0u64..3u64){
   {{defer REBIND;current=spare;}};
   current=source;
  };
  return output;
 }else{return step(source,spare,n-1u64);};
}
val a<-7,b<-8u64,c<-make local i64,d<-make local u64;
try{
 behave @print(step(a,c,DEPTHu64));
 behave @print(step(b,d,DEPTHu64));
}catch(e:const NullPointerException){behave @print(99u64);};
behave @print(9);
'''
runner.SPELLINGS={
    'call':('', '(=)(output,current)'),
    'intrinsic':('', 'intrinsic (=)(output,current)'),
}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
