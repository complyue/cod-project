#!/usr/bin/env python3
"""Caught-value owners supply result and operand evidence without escaping."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
TYPES = 'using "std:exceptions" MissingReturnError;using "std:exceptions" Exception;'
ERROR = ('func missing()->i64{}func error()->const MissingReturnError{'
         'try{missing():void;}catch(created:const MissingReturnError){return created;};}')
TAG = ('func tag(ref a:const A in O,ref b:const B in O)->bool where {A,B,owner O}{return true;}'
       'func tag(ref a:const A in O,ref b:const B in P)->i64 where {A,B,owner O,owner P}{return 0;}')
CODE = 'func code(value:bool)->u64{return 1u64;}func code(value:i64)->u64{return 2u64;}'
PICK = ('func pick(ref value:i64 in O,kind:bool)->i64 where {owner O}{value<-value+1;return value;}'
        'func pick(value:i64,kind:i64)->i64{return value;}')
OUT = lambda expression: 'behave @print(' + expression + ');'
BASE = IO + TYPES + ERROR + TAG + CODE
WARN = '[-Wmissing-return]'
runner.CASES = {
    'typed_catch_result': (BASE + 'func project(){try{throw error();}catch(e:const MissingReturnError){return tag(e,e);};}'
        + OUT('code(project())'), 0, '1\n', WARN),
    'common_catch_result': (BASE + 'func project(){try{throw error();}catch(e){return tag(e,e);};}'
        + OUT('code(project())'), 0, '1\n', WARN),
    'procedure_catch_result': (BASE + 'proc project(){try{throw error();}catch(e){return tag(e,e);};}'
        + OUT('code(project())'), 0, '1\n', WARN),
    'handler_expression_result': (BASE + 'func project(){return try{throw error();}catch(e){tag(e,e)};}'
        + OUT('code(project())'), 0, '1\n', WARN),
    'external_and_caught_owners_differ': (BASE + 'func project(ref input:const MissingReturnError in O) where {owner O}{'
        'try{throw input;}catch(e:const MissingReturnError){return tag(input,e);};}'
        'val saved<-error();' + OUT('code(project(saved))'), 0, '2\n', WARN),
    'nested_catches_have_distinct_owners': (BASE + 'func project(){try{throw error();}catch(outer){'
        'try{throw outer;}catch(inner){return tag(outer,inner);};};}' + OUT('code(project())'), 0, '2\n', WARN),
    'heterogeneous_catches_have_distinct_owners': (BASE + 'func project(){try{throw error();}catch(outer:const MissingReturnError){'
        'try{throw outer;}catch(inner){return tag(outer,inner);};};}' + OUT('code(project())'), 0, '2\n', WARN),
    'alias_retains_caught_owner': (BASE + 'func project(){try{throw error();}catch(e){{ref selected=e;return tag(e,selected);}};}'
        + OUT('code(project())'), 0, '1\n', WARN),
    'copied_exception_has_local_owner': (BASE + 'func project(){try{throw error();}catch(e){{val selected<-e;return tag(e,selected);}};}'
        + OUT('code(project())'), 0, '2\n', WARN),
    'recursive_handler_anchor': (BASE + 'func project(n:u64){if(n==0u64){try{throw error();}catch(e){return tag(e,e);};}'
        'else{return project(n-1u64);};}' + OUT('code(project(40u64))'), 0, '1\n', WARN),
    'mutual_handler_anchor': (BASE + 'func left(n:u64){return right(n);}func right(n:u64){'
        'if(n==0u64){try{throw error();}catch(e){return tag(e,e);};}else{return left(n-1u64);};}'
        + OUT('code(left(40u64))'), 0, '1\n', WARN),
    'generic_typed_handler': (BASE + 'func project(input:T) where T{try{throw input;}catch(e:const T){return tag(e,e);};}'
        + OUT('code(project(error()))'), 0, '1\n', WARN),
    'returned_query_context': (BASE + 'func factory(){try{throw error();}catch(e){return {{'
        'func inspect(){val query<-make local typeof(tag(e,e)){true};return query;};inspect}};};}'
        'func selected=factory();' + OUT('code(selected())'), 0, '1\n', WARN),
    'returned_nested_query_context': (BASE + 'func factory(){try{throw error();}catch(outer){try{throw outer;}catch(inner){return {{'
        'func inspect(){val query<-make local typeof(tag(outer,inner)){0};return query;};inspect}};};};}'
        'func selected=factory();' + OUT('code(selected())'), 0, '2\n', WARN),
    'deferred_handler_result': (BASE + 'func project(){defer catch(e:const MissingReturnError){return tag(e,e);};throw error();}'
        + OUT('code(project())'), 0, '1\n', WARN),
    'finally_replaces_handler_result': (BASE + 'func project(){defer catch(e){return tag(e,e);}finally{return false;};throw error();}'
        + OUT('project()'), 0, 'false\n', WARN),
    'delayed_caught_snapshot_result': (BASE + 'func project(){try{throw error();}catch(e){defer{return tag(e,e);};};}'
        + OUT('code(project())'), 0, '1\n', WARN),
    'later_handler_selects_borrow': (BASE + PICK + 'val slot<-3;'
        + OUT('pick(slot,try{throw error();}catch(e){slot<-9;tag(e,e)})') + OUT('slot'), 0, '10\n10\n', WARN),
    'later_handler_preserves_snapshot': (BASE + PICK + 'val slot<-3,saved<-error();'
        + OUT('pick(slot,try{throw saved;}catch(e){slot<-9;tag(saved,e)})') + OUT('slot'), 0, '3\n9\n', WARN),
    'caught_value_supplies_generic_argument': (BASE +
        'func identity(value:T)->const T where T{return value;}'
        'func apply(f:func(T)->const T,value:T)->const T where T{return f(value);}'
        'val saved<-apply(identity,try{throw error();}catch(e:const MissingReturnError){e});'
        + OUT('saved.category'), 0, '5\n', WARN),
    'shadowed_catch_restores_outer_binding': (BASE + 'func project(){try{throw error();}catch(e){'
        'try{throw e;}catch(e){tag(e,e);};return tag(e,e);};}' + OUT('code(project())'), 0, '1\n', WARN),
    'shadowed_owner_name_is_restored': (BASE + 'func project(ref input:const MissingReturnError in O) where {owner O}{'
        'try{throw input;}catch(O:const MissingReturnError){tag(O,O);};'
        'return {{ref selected:const MissingReturnError in O=input;tag(input,selected)}};}'
        'val saved<-error();' + OUT('code(project(saved))'), 0, '1\n', WARN),
    'caught_value_stays_readonly': (BASE + 'try{throw error();}catch(e){e<-error();};', 2, '', 'readonly'),
    'caught_borrow_cannot_escape': (BASE + 'func project(ref input:const MissingReturnError in O)->ref const MissingReturnError in O where {owner O}{'
        'try{throw input;}catch(e:const MissingReturnError){return e;};}val saved<-error();project(saved);', 2, '', 'owner'),
    'caught_query_does_not_initialize': (BASE + 'func project(){try{throw error();}catch(e){{'
        'val unread<-make local typeof(tag(e,e));return unread;}};}project();', 2, '', 'before definite initialization'),
    'external_equality_stays_unproved': (BASE + 'func project(ref left:const MissingReturnError in A,ref right:const MissingReturnError in B) where {owner A,owner B}{'
        'try{throw left;}catch(e){return tag(left,right);};}val saved<-error();project(saved,saved);', 2, '', 'unproved callable result'),
    'handler_query_surface_stays_bounded': (BASE + 'val value<-make local typeof(try{throw error();}catch(e){e});', 2, '', 'typeof for this expression form'),
}
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {
    'shadowed_catch_restores_outer_binding': ['e'],
    'shadowed_owner_name_is_restored': ['O'],
}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
