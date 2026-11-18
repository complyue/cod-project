#!/usr/bin/env python3
"""Independent helper cleanup preserves shared generic recursive native code."""
from pathlib import Path
import handler_payload_staging_tests as fixture

runner=fixture.runner
runner.SOURCE=runner.SOURCE.replace(
    ' defer catch(e){WRITE;{}};\n if(n==0u64){throw item;}else{step(marker,target,item,n-1u64);};',
    ' try{if(n==0u64){throw item;}else{step(marker,target,item,n-1u64);};}catch(e){write_in_helper(target,e);};')
runner.SOURCE+='func write_in_helper(ref target:Exception in O,e:Exception)->void where {owner O}{defer WRITE;}'
runner.SPELLINGS={
    'call':('(<-)(target,e)',''),
    'intrinsic':('intrinsic (<-)(target,e)',''),
    'grouped':('(<-)((target),(e))',''),
}
runner.EXTRA_SOURCES+=[Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
