#!/usr/bin/env python3
"""Reference intrinsics share operand preparation without reading target payloads."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
OBS = IO + 'proc observe(x:i64)->void {behave @print(x);}'
EX = 'using "std:exceptions" MissingReturnException;func missing()->const i64 {}'
runner.CASES = {
    'deferred_uninitialized': ('val x<-make local i64;ptr p=x;defer isnull(p);', 0, '', ''),
    'deferred_readonly': ('val x<-make local i64;const ptr p:const i64=x;defer intrinsic (isnull)(p);', 0, '', ''),
    'deferred_null_literal': ('defer isnull(null);', 0, '', ''),
    'registration_order': (OBS + '{{defer observe(3);ptr p:i64=null in here;defer isnull({observe(1);p});observe(2);}};', 0, '1\n2\n3\n', ''),
    'late_rebinding': (OBS + 'val x<-make local i64;ptr p:i64=null in here;{{defer isnull(p);defer {p=x;};}};'
        'behave @print(isnull(p));', 0, 'false\n', ''),
    'preparation_failure': (OBS + EX + '{{defer observe(3);ptr p:i64=null in here;'
        'try{defer isnull({observe(1);missing();p});}catch(e:const MissingReturnException){observe(2);};}};',
        0, '1\n2\n3\n', '[-Wmissing-return]'),
    'preparation_return': (OBS + 'proc f()->const i64 {ptr p:i64=null in here;defer observe(3);'
        'defer isnull({observe(1);return 7;p});observe(99);return 0;}observe(f());', 0, '1\n3\n7\n', ''),
    'scope_local': (OBS + '{{val x<-make local i64;defer isnull(x);defer observe(1);}};', 0, '1\n', ''),
    'expired_target': ('defer isnull({{val x<-1;x}});', 2, '', "owner has ended"),
    'iteration_target': ('for(i in 0..2){defer isnull(i);};', 2, '', 'does not outlive its cleanup scope'),
    'iteration_cleanup': (OBS + 'for(i in 0..2){{defer isnull(i);defer observe(i);}};', 0, '0\n1\n', ''),
    'no_initialization_from_test': ('val x<-make local i64;{{defer isnull(x);}};x;', 2, '', 'before definite initialization'),
    'generic_nullable_borrow': (IO + 'func test(ptr p:T in O)->void where {T,owner O}{defer isnull(p);}'
        'val x<-make local i64,y<-make local f64;ptr p:i64=null in here;test(p);test(x);test(y);behave @print(isnull(p));',
        0, 'true\n', ''),
    'recursive_borrow': (IO + 'func test(ptr p:T in O,n:i64)->bool where {T,owner O}{defer isnull(p);'
        'return if(n==0){isnull(p)}else{test(p,n-1)};}ptr p:i64=null in here;behave @print(test(p,40));',
        0, 'true\n', ''),
    'captured_callable_holder': (IO + 'func base()->i64{return 7;}func f=func[base]()->i64{return base();};'
        '{{ptr p=f;defer isnull(p);}};behave @print(f());', 0, '7\n', ''),
    'local_proc_test': (OBS + 'proc f()->void{ptr p:i64=null in here;defer isnull(p);defer observe(1);}f();', 0, '1\n', ''),
    'swap_saved_slots': (OBS + 'val x<-1,y<-2,z<-3;ptr p=x,q=y,r=z;'
        '(if(true){p}else{q})<=>{observe(8);p=r;q};observe(p);observe(q);', 0, '8\n2\n3\n', ''),
    'swap_cleanup_before_invocation': (OBS + 'val x<-1,y<-2,z<-3;ptr p=x,q=y;'
        '{{defer{p=z;};p}}<=>{observe(8);q};observe(p);observe(q);', 0, '8\n2\n3\n', ''),
    'null_test_operand_cleanup': (IO + 'ptr p:i64=null in here;val x<-1;'
        'behave @print(isnull({{defer{p=x;};p}}));behave @print(isnull(p));', 0, 'true\nfalse\n', ''),
    'fixed_reference_swap': ('ptr p=1;const ptr q=2;intrinsic (<=>)(p,q);', 2, '', 'fixed'),
    'deferred_slot_swap_boundary': ('ptr p=1,q=2;defer intrinsic (<=>)(p,q);', 2, '', 'deferred reference-swap intrinsics'),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
