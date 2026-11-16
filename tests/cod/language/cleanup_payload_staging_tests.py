#!/usr/bin/env python3
"""Cleanup payload transforms retain one shared generic recursive body."""
from pathlib import Path
import exception_writeback_staging_tests as fixture

runner=fixture.runner
runner.SOURCE=runner.SOURCE.replace(' if(n==0u64){\n  WRITE;\n  {}\n }else{step(marker,target,item,n-1u64);};', ' defer WRITE;\n if(n!=0u64){step(marker,target,item,n-1u64);}else{};')
runner.EXTRA_SOURCES=[Path(fixture.__file__),Path(__file__)]
runner.SPELLINGS={
    'call':('(<-)(target,item)',''),
    'intrinsic':('intrinsic (<-)(target,item)',''),
    'grouped':('(<-)((target),(item))',''),
}
if __name__=='__main__':
    raise SystemExit(runner.main())
