#!/usr/bin/env python3
"""Compare guard admission and execution with finite owner/locator identities."""
from itertools import product
from pathlib import Path
import foundation_tests as runner

ATOMS = (
    ('p is q', lambda p, q: p == q),
    ('p is not q', lambda p, q: p != q),
    ('p is null', lambda p, q: p[1] is None),
    ('p is not null', lambda p, q: p[1] is not None),
    ('q is null', lambda p, q: q[1] is None),
    ('q is not null', lambda p, q: q[1] is not None),
)
OUTER = [('x', (0, 7)), ('y', (0, 11)), ('absent', (0, None))]
INNER = [('z', (1, 13)), ('w', (1, 17)), ('empty', (1, None))]
runner.CASES = {}
for distinct_roles in (False, True):
    samples = list(product(OUTER, OUTER))
    if distinct_roles:
        samples += list(product(OUTER, INNER))
    for index, ((left, lhs), (right, rhs), conjunction) in enumerate(product(ATOMS, ATOMS, (False, True))):
        condition = '('+left+')'+('&&' if conjunction else '||')+'('+right+')'
        outcomes = [(lhs(p, q) and rhs(p, q)) if conjunction else (lhs(p, q) or rhs(p, q))
                    for (_, p), (_, q) in samples]
        # Contradictory locator relations require a stronger proof than nullness.
        if not any(outcomes):
            continue
        unsafe = any(taken and q[1] is None for taken, (_, (_, q)) in zip(outcomes, samples))
        parameters = 'ptr p:i64 in P,ptr q:i64 in Q' if distinct_roles else 'ptr p:i64 in O,ptr q:i64 in O'
        owners = 'owner P,owner Q' if distinct_roles else 'owner O'
        source = 'using symbol "std:io" @print;proc choose('+parameters+')->void where {'+owners+'}{'
        source += 'if('+condition+'){behave @print(q);}else{behave @print(0);};return;}'
        source += 'val x<-7,y<-11;ptr absent:i64=null in here;'
        source += ''.join('choose('+p+','+q+');' for (p, _), (q, _) in samples[:9])
        if distinct_roles:
            source += '{{val z<-13,w<-17;ptr empty:i64=null in here;'
            source += ''.join('choose('+p+','+q+');' for (p, _), (q, _) in samples[9:])+'}};'
        expected = ''.join(str(q[1] if taken else 0)+'\n' for taken, (_, (_, q)) in zip(outcomes, samples))
        name = ('independent_' if distinct_roles else 'same_')+str(index)
        runner.CASES[name] = (source, 2 if unsafe else 0, '' if unsafe else expected,
                              'requires a covering catch' if unsafe else '')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
