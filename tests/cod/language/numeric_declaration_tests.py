#!/usr/bin/env python3
"""Numeric declaration overlap retains original promises and established method results."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
DECL = 'func f(x:u32)->f64;'
METHOD = 'func f(x:f64)->f64{return x;}'
FAILURE = 'func f(x:u32)->f64 throws NullPointerException;'+METHOD
runner.CASES = {
 'inferred_body': (IO+DECL+'func f(x:f64){return x;}behave @print(f(16777217u32));',0,'16777217\n',''),
 'prepared_body_dispatch': (IO+DECL+'func tag(x:u32)->f64{return 32f64;}func tag(x:f64)->f64{return 64f64;}func f(x:f64){return tag(x);}behave @print(f(1u32));',0,'64\n',''),
 'established_narrow_result': (IO+'func f(x:u32)->f64;func f(x:f64)->u8{return 7u8;}func tag(x:u8)->u64{return 8u64;}func tag(x:f64)->u64{return 64u64;}behave @print(tag(f(1u32)));',0,'8\n',''),
 'cached_result': (IO+DECL+METHOD+'behave @print(f(3f64));behave @print(f(7u32));behave @print(f(9u8));',0,'3\n7\n9\n',''),
 'incompatible_result': ('func f(x:u32)->bool;'+METHOD+'f(1u32);',2,'','does not satisfy callable declaration'),
 'no_result_narrowing': ('func f(x:u32)->u32;func f(x:u64)->u64{return x;}f(1u8);',2,'','does not satisfy callable declaration'),
 'partial_cannot_anchor_recursion': (DECL+'func f(x:f64){return f(x);}',2,'','unproved callable result'),
 'covering_anchor': (IO+DECL+'func f(x:f64)->f64;func f(x:f64){if(x==0f64){return 7f64;}else{return f(0f64);};}behave @print(f(1u32));',0,'7\n',''),
 'independent_recursive_anchor': (IO+DECL+'func f(x:f64){if(x==0f64){return 7f64;}else{return f(0f64);};}behave @print(f(1u32));',0,'7\n',''),
 'generic_other_parameter': (IO+'func f(x:u32,y:T)->const T where T;func f(x:f64,y:U)->const U where U{return y;}behave @print(f(1u8,true));behave @print(f(1u32,7u64));',0,'true\n7\n',''),
 'original_generic_promise': ('func f(x:T,y:u32)->const T where T;func f(x:u64,y:f64)->u64{return x;}f(1u8,2u32);',2,'','does not satisfy callable declaration'),
 'same_type_inputs': (IO+'func f(x:T,y:T)->u64 where T;func f(x:u32,y:u64)->u64{return y;}behave @print(f(1u8,7u8));',0,'7\n',''),
 'same_type_failure': (NPE+'func f(x:T,y:T)->bool throws NullPointerException where T;func f(x:u32,y:u64)->bool{return true;}f(1u8,2u8);',2,'','requires a covering catch or throws'),
 'different_type_failure_inapplicable': (IO+NPE+'func f(x:T,y:T)->bool throws NullPointerException where T;func f(x:u32,y:u64)->bool{return true;}behave @print(f(1u32,2u64));',0,'true\n',''),
 'same_type_complete_failure': (IO+NPE+'func f(x:T,y:T)->bool throws NullPointerException where T;func f(x:u32,y:u64)->bool{return true;}func selected=f;try{behave @print(selected(1u8,2u8));}catch(e:const NullPointerException){};',0,'true\n',''),
 'query_result': (IO+DECL+METHOD+'val result<-make local typeof(f(1u32)){7f64};behave @print(result);',0,'7\n',''),
 'retained_query': (IO+DECL+METHOD+'func factory(){val input<-1u32;func inspect(){val value<-make local typeof(f(input)){7f64};return value;};return inspect;}func selected=factory();behave @print(selected());',0,'7\n',''),
 'checked_failure': (NPE+FAILURE+'f(1u32);',2,'','requires a covering catch or throws'),
 'checked_failure_caught': (IO+NPE+FAILURE+'try{behave @print(f(7u32));}catch(e:const NullPointerException){};',0,'7\n',''),
 'checked_failure_executes': (IO+NPE+'func f(x:u32)->f64 throws NullPointerException;func f(x:f64)->f64 throws NullPointerException{ptr missing:f64=null in here;return missing;}try{f(1u32);{}}catch(e:const NullPointerException){behave @print(7u64);};',0,'7\n',''),
 'disjoint_failure': (IO+NPE+FAILURE+'behave @print(f(7f32));',0,'7\n',''),
 'query_has_no_failure': (IO+NPE+FAILURE+'val result<-make local typeof(f(1u32)){7f64};behave @print(result);',0,'7\n',''),
 'deferred_failure': (NPE+FAILURE+'defer f(1u32);',2,'','requires a covering catch or throws'),
 'deferred_failure_executes': (IO+NPE+'func f(x:u32)->f64 throws NullPointerException;func f(x:f64)->f64 throws NullPointerException{ptr missing:f64=null in here;return missing;}try{ {{defer f(1u32);}}; }catch(e:const NullPointerException){behave @print(7u64);};',0,'7\n',''),
 'deferred_snapshot': (IO+'proc show(x:u32)->void;proc show(x:f64)->void{behave @print(x);return;}val input<-16777217u32;{{defer show(input);input<-1u32:void;}};',0,'16777217\n',''),
 'method_kind': ('proc f(x:u32)->f64;'+METHOD+'f(1u32);',2,'','implementation kind does not satisfy callable declaration'),
 'method_throws': (NPE+DECL+'func f(x:f64)->f64 throws NullPointerException{return x;}try{f(1u32);{}}catch(e:const NullPointerException){};',2,'','implementation exposes a checked exception'),
 'complete_view_result': (IO+DECL+METHOD+'func selected:func(u32)->f64=f;behave @print(selected(7u8));',0,'7\n',''),
 'complete_view_failure': (NPE+FAILURE+'func selected:func(f64)->f64=f;',2,'','callable view omits a required checked exception'),
 'complete_view_declared_failure': (IO+NPE+FAILURE+'func selected:func(f64)->f64 throws NullPointerException=f;try{behave @print(selected(7u32));}catch(e:const NullPointerException){};',0,'7\n',''),
 'complete_inferred_failure': (IO+NPE+FAILURE+'func selected=f;try{behave @print(selected(7u32));}catch(e:const NullPointerException){};',0,'7\n',''),
 'complete_inferred_failure_required': (NPE+FAILURE+'func selected=f;selected(7f32);',2,'','requires a covering catch or throws'),
 'complete_query_failure': (IO+NPE+FAILURE+'func factory(){func captured=f;func inspect(){val result<-make local typeof(captured(1u32)){7f64};return result;};return inspect;}func selected=factory();behave @print(selected());',0,'7\n',''),
 'returned_complete_failure': (IO+NPE+FAILURE+'func factory(){return f;}func selected=factory();try{behave @print(selected(7u32));}catch(e:const NullPointerException){};',0,'7\n',''),
 'returned_complete_requires_failure': (NPE+FAILURE+'func factory(){return f;}func selected=factory();selected(7f64);',2,'','requires a covering catch or throws'),
 'borrowed_result': (IO+'func choose(x:u32,ref input:i64 in O)->ref i64 in O where {owner O};func choose(x:f64,ref input:i64 in O)->ref i64 in O where {owner O}{return input;}val slot<-3;choose(1u32,slot)<-7;behave @print(slot);',0,'7\n',''),
 'owner_overlap': (IO+'func choose(x:u32,ref a:const i64 in O,ref b:const i64 in O)->ref const i64 in O where {owner O};func choose(x:f64,ref a:const i64 in A,ref b:const i64 in B)->ref const i64 in B where {owner A,owner B}{return b;}val outer<-3;{{val inner<-7;behave @print(choose(1u32,outer,inner));}};behave @print(choose(1u32,outer,outer));',0,'7\n3\n',''),
 'disjoint_owner_failure': (IO+NPE+'func choose(x:u32,ref a:const i64 in O,ref b:const i64 in O)->u64 throws NullPointerException where {owner O};func choose(x:f64,ref a:const i64 in A,ref b:const i64 in B)->u64 where {owner A,owner B}{return 7u64;}val outer<-3;{{val inner<-9;behave @print(choose(1u32,outer,inner));}};',0,'7\n',''),
 'same_owner_failure': (NPE+'func choose(x:u32,ref a:const i64 in O,ref b:const i64 in O)->u64 throws NullPointerException where {owner O};func choose(x:f64,ref a:const i64 in A,ref b:const i64 in B)->u64 where {owner A,owner B}{return 7u64;}val slot<-3;choose(1u32,slot,slot);',2,'','requires a covering catch or throws'),
 'variadic_overlap': (IO+'func f(rest:u32...)->u64;func f(x:u64,y:u64,z:u64){return z;}behave @print(f(1u8,2u32,7u32));',0,'7\n',''),
 'variadic_checked_failure': (NPE+'func f(rest:u32...)->u64 throws NullPointerException;func f(x:u64,y:u64)->u64{return y;}f(1u32,2u32);',2,'','requires a covering catch or throws'),
 'operator_overlap': (IO+'func (+)(x:u32,y:u32)->f64;func (+)(x:f64,y:f64)->f64{return intrinsic (+)(x,y);}behave @print(1u32+2u32);',0,'3\n',''),
 'imported_declaration': ({'main.cod':IO+'import "./lib.cod" f;func f(x:f64)->f64{return x;}behave @print(f(7u32));','lib.cod':'func f(x:u32)->f64;export f;'},0,'7\n',''),
}
for source, literal, expected, targets in (
    ('u8', '255u8', '255\n', ('u32', 'u64', 'i64', 'f32', 'f64')),
    ('u32', '16777217u32', '16777217\n', ('u64', 'i64', 'f64')),
    ('f32', '0.5f32', '0.5\n', ('f64',)),
):
    for target in targets:
        runner.CASES['overlap_'+source+'_'+target] = (
            IO+f'func f(x:{source})->{target};func f(x:{target}){{return x;}}behave @print(f({literal}));',
            0, expected, '')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
