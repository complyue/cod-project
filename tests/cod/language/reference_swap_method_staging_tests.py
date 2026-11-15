#!/usr/bin/env python3
"""Caller-slot methods share native bodies across spelling and recursion depth."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE = """
using symbol "std:io" @print;
func (<=>)(ref a:T in O,ref b:T in O)->void where {T,owner O}{intrinsic (<=>)(a,b);}
func step(value:T,next:T,n:u64)->const T where T{
 if(n==0u64){
  val a<-value,b<-next;ptr p=a,q=b;WRITE;return p;
 }else{return step(value,next,n-1u64);};
}
behave @print(step(1,7,DEPTHu64));behave @print(step(2u64,8u64,DEPTHu64));behave @print(9);
"""
runner.SPELLINGS = {'infix': ('p<=>q',''), 'call': ('(<=>)(p,q)','')}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
