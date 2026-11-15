#!/usr/bin/env python3
"""Exchange queries add no image work while captured callables remain live."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = runner.SOURCE.replace('observe(9010u64);val total<-0u64;',
    'ptr left=f,right=f;observe(9010u64);val total<-0u64;')
runner.SOURCE = runner.SOURCE.replace('total<-total+f(i);', '''
 {{val inspected<-make local typeof({left<=>right;1u64}){0u64};
   val raw<-make local u64;val observed<-make local typeof({raw<->raw;1u64}){0u64};
   total<-total+f(i)+inspected+observed;}};
''')
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
