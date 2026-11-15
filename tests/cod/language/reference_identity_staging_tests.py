#!/usr/bin/env python3
"""Identity spellings share native code without per-reference image allocation."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
func step(ptr target:T in O,ptr other:U in P,n:u64)->const i64 where {T,U,owner O,owner P}{
 if(n==0u64){if(WRITE){return 7;}else{return 8;};}
 else{return step(target,other,n-1u64);};
}
ptr first:i64=null in here;
behave @print(step(first,first,DEPTHu64));
{{ptr second:u64=null in here;behave @print(step(first,second,DEPTHu64));}};
behave @print(9);
'''
runner.SPELLINGS = {'infix': ('target is other',''), 'call': ('(is)(target,other)',''), 'intrinsic': ('intrinsic (is)(target,other)','')}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
