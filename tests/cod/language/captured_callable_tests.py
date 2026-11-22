#!/usr/bin/env python3
"""Ordinary function selections carry code and no live lexical state."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
BASE = 'func base(x:i64)->i64{return x;}func other(x:i64)->i64{return x+10;}'
CAPTURE = 'a storable func must be capture-free'

runner.CASES = {
    'explicit_lexical_capture_rejected': (
        BASE + 'func f=base;func g=func[f](x:i64)->i64{return f(x)+1;};', 2, '', CAPTURE),
    'inferred_lexical_capture_rejected': (
        BASE + 'func f=base;func g=func(x:i64)->i64{return f(x)+2;};', 2, '', CAPTURE),
    'unused_lexical_capture_rejected': (
        BASE + 'func f=base;func g=func[f](x:i64)->i64{return x;};', 2, '', CAPTURE),
    'generic_function_factory_cannot_retain_its_input': (
        BASE + 'func wrap(input:T)->func(i64)->i64 where T{return func[input](n:i64)->i64{return input(n);};}'
        'wrap(base);', 2, '', CAPTURE),
    'nested_function_cannot_retain_its_input': (
        BASE + 'func f=base;func factory=func()->func(i64)->i64{return func(x:i64)->i64{return f(x);};};',
        2, '', CAPTURE),
    'pointer_capture_rejected': (
        BASE + 'func f=base;ptr p=f;func g=func[p](x:i64)->i64{return p(x);};', 2, '', CAPTURE),
    'missing_explicit_capture_diagnostic': (
        BASE + 'func f=base;func g=func[](x:i64)->i64{return f(x);};', 2, '', 'explicit capture list omits'),
    'duplicate_explicit_capture_diagnostic': (
        BASE + 'func f=base;func g=func[f,f](x:i64)->i64{return f(x);};', 2, '', 'duplicate callable capture'),
    'selection_survives_later_operand_replacement': (
        IO + BASE + 'func chosen=base;behave @print(chosen({chosen<-other;6}));behave @print(chosen(6));',
        0, '6\n16\n', ''),
    'direct_lexical_definition_is_available_in_a_full_scope': (
        IO + 'proc run()->void{proc helper(x:u64)->void{behave @print(x);return;};'
        '{{helper(7u64);}};return;}run();', 0, '7\n', ''),
    'direct_lexical_definition_is_available_to_another_definition': (
        IO + 'proc run()->void{proc helper(x:u64)->void{behave @print(x);return;};'
        'proc relay(x:u64)->void{helper(x);return;};relay(8u64);return;}run();', 0, '8\n', ''),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
