#!/usr/bin/env python3
"""Returned selections preserve shared generic and recursive native bodies."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" NullPointerException;
func relay(ptr p:T in O,n:u64)->ptr T in O where {T,owner O}{
 if(n==0u64){return p;}else{return relay(p,n-1u64);};
}
func step(marker:T,n:u64,choose:bool)->const T where T{
 if(n==0u64){
  val a<-make local T,b<-make local T;
  ptr selected=if(choose){a}else{b};
  ptr saved=relay(selected,DEPTHu64);
  try{WRITE;return selected;}catch(e:const NullPointerException){return marker;};
 }else{return step(marker,n-1u64,choose);};
}
behave @print(step(7,DEPTHu64,false));
behave @print(step(8u64,DEPTHu64,true));
behave @print(9);
'''
runner.SPELLINGS = {
    'infix': ('saved<-marker', ''),
    'call': ('(<-)(saved,marker)', ''),
    'intrinsic': ('intrinsic (<-)(saved,marker)', ''),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
