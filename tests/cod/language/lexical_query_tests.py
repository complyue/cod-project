#!/usr/bin/env python3
"""Lexical declaration evidence stays independent of captured runtime values."""
from pathlib import Path
import foundation_tests as runner

IO='using symbol "std:io" @print;'
CLASSIFY='func classify(ptr x:i64 in O)->const i64 where {owner O}{return 1;}func classify(ptr x:bool in O)->const i64 where {owner O}{return 2;}'
FACTORY='func factory(x:T)->func(u64)->const i64 where T{func inner(n:u64)->const i64{val y<-make local typeof(x);if(n==0u64){return classify(y);}else{return inner(n-1u64);};};return inner;}'
ACCESS='func access(ref x:i64 in O)->const i64 where {owner O}{return x;}func access(ref x:const i64 in O)->const bool where {owner O}{return true;}'
OWNERS='func owners(ref x:i64 in O,ref y:i64 in O)->const bool where {owner O}{return true;}func owners(ref x:i64 in O,ref y:i64 in P)->const i64 where {owner O,owner P}{return 1;}'
runner.CASES={
 'outer_uninitialized_slot':(IO+'func outer()->const i64{val x<-make local i64;func inner()->const i64{val y<-make local typeof(x){7};return y;};return inner();}behave @print(outer());',0,'7\n',''),
 'outer_null_slot':(IO+'func outer()->const i64{ptr x:i64=null in here;func inner()->const i64{val y<-make local typeof(x){8};return y;};return inner();}behave @print(outer());',0,'8\n',''),
 'same_signature_distinct_contexts':(IO+CLASSIFY+FACTORY+'func a=factory(1);func b=factory(true);behave @print(a(12u64));behave @print(b(13u64));behave @print(a(14u64));',0,'1\n2\n1\n',''),
 'reversed_context_demand':(IO+CLASSIFY+FACTORY+'func b=factory(true);func a=factory(1);behave @print(b(3u64));behave @print(a(4u64));',0,'2\n1\n',''),
 'copied_contexts':(IO+CLASSIFY+FACTORY+'func a=factory(1);func b=factory(true);func c=a; a<-b;behave @print(a(2u64));behave @print(c(2u64));',0,'2\n1\n',''),
 'recursive_capture_type_and_query_context':(IO+CLASSIFY+'func base()->const i64{return 10;}func factory(x:T,f:func()->const i64)->func(T,u64)->const i64 where T{func inner(y:T,n:u64)->const i64{val z<-make local typeof(x);if(n==0u64){return f()+classify(z);}else{return inner(y,n-1u64);};};return inner;}func a=factory(1,base);func b=factory(true,base);behave @print(a(2,12u64));behave @print(b(false,13u64));',0,'11\n12\n',''),
 'transitive_query_dependencies':(IO+CLASSIFY+'func factory(x:T)->func()->const i64 where T{return func()->const i64{func inner()->const i64{val y<-make local typeof(x);return classify(y);};return inner();};}func a=factory(1);func b=factory(true);behave @print(a());behave @print(b());',0,'1\n2\n',''),
 'transitive_returned_callable':(IO+CLASSIFY+'func factory(x:T)->func()->func()->const i64 where T{return func()->func()->const i64{return func()->const i64{val y<-make local typeof(x);return classify(y);};};}func a=factory(1)();func b=factory(true)();behave @print(a());behave @print(b());',0,'1\n2\n',''),
 'shadowing_outer_parameter':(IO+CLASSIFY+'func outer(x:bool)->const i64{func inner(x:i64)->const i64{val y<-make local typeof(x);return classify(y);};return inner(1);}behave @print(outer(true));',0,'1\n',''),
 'shadowing_enclosing_scope':(IO+CLASSIFY+'func outer()->const i64{val x<-true;return {{val x<-1;func inner()->const i64{val y<-make local typeof(x);return classify(y);};inner()}};}behave @print(outer());',0,'1\n','shadow'),
 'initializer_sees_outer_name':(IO+'func outer()->const i64{val x<-1;func inner()->const i64{val x<-make local typeof(x){9};return x;};return inner();}behave @print(outer());',0,'9\n',''),
 'query_only_explicit_empty_captures':(IO+'func outer()->const i64{val x<-1;func inner []()->const i64{val y<-make local typeof(x){10};return y;};return inner();}behave @print(outer());',0,'10\n',''),
 'query_local_callable_signature':(IO+'func id(x:i64)->const i64{return x;}func outer()->const i64{func f=id;func inner()->const i64{val y<-make local typeof(f(1)){11};return y;};return inner();}behave @print(outer());',0,'11\n',''),
 'query_callable_and_runtime_capture':(IO+'func id(x:i64)->const i64{return x;}func outer()->const i64{func f=id;func inner()->const i64{val y<-make local typeof(f(1)){12};return f(y);};return inner();}behave @print(outer());',0,'12\n',''),
 'explicit_capture_stays_retained':(IO+'func id(x:i64)->const i64{return x;}func outer()->const i64{func f=id;func inner [f]()->const i64{val y<-make local typeof(f(1)){13};return y;};return inner();}behave @print(outer());',0,'13\n','unused callable capture'),
 'explicit_scalar_capture_still_rejected':('func outer()->const i64{val x<-1;func inner [x]()->const i64{val y<-make local typeof(x){1};return y;};return inner();}outer();',2,'','ordinary lexical cells'),
 'runtime_scalar_capture_still_rejected':('func outer()->const i64{val x<-1;func inner()->const i64{val y<-make local typeof(x){1};return x+y;};return inner();}outer();',2,'','ordinary lexical cells'),
 'query_does_not_initialize_outer':('func outer()->const i64{val x<-make local i64;func inner()->const i64{val y<-make local typeof(x){1};return y;};inner();return x;}outer();',2,'','before definite initialization'),
 'unavailable_query_binding':('func outer(b:bool)->const i64{if(b){val x<-1;}else{};func inner()->const i64{val y<-make local typeof(x){1};return y;};return inner();}outer(true);',2,'','not available on every reaching path'),
 'unknown_query_binding':('func outer()->const i64{func inner()->const i64{val y<-make local typeof(missing){1};return y;};return inner();}outer();',2,'','known lexical declaration'),
 'future_declaration_not_visible':('func outer()->const i64{func inner()->const i64{val y<-make local typeof(x){1};return y;};val x<-1;return inner();}outer();',2,'','known lexical declaration'),
 'owner_binding_not_a_value':('func outer(ref x:i64 in O)->const i64 where {owner O}{func inner()->const i64{val y<-make local typeof(O){1};return y;};return inner();}val x<-1;outer(x);',2,'','owner binding is not a value'),
 'readonly_query_rejected':('func take(ref x:i64 in O)->const i64 where {owner O}{return x;}func outer()->const i64{const val x<-1;func inner()->const i64{val y<-make local typeof(take(x)){1};return y;};return inner();}outer();',2,'','readonly argument'),
 'access_selects_evidence_context':(IO+CLASSIFY+ACCESS+'func choose(k:i64,ref x:i64 in O)->ref i64 in O where {owner O}{return x;}func choose(k:bool,ref x:i64 in O)->ref const i64 in O where {owner O}{return x;}func factory(k:T)->func()->const i64 where T{val x<-1;ref y=choose(k,x);return func()->const i64{val z<-make local typeof(access(y));return classify(z);};}func a=factory(1);func b=factory(true);behave @print(a());behave @print(b());',0,'1\n2\n',''),
 'owner_relation_selects_evidence_context':(IO+CLASSIFY+OWNERS+'func choose(k:i64,ref a:i64 in A,ref b:i64 in B)->ref i64 in A where {owner A,owner B}{return a;}func choose(k:bool,ref a:i64 in A,ref b:i64 in B)->ref i64 in B where {owner A,owner B}{return b;}func factory(k:T,ref b:i64 in B)->func()->const i64 where {T,owner B}{val a<-1;ref y=choose(k,a,b);return func()->const i64{val z<-make local typeof(owners(a,y));return classify(z);};}val x<-1;func a=factory(1,x);func b=factory(true,x);behave @print(a());behave @print(b());',0,'2\n1\n',''),
 'unknown_outer_owner_equality':(OWNERS+'func outer(ref a:i64 in A,ref b:i64 in B)->func()->const i64 where {owner A,owner B}{return func()->const i64{val z<-make local typeof(owners(a,b)){1};return 1;};}val x<-1;outer(x,x)();',2,'','unproved owner equality'),
 'outer_owner_vs_current_formal_unproved':(OWNERS+'func outer(ref x:i64 in O)->const i64 where {owner O}{val a<-1;func inner(ref b:i64 in B)->const i64 where {owner B}{val z<-make local typeof(owners(a,b)){1};return 1;};return inner(x);}val x<-1;outer(x);',2,'','unproved owner equality'),
 'outer_owner_differs_from_current_scope':(IO+CLASSIFY+OWNERS+'func outer()->func()->const i64{val a<-1;return func()->const i64{val b<-1;val z<-make local typeof(owners(a,b));return classify(z);};}behave @print(outer()());',0,'1\n',''),
 'outer_owner_relation_survives_nesting':(IO+CLASSIFY+OWNERS+'func outer()->func()->const i64{val a<-1;return {{val b<-2;func middle()->const i64{func inner()->const i64{val z<-make local typeof(owners(a,b));return classify(z);};return inner();};middle}};}behave @print(outer()());',0,'1\n',''),
 'module_lexical_query':(IO+'val x<-make local i64;func f=func()->const i64{val y<-make local typeof(x){14};return y;};behave @print(f());',0,'14\n',''),
 'query_only_catch_value':(IO+'using "std:exceptions" NullPointerException;proc outer()->void{ptr p:i64=null in here;try{p;{}}catch(e:const NullPointerException){func inner()->const u64{val y<-make local typeof(e.category){15u64};return y;};behave @print(inner());};}outer();',0,'15\n',''),
 'query_only_loop_control':(IO+'proc outer()->void{for(i in 0u64..2u64){ {{func inner()->const u64{val y<-make local typeof(i){16u64};return y;};behave @print(inner());}}; };}outer();',0,'16\n16\n',''),
 'initializer_keeps_runtime_capture':(IO+'func id(x:i64)->const i64{return x;}func outer()->const i64{func f=id;val x<-1;func inner()->const i64{val y<-make local typeof(x){f(17)};return y;};return inner();}behave @print(outer());',0,'17\n',''),
}
runner.CASE_ARGUMENTS={}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
 raise SystemExit(runner.main())
