#!/usr/bin/env python3
"""Partial declaration proofs preserve complete images and failure cleanup."""
from pathlib import Path
from native_test_support import run_native
import declaration_domain_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
old = '''func shared(ref x:const u64 in A,ref y:const u64 in B)->ref const u64 in A where {owner A,owner B};
func shared(ref x:u64 in O,ref y:u64 in O) where {owner O}{return x;}'''
new = '''func shared(ref x:const u64 in O,ref y:const u64 in O)->ref const u64 in O where {owner O};
func shared(ref x:const u64 in A,ref y:const u64 in B)->ref const u64 in A where {owner A,owner B}{return x;}'''
assert old in FIXTURE.source
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(old, new))
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func snapshot(ref input:const (func(u64)->const u64) in O,ref marker:const u64 in O)->func(u64)->const u64 where {owner O};
func snapshot(ref input:const (func(u64)->const u64) in A,ref marker:const u64 in B)->func(u64)->const u64 where {owner A,owner B}{return input;}
{{
 func original=wrap(base);val outer<-7u64;
 {{val inner<-9u64;observe(shared(outer,inner));
  observe(9030u64);
  try{func copied=snapshot(original,inner);}catch(e:const OutOfMemoryError){observe(9031u64);};
  observe(original(3u64));
 }};
}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==5', 'checkpoints==6').replace('copy_failures==3', 'copy_failures==4'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['7', '4'])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
