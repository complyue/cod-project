#!/usr/bin/env python3
"""Result dependencies preserve independent anchors as additional calls become known."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
TAGS = 'func tag(x:u32)->u64{return 32u64;}func tag(x:u64)->u64{return 64u64;}'
SEED = 'func seed(){return 1u64;}'
runner.CASES = {
    'late_nonrecursive_join': (IO + TAGS +
        'func result(flag:bool){if(flag){return 1u32;}else{return select(seed());};}'
        'func select(x:u64){return 2u64;}' + SEED +
        'behave @print(tag(result(true)));behave @print(result(false));', 0, '64\n2\n', ''),
    'late_recursive_anchor': (IO +
        'func recur(n:u64){if(n==0u64){return 7u64;}else{return select(seed(),n-1u64);};}'
        'func select(x:u64,n:u64){return recur(n);}' + SEED +
        'behave @print(recur(20u64));', 0, '7\n', ''),
    'late_unanchored_cycle': (
        'func recur(){return select(seed());}func select(x:u64){return recur();}' + SEED,
        2, '', 'unproved callable result'),
    'late_cycle_cannot_widen_its_anchor': (
        'func recur(flag:bool){if(flag){return 1u32;}else{return select(seed());};}'
        'func select(x:u64){return recur(true)+1u64;}' + SEED,
        2, '', 'return type'),
    'late_cycle_cannot_invent_void': (
        'func recur(flag:bool){if(flag){return select(seed());}else{};}'
        'func select(x:u64){return recur(false);}' + SEED,
        2, '', 'unproved callable result'),
    'independent_instances_keep_types': (IO + TAGS +
        'func result(x:T,n:u64) where T{if(n==0u64){return x;}else{return relay(x,n-1u64);};}'
        'func relay(x:T,n:u64) where T{return result(x,n);}'
        'behave @print(tag(result(1u32,20u64)));behave @print(tag(result(2u64,20u64)));'
        'behave @print(result(true,20u64));', 0, '32\n64\ntrue\n', ''),
}
for reverse in (False, True):
    definitions = [SEED,
        'func left(n:u64){if(n==0u64){return seed();}else{return right(n-1u64);};}',
        'func right(n:u64){return left(n);}',
        'func independent(){return true;}',
        'func another(){return left(2u64);}']
    if reverse:
        definitions.reverse()
    runner.CASES['shared_dependencies_' + str(reverse)] = (
        IO + ''.join(definitions) + 'behave @print(left(10u64));behave @print(another());'
        'behave @print(independent());', 0, '1\n1\ntrue\n', '')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
