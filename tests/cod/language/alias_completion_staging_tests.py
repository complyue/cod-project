#!/usr/bin/env python3
"""Alias selector exits retain shared generic recursion without alias storage."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
func step(value:T,n:u64)->const T where T{
 if(n==0u64){ WRITE;return value;}
 else{return step(value,n-1u64);};
}
behave @print(step(7,DEPTHu64));behave @print(step(8u64,DEPTHu64));behave @print(9);
'''
runner.SPELLINGS = {
    'alias': ('var ignored->{{return value;}}', ''),
    'kind_alias': ('val var ignored->{{return value;}}', ''),
    'typed_alias': ('val var ignored:T->{{return value;}}', ''),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
