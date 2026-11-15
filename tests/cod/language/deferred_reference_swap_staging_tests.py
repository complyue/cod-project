#!/usr/bin/env python3
"""Deferred exchange retains one general recursive native method."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
func step(ref first:T in O,ref second:T in O,n:u64)->const T where {T,owner O}{
 if(n==0u64){ {{defer WRITE;}};return first;}
 else{return step(first,second,n-1u64);};
}
val a<-1,b<-7,c<-2u64,d<-8u64,e<-9;
ptr selected=a,other=e;
{{defer REBIND;}};
behave @print(step(a,b,DEPTHu64));behave @print(step(c,d,DEPTHu64));behave @print(selected);
'''
runner.SPELLINGS = {
    'call': ('(<=>)(first,second)', '(<=>)(selected,other)'),
    'intrinsic': ('intrinsic (<=>)(first,second)', 'intrinsic (<=>)(selected,other)'),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
