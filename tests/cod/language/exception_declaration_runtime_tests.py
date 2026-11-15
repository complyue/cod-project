#!/usr/bin/env python3
"""Exception overlap preserves retained causes, deferred failures and exact cleanup."""
from pathlib import Path
import unknown_exception_runtime_tests as runner

marker = 'func relay_exception(e:SystemException)->void throws SystemException{throw e;}'
assert marker in runner.SOURCE
runner.SOURCE = runner.SOURCE.replace(marker, '''
func relay_exception(e:MissingReturnException)->void throws SystemException;
func mark(a:T,b:T)->u64 where T;
func mark(a:SystemException,b:SystemException)->u64{return 1u64;}
''' + marker)
marker = 'catch(e:const StaticallyUnknownType){total<-total+1u64;}'
assert marker in runner.SOURCE
runner.SOURCE = runner.SOURCE.replace(marker, 'catch(e:const StaticallyUnknownType){total<-total+mark(e,e);}')
runner.EXTRA_SOURCES += [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
