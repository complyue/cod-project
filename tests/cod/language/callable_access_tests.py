#!/usr/bin/env python3
"""Callable views constrain target access while retaining actual input identities."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
OUT = lambda expression: 'behave @print(' + expression + ');'
VALUES = 'func choose(x:i64)->i64{return 1;}func choose(x:const i64)->i64{return 2;}'
VIEWS = 'func writable:func(i64)->i64=choose;func readonly:func(const i64)->i64=choose;'
BORROWS = ('func choose(ref x:i64 in O)->i64 where {owner O}{x<-x+1;return 1;}'
           'func choose(ref x:const i64 in O)->i64 where {owner O}{return 2;}')
READ = 'func prototype(ref x:const i64 in O)->i64 where {owner O}{return x;}func selected=prototype;selected<-choose;'
WRITE = 'func prototype(ref x:i64 in O)->i64 where {owner O}{return x;}func selected=prototype;selected<-choose;'
ACTUAL = ('func choose(x:u64)->u64{return 64u64;}func choose(x:const u64)->u64{return 164u64;}'
          'func choose(x:T)->u64 where T{return 1u64;}func choose(x:const T)->u64 where T{return 101u64;}')
runner.CASES = {
    'view_selects_access': (IO + VALUES + VIEWS + OUT('writable(7)') + OUT('readonly(7)'), 0, '1\n2\n', ''),
    'direct_and_view_boundaries': (IO + VALUES + VIEWS + 'const val slot<-7;'
        + OUT('choose(slot)') + OUT('writable(slot)') + OUT('readonly(slot)') + OUT('choose(7)'), 0, '2\n1\n2\n1\n', ''),
    'private_copy_from_readonly_source': (IO + 'func choose(x:i64)->i64{x<-x+1;return x;}func choose(x:const i64)->i64{return x;}'
        + VIEWS + 'const val slot<-7;' + OUT('writable(slot)') + OUT('readonly(slot)') + OUT('slot'), 0, '8\n7\n7\n', ''),
    'readonly_borrow_from_writable': (IO + BORROWS + READ + 'val slot<-7;' + OUT('selected(slot)') + OUT('slot'), 0, '2\n7\n', ''),
    'readonly_borrow_from_readonly': (IO + BORROWS + READ + 'const val slot<-7;' + OUT('selected(slot)'), 0, '2\n', ''),
    'writable_borrow_from_writable': (IO + BORROWS + WRITE + 'val slot<-7;' + OUT('selected(slot)') + OUT('slot'), 0, '1\n8\n', ''),
    'writable_borrow_rejects_readonly': (BORROWS + WRITE + 'const val slot<-7;selected(slot);', 2, '', 'readonly argument'),
    'borrowed_result_retains_owner': (IO +
        'func choose(ref x:i64 in O)->ref i64 in O where {owner O}{x<-99;return x;}'
        'func choose(ref x:const i64 in O)->ref const i64 in O where {owner O}{return x;}'
        'func prototype(ref x:const i64 in O)->ref const i64 in O where {owner O}{return x;}'
        'func selected=prototype;selected<-choose;val slot<-7;ref alias=selected(slot);slot<-8;' + OUT('alias'), 0, '8\n', ''),
    'borrowed_result_remains_readonly': (
        'func choose(ref x:const i64 in O)->ref const i64 in O where {owner O}{return x;}'
        'func selected=choose;val slot<-7;selected(slot)<-8;', 2, '', 'readonly destination'),
    'nullable_readonly_argument': (IO + 'using "std:exceptions" NullPointerException;'
        'func choose(ptr x:i64 in O)->i64 where {owner O}{return 1;}'
        'func choose(ptr x:const i64 in O)->i64 where {owner O}{return if((x is null)){2}else{3};}'
        'func prototype(ptr x:const i64 in O)->i64 where {owner O}{return 0;}'
        'func selected=prototype;selected<-choose;ptr slot:i64=null in here;' + OUT('selected(slot)'), 0, '2\n', ''),
    'unselected_writer_does_not_initialize': ('using "std:exceptions" NullPointerException;'
        'func choose(ptr x:i64 in O)->void throws NullPointerException where {owner O}{x<-7:void;}'
        'func choose(ptr x:const i64 in O)->void throws NullPointerException where {owner O}{}'
        'func prototype(ptr x:const i64 in O)->void throws NullPointerException where {owner O}{}'
        'func selected=prototype;selected<-choose;val slot<-make local i64;'
        'try{selected(slot);}catch(e:const NullPointerException){};slot;', 2, '', 'before definite initialization'),
    'selected_reader_requires_initialization': ('using "std:exceptions" NullPointerException;'
        'func choose(ptr x:i64 in O)->void throws NullPointerException where {owner O}{x<-7:void;}'
        'func choose(ptr x:const i64 in O)->void throws NullPointerException where {owner O}{x;{}}'
        'func prototype(ptr x:const i64 in O)->void throws NullPointerException where {owner O}{}'
        'func selected=prototype;selected<-choose;val slot<-make local i64;'
        'try{selected(slot);}catch(e:const NullPointerException){};', 2, '', 'cannot prove initialization'),
    'actual_types_survive_access': (IO + ACTUAL +
        'func writable:func(u64)->u64=choose;func readonly:func(const u64)->u64=choose;'
        + OUT('writable(1u8)') + OUT('writable(1u64)') + OUT('readonly(1u8)') + OUT('readonly(1u64)'), 0, '1\n64\n101\n164\n', ''),
    'joint_access_contract': (IO +
        'func choose(x:i64,y:const i64)->i64{return 1;}func choose(x:const i64,y:i64)->i64{return 2;}'
        'func left:func(i64,const i64)->i64=choose;func right:func(const i64,i64)->i64=choose;'
        + OUT('left(7,8)') + OUT('right(7,8)'), 0, '1\n2\n', ''),
    'contextual_family': (IO + VALUES + 'func apply(f:func(const i64)->i64)->i64{return f(7);}' + OUT('apply(choose)'), 0, '2\n', ''),
    'returned_family': (IO + VALUES + 'func factory()->func(const i64)->i64{return choose;}' + OUT('factory()(7)'), 0, '2\n', ''),
    'captured_family': (IO + VALUES +
        'func wrap(f:func(const i64)->i64)->func(const i64)->i64{return func[f](n:const i64)->i64{return f(n);};}'
        'func selected=wrap(choose);' + OUT('selected(7)'), 0, '2\n', ''),
    'recursive_forwarding': (IO + VALUES +
        'func relay(f:func(const i64)->i64,n:u64)->i64{if(n==0u64){return f(7);}else{return relay(f,n-1u64);};}'
        + OUT('relay(choose,30u64)'), 0, '2\n', ''),
    'deferred_value_snapshot': (IO +
        'proc choose(x:i64)->void{behave @print(1);return;}proc choose(x:const i64)->void{behave @print(x);return;}'
        'proc selected:proc(const i64)->void=choose;val slot<-7;{{defer selected(slot);slot<-8:void;}};', 0, '7\n', ''),
    'deferred_borrow_reads_late': (IO +
        'proc choose(ref x:i64 in O)->void where {owner O}{behave @print(1);return;}'
        'proc choose(ref x:const i64 in O)->void where {owner O}{behave @print(x);return;}'
        'proc prototype(ref x:const i64 in O)->void where {owner O}{behave @print(0);return;}'
        'proc selected=prototype;selected<-choose;val slot<-7;{{defer selected(slot);slot<-8:void;}};', 0, '8\n', ''),
    'query_uses_view_access': (IO + VALUES + 'func apply(f:func(const i64)->i64)->i64{return f(7);}'
        'val slot<-make local typeof(apply(choose)){9};' + OUT('slot'), 0, '9\n', ''),
    'symbolic_declared_access': (IO + 'symbol selected:proc(const i64)->i64;'
        'proc @selected(x:i64)->i64{return 1;}proc @selected(x:const i64)->i64{return 2;}effect @selected;'
        + OUT('perform @selected(7)') + OUT('behave @selected(7)'), 0, '2\n2\n', ''),
    'symbolic_captures_only_selected_access': (IO + 'symbol selected:proc(const i64)->u64;symbol @saved:u64,@unused:u64;'
        'proc @selected(x:i64)->u64{return @unused;}proc @selected(x:const i64)->u64{return @saved;}'
        'effect @saved<-7u64;effect @selected;effect @saved<-8u64;'
        + OUT('perform @selected(7)') + OUT('behave @selected(7)'), 0, '7\n8\n', ''),
    'selected_generic_body_checks_readonly': (
        'func choose(x:T)->u64 where T{return 1u64;}func choose(x:const T)->u64 where T{x<-x;return 2u64;}'
        'func selected:func(const u64)->u64=choose;', 2, '', 'readonly destination'),
    'selected_generic_throws_remain': ('using "std:exceptions" NullPointerException;'
        'func choose(x:T)->u64 where T{return 1u64;}func choose(x:const T)->u64 throws NullPointerException where T{return 2u64;}'
        'func selected:func(const u64)->u64=choose;', 2, '', 'required checked exception'),
    'unselected_access_throws_not_required': (IO + 'using "std:exceptions" NullPointerException;'
        'func choose(x:T)->u64 throws NullPointerException where T{return 1u64;}func choose(x:const T)->u64 where T{return 2u64;}'
        'func selected:func(const u64)->u64=choose;' + OUT('selected(1u64)'), 0, '2\n', ''),
    'unused_actual_input_still_checked': (
        'func choose(x:const u64)->u64{return 1u64;}func choose(x:const T)->u64 where T{return !x;}'
        'func selected:func(const u64)->u64=choose;selected(1u64);', 2, '', 'logical negation requires bool'),
    'owner_dependent_selection_still_unproved': (
        'func choose(ref x:const i64 in A,ref y:const i64 in B)->i64 where {owner A,owner B}{return 1;}'
        'func choose(ref x:const i64 in O,ref y:const i64 in O)->i64 where {owner O}{return 2;}'
        'func prototype(ref x:const i64 in A,ref y:const i64 in B)->i64 where {owner A,owner B}{return 0;}'
        'func selected=prototype;selected<-choose;', 2, '', 'unproved owner equality'),
}
parameters = ','.join(f'a{i}:const bool' for i in range(16))
view = ','.join(['const bool'] * 16)
arguments = ','.join(['true'] * 16)
runner.CASES['independent_parameter_access'] = (IO + f'func first({parameters})->bool{{return a0;}}'
    f'func selected:func({view})->bool=first;' + OUT(f'selected({arguments})'), 0, 'true\n', '')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
