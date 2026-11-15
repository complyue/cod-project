#!/usr/bin/env python3
"""Prepared assignments retain shared recursive native implementations."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
func step(ref target:T in O,next:T,n:u64)->void where {T,owner O}{
 defer WRITE;
 if(n>0u64){step(target,next,n-1u64);}else{};
}
val a<-1,b<-2u64,c<-9;
ptr selected=a;
{{defer REBIND;}};
step(a,7,DEPTHu64);step(b,8u64,DEPTHu64);
behave @print(a);behave @print(b);behave @print(selected);
'''
runner.SPELLINGS = {name: forms for name, forms in runner.SPELLINGS.items() if name != 'infix'}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
