#!/usr/bin/env python3
"""Scalar target/value reception adds no images to repeated captured invocation."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('total<-total+f(i);', '''
 {{val local<-0u64;ref selected=if(i%2u64==0u64){local}else{0u64};
   total<-total+f(i)+selected:void;}};
'''))
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
