#!/usr/bin/env python3
"""Signature inspection emits neither printing nor dynamic invocation work."""
from pathlib import Path
import declaration_query_staging_tests as runner

runner.CASES = (
    ('uninitialized_print', 'val slot<-make local i64;', '{behave @print(slot);7}', 'i64', '9', '9\n'),
    ('null_target_print', 'ptr slot:i64=null in here;', '{perform @print(slot);7}', 'i64', '10', '10\n'),
    ('uninstalled_scalar', 'symbol @value:u64;', '{behave @print(@value);7}', 'i64', '11', '11\n'),
    ('uninstalled_procedure', 'symbol @action:proc(u64)->u64;', 'perform @action(1u64)', 'u64', '12u64', '12\n'),
    ('checked_procedure', 'using "std:exceptions" NullPointerException;symbol @action:proc(u64)->u64 throws NullPointerException;', 'behave @action(1u64)', 'u64', '13u64', '13\n'),
    ('operand_not_called', 'proc noisy()->i64{behave @print(99);return 1;}', '{perform @print(noisy());7}', 'i64', '14', '14\n'),
    ('primitive_print', '', '{behave @print(true);perform @print(0.5f32);7}', 'i64', '15', '15\n'),
    ('function_value_call', 'symbol @action:func(u64)->u64;', '(@action)(1u64)', 'u64', '16u64', '16\n'),
)
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
