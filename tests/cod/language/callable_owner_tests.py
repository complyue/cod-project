#!/usr/bin/env python3
"""Callable owner roles use universal symbolic proofs without partition expansion."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
OUT = lambda expression: 'behave @print(' + expression + ');'
COUNT = 12
PARAMS = ','.join(f'ref p{i}:const i64 in O{i}' for i in range(COUNT))
ROLES = ','.join(f'owner O{i}' for i in range(COUNT))
NAMES = ','.join(f'p{i}' for i in range(COUNT))
ARGS = ','.join(['slot'] * COUNT)
MIXED = ','.join(['external', 'inside'] * (COUNT // 2))
FIRST = f'func first({PARAMS})->ref const i64 in O0 where {{{ROLES}}}{{return p0;}}'
SECOND = f'func second({PARAMS})->ref const i64 in O1 where {{{ROLES}}}{{return p1;}}'
VALUE = f'func value({PARAMS})->i64 where {{{ROLES}}}{{return p0;}}'
CHOICE = ('func choose(ref a:const i64 in A,ref b:const i64 in B)->i64 where {owner A,owner B}{return 1;}'
          'func choose(ref a:const i64 in O,ref b:const i64 in O)->i64 where {owner O}{return 2;}')
SAME = 'func prototype(ref a:const i64 in O,ref b:const i64 in O)->i64 where {owner O}{return 0;}'
ANY = 'func prototype(ref a:const i64 in A,ref b:const i64 in B)->i64 where {owner A,owner B}{return 0;}'


def borrowed_selection(definition):
    return ('func choose_result(ref external:const i64 in O)->ref const i64 in O where {owner O}{'
            f'val inside<-8;func chosen={definition};return chosen({MIXED});}}')


runner.CASES = {
    'many_roles_same_actual_owner': (IO + FIRST + 'func selected=first;val slot<-7;' + OUT(f'selected({ARGS})'), 0, '7\n', ''),
    'many_roles_different_actual_owners': (IO + FIRST + 'func selected=first;val outside<-7;ref external=outside;{{val inside<-8;'
        f'ref borrowed=selected({MIXED});external<-9;' + OUT('borrowed') + '}};', 0, '9\n', ''),
    'result_retains_only_its_owner': (IO + FIRST + borrowed_selection('first')
        + 'val outside<-7;ref selected=choose_result(outside);outside<-9;' + OUT('selected'), 0, '9\n', ''),
    'expired_result_rejects': (SECOND + borrowed_selection('second') + 'val outside<-7;choose_result(outside);',
        2, '', 'borrowed result does not retain its declared external owner'),
    'result_access_preserved': (FIRST + 'func selected=first;val slot<-7;' + f'selected({ARGS})<-8;', 2, '', 'readonly destination'),
    'recursive_unknown_relationships': (IO + FIRST + 'func selected=first;'
        'func relay(f:F,ref left:const i64 in A,ref right:const i64 in B,n:u64)->ref const i64 in A where {F,owner A,owner B}{'
        'if(n==0u64){return f(' + ','.join(['left','right'] * (COUNT // 2)) + ');}else{return relay(f,left,right,n-1u64);};}'
        'val outside<-7;ref external=outside;{{val inside<-8;' + OUT('relay(selected,external,inside,40u64)') + OUT('relay(selected,external,external,40u64)') + '}};', 0, '7\n7\n', ''),
    'complete_capture': (IO + FIRST + f'func wrap(f:F) where F{{return func[f]({PARAMS})->ref const i64 in O0 where {{{ROLES}}}{{return f({NAMES});}};}}'
        'func selected=wrap(first);val slot<-7;' + OUT(f'selected({ARGS})'), 0, '7\n', ''),
    'anonymous_family': (IO + f'func selected=func({PARAMS})->ref const i64 in O0 where {{{ROLES}}}{{return p0;}};val slot<-7;'
        + OUT(f'selected({ARGS})'), 0, '7\n', ''),
    'generic_borrowed_targets': (IO + FIRST +
        f'func generic({PARAMS.replace("i64", "T")})->ref const T in O0 where {{T,{ROLES}}}{{return p0;}}'
        'func selected=first;selected<-generic;val slot<-7;' + OUT(f'selected({ARGS})'), 0, '7\n', ''),
    'query_uses_symbolic_proof': (IO + VALUE + 'func apply(f:F)->i64 where F{return 0;}'
        'val result<-make local typeof(apply(value)){9};' + OUT('result'), 0, '9\n', ''),
    'deferred_borrows_read_current_payload': (IO + f'proc show({PARAMS})->void where {{{ROLES}}}{{behave @print(p0);}}'
        'proc selected=show;val slot<-7;{{' + f'defer selected({ARGS});slot<-8;' + '}};', 0, '8\n', ''),
    'symbolic_capture_and_current': (IO + 'symbol @bias:i64;'
        f'proc @selected({PARAMS})->i64 where {{{ROLES}}}{{return p0+@bias;}}'
        'effect @bias<-1;effect @selected;effect @bias<-2;val slot<-7;'
        + OUT(f'perform @selected({ARGS})') + OUT(f'behave @selected({ARGS})'), 0, '8\n9\n', ''),
    'view_equality_selects_method': (IO + CHOICE + SAME + 'func selected=prototype;selected<-choose;val slot<-7;'
        + OUT('selected(slot,slot)'), 0, '2\n', ''),
    'view_equality_rejects_distinct_actuals': (CHOICE + SAME + 'func selected=prototype;selected<-choose;val outside<-7;ref external=outside;'
        '{{val inside<-8;selected(external,inside);}};', 2, '', 'different actual owners'),
    'independent_roles_remain_unknown': (CHOICE + ANY + 'func selected=prototype;selected<-choose;', 2, '', 'unproved owner equality'),
    'observed_equal_call_does_not_narrow_view': (CHOICE + ANY + 'func selected=prototype;selected<-choose;val slot<-7;selected(slot,slot);', 2, '', 'unproved owner equality'),
    'required_equality_is_not_assumed': (SAME + ANY.replace('prototype', 'general') + 'func selected=general;selected<-prototype;', 2, '', 'unproved owner equality'),
    'renamed_roles_are_equivalent': (IO + FIRST + FIRST.replace('first','renamed').replace(' O',' R') +
        'func selected=first;selected<-renamed;val slot<-7;' + OUT(f'selected({ARGS})'), 0, '7\n', ''),
    'selected_body_checks_owners': (FIRST.replace('return p0;', 'return p1;') + 'func selected=first;', 2, '', 'owner'),
    'body_cannot_assume_independent_roles_equal': (
        'func require_same(ref a:const i64 in O,ref b:const i64 in O)->i64 where {owner O}{return a;}'
        + VALUE.replace('return p0;', 'return require_same(p0,p1);') + 'func selected=value;', 2, '', 'unproved owner equality'),
}
grouped_parameters = ','.join(f'ref p{i}:const i64 in O{i // 2}' for i in range(COUNT))
grouped_roles = ','.join(f'owner O{i}' for i in range(COUNT // 2))
grouped = f'func first({grouped_parameters})->ref const i64 in O0 where {{{grouped_roles}}}{{return p0;}}func selected=first;'
grouped_arguments = ','.join(['external','external','inside','inside'] * (COUNT // 4))
runner.CASES['repeated_roles_propagate_equalities'] = (IO + grouped + 'val outside<-7;ref external=outside;{{val inside<-8;'
    + OUT(f'selected({grouped_arguments})') + '}};', 0, '7\n', '')
runner.CASES['repeated_roles_reject_mismatch'] = (grouped + 'val outside<-7;ref external=outside;{{val inside<-8;'
    + f'selected({MIXED});' + '}};', 2, '', 'different actual owners')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
