#!/usr/bin/env python3
"""Loop rebinding effects preserve invariant targets and reject stale proofs."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
OUT = lambda value: 'behave @print(' + value + ');'
SETUP = 'val x<-7u64;ptr p:u64=null in here;p=x;'
COPY = 'func copy(ref value:const T in O)->const T where {T,owner O}{return value;}'
SWAP = 'func (<=>)(ptr a:u64 in O,ptr b:u64 in O)->void where {owner O}{intrinsic (<=>)(a,b);}'

runner.CASES = {
    'unchanged_read': (IO + SETUP + 'for(i in 0u64..2u64){' + OUT('p') + '};', 0, '7\n7\n', ''),
    'unchanged_ref_argument': (IO + COPY + SETUP + 'for(i in 0u64..2u64){' + OUT('copy(p)') + '};', 0, '7\n7\n', ''),
    'payload_writes_keep_target': (IO + SETUP + 'for(i in 0u64..2u64){p<-i;' + OUT('p') + '};', 0, '0\n1\n', ''),
    'later_rebinding_does_not_reach_header': (IO + SETUP + 'for(i in 0u64..2u64){' + OUT('p') + '};p=null;', 0, '7\n7\n', ''),
    'invariant_alias': (IO + COPY + SETUP + 'var selected->p;for(i in 0u64..2u64){' + OUT('copy(selected)') + '};', 0, '7\n7\n', ''),
    'unrelated_rebinding': (IO + SETUP + 'ptr q=x;for(i in 0u64..2u64){q=null;' + OUT('p') + 'q=x;};', 0, '7\n7\n', ''),
    'nested_invariant': (IO + SETUP + 'ptr q=x;for(i in 0u64..2u64){for(j in 0u64..2u64){q=null;' + OUT('p') + 'q=x;};};', 0, '7\n' * 4, ''),
    'fold_invariant': (IO + SETUP + OUT('fold(i in 0u64..3u64,total=0u64){total+p}'), 0, '21\n', ''),
    'dynamic_count_and_else': (IO + 'proc run(n:u64)->void{' + SETUP + 'for(i in 0u64..n){' + OUT('p') + '}else{' + OUT('9u64') + '};}run(0u64);run(2u64);', 0, '9\n7\n7\n', ''),
    'incoming_guard_is_invariant': (IO + 'proc run(ptr p:const u64 in O,n:u64)->void where {owner O}{if(p is null){}else{for(i in 0u64..n){' + OUT('p') + '};};}'
        'val x<-7u64;ptr empty:const u64=null in here;run(x,2u64);run(empty,2u64);', 0, '7\n7\n', ''),
    'prepared_value_snapshot': (IO + 'proc observe(value:u64)->void{' + OUT('value') + '}' + SETUP + 'for(i in 0u64..2u64){defer observe(p);};p=null;', 0, '7\n7\n', ''),
    'prepared_borrow_retains_target': (IO + 'proc observe(ref value:const u64 in O)->void where {owner O}{' + OUT('value') + '}' + SETUP
        + 'for(i in 0u64..2u64){defer observe(p);};p=null;', 0, '7\n7\n', ''),
    'invariant_callable_target': (IO + 'func value()->u64{return 7u64;}func held=value;ptr p:func()->u64=null in here;p=held;'
        'for(i in 0u64..2u64){' + OUT('p()') + '};', 0, '7\n7\n', ''),
    'empty_loop_does_not_initialize': ('val x<-make local u64;ptr p=x;for(i in 0u64..0u64){p<-1u64;};x;', 2, '', 'before definite initialization'),
    'invariant_uninitialized_target': ('val x<-make local u64;ptr p:u64=null in here;p=x;for(i in 0u64..2u64){p;};', 2, '', 'uninitialized'),
    'invariant_null_target': ('ptr p:u64=null in here;for(i in 0u64..2u64){p;};', 2, '', 'requires a covering catch'),
    'rebinding_invalidates_read': (SETUP + 'for(i in 0u64..2u64){p;p=null;};', 2, '', 'requires a covering catch'),
    'rebinding_invalidates_ref': (COPY + SETUP + 'for(i in 0u64..2u64){copy(p);p=null;};', 2, '', 'proven non-null target'),
    'alias_rebinding_invalidates': (SETUP + 'var selected->p;for(i in 0u64..2u64){p;selected=null;};', 2, '', 'requires a covering catch'),
    'nested_rebinding_invalidates_outer': (SETUP + 'for(i in 0u64..2u64){p;for(j in 0u64..1u64){p=null;};};', 2, '', 'requires a covering catch'),
    'swap_invalidates': (SETUP + 'ptr q:u64=null in here;for(i in 0u64..2u64){p;p<=>q;};', 2, '', 'requires a covering catch'),
    'caller_binding_method_invalidates': (SWAP + SETUP + 'ptr q:u64=null in here;for(i in 0u64..2u64){p;(<=>)(p,q);};', 2, '', 'requires a covering catch'),
    'scope_cleanup_invalidates': (SETUP + 'for(i in 0u64..2u64){p;{{defer {p=null;};}};};', 2, '', 'requires a covering catch'),
    'finally_invalidates': (SETUP + 'for(i in 0u64..2u64){p;try{}finally{p=null;{}};};', 2, '', 'requires a covering catch'),
    'covered_rebinding_failure': (IO + NPE + SETUP + 'try{for(i in 0u64..2u64){' + OUT('p') + 'p=null;};}catch(e:const NullPointerException){' + OUT('4u64') + '};', 0, '7\n4\n', ''),
    'single_iteration_rebind': (IO + SETUP + 'for(i in 0u64..1u64){' + OUT('p') + 'p=null;};', 0, '7\n', ''),
    'single_iteration_deferred_guard': (IO + SWAP.replace('ptr a:', 'ref a:').replace('ptr b:', 'ref b:')
        + SETUP + 'val y<-8u64;ptr q=y;{{for(i in 0u64..1u64){p=x;defer (<=>)(p,q);};}};' + OUT('p') + OUT('q'), 0, '8\n7\n', ''),
    'repeated_mutation_has_no_saved_guard': (SWAP.replace('ptr a:', 'ref a:').replace('ptr b:', 'ref b:')
        + SETUP + 'val y<-8u64;ptr q=y;for(i in 0u64..2u64){p=x;defer (<=>)(p,q);p=null;};', 2, '', 'proven non-null target'),
    'alias_read_cannot_hide_mutation': (SETUP + 'var selected->p;for(i in 0u64..2u64){selected;p=null;};', 2, '', 'requires a covering catch'),
}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
