#!/usr/bin/env python3
"""Independent return alternatives anchor recursion without operand guesses."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
MISSING = 'using "std:exceptions" MissingReturnError;'
runner.CASES = {
    'conditional_recursive_sum': (IO + 'func sum(n:u64){return if(n==0u64){0u64}else{n+sum(n-1u64)};}behave @print(sum(30u64));', 0, '465\n', ''),
    'reverse_recursive_sum': (IO + 'func sum(n:u64){return if(n!=0u64){n+sum(n-1u64)}else{0u64};}behave @print(sum(30u64));', 0, '465\n', ''),
    'scoped_conditional_result': (IO + 'func f(n:u64){return {{val x<-7u64;if(n==0u64){x}else{f(n-1u64)}}};}behave @print(f(30u64));', 0, '7\n', ''),
    'generic_parameter_alternative': (IO + 'func f(x:T,n:u64) where T{return if(n==0u64){x}else{f(x,n-1u64)};}behave @print(f(true,30u64));behave @print(f(7u32,30u64));', 0, 'true\n7\n', ''),
    'mutual_expression_results': (IO + 'func a(n:u64){return b(n);}func b(n:u64){return if(n==0u64){7u64}else{a(n-1u64)};}behave @print(a(30u64));', 0, '7\n', ''),
    'nested_retained_generic_type': (IO + 'func outer(x:T) where T{func f(v:T,n:u64){return if(n==0u64){v}else{f(v,n-1u64)};};return f(x,10u64);}behave @print(outer(true));', 0, 'true\n', ''),
    'nested_callable_capture': (IO + 'func seven()->u64{return 7u64;}func outer(value:func()->u64){func f(n:u64){return if(n==0u64){value()}else{f(n-1u64)};};return f;}func f=outer(seven);behave @print(f(10u64));', 0, '7\n', ''),
    'catch_result_anchors_recursive_body': (MISSING + 'func f(n:u64){return try{f(n);}catch(e:const MissingReturnError){7u64};}func value=f;', 0, '', ''),
    'try_body_and_catch_return_values': (IO + MISSING + 'func missing()->u64{}func f(n:u64){return try{if(n==0u64){missing()}else{f(n-1u64)}}catch(e:const MissingReturnError){7u64};}behave @print(f(10u64));', 0, '7\n', '[-Wmissing-return]'),
    'cleanup_return_replaces_expression_result': (IO + 'func f(n:u64){return {{defer{return 3u64;};if(n==0u64){1u64}else{f(n-1u64)}}};}behave @print(f(10u64));', 0, '3\n', ''),
    'void_expression_alternative': (IO + 'proc f(n:u64){return if(n==0u64){behave @print(7u64);}else{f(n-1u64);};}f(10u64);', 0, '7\n', ''),
    'operator_operand_is_not_a_result_alternative': ('func f(n:u64){return (if(n==0u64){1u64}else{f(n-1u64)})+0u64;}', 2, '', 'unproved callable result'),
    'call_argument_is_not_a_result_alternative': ('func id(x:u64)->u64{return x;}func f(n:u64){return id(if(n==0u64){1u64}else{f(n-1u64)});}', 2, '', 'unproved callable result'),
    'unanchored_conditional_cycle': ('func f(n:u64){return if(n==0u64){f(n)}else{f(n-1u64)};}', 2, '', 'unproved callable result'),
    'unreachable_block_tail_is_not_an_anchor': ('func f(){return {{return f();1u64}};}', 2, '', 'unproved callable result'),
    'abrupt_condition_hides_value_alternatives': ('func f(){return if({return f();true}){1u64}else{2u64};}', 2, '', 'unproved callable result'),
    'finally_throw_discards_pending_expression_evidence': (MISSING + 'func f(e:MissingReturnError){return try{if(true){7u64}else{f(e)}}finally{throw e;};}', 2, '', 'unproved callable result'),
    'dependent_branch_still_must_match': ('func f(flag:bool){return if(flag){1u32}else{f(true)+1u64};}', 2, '', 'conditional join with different scalar result types'),
    'conditional_does_not_gain_implicit_widening': ('func f(flag:bool){return if(flag){1u32}else{2u64};}', 2, '', 'conditional join with different scalar result types'),
    'module_interpretations_keep_independent_anchors': ({
        'base.cod': 'func score(x:T)->u64 where T{return 1u64;}func f(n:u64){return if(n==0u64){score(n)}else{f(n-1u64)};}export score;export f;',
        'ext.cod': 'import "base.cod" score;func score(x:u64)->u64{return 7u64;}using "base.cod" f as frozen;export frozen;export import "base.cod" f as adapted;',
        'main.cod': IO + 'using "ext.cod" frozen;using "ext.cod" adapted;behave @print(frozen(10u64));behave @print(adapted(10u64));',
    }, 0, '1\n7\n', ''),
}
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {
    'nested_callable_capture': ['f'],
}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
