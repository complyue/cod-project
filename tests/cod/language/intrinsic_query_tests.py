#!/usr/bin/env python3
"""Unevaluated symbolic calls retain their executable signature requirements."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'

def query(expression, prefix='', value='9'):
    return IO+prefix+'val result<-make local typeof({'+expression+';7}){'+value+'};behave @print(result);'

runner.CASES = {
    'uninitialized_print': (query('behave @print(slot)', 'val slot<-make local i64;'),0,'9\n',''),
    'null_target_print': (query('perform @print(slot)', 'ptr slot:i64=null in here;'),0,'9\n',''),
    'primitive_prints': (query('behave @print(1u8);perform @print(2u32);behave @print(3u64);perform @print(-4);behave @print(0.5f32);perform @print(0.25f64);behave @print(true)'),0,'9\n',''),
    'operand_not_called': (query('behave @print(noisy())', 'proc noisy()->i64{behave @print(99);return 1;}'),0,'9\n',''),
    'operand_body_not_demanded': (query('behave @print(noisy(true))', 'proc noisy(input:T)->i64 where T{return input+input;}'),0,'9\n',''),
    'declared_operand_without_body': (query('perform @print(declared(1))', 'proc declared(input:T)->i64 where T;'),0,'9\n',''),
    'checked_operand_not_called': (query('perform @print(read(slot))', NPE+'func read(ptr input:i64 in O)->i64 throws NullPointerException where {owner O}{return input;}ptr slot:i64=null in here;'),0,'9\n',''),
    'uninstalled_operand': (query('behave @print(@value)', 'symbol @value:i64;'),0,'9\n',''),
    'print_alias': (query('perform @output(slot)', 'using symbol "std:io" @print as @output;val slot<-make local i64;'),0,'9\n',''),
    'function_can_inspect': (IO+'func inspect()->i64{val slot<-make local i64;val result<-make local typeof({behave @print(slot);7}){9};return result;}behave @print(inspect());',0,'9\n',''),
    'retained_outer_type': (IO+'func factory(){val slot<-make local i64;func inspect()->i64{val result<-make local typeof({perform @print(slot);7}){9};return result;};return inspect;}func selected=factory();behave @print(selected());',0,'9\n',''),
    'query_does_not_initialize': (IO+'val slot<-make local i64;val result<-make local typeof({behave @print(slot);7}){9};slot;',2,'','before definite initialization'),
    'bare_print_query_remains_unproved': (IO+'val result<-make local typeof(@print);',2,'','typeof requires a known declaration'),
    'print_arity_zero': (query('behave @print()'),2,'','print requires one scalar argument'),
    'print_arity_two': (query('perform @print(1,2)'),2,'','print requires one scalar argument'),
    'print_callable': (query('behave @print(value)', 'func value()->u64{return 1u64;}'),2,'','print requires one primitive scalar argument'),
    'print_anonymous_callable': (query('perform @print(func()->u64{return 1u64;})'),2,'','print requires one primitive scalar argument'),
    'print_exception': (query('behave @print(value)', NPE+'val value<-make local NullPointerException;'),2,'','print requires one primitive scalar argument'),
    'print_null_literal': (query('perform @print(null in here)'),2,'','print requires one primitive scalar argument'),
    'print_void': (query('behave @print(empty())', 'func empty()->void{}'),2,'','print requires one primitive scalar argument'),
    'print_invalid_operand': (query('perform @print(true+1)'),2,'','no applicable numeric operation'),
    'print_shadow': (IO+'func inspect(print:i64)->i64{val result<-make local typeof({behave @print(1);7}){9};return result;}inspect(1);',2,'',"nearest binding 'print' is not a symbol"),
    'valid_uninstalled_procedure': (query('perform @action(1u64)', 'symbol @action:proc(u64)->u64;'),0,'9\n',''),
    'valid_checked_procedure': (query('behave @action(1u64)', NPE+'symbol @action:proc(u64)->u64 throws NullPointerException;'),0,'9\n',''),
    'procedure_borrow_without_read': (query('perform @action(slot)', 'proc @action(ref input:i64 in O)->i64 where {owner O}{return input;}val slot<-make local i64;'),0,'9\n',''),
    'function_value_call_query': (query('(@action)(1u64)', 'symbol @action:func(u64)->u64;'),0,'9\n',''),
    'procedure_argument_type': (query('behave @action(true)', 'symbol @action:proc(u64)->u64;'),2,'','argument does not satisfy the callable view type'),
    'procedure_argument_count': (query('perform @action()', 'symbol @action:proc(u64)->u64;'),2,'','callable argument count'),
    'procedure_still_requires_installation': ('symbol @action:proc(u64)->u64;behave @action(1u64);',2,'','cannot prove an installed binding'),
    'real_function_effect_still_rejects': (IO+'func inspect()->void{behave @print(1);}inspect();',2,'','a function cannot invoke a dynamic procedure'),
}
for mode in ('behave', 'perform'):
    for kind in ('func(u64)->u64', 'u64'):
        key = mode+'_'+('function' if kind.startswith('func') else 'scalar')
        declaration = 'symbol @action:'+kind+';'
        expression = mode+' @action(1u64)'
        runner.CASES[key+'_query'] = (query(expression,declaration),2,'','dynamic invocation requires a procedure symbol')
        runner.CASES[key+'_execution'] = (declaration+expression+';',2,'','dynamic invocation requires a procedure symbol')
runner.CASES['imported_print_identity'] = ({'io.cod':IO+'export proc @print;', 'main.cod':'using symbol "./io.cod" @print as @output;val slot<-make local i64;val result<-make local typeof({perform @output(slot);7}){9};behave @output(result);'},0,'9\n','')
runner.CASES['imported_function_symbol_rejects'] = ({'api.cod':'symbol @action:func(u64)->u64;export var @action;', 'main.cod':query('behave @selected(1u64)','using symbol "./api.cod" @action as @selected;')},2,'','dynamic invocation requires a procedure symbol')
for name, prefix, arguments, diagnostic in (
    ('zero', '', '', 'print requires one scalar argument'),
    ('two', '', '1,2', 'print requires one scalar argument'),
    ('callable', 'func value()->u64{return 1u64;}', 'value', 'print requires one primitive scalar argument'),
    ('null', '', 'null in here', 'untyped null in a payload expression'),
    ('void', 'func empty()->void{}', 'empty()', 'print requires one primitive scalar argument'),
):
    runner.CASES['real_print_'+name] = (IO+prefix+'behave @print('+arguments+');',2,'',diagnostic)
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
