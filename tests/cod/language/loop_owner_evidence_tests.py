#!/usr/bin/env python3
"""Loop-control owners supply independent evidence without extending lifetime."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
OUT = lambda expression: 'behave @print(' + expression + ');'
TAG = ('func tag(ref a:const T in O,ref b:const T in O)->bool where {T,owner O}{return true;}'
       'func tag(ref a:const T in O,ref b:const T in P)->i64 where {T,owner O,owner P}{return 0;}')
CODE = 'func code(value:bool)->u64{return 1u64;}func code(value:i64)->u64{return 2u64;}'
BASE = IO + TAG + CODE
runner.CASES = {
    'index_supplies_result': (BASE + 'func project(){for(i in 0..1){return tag(i,i);};}'
        + OUT('code(project())'), 0, '1\n', ''),
    'procedure_index_result': (BASE + 'proc project(){for(i in 0..1){return tag(i,i);};}'
        + OUT('code(project())'), 0, '1\n', ''),
    'external_owner_differs': (BASE + 'func project(ref input:const i64 in O) where {owner O}{'
        'for(i in 0..1){return tag(input,i);};}val slot<-4;' + OUT('code(project(slot))'), 0, '2\n', ''),
    'nested_controls_differ': (BASE + 'func project(){for(i in 0..1){for(j in 0..1){return tag(i,j);};};}'
        + OUT('code(project())'), 0, '2\n', ''),
    'full_scope_local_differs': (BASE + 'func project(){for(i in 0..1){{val local<-1;return tag(i,local);}};}'
        + OUT('code(project())'), 0, '2\n', ''),
    'alias_preserves_control_owner': (BASE + 'func project(){for(i in 0..1){{ref selected=i;return tag(i,selected);}};}'
        + OUT('code(project())'), 0, '1\n', ''),
    'copied_control_has_local_owner': (BASE + 'func project(){for(i in 0..1){{val selected<-i;return tag(i,selected);}};}'
        + OUT('code(project())'), 0, '2\n', ''),
    'fold_controls_share_owner': (BASE + 'func project(){fold(i in 0..1,sum=0){return tag(i,sum);};}'
        + OUT('code(project())'), 0, '1\n', ''),
    'heterogeneous_fold_controls_share_owner': (IO + CODE +
        'func inspect(ref a:const A in O,ref b:const B in O)->bool where {A,B,owner O}{return true;}'
        'func inspect(ref a:const A in O,ref b:const B in P)->i64 where {A,B,owner O,owner P}{return 0;}'
        'func base(value:u64)->u64{return value;}'
        'func project(){fold(i in 0..1,callback=base){return inspect(i,callback);};}'
        + OUT('code(project())'), 0, '1\n', ''),
    'nested_fold_controls_differ': (BASE + 'func project(){fold(i in 0..1,sum=0){'
        'fold(j in 0..1,total=0){return tag(sum,total);};sum};}' + OUT('code(project())'), 0, '2\n', ''),
    'generic_index_types': (BASE + 'func project(start:T,end:T) where T{for(i in start..end){return tag(i,i);};}'
        + OUT('code(project(0,1))') + OUT('code(project(0u64,1u64))'), 0, '1\n1\n', '[-Wmissing-return]'),
    'empty_range_keeps_missing_return': (BASE + 'using "std:exceptions" MissingReturnException;'
        'func project(end:i64){for(i in 0..end){return tag(i,i);};}'
        'try{project(0);{}}catch(e:const MissingReturnException){behave @print(7);};', 0, '7\n', '[-Wmissing-return]'),
    'recursive_control_anchor': (BASE + 'func project(n:u64){if(n==0u64){for(i in 0..1){return tag(i,i);};}'
        'else{return project(n-1u64);};}' + OUT('code(project(40u64))'), 0, '1\n', ''),
    'mutual_control_anchor': (BASE + 'func left(n:u64){return right(n);}func right(n:u64){'
        'if(n==0u64){fold(i in 0..1,sum=0){return tag(i,sum);};}else{return left(n-1u64);};}'
        + OUT('code(left(40u64))'), 0, '1\n', ''),
    'anonymous_control_anchor': (BASE + 'func selected=func(){for(i in 0..1){return tag(i,i);};};'
        + OUT('code(selected())'), 0, '1\n', ''),
    'retained_control_query': (BASE + 'func factory(){for(i in 0..1){return {{'
        'func inspect(){val value<-make local typeof(tag(i,i)){true};return value;};inspect}};};}'
        'func selected=factory();' + OUT('code(selected())'), 0, '1\n', ''),
    'retained_nested_control_query': (BASE + 'func factory(){for(i in 0..1){for(j in 0..1){return {{'
        'func inspect(){val value<-make local typeof(tag(i,j)){0};return value;};inspect}};};};}'
        'func selected=factory();' + OUT('code(selected())'), 0, '2\n', ''),
    'selected_borrow_preserves_owner': (BASE +
        'func first(ref a:const i64 in A,ref b:const i64 in B)->ref const i64 in A where {owner A,owner B}{return a;}'
        'func project(){for(i in 0..1){for(j in 0..1){return tag(i,first(i,j));};};}'
        + OUT('code(project())'), 0, '1\n', ''),
    'deferred_inferred_value_snapshot': (BASE + 'func project(){for(i in 0..1){return tag(i,i);};}'
        'proc show(value:bool)->void{behave @print(value);}{{defer show(project());}};', 0, 'true\n', ''),
    'here_remains_full_scope': (IO + 'val outer<-1;ptr p:i64=null in here;'
        'for(i in 0..2){p=outer;p=null in here;behave @print(isnull(p));};'
        'for(j in 0..0){}else{p=outer;p=null in here;behave @print(isnull(p));};', 0, 'true\ntrue\ntrue\n', ''),
    'control_storage_is_readonly': (TAG + 'for(i in 0..1){i<-2;};', 2, '', 'readonly'),
    'controls_do_not_escape': ('func bad(ref input:const i64 in O)->ref const i64 in O where {owner O}{'
        'for(i in 0..1){return i;};}val slot<-1;bad(slot);', 2, '', 'owner'),
    'outer_defer_cannot_borrow_control': ('proc save(ref input:const i64 in O)->void where {owner O}{}'
        'for(i in 0..1){defer save(i);};', 2, '', 'outlive its cleanup scope'),
    'delayed_control_capture_rejects': (IO + 'for(i in 0..1){defer{behave @print(i);};};', 2, '', 'loop-control'),
    'body_declaration_rejects': ('for(i in 0..1){val local<-i;};', 2, '', 'nested per-iteration'),
    'else_declaration_rejects': ('for(i in 0..0){}else{val local<-1;};', 2, '', 'nested per-iteration'),
    'unproved_external_equality_stays_unproved': (TAG + 'func project(ref left:const i64 in A,ref right:const i64 in B) where {owner A,owner B}{'
        'for(i in 0..1){return tag(left,right);};}val slot<-1;project(slot,slot);', 2, '', 'unproved callable result'),
    'control_query_does_not_initialize': (TAG + 'func project(){for(i in 0..1){{'
        'val unread<-make local typeof(tag(i,i));return unread;}};}project();', 2, '', 'before definite initialization'),
    'loop_query_surface_stays_bounded': ('val value<-make local typeof(fold(i in 0..1,sum=0){sum+i});', 2, '', 'typeof for this expression form'),
}
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
