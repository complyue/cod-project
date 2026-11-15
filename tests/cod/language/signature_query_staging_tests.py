#!/usr/bin/env python3
"""Inspecting callable declarations introduces no native implementations or effects."""
from pathlib import Path
import declaration_query_staging_tests as runner

DECL = 'func declared(input:T)->const T where T;'
INSPECT = 'func inspect(callback:func(T)->const T,input:T)->const T where T;'
runner.CASES = (
    ('generic_signature', DECL+INSPECT, 'inspect(declared,1)', 'i64', '7', '7\n'),
    ('nested_signatures', DECL+INSPECT, 'inspect(declared,inspect(declared,1))', 'i64', '8', '8\n'),
    ('variadic_signature', 'func declared(inputs:i64...)->i64;func inspect(callback:func(i64,i64)->i64)->i64;',
     'inspect(declared)', 'i64', '9', '9\n'),
    ('procedure_signature', 'proc declared(input:T)->const T where T;func inspect(callback:proc(T)->const T,input:T)->const T where T;',
     'inspect(declared,true)', 'bool', 'false', 'false\n'),
    ('result_conversion', 'func declared(input:u64)->u8;'+INSPECT, 'inspect(declared,1u64)', 'u64', '10u64', '10\n'),
    ('inferred_signature', 'func declared(input:i64)->i64;func inspect(callback:F)->i64 where F;',
     'inspect(declared)', 'i64', '11', '11\n'),
    ('exception_contract', 'using "std:exceptions" NullPointerException;func declared(input:T)->const T throws NullPointerException where T;'
     'func inspect(callback:func(T)->const T throws NullPointerException,input:T)->const T where T;',
     'inspect(declared,1)', 'i64', '12', '12\n'),
)
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
