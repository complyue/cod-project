#!/usr/bin/env python3
"""Per-input recovery proofs retain shared generic recursive native bodies."""
from pathlib import Path
import exception_writeback_staging_tests as fixture

runner=fixture.runner
runner.SOURCE=runner.SOURCE.replace(' if(n==0u64){\n  WRITE;\n  {}\n }else{step(marker,target,item,n-1u64);};', ' defer catch(e){WRITE;{}};\n if(n==0u64){throw item;}else{step(marker,target,item,n-1u64);};')
runner.SPELLINGS={
    'infix':('target<-e',''),
    'call':('(<-)(target,e)',''),
    'intrinsic':('intrinsic (<-)(target,e)',''),
}
runner.EXTRA_SOURCES=[Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
