#!/usr/bin/env python3
"""Caller target bounds discharge generic borrowed reads without native cloning."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE='''
using symbol "std:io" @print;
using "std:exceptions" NullPointerException;
func fill(ptr selected:const T in O,ptr first:T in O,ptr second:T in O,value:T)->const T throws NullPointerException where {T,owner O}{
 ptr output=first;
 if(selected is first){
  if(output is null){output;{}}else{};
  WRITE;
  {}
 }else{
  output=second;
  if(output is null){output;{}}else{};
  WRITE;
  {}
 };
 return selected;
}
func step(marker:T,n:u64,choose:bool)->const T where T{
 if(n==0u64){
  val a<-make local T,b<-make local T;
  ptr selected=if(choose){a}else{b};
  try{return fill(selected,a,b,marker);}
  catch(e:const NullPointerException){return marker;};
 }else{return step(marker,n-1u64,choose);};
}
behave @print(step(7,DEPTHu64,false));
behave @print(step(8u64,DEPTHu64,true));
behave @print(9);
'''
runner.SPELLINGS={
    'infix':('output<-value',''),
    'call':('(<-)(output,value)',''),
    'intrinsic':('intrinsic (<-)(output,value)',''),
}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
