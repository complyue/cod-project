#!/usr/bin/env python3
"""Owner equality composes with type/access selection and ordered preparation."""
from pathlib import Path
import foundation_tests as runner
IO='using symbol "std:io" @print;'
PAIR='func choose(ref a:i64 in O,ref b:i64 in O)->const i64 where {owner O}{return 1;}func choose(ref a:i64 in O,ref b:i64 in P)->const i64 where {owner O,owner P}{return 2;}'
OBS=lambda x:'behave @print('+x+');'
PROJECT='func first(ref a:i64 in O,ref b:i64 in P)->ref i64 in O where {owner O,owner P}{return a;}func second(ref a:i64 in O,ref b:i64 in P)->ref i64 in P where {owner O,owner P}{return b;}'
runner.CASES={
 'same_and_different':(IO+PAIR+'ref a:i64=3,b:i64=4;'+OBS('choose(a,b)')+'{{val c<-5;'+OBS('choose(a,c)')+'}};',0,'1\n2\n',''),
 'reversed_declarations':(IO+PAIR[PAIR.index('func choose(ref a:i64 in O,ref b:i64 in P)'):]+PAIR[:PAIR.index('func choose(ref a:i64 in O,ref b:i64 in P)')]+'val a<-3,b<-4;'+OBS('choose(a,b)'),0,'1\n',''),
 'same_target':(IO+PAIR+'val a<-3;'+OBS('choose(a,a)'),0,'1\n',''),
 'declared_place_owner_via_reference':(IO+PAIR+'ref a:i64=3;ref p=a;var q->p;ref snapshot=q;{{val b<-4;'+OBS('choose(snapshot,a)')+OBS('choose(snapshot,b)')+'}};',0,'1\n2\n',''),
 'scope_value_result_is_not_an_existing_target':(IO+PAIR+'val a<-3,b<-4;'+OBS('choose(a,{{ref c=b;return c;}})'),2,'','borrowed arguments require existing target views'),
 'scoped_copy_owner':(IO+PAIR+'ref a:i64=3;{{ptr b<-a;'+OBS('choose(a,b)')+'}};',0,'2\n',''),
 'ordinary_group_declaration':(IO+PAIR+'val a<-3;'+OBS('choose(a,{val b<-4;b})'),0,'1\n',''),
 'conditional_owner':(IO+PAIR+'val a<-3,b<-4;'+OBS('choose(a,if(true){a}else{b})'),0,'1\n',''),
 'guarded_owner':(IO+PAIR+'val a<-3,b<-4;'+OBS('choose(a,try{b}finally{})'),0,'1\n',''),
 'named_result_owner':(IO+PAIR+PROJECT+'ref a:i64=3;{{val b<-4;'+OBS('choose(a,first(a,b))')+OBS('choose(a,second(a,b))')+'}};',0,'1\n2\n',''),
 'selected_result_owner':(IO+PAIR+PROJECT+'func f=first;func g=second;ref a:i64=3;{{val b<-4;'+OBS('choose(a,f(a,b))')+OBS('choose(a,g(a,b))')+'}};',0,'1\n2\n',''),
 'owner_and_type_specific':(IO+'func choose(ref a:T in O,ref b:U in P)->const i64 where {T,U,owner O,owner P}{return 2;}func choose(ref a:i64 in O,ref b:i64 in O)->const i64 where {owner O}{return 1;}ref a:i64=3,b:i64=4;'+OBS('choose(a,b)')+'{{val c<-5;'+OBS('choose(a,c)')+'}};',0,'1\n2\n',''),
 'owner_type_tradeoff':('func choose(ref a:T in O,ref b:T in O)->const i64 where {T,owner O}{return 1;}func choose(ref a:i64 in O,ref b:i64 in P)->const i64 where {owner O,owner P}{return 2;}val a<-3,b<-4;choose(a,b);',2,'','ambiguous methods'),
 'owner_access_tradeoff':('func choose(ref a:const i64 in O,ref b:const i64 in O)->const i64 where {owner O}{return 1;}func choose(ref a:i64 in O,ref b:i64 in P)->const i64 where {owner O,owner P}{return 2;}val a<-3,b<-4;choose(a,b);',2,'','ambiguous methods'),
 'owner_access_specific':(IO+'func choose(ref a:i64 in O,ref b:i64 in O)->const i64 where {owner O}{return 1;}func choose(ref a:const i64 in O,ref b:const i64 in P)->const i64 where {owner O,owner P}{return 2;}val a<-3,b<-4;'+OBS('choose(a,b)')+'const ptr observed:const i64=a;'+OBS('choose(observed,b)'),0,'1\n2\n',''),
 'crossed_owner_constraints':('func f(ref a:i64 in O,ref b:i64 in O,ref c:i64 in P)->const i64 where {owner O,owner P}{return 1;}func f(ref a:i64 in O,ref b:i64 in P,ref c:i64 in P)->const i64 where {owner O,owner P}{return 2;}val a<-3,b<-4,c<-5;f(a,b,c);',2,'','ambiguous methods'),
 'transitive_owner_specific':(IO+'func f(ref a:i64 in O,ref b:i64 in O,ref c:i64 in P)->const i64 where {owner O,owner P}{return 1;}func f(ref a:i64 in O,ref b:i64 in P,ref c:i64 in P)->const i64 where {owner O,owner P}{return 2;}func f(ref a:i64 in O,ref b:i64 in O,ref c:i64 in O)->const i64 where {owner O}{return 3;}val a<-3,b<-4,c<-5;'+OBS('f(a,b,c)'),0,'3\n',''),
 'renamed_owner_duplicate':('func f(ref a:i64 in O,ref b:i64 in O)->void where {owner O}{}func f(ref a:i64 in P,ref b:i64 in P)->void where {owner P}{}',2,'','duplicate method signature'),
 'independent_owner_duplicate':('func f(ref a:i64 in O,ref b:i64 in P)->void where {owner O,owner P}{}func f(ref a:i64 in Q,ref b:i64 in R)->void where {owner Q,owner R}{}',2,'','duplicate method signature'),
 'single_constraint_rejects_distinct':('func f(ref a:i64 in O,ref b:i64 in O)->void where {owner O}{}ref a:i64=3;{{val b<-4;f(a,b);}};',2,'','different actual owners'),
 'unproved_formal_equality':(PAIR+'func f(ref a:i64 in O,ref b:i64 in P)->const i64 where {owner O,owner P}{return choose(a,b);}',2,'','unproved owner equality'),
 'proved_formal_equality':(IO+PAIR+'func f(ref a:i64 in O,ref b:i64 in O)->const i64 where {owner O}{return choose(a,b);}val a<-3,b<-4;'+OBS('f(a,b)'),0,'1\n',''),
 'local_and_external_distinct':(IO+PAIR+'func f(ref a:i64 in O)->const i64 where {owner O}{val b<-4;return choose(a,b);}val a<-3;'+OBS('f(a)'),0,'2\n',''),
 'recursive_same_owner':(IO+PAIR+'func f(ref a:i64 in O,ref b:i64 in O,n:i64)->const i64 where {owner O}{if(n==0){return choose(a,b);}else{return f(a,b,n-1);};}val a<-3,b<-4;'+OBS('f(a,b,40)'),0,'1\n',''),
 'shared_projected_owner':(IO+PAIR+PROJECT+'func run(f:F,ref a:i64 in O,n:i64)->const i64 where {F,owner O}{if(n>0){return run(f,a,n-1);}else{val b<-4;ptr p=f(a,b);return choose(a,p);};}func f=first;func g=second;val a<-3;'+OBS('run(f,a,40)')+OBS('run(g,a,40)'),0,'1\n2\n',''),
 'nullable_fixed_owner':(IO+'func f(ptr a:i64 in O,ptr b:i64 in O)->const i64 where {owner O}{return 1;}func f(ptr a:i64 in O,ptr b:i64 in P)->const i64 where {owner O,owner P}{return 2;}val a<-3,b<-4;ptr p=a;p=null;'+OBS('f(p,b)'),0,'1\n',''),
 'selected_writes_initialize':(IO+'using "std:exceptions" NullPointerException;proc init(ptr a:i64 in O,ptr b:i64 in O)->void throws NullPointerException where {owner O}{a<-41;b<-42;}proc init(ptr a:i64 in O,ptr b:i64 in P)->void throws NullPointerException where {owner O,owner P}{a<-43;b<-44;}val a<-make local i64,b<-make local i64;try{init(a,b);'+OBS('a')+OBS('b')+'}catch(e:const NullPointerException){};',0,'41\n42\n',''),
 'deferred_owner_selection':(IO+'proc show(ref a:i64 in O,ref b:i64 in O)->void where {owner O}{'+OBS('a+b')+'}proc show(ref a:i64 in O,ref b:i64 in P)->void where {owner O,owner P}{'+OBS('a-b')+'}ref a:i64=3;{{val b<-4;defer show(a,b);defer show(b,b);b<-7;}};',0,'14\n-4\n',''),
 'builtin_owner_fallback_snapshot':(IO+'func (+)(ref a:i64 in O,ref b:i64 in O)->const i64 where {owner O}{return 99;}ref a:i64=3;{{val b<-4;'+OBS('a+{a<-9;b}')+OBS('a+a')+'}};',0,'7\n99\n',''),
 'unknown_owner_no_builtin_guess':('func (+)(ref a:i64 in O,ref b:i64 in O)->const i64 where {owner O}{return 99;}func f(ref a:i64 in O,ref b:i64 in P)->const i64 where {owner O,owner P}{return a+b;}',2,'','unproved owner equality'),
 'later_owner_result_prepares_copy':(IO+'func tag(ref a:i64 in O,ref b:i64 in O)->const bool where {owner O}{return true;}func tag(ref a:i64 in O,ref b:i64 in P)->const i64 where {owner O,owner P}{return 0;}func pick(ref a:i64 in O,tag:bool)->const i64 where {owner O}{return a;}func pick(a:i64,tag:i64)->const i64{return a;}ref a:i64=3;{{val b<-4;'+OBS('pick(a,{a<-9;tag(a,b)})')+'a<-3;'+OBS('pick(a,{a<-9;tag(a,a)})')+'}};',0,'3\n9\n',''),
 'selected_owner_contract':(IO+'func f(ref a:i64 in O,ref b:i64 in O)->const i64 where {owner O}{return a;}func g=f;val a<-3,b<-4;'+OBS('g(a,b)'),0,'3\n',''),
 'selected_owner_mismatch':('func f(ref a:i64 in O,ref b:i64 in O)->const i64 where {owner O}{return a;}func g=f;ref a:i64=3;{{val b<-4;g(a,b);}};',2,'','different actual owners'),
 'selected_unproved_equality':('func f(ref a:i64 in O,ref b:i64 in O)->const i64 where {owner O}{return a;}func run(g:F,ref a:i64 in O,ref b:i64 in P)->const i64 where {F,owner O,owner P}{return g(a,b);}func g=f;val a<-3,b<-4;run(g,a,b);',2,'','unproved owner equality'),
 'queried_selected_arity':(PAIR+'func f(ref a:i64 in O)->ref i64 in O where {owner O}{return a;}func g=f;val a<-3;choose(a,g(a,a));',2,'','argument count'),
 'operand_effects_once':(IO+PAIR+'val a<-3,b<-4;'+OBS('choose({'+OBS('7')+'a},{'+OBS('8')+'b})'),0,'7\n8\n1\n',''),
}
runner.CASE_ARGUMENTS={}
runner.CASE_SHADOWS = {
    'same_and_different': ['a', 'b', 'a', 'b'],
    'reversed_declarations': ['a', 'b', 'a', 'b'],
    'same_target': ['a', 'a'],
    'declared_place_owner_via_reference': ['a', 'a'],
    'scoped_copy_owner': ['a', 'a'],
    'ordinary_group_declaration': ['a', 'b', 'a', 'b'],
    'conditional_owner': ['a', 'b', 'a', 'b'],
    'guarded_owner': ['a', 'b', 'a', 'b'],
    'named_result_owner': ['a', 'a', 'a', 'a'],
    'selected_result_owner': ['a', 'a', 'a', 'a'],
    'owner_and_type_specific': ['a', 'b', 'a', 'b'],
    'owner_access_specific': ['a', 'b', 'a', 'b'],
    'transitive_owner_specific': ['a', 'b', 'c', 'a', 'b', 'c', 'a', 'b', 'c'],
    'proved_formal_equality': ['a', 'b', 'a', 'b', 'a', 'b'],
    'local_and_external_distinct': ['a', 'a', 'a'],
    'recursive_same_owner': ['a', 'b', 'a', 'b', 'a', 'b'],
    'shared_projected_owner': ['a', 'a', 'a', 'a', 'f', 'a'],
    'nullable_fixed_owner': ['a', 'b', 'a', 'b'],
    'selected_writes_initialize': ['a', 'b', 'a', 'b'],
    'deferred_owner_selection': ['a', 'a'],
    'builtin_owner_fallback_snapshot': ['a'],
    'later_owner_result_prepares_copy': ['a', 'a', 'a', 'tag', 'a', 'tag'],
    'selected_owner_contract': ['a', 'b'],
    'operand_effects_once': ['a', 'b', 'a', 'b'],
}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
