#!/usr/bin/env python3
"""Nested definitions preserve compile-time type context through native calls."""
from pathlib import Path
import foundation_tests as runner

IO='using symbol "std:io" @print;\n'
ID='func id(x:T)->const T where T{return x;}\n'
BASE='func inc(x:i64)->const i64{return x+1;}func yes(x:bool)->const bool{return x;}\n'
FACTORY='func factory(sample:T)->func(T)->const T where T{return func[](x:T)->const T{return x;};}\n'
runner.CASES={
 'inherited_signature':(IO+FACTORY+'behave @print(factory(1)(2));behave @print(factory(false)(true));',0,'2\ntrue\n',''),
 'inherited_capture':(IO+BASE+'func wrap(f:func(T)->const T)->func(T)->const T where T{return func[f](x:T)->const T{return f(x);};}behave @print(wrap(inc)(3));behave @print(wrap(yes)(true));',0,'4\ntrue\n',''),
 'mixed_void_result_conversion':(IO+'func empty()->const void{}func small()->const u8{return 1u8;}func number()->const f64{return 1.0f64;}func convert(g:func()->const S,f:func()->const T)->const T where {S,T}{return g();}convert(empty,empty);behave @print(convert(small,number));',0,'1\n',''),
 'constant_generic_return_conversion':(IO+'func one(x:T)->const T where T{return 1u8;}behave @print(one(1));behave @print(one(1.0f64));',0,'1\n1\n',''),
 'local_annotation':(IO+'func factory(x:T)->func()->const T where T{return func[]()->const T{val y:T<-1u8;return y;};}behave @print(factory(1)());behave @print(factory(1.0f64)());',0,'1\n1\n',''),
 'retained_only_in_body':(IO+ID+'func factory(x:T)->func()->const bool where T{return func[]()->const bool{func local:func(T)->const T=id;return true;};}behave @print(factory(1)());behave @print(factory(false)());',0,'true\ntrue\n',''),
 'transitive_type_context':(IO+'func factory(x:T)->func()->func(T)->const T where T{return func[]()->func(T)->const T{return func[](y:T)->const T{return y;};};}behave @print(factory(1)()(5));behave @print(factory(false)()(true));',0,'5\ntrue\n',''),
 'own_binder_shadows_outer':(IO+'func factory(x:T)->func(bool)->const bool where T{return func[](y:T)->const T where T{return y;};}behave @print(factory(1)(true));',0,'true\n',''),
 'own_and_retained_binders':(IO+'func factory(x:T)->func(bool,T)->const T where T{return func[](y:U,z:T)->const T where U{return z;};}behave @print(factory(1)(true,6));',0,'6\n',''),
 'named_recursive_context':(IO+'func factory(x:T)->func(T,i64)->const T where T{func recur(y:T,n:i64)->const T{if(n==0){return y;}else{return recur(y,n-1);};};return recur;}behave @print(factory(1)(7,30));behave @print(factory(false)(true,40));',0,'7\ntrue\n',''),
 'unused_undetermined_outer':(IO+'func factory()->func()->const i64 where T{return func[]()->const i64{return 8;};}behave @print(factory()());',0,'8\n',''),
 'used_undetermined_outer':('func factory()->func()->const bool where T{return func[]()->const bool{val x:T<-1;return true;};}factory();',2,'','enclosing type parameter is not determined'),
 'retained_type_not_reinferred':(FACTORY+'func f:func(bool)->const bool=factory(1);',2,'','initializer does not satisfy declared func type'),
 'normal_cells_not_promoted':('func factory(x:T)->func()->const T where T{return func[x]()->const T{return x;};}factory(1);',2,'','capture of ordinary lexical cells'),
 'retained_callback_mismatch':(BASE+'func wrap(f:func(T)->const T)->func(bool)->const bool where T{return func[f](x:T)->const T{return f(x);};}wrap(inc);',2,'','no applicable method'),
 'retained_void_completion':(IO+'func empty()->const void{}func factory(f:func()->const T)->func()->const T where T{return func[f]()->const T{f();};}factory(empty)();',0,'',''),
}

runner.CASES.update({
 'transitive_body_only_context':(IO+'func factory(x:T)->func()->func()->const bool where T{return func[]()->func()->const bool{return func[]()->const bool{val y:T<-1u8;return true;};};}behave @print(factory(1)()());behave @print(factory(1.0f64)()());',0,'true\ntrue\n',''),
 'fixed_result_conversion':(IO+'func factory(x:T)->func()->const T where T{return func[]()->const T{return 2u8;};}behave @print(factory(1)());behave @print(factory(1.0f64)());',0,'2\n2\n',''),
 'retained_transfer_conversion':(IO+'func factory(x:T)->func()->const T where T{return func[]()->const T{val y<-make local T;y<-3u8;return y;};}behave @print(factory(1)());behave @print(factory(1.0f64)());',0,'3\n3\n',''),
 'same_view_distinct_retained_types':(IO+'func pick(x:i64)->const i64{return 1;}func pick(x:f64)->const i64{return 2;}func factory(x:T)->func()->const i64 where T{return func[]()->const i64{val y:T<-1u8;return pick(y);};}func wrap(f:func()->const i64)->func()->const i64{return func[f]()->const i64{return f();};}func a=factory(1);func b=factory(1.0f64);func c=a;a<-b;behave @print(c());behave @print(a());func d=wrap(c);c<-b;behave @print(d());',0,'1\n2\n1\n',''),
 'copied_and_returned_context':(IO+FACTORY+'func pass(f:F)->const F where F{return f;}func a=factory(1);func b=pass(a);a<-factory(2);behave @print(b(9));',0,'9\n',''),
 'deferred_context':(IO+BASE+'proc factory(f:func(T)->const T)->proc(T)->void where T{return proc[f](x:T)->void{behave @print(f(x));};}{{proc a=factory(inc);proc b=factory(yes);defer a(10);defer b(true);}};',0,'true\n11\n',''),
 'generic_name_shadows_exception':('using "std:exceptions" SystemException;func factory(x:SystemException)->func(SystemException)->const SystemException where SystemException{return func[](y:SystemException)->const SystemException{return y;};}factory(1)(2);',0,'',''),
})
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
 raise SystemExit(runner.main())
