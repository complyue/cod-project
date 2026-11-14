#!/usr/bin/env python3
"""Complete callable captures, independent images and lexical cleanup."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;\n'
BASE = 'func base(x:i64)->const i64{return x;}func other(x:i64)->const i64{return x+10;}\n'
WRAP = 'func wrap(f:func(i64)->const i64)->func(i64)->const i64{return func[f](x:i64)->const i64{return f(x)+1;};}\n'
PROC = 'proc one(x:i64)->void{behave @print(x);}proc two(x:i64)->void{behave @print(x+10);}\n'
runner.CASES = {
    'explicit_capture': (IO+BASE+'func f=base;func g=func[f](x:i64)->const i64{return f(x)+1;};behave @print(g(1));',0,'2\n',''),
    'inferred_capture': (IO+BASE+'func f=base;func g=func(x:i64)->const i64{return f(x)+2;};behave @print(g(1));',0,'3\n',''),
    'independent_snapshot': (IO+BASE+'func f=base;func g=func[f](x:i64)->const i64{return f(x);};f<-other;behave @print(g(4));behave @print(f(4));',0,'4\n14\n',''),
    'factory_result': (IO+BASE+WRAP+'func f=wrap(base);behave @print(f(4));',0,'5\n',''),
    'nested_value_copy': (IO+BASE+WRAP+'func f=wrap(wrap(base));func g=f;f<-wrap(other);behave @print(g(4));behave @print(f(4));',0,'6\n15\n',''),
    'runtime_growing_loop': (IO+BASE+WRAP+'func f=base;for(i in 0..80){{f<-wrap(f);}};behave @print(f(1));',0,'81\n',''),
    'runtime_growing_recursion': (IO+BASE+WRAP+'func grow(f:func(i64)->const i64,n:i64)->func(i64)->const i64{if(n==0){return f;}else{return grow(wrap(f),n-1);};}behave @print(grow(base,30)(2));',0,'32\n',''),
    'selected_target_before_argument': (IO+BASE+WRAP+'func f=wrap(base);behave @print(f({{f<-wrap(other);6}}));behave @print(f(6));',0,'7\n17\n',''),
    'prepared_parameter_before_argument': (IO+BASE+WRAP+'func apply(f:func(i64)->const i64,x:i64)->const i64{return f(x);}func f=wrap(base);behave @print(apply(f,{{f<-wrap(other);7}}));',0,'8\n',''),
    'deferred_procedure_snapshot': (IO+PROC+'proc f=one;proc g=proc[f](x:i64)->void{f(x);};{{defer g(9);g<-two;f<-two;}};',0,'9\n',''),
    'scope_result': (IO+BASE+WRAP+'func f={{func inner=wrap(base);inner}};behave @print(f(9));',0,'10\n',''),
    'finally_result_snapshot': (IO+BASE+WRAP+'func f=wrap(base);func g=try{f}finally{f<-wrap(other);{}};behave @print(g(10));',0,'11\n',''),
    'return_replacement': (IO+BASE+WRAP+'func factory()->func(i64)->const i64{defer{return wrap(other);};return wrap(base);}behave @print(factory()(1));',0,'12\n',''),
    'generic_capture_type': (IO+'func id(x:T)->const T where T{return x;}func wrap(f:F)->const F where F{return func[f](x:i64)->const i64{return f(x);};}func f:func(i64)->const i64=id;behave @print(wrap(f)(13));',0,'13\n',''),
    'named_recursive_capture': (IO+BASE+'proc run()->void{func f=base;func recurse[f](n:i64)->const i64{return if(n==0){f(14)}else{recurse(n-1)};};behave @print(recurse(20));}run();',0,'14\n',''),
    'transitive_capture': (IO+BASE+'func f=base;func factory=func()->func(i64)->const i64{return func(x:i64)->const i64{return f(x);};};behave @print(factory()(15));',0,'15\n',''),
    'explicit_module_method_capture': (IO+BASE+'func f=func[base](x:i64)->const i64{return base(x);};behave @print(f(16));',0,'16\n',''),
    'capture_types_share_generic_body': (IO+'func number()->const i64{return 1;}func truth()->const bool{return true;}func inspect(f:F)->func()->const bool where F{return func[f]()->const bool{f();return true;};}behave @print(inspect(number)());behave @print(inspect(truth)());',0,'true\ntrue\n',''),
    'local_shadow_capture': (IO+BASE+'proc run()->void{func base=other;func f=func(x:i64)->const i64{return base(x);};behave @print(f(8));}run();',0,'18\n',''),
    'nested_scope_shadow': (IO+BASE+'func f=base;func g=func[f](x:i64)->const i64{0;{{func f=other;f(x);}};return f(x);};behave @print(g(19));',0,'19\n','declaration shadows'),
    'default_parameter_replacement': (IO+BASE+WRAP+'func apply(f:func(i64)->const i64,x:i64)->const i64{f<-wrap(other);return f(x);}func f=wrap(base);behave @print(apply(f,9));behave @print(f(9));',0,'20\n10\n',''),
    'borrowed_holder_replacement': (IO+BASE+WRAP+'proc change(f:func(i64)->const i64 in O)->void where {owner O}{f<-wrap(other);}func f=wrap(base);change(f);behave @print(f(10));',0,'21\n',''),
    'nullable_captured_holder': (IO+BASE+WRAP+'using "std:exceptions" NullPointerException;func f=wrap(base);ptr p=f;p=null;try{p(1);{}}catch(e:const NullPointerException){behave @print(22);};',0,'22\n',''),
    'captured_checked_failure': (IO+'using "std:exceptions" NullPointerException;func failing()->const i64 throws NullPointerException{ptr p:i64=null in here;return p;}func f=failing;func g=func[f]()->const i64 throws NullPointerException{return f();};try{g();{}}catch(e:const NullPointerException){behave @print(23);};',0,'23\n',''),
    'copy_failure_preserves_initialization_boundary': (IO+BASE+WRAP+'using "std:exceptions" OutOfMemory;val x<-make local i64;func f=wrap(base);try{func g=f;x<-1;{}}catch(e:const OutOfMemory){behave @print(x);};',2,'','before definite initialization'),
    'replacement_failure_preserves_initialization_boundary': (IO+BASE+WRAP+'using "std:exceptions" OutOfMemory;val x<-make local i64;func f=wrap(base);func g=wrap(other);try{f<-g;x<-1;{}}catch(e:const OutOfMemory){behave @print(x);};',2,'','before definite initialization'),
    'nested_explicit_omission_preserves_shadow': ('func f(x:i64)->const i64{return x;}proc run()->void{func f=func(x:i64)->const i64{return x+10;};func factory=func()->func(i64)->const i64{return func[](x:i64)->const i64{return f(x);};};factory()(1);}run();',2,'','explicit capture list omits'),
    'recursive_capture_context_survives_shadow': (IO+BASE+'func truth(x:bool)->const bool{return x;}proc run()->void{func f=base;func recurse[f](n:i64)->const i64{if(n==0){return f(24);}else{{{func f=truth;return recurse(n-1);}}};};behave @print(recurse(2));}run();',0,'24\n','declaration shadows'),
    'missing_capture': (BASE+'func f=base;func g=func[](x:i64)->const i64{return f(x);};',2,'','explicit capture list omits'),
    'duplicate_capture': (BASE+'func f=base;func g=func[f,f](x:i64)->const i64{return f(x);};',2,'','duplicate callable capture'),
    'unused_retained_capture': (IO+BASE+'func f=base;func g=func[f](x:i64)->const i64{return x;};behave @print(g(17));',0,'17\n','unused callable capture'),
    'no_lexical_scalar_promotion': ('val x<-1;func f=func[x]()->const i64{return x;};',2,'','implementation gap'),
    'no_pointer_cell_promotion': (BASE+'func f=base;ptr p=f;func g=func[p](x:i64)->const i64{return p(x);};',2,'','implementation gap'),
    'captured_replacement_gap': (BASE+'func f=base;func g=func[f](x:i64)->const i64{f<-other;return f(x);};',2,'','replacement of a captured callable payload'),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
