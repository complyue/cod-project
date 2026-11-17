#!/usr/bin/env python3
"""Inductive loop initialization preserves generic recursive native sharing."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE='''
using symbol "std:io" @print;
func step(marker:T,n:u64)->const T where T{
 if(n==0u64){
  val a<-marker,b<-make local T;
  ptr selected=a,output=a;
  for(i in 0u64..3u64){
   WRITE;
   if(i==0u64){b<-marker;selected=b;}else{selected=a;};
  };
  return selected;
 }else{return step(marker,n-1u64);};
}
behave @print(step(7,DEPTHu64));
behave @print(step(8u64,DEPTHu64));
behave @print(9);
'''
runner.SPELLINGS={
    'infix':('output<-selected',''),
    'call':('(<-)(output,selected)',''),
    'intrinsic':('intrinsic (<-)(output,selected)',''),
}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
