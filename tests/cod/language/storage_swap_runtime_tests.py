#!/usr/bin/env python3
"""Swap selection, failure and cleanup retain exact native storage behavior."""
from pathlib import Path
import mutable_reference_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = runner.SOURCE.replace('ptr p=a;', 'ptr p=a,q=b;').replace('p=b;', 'p<=>q;')
runner.SOURCE = runner.SOURCE.replace('defer{p=a;};', 'defer{p<=>q;};')
runner.SOURCE += '''
using "std:exceptions" NullPointerException;
proc bits_order()->void{
 val x=71u64,y=72u64;ptr p=x,q=y;
 p<->{p=q;q};observe(x);observe(y);observe(p);
 try{x<->{y<-73u64;missing();y};}catch(e:const MissingReturnException){};
 observe(x);observe(y);
 ptr absent:u64=null;
 try{absent<->{observe(75u64);q};}catch(e:const NullPointerException){observe(76u64);};
 observe(y);
}
bits_order();
'''
runner.EXPECTED_VALUES += ['72','71','71','72','73','75','76','73']
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
