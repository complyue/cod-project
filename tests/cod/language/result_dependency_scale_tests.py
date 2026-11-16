#!/usr/bin/env python3
"""Shared result chains, recursive components and the explicit inference limit."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
runner.CASES = {}
for count in (8, 128):
    chain = [f'func node_{i}(){{return node_{i + 1}();}}' for i in range(count - 1)]
    chain.append(f'func node_{count - 1}(){{return 7u64;}}')
    ring = ['func node_0(n:u64){if(n==0u64){return 7u64;}else{return node_1(n-1u64);};}']
    ring += [f'func node_{i}(n:u64){{return node_{(i + 1) % count}(n);}}' for i in range(1, count)]
    for reverse in (False, True):
        for shape, definitions, call in [('chain', chain, 'node_0()'), ('ring', ring, 'node_0(3u64)')]:
            ordered = list(reversed(definitions)) if reverse else definitions
            runner.CASES[f'{shape}_{count}_{reverse}'] = (
                IO + ''.join(ordered) + f'behave @print({call});', 0, '7\n', '')
    unanchored = ''.join(f'func node_{i}(n:u64){{return node_{(i + 1) % count}(n);}}' for i in range(count))
    runner.CASES[f'unanchored_{count}'] = (unanchored, 2, '', 'unproved callable result')
runner.CASES['result_obligation_limit'] = (
    ''.join(f'func node_{i}(x:T) where T{{return node_{i + 1}(x);}}' for i in range(4096)) +
    'func node_4096(x:T) where T{return x;}val query<-make local typeof(node_0(0u64)){0u64};',
    2, '', 'compiler resource limit: 4096 callable result obligations; result remains unproved')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
