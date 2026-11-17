#!/usr/bin/env python3
"""Symbol type inspection creates no runtime binding or captured image."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
symbol @amount:u64;
{{
 func retained=wrap(base);
 observe(9010u64);
 func inspect=func[]()->u64 {val x<-make local typeof(@amount){17u64};return x;};
 observe(inspect());
 func nested=func[]()->u64 {
   func inner=func[]()->u64 {val x<-make local typeof(@amount){18u64};return x;};
   return inner();
 };
 observe(nested());
 observe(9011u64);
}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==5', 'checkpoints==6'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['17', '18'])
FIXTURE = FIXTURE.updated(expected_shadows=['retained'])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
