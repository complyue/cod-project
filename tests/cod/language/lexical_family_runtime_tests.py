#!/usr/bin/env python3
"""Every lexical generic implementation uses the complete captured image."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = runner.SOURCE.replace('func[f](x:u64)->const u64{', 'func[f](x:T)->const u64 where T{')
runner.SOURCE = runner.SOURCE.replace('proc[p](x:u64)->void{', 'proc[p](x:T)->void where T{')
for before, after in [('g(1u64)', 'g(1u8)'), ('f(1u64)', 'f(1u32)'), ('f(2u64)', 'f(2u8)'),
                      ('f(3u64)', 'f(3u32)'), ('f(5u64)', 'f(5u8)'), ('()(7u64)', '()(7u8)')]:
    runner.SOURCE = runner.SOURCE.replace(before, after)
runner.EXPECTED_SHADOWS = []
runner.EXTRA_SOURCES += [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
