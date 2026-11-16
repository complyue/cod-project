#!/usr/bin/env python3
"""Nested recovery registrations preserve shared generic recursive native code."""
from pathlib import Path
import handler_payload_staging_tests as fixture

runner=fixture.runner
runner.SOURCE=runner.SOURCE.replace('defer catch(e){WRITE;{}};', 'defer catch(e){defer WRITE;};')
runner.SPELLINGS={
    'call':('(<-)(target,e)',''),
    'intrinsic':('intrinsic (<-)(target,e)',''),
    'grouped':('(<-)((target),(e))',''),
}
runner.EXTRA_SOURCES+=[Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
