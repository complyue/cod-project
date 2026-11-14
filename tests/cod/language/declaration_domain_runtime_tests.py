#!/usr/bin/env python3
"""Contained declaration domains preserve complete-image cleanup and allocation-free borrowing."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
head = 'func wrap(f:func(u64)->const u64)'
result = '->func(u64)->const u64'
assert head + result + '{' in runner.SOURCE
runner.SOURCE = runner.SOURCE.replace(head + result + '{', head.replace('f:', 'f:const ') + result + ';' + head + '{')
runner.SOURCE = '''
func shared(ref x:const u64 in A,ref y:const u64 in B)->ref const u64 in A where {owner A,owner B};
func shared(ref x:u64 in O,ref y:u64 in O) where {owner O}{return x;}
''' + runner.SOURCE
assert 'total<-total+f(i);' in runner.SOURCE
runner.SOURCE = runner.SOURCE.replace('total<-total+f(i);', 'total<-shared(total,total)+f(i);')
runner.EXTRA_SOURCES += [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
