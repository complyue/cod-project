#!/usr/bin/env python3
"""Partial method coverage and declarations add no executable query work."""
from pathlib import Path
import declaration_query_staging_tests as runner

DECL = 'func target(input:T)->const T where T;'
METHOD = 'func target(input:bool)->bool{return input;}'
INSPECT = 'func inspect(callback:func(T)->const T,input:T)->const T where T;'
runner.CASES = (
    ('uncovered_call', DECL+METHOD, 'target(1)', 'i64', '7', '7\n'),
    ('covered_call', DECL+METHOD, 'target(true)', 'bool', 'false', 'false\n'),
    ('contextual_family', DECL+METHOD+INSPECT, 'inspect(target,1)', 'i64', '8', '8\n'),
    ('nested_calls', DECL+METHOD, 'target(target(1))', 'i64', '9', '9\n'),
    ('partial_consumer', DECL+METHOD+INSPECT+'func inspect(callback:func(bool)->bool,input:bool)->bool{return callback(input);}',
     'inspect(target,1)', 'i64', '10', '10\n'),
    ('numeric_coverage', 'func target(input:u64)->u64;func target(input:u8)->u8{return input;}'+INSPECT,
     'inspect(target,1u64)', 'u64', '11u64', '11\n'),
    ('unproved_body', DECL+'func target(first:T,second:T)->const T where T{return first+second;}',
     'target(1)', 'i64', '12', '12\n'),
    ('borrowed', 'func target(ref input:T in O)->ref T in O where {T,owner O};func target(ref input:bool in O)->ref bool in O where {owner O}{return input;}val slot<-make local i64;',
     'target(slot)', 'i64', '13', '13\n'),
)
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
