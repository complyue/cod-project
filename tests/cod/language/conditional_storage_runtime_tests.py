#!/usr/bin/env python3
"""Scalar target/value reception adds no images to repeated captured invocation."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = runner.SOURCE.replace('total<-total+f(i);', '''
 {{val local<-0u64;ref selected=if(i%2u64==0u64){local}else{0u64};
   total<-total+f(i)+selected;}};
''')
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
