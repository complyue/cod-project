#!/usr/bin/env python3
"""Partial declaration proofs preserve complete images and failure cleanup."""
from pathlib import Path
import declaration_domain_runtime_tests as fixture

runner = fixture.runner
old = '''func shared(ref x:const u64 in A,ref y:const u64 in B)->ref const u64 in A where {owner A,owner B};
func shared(ref x:u64 in O,ref y:u64 in O) where {owner O}{return x;}'''
new = '''func shared(ref x:const u64 in O,ref y:const u64 in O)->ref const u64 in O where {owner O};
func shared(ref x:const u64 in A,ref y:const u64 in B)->ref const u64 in A where {owner A,owner B}{return x;}'''
assert old in runner.SOURCE
runner.SOURCE = runner.SOURCE.replace(old, new)
runner.SOURCE += '''
func snapshot(ref input:const (func(u64)->const u64) in O,ref marker:const u64 in O)->func(u64)->const u64 where {owner O};
func snapshot(ref input:const (func(u64)->const u64) in A,ref marker:const u64 in B)->func(u64)->const u64 where {owner A,owner B}{return input;}
{{
 func original=wrap(base);val outer<-7u64;
 {{val inner<-9u64;observe(shared(outer,inner));
  observe(9030u64);
  try{func copied=snapshot(original,inner);}catch(e:const OutOfMemory){observe(9031u64);};
  observe(original(3u64));
 }};
}};
observe(9000u64);
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==5', 'checkpoints==6').replace('copy_failures==3', 'copy_failures==4')
runner.EXPECTED_VALUES += ['7', '4']
runner.EXTRA_SOURCES += [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
