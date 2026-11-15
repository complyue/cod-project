#!/usr/bin/env python3
"""Caller-slot callable invocation shares native bodies across recursive depths."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE = runner.SOURCE.replace('func step(', '''
func leaf()->const u64{return 1u64;}
func (<=>)(ref left:func()->const u64 in O,ref right:func()->const u64 in O)->void where {owner O}{
 if(left()==right()){intrinsic (<=>)(left,right);}else{};
}
func step(''').replace(' if(n==0u64){', ' func first=leaf;func second=leaf;ptr p=first,q=second;\n if(n==0u64){')
runner.SPELLINGS = {'infix':('p<=>q;target<-next','selected=c'),
                    'call':('(<=>)(p,q);target<-next','selected=c')}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
