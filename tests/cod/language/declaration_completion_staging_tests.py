#!/usr/bin/env python3
"""Aborted receiving declarations match independently typed native programs."""
from pathlib import Path
from declaration_result_equivalence_tests import main

IO = 'using symbol "std:io" @print;'


def cases():
    for kind, annotation, assignment in (
        ('val', 'u64', '<-'), ('ref', 'u64', '='), ('ptr', 'u64', '='),
        ('func', 'func()->u64', '='), ('proc', 'proc()->u64', '='),
    ):
        typed = f'{kind} pending:{annotation}{assignment}'
        untyped = f'{kind} pending' + ' ' * (len(annotation) + 1) + assignment
        prefix = IO + 'proc observe(value:u64)->void{behave @print(value);}proc finish()->u64{defer observe(1u64);'
        suffix = '{{defer observe(2u64);return 7u64;}};observe(99u64);}observe(finish());'
        yield kind, prefix + typed + suffix, prefix + untyped + suffix, '2\n1\n7\n'
    for name, prefix, annotation, suffix, expected in (
        ('generic', IO+'func finish(input:T)->const T where T{val pending', 'T',
         '<-{return input;};}behave @print(finish(7u64));behave @print(finish(true));behave @print(finish(1.5f32));', '7\ntrue\n1.5\n'),
        ('recursive', IO+'func finish(n:u64)->u64{if(n==0u64){return 7u64;}else{};val pending', 'u64',
         '<-{return finish(n-1u64);};}behave @print(finish(100u64));', '7\n'),
        ('complete_result', IO+'func base()->u64{return 7u64;}func wrap(input:func()->u64)->func()->u64{return func[input]()->u64{return input();};}func finish(source:func()->u64)->func()->u64{func pending', 'func()->u64',
         '={{return wrap(source);}};}func retained=finish(base);behave @print(retained());', '7\n'),
    ):
        yield name, prefix + ':' + annotation + suffix, prefix + ' ' * (len(annotation) + 1) + suffix, expected


if __name__ == '__main__':
    raise SystemExit(main(cases(), extra_sources=[Path(__file__)]))
