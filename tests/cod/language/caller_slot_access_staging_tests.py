#!/usr/bin/env python3
"""Access-preserving caller-slot methods reuse shared native bodies."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE='''
using symbol "std:io" @print;
func (<=>)(ptr left:const T in O,ptr right:const T in O)->void where {T,owner O}{
 intrinsic (<=>)(left,right);
}
func step(marker:T,n:u64)->const T where T{
 if(n==0u64){
  val x<-marker,y<-marker+marker;
  ptr p=x,q=y;
  ptr var ro:const->p;ptr var other:const->q;
  REBIND;
  return p-marker;
 }else{return step(marker,n-1u64);};
}
behave @print(step(7,DEPTHu64));
behave @print(step(8u64,DEPTHu64));
behave @print(9);
'''
runner.SPELLINGS={'infix':('', 'ro<=>other'),'call':('', '(<=>)(ro,other)')}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
