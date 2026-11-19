#!/usr/bin/env python3
"""Fresh lexical owners remain known during preparation and result inference."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
OUT = lambda expression: 'behave @print(' + expression + ');'
TAG = ('func tag(ref a:i64 in O,ref b:i64 in O)->bool where {owner O}{return true;}'
       'func tag(ref a:i64 in O,ref b:i64 in P)->i64 where {owner O,owner P}{return 0;}')
PICK = ('func pick(ref a:i64 in O,k:bool)->i64 where {owner O}{a<-a+1;return a;}'
        'func pick(a:i64,k:i64)->i64{return a;}')
CLASSIFY = 'func classify(x:bool)->u64{return 1u64;}func classify(x:i64)->u64{return 2u64;}'
BASE = IO + TAG + PICK
runner.CASES = {
    'scoped_operand_preserves_snapshot': (BASE + 'val slot<-3;'
        + OUT('pick(slot,{{val inner<-4;slot<-9;tag(slot,inner)}})') + OUT('slot'), 0, '3\n9\n', ''),
    'scoped_operand_selects_borrow': (BASE + 'val slot<-3;'
        + OUT('pick(slot,{{val inner<-4;slot<-9;tag(inner,inner)}})') + OUT('slot'), 0, '10\n10\n', ''),
    'group_preserves_current_owner': (BASE + 'val slot<-3;'
        + OUT('pick(slot,{val inner<-4;slot<-9;tag(slot,inner)})') + OUT('slot'), 0, '10\n10\n', ''),
    'nested_scopes_have_distinct_owners': (BASE + 'val slot<-3;'
        + OUT('pick(slot,{{val parent<-4;{{val inner<-5;slot<-9;tag(parent,inner)}}}})') + OUT('slot'), 0, '3\n9\n', ''),
    'alias_retains_existing_owner': (BASE + 'val slot<-3;'
        + OUT('pick(slot,{{ref inner=slot;slot<-9;tag(slot,inner)}})') + OUT('slot'), 0, '10\n10\n', ''),
    'copy_has_fresh_scope_owner': (BASE + 'val slot<-3;'
        + OUT('pick(slot,{{ptr inner<-slot;slot<-9;tag(slot,inner)}})') + OUT('slot'), 0, '3\n9\n', ''),
    'scoped_alias_name_can_end': (BASE + 'val slot<-3;'
        + OUT('pick(slot,{slot<-9;tag(slot,{{ref inner=slot;inner}})})') + OUT('slot'), 0, '10\n10\n', ''),
    'inferred_external_and_local': (IO + TAG + CLASSIFY +
        'func project(ref input:i64 in O) where {owner O}{return {{val inner<-4;tag(input,inner)}};}'
        'val slot<-3;' + OUT('classify(project(slot))'), 0, '2\n', ''),
    'inferred_two_locals': (IO + TAG + CLASSIFY +
        'func project(){return {{val left<-1,right<-2;tag(left,right)}};}' + OUT('classify(project())'), 0, '1\n', ''),
    'inferred_nested_scopes': (IO + TAG + CLASSIFY +
        'func project(){return {{val parent<-1;{{val inner<-2;tag(parent,inner)}}}};}' + OUT('classify(project())'), 0, '2\n', ''),
    'recursive_independent_scoped_anchor': (IO + TAG +
        'func walk(n:u64){if(n==0u64){return {{val left<-1,right<-2;tag(left,right)}};}else{return walk(n-1u64);};}'
        + OUT('walk(40u64)'), 0, 'true\n', ''),
    'mutual_independent_scoped_anchor': (IO + TAG +
        'func left(n:u64){return right(n);}func right(n:u64){if(n==0u64){return {{val a0<-1,b0<-2;tag(a0,b0)}};}else{return left(n-1u64);};}'
        + OUT('left(40u64)'), 0, 'true\n', ''),
    'scoped_callable_binding': (IO + TAG + CLASSIFY +
        'func factory(){return {{func local(){return {{val a0<-1,b0<-2;tag(a0,b0)}};};local}};}'
        'func selected=factory();' + OUT('classify(selected())'), 0, '1\n', ''),
    'scoped_complete_capture': (IO + TAG + CLASSIFY + 'func base(x:i64)->i64{return x;}'
        'func factory(f:func(i64)->i64){return func[f](){return {{val local<-f(1);tag(local,local)}};};}'
        'func selected=factory(base);' + OUT('classify(selected())'), 0, '1\n', ''),
    'retained_query_context_and_fresh_scope': (IO + TAG + CLASSIFY +
        'func factory(ref external:i64 in O)->func()->i64 where {owner O}{val local<-1;'
        'func inspect(){val query<-make local typeof(tag(external,local)){0};return {{val inner<-1;tag(query,inner)}};};return inspect;}'
        'val slot<-3;func selected=factory(slot);' + OUT('classify(selected())'), 0, '2\n', ''),
    'retained_scoped_query_context': (IO + TAG + CLASSIFY +
        'func factory(ref external:i64 in O)->func()->i64 where {owner O}{return {{val local<-1;'
        'func inspect(){val query<-make local typeof(tag(external,local)){0};return {{val inner<-1;tag(query,inner)}};};inspect}};}'
        'val slot<-3;func selected=factory(slot);' + OUT('classify(selected())'), 0, '2\n', ''),
    'independent_formals_remain_unknown': (TAG +
        'func project(ref left:i64 in A,ref right:i64 in B) where {owner A,owner B}{return {{tag(left,right)}};}'
        'val slot<-3;project(slot,slot);', 2, '', 'unproved callable result'),
    'retained_and_new_formal_remain_unknown': (TAG +
        'proc run(ref outer:i64 in O)->void where {owner O}{'
        'func inspect(ref input:i64 in P)->i64 where {owner P}{val query<-make local typeof(tag(outer,input)){0};return query;};'
        'val local<-1;inspect(local);}val slot<-3;run(slot);', 2, '', 'unproved owner equality'),
    'uninitialized_borrow_still_rejects': (BASE + 'val slot<-3;'
        + 'pick(slot,{{val inner<-make local i64;tag(slot,inner)}});', 2, '', 'before definite initialization'),
    'expired_target_still_rejects': (TAG + 'val slot<-3;ref escaped={{val inner<-4;inner}};tag(slot,escaped);', 2, '', 'ended'),
    'declared_result_owner_still_checked': (TAG +
        'func bad(ref input:i64 in O)->ref i64 in O where {owner O}{return {{val inner<-4;inner}};}'
        'val slot<-3;bad(slot);', 2, '', 'owner'),
    'readonly_borrow_still_rejects': (TAG + 'const val slot<-3;{{val inner<-4;tag(slot,inner);{}}};', 2, '', 'readonly argument'),
    'query_declarations_remain_separate': ('val result<-make local typeof({val inner<-1;inner});', 2, '', 'typeof for this expression form'),
}
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
