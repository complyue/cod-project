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
    'direct_named_function_cannot_access_enclosing_value': (
        'proc run()->u64{val n<-1u64;func f()->u64{return n;};n<-7u64;return f();}run();',
        2, '', "an ordinary func cannot access enclosing runtime binding 'n'"),
    'direct_named_function_uses_enclosing_static_definition': (
        IO + 'proc run()->u64{func left(n:u64)->u64{return n+1u64;};'
        'func right(n:u64)->u64{return left(n);};return right(7u64);}behave @print(run());', 0, '8\n', ''),
    'named_function_uses_enclosing_type_evidence': (
        IO + 'proc run()->u64{val n<-1u64;func produce()->u64{val result<-make typeof(n){7u64};'
        'return result;};return produce();}behave @print(run());', 0, '7\n', ''),
    'selection_survives_later_operand_replacement': (
        IO + BASE + 'func chosen=base;behave @print(chosen({chosen<-other;6}));behave @print(chosen(6));',
        0, '6\n16\n', ''),
    'direct_lexical_definition_is_available_in_a_full_scope': (
        IO + 'proc run()->void{proc helper(x:u64)->void{behave @print(x);return;};'
        '{{helper(7u64);}};return;}run();', 0, '7\n', ''),
    'direct_lexical_definition_is_available_to_another_definition': (
        IO + 'proc run()->void{proc helper(x:u64)->void{behave @print(x);return;};'
        'proc relay(x:u64)->void{helper(x);return;};relay(8u64);return;}run();', 0, '8\n', ''),
    'lexical_procedure_writes_each_recursive_activation': (IO + '''
proc run(depth:u64)->u64 {
    val n <- depth;
    proc shift(amount:u64)->u64 { n <- n + amount; return n; };
    if (depth == 0u64) return shift(10u64);
    return shift(run(depth - 1u64)) + n;
}
behave @print(run(2u64));
''', 0, '48\n', ''),
    'lexical_procedure_calls_its_enclosing_definition': (IO + '''
proc run()->u64 {
    val n <- 0u64;
    proc advance(depth:u64)->void {
        n <- n + 1u64;
        proc recurse(next:u64)->void { advance(next); return; };
        if (depth == 0u64) return;
        recurse(depth - 1u64);
        return;
    };
    advance(3u64);
    return n;
}
behave @print(run());
''', 0, '4\n', ''),
    'lexical_procedure_forwards_its_original_slot': (IO + '''
proc run()->void {
    val total <- 1u64;
    proc update(amount:u64)->void { total <- total + amount; return; };
    proc relay(amount:u64)->void { update(amount); return; };
    {{ val total <- 20u64; relay(6u64); behave @print(total); }};
    behave @print(total);
    return;
}
run();
''', 0, '20\n7\n', ''),
    'lexical_procedure_reads_current_reference_after_argument_preparation': (IO + '''
proc run()->void {
    val first <- 1u64;
    val second <- 7u64;
    ref selected:u64 = first;
    proc observe(old:const u64 in O,ignored:u64)->void where {const owner O} {
        behave @print(old);
        selected <- selected + 1u64;
        behave @print(selected);
        return;
    };
    observe(selected, { selected = second; 0u64 });
    return;
}
run();
''', 0, '1\n8\n', ''),
    'deferred_lexical_procedure_retains_slot_and_prepared_argument': (IO + '''
proc run()->void {
    val first <- 1u64;
    val second <- 7u64;
    ref current:u64 = first;
    proc observe(prepared:u64)->void {
        behave @print(prepared);
        behave @print(current);
        return;
    };
    defer observe(current);
    current = second;
    return;
}
run();
''', 0, '1\n7\n', ''),
    'deferred_lexical_procedure_writes_current_target': (IO + '''
proc run()->void {
    val first <- 1u64;
    val second <- 7u64;
    ref current:u64 = first;
    proc update()->void { current <- 9u64; return; };
    defer { behave @print(first); behave @print(second); };
    defer update();
    current = second;
    return;
}
run();
''', 0, '1\n9\n', ''),
    'deferred_lexical_procedure_initializes_original_storage': (IO + '''
proc run()->void {
    val n <- make u64;
    proc init()->void { n <- 7u64; return; };
    defer { behave @print(n); };
    defer init();
    return;
}
run();
''', 0, '7\n', ''),
    'lexical_procedure_requires_initialized_storage_for_read': (IO + '''
proc run()->void {
    val n <- make u64;
    proc observe()->void { behave @print(n); return; };
    observe();
    return;
}
run();
''', 2, '', "cannot prove initialization of 'n'"),
    'lexical_write_survives_checked_failure': (IO + '''
using "std:exceptions" Exception;
data Bad <: Exception {code:u64;};
proc run()->void {
    val n <- 1u64;
    proc update()->void throws Bad {n <- 7u64; throw Bad {code <- 1u64;};};
    try { update(); } catch (e:Bad) { behave @print(n); };
    return;
}
run();
''', 0, '7\n', ''),
    'lexical_procedure_preserves_readonly_view': ('''
proc run()->void {
    val value <- 1u64;
    ref seen:const u64 = value;
    proc forbidden()->void { seen <- 9u64; return; };
    forbidden();
    return;
}
run();
''', 2, '', 'cannot write through a readonly destination'),
    'unused_lexical_procedure_checks_its_return': ('''
proc run()->u64 {
    val value <- 1u64;
    proc bad()->u64 { return value == 1u64; };
    return value;
}
run();
''', 2, '', "return type 'bool' does not satisfy 'u64'"),
}
runner.CASE_SHADOWS = {'lexical_procedure_forwards_its_original_slot': ['total']}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
