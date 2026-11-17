#!/usr/bin/env python3
"""Assignment type inspection adds no copies, mutation or repeated-invocation work."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('observe(9010u64);val total<-0u64;',
    'ptr left=f,right=f;observe(9010u64);val total<-0u64;'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('total<-total+f(i);', '''
 {{val inspected<-make local typeof((left=right)(i)){0u64};
   val copied<-make local typeof((f<-f)(i)){0u64};
   val raw<-make local u64;val observed<-make local typeof(raw<-raw){0u64};
   total<-total+f(i)+inspected+copied+observed;}};
'''))
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
