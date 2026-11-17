#!/usr/bin/env python3
"""Exchange queries add no image work while captured callables remain live."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('observe(9010u64);val total<-0u64;',
    'ptr left=f,right=f;observe(9010u64);val total<-0u64;'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('total<-total+f(i);', '''
 {{val inspected<-make local typeof({left<=>right;1u64}){0u64};
   val raw<-make local u64;val observed<-make local typeof({raw<->raw;1u64}){0u64};
   total<-total+f(i)+inspected+observed;}};
'''))
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
