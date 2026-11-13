#!/usr/bin/env python3
"""Native scalar loops, lexical initialization and dynamic deferred work."""
from pathlib import Path
import sys
import foundation_tests as runner

PRINT = 'using symbol "std:io" @print;\n'
OBSERVE = PRINT + 'proc observe(x:i64)->void {behave @print(x);}\n'
runner.CASES = {
    "loops": (None, 0, "-1\n-1\n3\n10\n10\n10\n0\n1\n1\n1\n1\n7.5\n1\n3\n11\n12\n91\n2\n50\n1\n0\n90\n0\n1\n2\n11\n10\n1\n0\n10\n11\n12\n2\n1\n0\n40\n2\n30\n80\n", ""),
    "loop_body_failure": (OBSERVE + 'func bad()->const i64 {} {{defer observe(9); for(i in 0..3){defer observe(i); if(i==1){bad();{}}else{};} else {observe(99);};}};', 70, "1\n0\n9\n", "uncaught MissingReturnException"),
    "loop_bound_failure": (OBSERVE + 'func bad()->const i64 {} {{defer observe(9); for(i in 0..bad()){observe(98);}else{observe(99);};}};', 70, "9\n", "uncaught MissingReturnException"),
    "repeated_defer_failure": (OBSERVE + 'proc bad(x:i64)->const i64 {observe(x); if(x==1){return x;}else{};} {{defer observe(9);for(i in 0..3){defer bad(i);};}};', 70, "2\n1\n0\n9\n", "uncaught MissingReturnException"),
    "loop_preparation_failure": (OBSERVE + 'proc arg(x:i64)->const i64 {if(x==2){}else{return x;};} {{defer observe(9);for(i in 0..3){defer observe(arg(i));};}};', 70, "1\n0\n9\n", "uncaught MissingReturnException"),
    "fold_snapshots": (OBSERVE + '{{behave @print(fold(i in 0..3,n=0){defer observe(n);n+i});}};', 0, "3\n1\n0\n0\n", ""),
    "nested_preparation": (OBSERVE + 'proc both(x:i64,y:i64)->void{observe(x);observe(y);} {{defer both({defer observe(7);1},{defer observe(8);2});defer observe(9);}};', 0, "9\n1\n2\n8\n7\n", ""),
    "fold_recursive": (PRINT + 'func f(n:i64)->const i64 {return if(n==0){1}else{fold(i in 0..2,a=0){a+f(n-1)}};} behave @print(f(5));', 0, "32\n", ""),
    "loop_bound_snapshot": (PRINT + 'val end=3;for(i in 0..end){end<-0;behave @print(i);};', 0, "0\n1\n2\n", ""),
    "loop_empty_initialization": ('func f(n:i64)->const i64 {val x=make local i64;for(i in 0..n){{x<-i;}};return x;} f(0);', 2, "", "before definite initialization"),
    "loop_partial_initialization": ('func f(n:i64)->const i64 {val x=make local i64;for(i in 0..n){{if(i==0){x<-1;{}}else{};}}else{x<-0;};return x;} f(1);', 2, "", "before definite initialization"),
    "loop_read_before_write": ('val x=make local i64;for(i in 0..3){{x<-x+1;}};', 2, "", "before definite initialization"),
    "loop_skipped_body_checked": ('for(i in 0..0){{val x=make local i64;x;}};', 2, "", "before definite initialization"),
    "loop_skipped_else_checked": ('for(i in 0..1){}else{{val x=make local i64;x;}};', 2, "", "before definite initialization"),
    "loop_direct_declaration": ('for(i in 0..2){val x=i;};', 2, "", "nested per-iteration"),
    "loop_group_declaration": ('for(i in 0..2){if(true){val x=i;}else{};};', 2, "", "nested per-iteration"),
    "loop_else_declaration": ('for(i in 0..0){}else{val x=1;};', 2, "", "nested per-iteration"),
    "loop_index_else": ('for(i in 0..0){}else{i;};', 2, "", "unknown scalar value 'i'"),
    "loop_index_after": ('for(i in 0..1){};i;', 2, "", "unknown scalar value 'i'"),
    "loop_index_readonly": ('for(i in 0..2){i<-1;};', 2, "", "readonly destination"),
    "fold_accumulator_readonly": ('fold(i in 0..2,n=0){n<-1};', 2, "", "readonly destination"),
    "loop_noninteger": ('for(i in 0.0..1.0){};', 2, "", "integer endpoints"),
    "fold_bad_result": ('fold(i in 0..2,n=0){{true}};', 2, "", "fold body does not satisfy"),
    "fold_else": ('fold(i in 0..2,n=0){n+i}else{0};', 2, "", "fold already has an initial accumulator"),
    "loop_return_paths": (PRINT + 'func f(n:i64)->const i64 {val x=make local i64;for(i in 0..n){return i;}else{x<-9;};return x;} behave @print(f(0));behave @print(f(2));', 0, "9\n0\n", ""),
    "loop_known_return": (PRINT + 'func f()->const i64 {for(i in 0..1){return i;};} behave @print(f());', 0, "0\n", ""),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == "__main__":
    sys.exit(runner.main())
