#!/usr/bin/env python3
"""Swap selection, failure and cleanup retain exact native storage behavior."""
from pathlib import Path
from native_test_support import run_native
import mutable_reference_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('ptr p=a;', 'ptr p=a,q=b;').replace('p=b;', 'p<=>q;'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('defer{p=a;};', 'defer{p<=>q;};'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
using "std:exceptions" NullPointerException;
proc bits_order()->void{
 val x<-71u64,y<-72u64;ptr p=x,q=y;
 p<->{p=q;q};observe(x);observe(y);observe(p);
 try{x<->{y<-73u64;missing();y};}catch(e:const MissingReturnError){};
 observe(x);observe(y);
 ptr absent:u64=null in here;
 try{absent<->{observe(75u64);q};}catch(e:const NullPointerException){observe(76u64);};
 observe(y);
}
bits_order();
''')
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['72','71','71','72','73','75','76','73'])
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
