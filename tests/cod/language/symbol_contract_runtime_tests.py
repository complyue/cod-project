#!/usr/bin/env python3
"""Symbol type inspection creates no runtime binding or captured image."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
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
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==5', 'checkpoints==6')
runner.EXPECTED_VALUES += ['17', '18']
runner.EXTRA_SOURCES += [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
