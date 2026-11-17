#!/usr/bin/env python3
"""Exception overlap preserves retained causes, deferred failures and exact cleanup."""
from pathlib import Path
from native_test_support import run_native
import unknown_exception_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])

marker = 'func relay_exception(e:Exception)->void throws Exception{throw e;}'
assert marker in FIXTURE.source
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(marker, '''
func relay_exception(e:MissingReturnError)->void throws Exception;
func mark(a:T,b:T)->u64 where T;
func mark(a:Exception,b:Exception)->u64{return 1u64;}
''' + marker))
marker = 'catch(e:const StaticallyUnknownTypeException){total<-total+1u64;}'
assert marker in FIXTURE.source
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(marker, 'catch(e:const StaticallyUnknownTypeException){total<-total+mark(e,e);}'))
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
