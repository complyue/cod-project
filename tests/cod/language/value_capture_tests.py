#!/usr/bin/env python3
"""Explicit state inputs replace ordinary retained lexical values."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
BASE = 'func one()->u64{return 1u64;}func two()->u64{return 2u64;}'

runner.CASES = {
    'value_input_mutation_stays_local': (
        IO + 'func next(val state:u64)->u64{state<-state+1u64;return state;}val seed<-0u64;'
        'behave @print(next(seed));behave @print(next(seed));behave @print(seed);', 0, '1\n1\n0\n', ''),
    'reference_input_mutates_the_selected_target': (
        IO + 'func next(state:u64 in O)->u64 where {owner O}{state<-state+1u64;return state;}val seed<-3u64;'
        'behave @print(next(seed));behave @print(next(seed));behave @print(seed);', 0, '4\n5\n5\n', ''),
    'function_input_can_be_replaced_locally': (
        IO + BASE + 'func read(input:func()->u64)->u64{input<-two;return input();}'
        'func stored=one;behave @print(read(stored));behave @print(stored());', 0, '2\n1\n', ''),
    'direct_lexical_here_writes_the_live_supplier_slot': (
        IO + 'proc enter(here step()->void){step();return;}proc run()->void{val seed<-3u64;'
        'enter(){seed<-7u64;};behave @print(seed);return;}run();', 0, '7\n', ''),
    'function_result_cannot_retain_a_local_bits_value': (
        'proc factory()->func()->u64{val seed<-4u64;return func[seed]()->u64{return seed;};}factory();',
        2, '', 'a storable func must be capture-free'),
    'function_result_cannot_retain_a_function_holder': (
        BASE + 'proc factory()->func()->u64{func seed=one;return func()->u64{return seed();};}factory();',
        2, '', 'a storable func must be capture-free'),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
