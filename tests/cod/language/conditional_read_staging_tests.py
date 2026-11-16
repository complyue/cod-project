#!/usr/bin/env python3
"""Conditional deferred reads preserve shared recursive native implementations."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
func step(marker:T,n:u64)->const T where {T}{
 if(n==0u64){
  val gate<-false;
  defer{ {{
   val saved<-make local T;
   if(gate){saved<-marker;{}}else{};
   return saved;
  }}; };
  WRITE;
  return marker;
 }else{return step(marker,n-1u64);};
}
behave @print(step(7,DEPTHu64));
behave @print(step(8u64,DEPTHu64));
behave @print(9);
'''
runner.SPELLINGS = {
    'infix': ('gate<-true', ''),
    'call': ('(<-)(gate,true)', ''),
    'intrinsic': ('intrinsic (<-)(gate,true)', ''),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
