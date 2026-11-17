#!/usr/bin/env python3
"""Contained declaration domains preserve complete-image cleanup and allocation-free borrowing."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
head = 'func wrap(f:func(u64)->const u64)'
result = '->func(u64)->const u64'
assert head + result + '{' in FIXTURE.source
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(head + result + '{', head.replace('f:', 'f:const ') + result + ';' + head + '{'))
FIXTURE = FIXTURE.updated(source='''
func shared(ref x:const u64 in A,ref y:const u64 in B)->ref const u64 in A where {owner A,owner B};
func shared(ref x:u64 in O,ref y:u64 in O) where {owner O}{return x;}
''' + FIXTURE.source)
assert 'total<-total+f(i);' in FIXTURE.source
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('total<-total+f(i);', 'total<-shared(total,total)+f(i);'))
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
