#!/usr/bin/env python3
"""Assignment type inspection adds no copies, mutation or repeated-invocation work."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = runner.SOURCE.replace('observe(9010u64);val total<-0u64;',
    'ptr left=f,right=f;observe(9010u64);val total<-0u64;')
runner.SOURCE = runner.SOURCE.replace('total<-total+f(i);', '''
 {{val inspected<-make local typeof((left=right)(i)){0u64};
   val copied<-make local typeof((f<-f)(i)){0u64};
   val raw<-make local u64;val observed<-make local typeof(raw<-raw){0u64};
   total<-total+f(i)+inspected+copied+observed;}};
''')
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
