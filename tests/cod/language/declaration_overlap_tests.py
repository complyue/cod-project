#!/usr/bin/env python3
"""Overlapping declarations constrain independently established method contracts."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
DECL = 'func choose(ref a:const i64 in O,ref b:const i64 in O)->ref const i64 in O where {owner O};'
METHOD = 'func choose(ref a:const i64 in A,ref b:const i64 in B)->ref const i64 in B where {owner A,owner B}{return b;}'
THROW = 'func f(ref a:const i64 in O,ref b:const i64 in O)->i64 throws NullPointerException where {owner O};'
BODY = 'func f(ref a:const i64 in A,ref b:const i64 in B)->i64 where {owner A,owner B}{return b;}'
ACCESS = 'func read(ref input:i64 in O)->i64 throws NullPointerException where {owner O};func read(ref input:const i64 in O)->i64 where {owner O}{return input;}'
OUT = lambda expression: 'behave @print(' + expression + ');'
runner.CASES = {
 'same_and_distinct_owners': (IO+DECL+METHOD+'val outer<-3;{{val inner<-7;'+OUT('choose(outer,inner)')+'}};'+OUT('choose(outer,outer)'),0,'7\n3\n',''),
 'method_result_owner_stays_independent': (DECL+METHOD+'val outer<-3;{{val inner<-7;ref selected=choose(outer,inner);selected=outer;}};',2,'','owner'),
 'wrong_result_owner': ('func f(ref a:i64 in O,ref b:i64 in O,ref c:i64 in P)->ref i64 in O where {owner O,owner P};func f(ref a:i64 in A,ref b:i64 in B,ref c:i64 in C)->ref i64 in C where {owner A,owner B,owner C}{return c;}',2,'','does not satisfy callable declaration'),
 'transitive_overlap_owners': (IO+'func f(ref a:i64 in O,ref b:i64 in P,ref c:i64 in O,ref d:i64 in P)->ref i64 in O where {owner O,owner P};func f(ref a:i64 in A,ref b:i64 in A,ref c:i64 in B,ref d:i64 in C)->ref i64 in C where {owner A,owner B,owner C}{return d;}val outer<-3;{{val middle<-5;{{val inner<-7;f(outer,outer,middle,inner)<-9;'+OUT('inner')+OUT('outer')+'}};}};',0,'9\n3\n',''),
 'body_result_anchor': (IO+'func f(ref a:const i64 in O,ref b:const i64 in O)->i64 where {owner O};'+BODY.replace('->i64 where','where')+'val slot<-7;'+OUT('f(slot,slot)'),0,'7\n',''),
 'partial_declaration_cannot_anchor_cycle': ('func f(ref a:const i64 in O,ref b:const i64 in O)->i64 where {owner O};func f(ref a:const i64 in A,ref b:const i64 in B) where {owner A,owner B}{return f(a,b);}',2,'','unproved callable result'),
 'covering_declaration_supplies_borrow': (IO+DECL+'func choose(ref a:const i64 in A,ref b:const i64 in B)->ref const i64 in B where {owner A,owner B};'+METHOD.replace('->ref const i64 in B where','where')+'val slot<-7;'+OUT('choose(slot,slot)'),0,'7\n',''),
 'partial_result_cannot_retarget_body': ('func f(ref a:const i64 in O,ref b:const i64 in O)->bool where {owner O};'+BODY.replace('->i64 where','where'),2,'','does not satisfy callable declaration'),
 'partial_result_mode_does_not_make_borrow': (DECL+METHOD.replace('->ref const i64 in B where','where'),2,'','does not satisfy callable declaration'),
 'result_access_is_checked': (DECL.replace('ref const i64 in O','ref i64 in O')+METHOD,2,'','does not satisfy callable declaration'),
 'readonly_method_body_stays_readonly': ('func f(ref a:i64 in O)->i64 where {owner O};func f(ref a:const i64 in O)->i64 where {owner O}{a<-9;return a;}',2,'','readonly'),
 'partial_access_body_inference': (IO+'func f(ref a:i64 in O)->i64 where {owner O};func f(ref a:const i64 in O) where {owner O}{return a;}const val slot<-7;'+OUT('f(slot)'),0,'7\n',''),
 'generic_partial_owners': (IO+'func f(ref a:const T in O,ref b:const T in O)->const T where {T,owner O};func f(ref a:const U in A,ref b:const U in B) where {U,owner A,owner B}{return b;}val number<-7,flag<-true;'+OUT('f(number,number)')+OUT('f(flag,flag)'),0,'7\ntrue\n',''),
 'declaration_does_not_change_body_equalities': ('func same(ref a:const i64 in O,ref b:const i64 in O)->i64 where {owner O}{return a;}func f(ref a:const i64 in O,ref b:const i64 in O)->i64 where {owner O};'+BODY.replace('return b;','return same(a,b);'),2,'','unproved owner equality'),
 'different_owners_skip_declared_failure': (IO+NPE+THROW+BODY+'val outer<-3;{{val inner<-7;'+OUT('f(outer,inner)')+'}};',0,'7\n',''),
 'equal_owners_keep_declared_failure': (NPE+THROW+BODY+'val slot<-7;f(slot,slot);',2,'','requires a covering catch or throws'),
 'unknown_owner_relation_keeps_failure': (NPE+THROW+BODY+'func relay(ref a:const i64 in A,ref b:const i64 in B)->i64 where {owner A,owner B}{return f(a,b);}',2,'','requires a covering catch or throws'),
 'readonly_actual_skips_mutable_declaration': (IO+NPE+ACCESS+'const val slot<-7;'+OUT('read(slot)'),0,'7\n',''),
 'writable_actual_keeps_declared_failure': (NPE+ACCESS+'val slot<-7;read(slot);',2,'','requires a covering catch or throws'),
 'partial_declaration_restricts_implementation_throws': (NPE+THROW.replace(' throws NullPointerException','')+BODY.replace(' where {owner A',' throws NullPointerException where {owner A'),2,'','outside its callable declaration'),
 'partial_declaration_keeps_kind': (THROW.replace(' throws NullPointerException','')+BODY.replace('func f','proc f'),2,'','implementation kind'),
 'deferred_distinct_owner_call': (IO+NPE+THROW.replace('func f','proc f').replace('->i64','->void')+BODY.replace('func f','proc f').replace('->i64','->void').replace('return b;',OUT('b'))+'val outer<-3;{{val inner<-7;{{defer f(outer,inner);inner<-9;}};}};',0,'9\n',''),
 'deferred_same_owner_call_keeps_failure': (NPE+THROW+BODY+'val slot<-7;defer f(slot,slot);',2,'','requires a covering catch or throws'),
 'complete_view_retains_possible_failure': (NPE+THROW+BODY+'func selected=f;val outer<-3;{{val inner<-7;selected(outer,inner);}};',2,'','requires a covering catch or throws'),
 'complete_view_cannot_omit_failure': (NPE+THROW+BODY+BODY.replace('func f','func prototype')+'func selected=prototype;selected<-f;',2,'','required checked exception'),
 'partial_borrowed_callable': (IO+DECL+METHOD+'func selected=choose;val outer<-3;{{val inner<-7;'+OUT('selected(outer,inner)')+'}};',0,'7\n',''),
 'type_query_does_not_add_failure': (IO+NPE+THROW+BODY+'val slot<-7;val result<-make local typeof(f(slot,slot)){9};'+OUT('result'),0,'9\n',''),
 'swap_distinct_owners_skip_failure': (IO+NPE+'func (<->)(a:i64 in O,b:i64 in O)->void throws NullPointerException where {owner O};val outer<-3;{{val inner<-7;outer<->inner;'+OUT('inner')+'}};'+OUT('outer'),0,'3\n7\n',''),
 'swap_same_owner_keeps_failure': (NPE+'func (<->)(a:i64 in O,b:i64 in O)->void throws NullPointerException where {owner O};val left<-3,right<-7;left<->right;',2,'','requires a covering catch or throws'),
 'nullable_result_stays_nullable': (IO+'func f(ptr a:const i64 in O,ptr b:const i64 in O)->ptr const i64 in O where {owner O};func f(ptr a:const i64 in A,ptr b:const i64 in B)->ptr const i64 in B where {owner A,owner B}{return b;}val outer<-3;{{ptr absent:i64=null in here;ptr selected=f(outer,absent);'+OUT('isnull(selected)')+'}};',0,'true\n',''),
 'imported_overlap_preserves_contract': ({'base.cod':DECL+'export choose;', 'main.cod':IO+'import "base.cod" choose;'+METHOD+'val outer<-3;{{val inner<-7;'+OUT('choose(outer,inner)')+'}};'},0,'7\n',''),
}
runner.CASES['variadic_owner_overlap'] = (IO+'func f(ref rest:const i64 in O...)->ref const i64 in O where {owner O};func f(ref a:const i64 in A,ref b:const i64 in B,ref c:const i64 in C)->ref const i64 in C where {owner A,owner B,owner C}{return c;}val outer<-3;{{val middle<-5;{{val inner<-7;'+OUT('f(outer,middle,inner)')+'}};}};',0,'7\n','')
runner.CASES['variadic_overlap_keeps_prefix_owner'] = ('func f(ref first:const i64 in O,ref rest:const i64 in P...)->ref const i64 in O where {owner O,owner P};func f(ref a:const i64 in A,ref b:const i64 in B,ref c:const i64 in C)->ref const i64 in C where {owner A,owner B,owner C}{return c;}',2,'','does not satisfy callable declaration')
runner.CASES['partial_contract_does_not_permit_local_escape'] = (DECL+METHOD.replace('return b;', 'val local<-7;return local;'),2,'','owner')
SWAP_KIND = 'proc (<->)(a:i64 in O,b:i64 in O)->void where {owner O};'
SWAP_RESULT = 'func (<->)(a:i64 in O,b:i64 in O)->i64 where {owner O};'
EAGER_KIND = 'proc (+)(ref a:const i64 in O,ref b:const i64 in O)->bool where {owner O};'
runner.CASES['builtin_disjoint_owner_kind'] = (IO+SWAP_KIND+'val outer<-3;{{val inner<-7;outer<->inner;'+OUT('inner')+'}};'+OUT('outer'),0,'3\n7\n','')
runner.CASES['builtin_matching_owner_kind'] = (SWAP_KIND+'val left<-3,right<-7;left<->right;',2,'','implementation kind')
runner.CASES['invalid_swap_result_with_distinct_owners'] = (IO+SWAP_RESULT+'val outer<-3;{{val inner<-7;outer<->inner;'+OUT('inner')+'}};'+OUT('outer'),2,'','swap method must return void')
runner.CASES['invalid_swap_result_with_equal_owners'] = (SWAP_RESULT+'val left<-3,right<-7;left<->right;',2,'','swap method must return void')
runner.CASES['eager_disjoint_owner_kind'] = (IO+EAGER_KIND+'val outer<-3;{{val inner<-7;'+OUT('outer+inner')+'}};',0,'10\n','')
runner.CASES['query_disjoint_owner_kind'] = (IO+EAGER_KIND+'val outer<-3;{{val inner<-7;val result<-make local typeof(outer+inner){9};'+OUT('result')+'}};',0,'9\n','')
runner.CASES['retained_query_disjoint_owner_kind'] = (IO+EAGER_KIND+'func factory(){val outer<-3;return {{val inner<-7;func inspect(){val result<-make local typeof(outer+inner){9};return result;};inspect}};}func selected=factory();'+OUT('selected()'),0,'9\n','')
runner.CASES['deferred_builtin_disjoint_owner_kind'] = (IO+SWAP_KIND+'val outer<-3;{{val inner<-7;{{defer (<->)(outer,inner);}};'+OUT('inner')+'}};'+OUT('outer'),0,'3\n7\n','')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
