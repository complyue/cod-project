#!/usr/bin/env python3
"""Family input domains preserve concrete exceptions, complete values and borrows."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
ERROR = IO + NPE + 'using "std:exceptions" SystemException;using "std:exceptions" MissingReturnException;'
ERROR += 'func missing()->u64{}func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}'
ERROR += 'func null_error()->const NullPointerException{val x<-1;ptr p=x;p=null;try{p;{}}catch(e:const NullPointerException){return e;};}'
F = 'symbol @f:proc(SystemException)->u64;proc @f(e:MissingReturnException)->u64{return 7u64;}proc @f(e:T)->u64 where T{return e.category;}'
runner.CASES = {
    'common_exception_actual_identity': (ERROR + F + 'effect @f;val e:SystemException<-error();behave @print(behave @f(e));e<-null_error();behave @print(behave @f(e));', 0, '7\n4\n', ''),
    'exact_exception_input': (ERROR + 'proc @f(e:MissingReturnException)->u64{return e.category;}effect @f;behave @print(behave @f(error()));', 0, '5\n', ''),
    'common_method_parameter_preparation': (ERROR + 'proc @f(e:SystemException)->u64{return e.category;}effect @f;val e:SystemException<-error();behave @print(behave @f(e));behave @print(behave @f(null_error()));', 0, '5\n4\n', ''),
    'common_exception_through_generic_caller': (ERROR + F + 'proc relay(e:T)->u64 where T{return behave @f(e);}effect @f;val e:SystemException<-error();behave @print(relay(e));behave @print(relay(null_error()));', 0, '7\n4\n', ''),
    'common_exception_through_holder': (ERROR + F + 'effect @f;proc p=@f;val e:SystemException<-error();behave @print(p(e));e<-null_error();behave @print(p(e));', 0, '7\n4\n', ''),
    'common_exception_joint_identity': (ERROR + 'symbol @f:proc(SystemException,SystemException)->u64;proc @f(a:T,b:T)->u64 where T{return 1u64;}proc @f(a:A,b:B)->u64 where {A,B}{return 7u64;}'
        'effect @f;val a:SystemException<-error(),b:SystemException<-null_error();behave @print(behave @f(a,b));b<-a;behave @print(behave @f(a,b));', 0, '7\n1\n', ''),
    'common_exception_deferred_snapshot': (ERROR + 'symbol @f:proc(SystemException)->void;proc @f(e:MissingReturnException)->void{behave @print(7u64);}proc @f(e:T)->void where T{behave @print(e.category);}'
        'effect @f;val e:SystemException<-error();defer behave @f(e);e<-null_error();defer{behave @f(e);};', 0, '4\n7\n', ''),
    'common_exception_complete_capture': (ERROR + 'symbol @x:u64,@y:u64,@f:proc(SystemException)->u64;proc @f(e:MissingReturnException)->u64{return @x;}proc @f(e:T)->u64 where T{return @y;}'
        'effect @x<-1u64;effect @y<-7u64;effect @f;effect @x<-2u64;effect @y<-8u64;val e:SystemException<-null_error();behave @print(perform @f(e));e<-error();behave @print(perform @f(e));behave @print(behave @f(e));', 0, '7\n1\n2\n', ''),
    'common_exception_missing_method': (ERROR + 'symbol @f:proc(SystemException)->u64;proc @f(e:MissingReturnException)->u64{return 1u64;}effect @f;', 2, '', 'no applicable method'),
    'common_exception_borrow_is_unproved': (ERROR + 'proc @f(ref e:const SystemException in O)->u64 where {owner O}{return e.category;}effect @f;', 2, '', 'unproved symbolic family borrowing'),
    'borrowed_exception_adaptation_is_unproved': (ERROR + 'proc @narrow(ref e:const MissingReturnException in O)->u64 where {owner O}{return e.category;}proc @broad(ref e:const SystemException in O)->u64 where {owner O}{return e.category;}effect @narrow=broad;', 2, '', 'borrowed target adaptation'),
    'callable_value_parameter': (IO + 'proc @apply(f:func()->u64)->u64{return f();}func one()->u64{return 1u64;}func two()->u64{return 7u64;}'
        'effect @apply;behave @print(behave @apply(one));behave @print(behave @apply(two));', 0, '1\n7\n', ''),
    'captured_callable_parameter': (IO + 'proc @apply(f:func()->u64)->u64{return f();}func one()->u64{return 7u64;}func wrapped(f:func()->u64)->func()->u64{return func[f]()->u64{return f()+1u64;};}'
        'effect @apply;func held=wrapped(one);behave @print(perform @apply(held));', 0, '8\n', ''),
    'callback_dynamic_projection': (IO + 'symbol @x:u64;proc read()->u64{return @x;}proc @apply(f:proc()->u64)->u64{return f();}'
        'effect @x<-1u64;effect @apply;effect @x<-7u64;proc held=read;behave @print(perform @apply(held));behave @print(behave @apply(held));behave @print(@x);', 0, '1\n7\n7\n', ''),
    'unused_callback_requires_capture': ('symbol @x:u64;proc read()->u64{return @x;}proc @apply(f:proc()->u64)->u64{return f();}'
        'effect @apply;effect @x<-7u64;proc held=read;', 2, '', 'cannot prove an installed binding'),
    'borrowed_return_alias': (IO + 'proc @id(ref x:i64 in O)->ref i64 in O where {owner O}{return x;}effect @id;val x<-1;behave @id(x)<-7;behave @print(x);', 0, '7\n', ''),
    'borrowed_readonly_return': (IO + 'proc @id(ref x:const i64 in O)->ref const i64 in O where {owner O}{return x;}effect @id;val x<-1;ref y=perform @id(x);x<-7;behave @print(y);', 0, '7\n', ''),
    'borrowed_readonly_argument_rejected': ('proc @id(ref x:i64 in O)->ref i64 in O where {owner O}{return x;}effect @id;const val x<-1;behave @id(x);', 2, '', 'readonly argument'),
    'borrowed_readonly_result_rejected': ('proc @id(ref x:const i64 in O)->ref const i64 in O where {owner O}{return x;}effect @id;val x<-1;behave @id(x)<-7;', 2, '', 'readonly destination'),
    'borrowed_owner_mismatch': ('proc @pick(ref x:i64 in O,ref y:i64 in O)->ref i64 in O where {owner O}{return x;}effect @pick;val x<-1;{{val y<-2;behave @pick(x,y);}};', 2, '', 'different actual owners'),
    'independent_owner_roles': (IO + 'proc @pick(ref x:i64 in A,ref y:i64 in B)->ref i64 in B where {owner A,owner B}{return y;}effect @pick;val x<-1;behave @pick(x,x)<-3;{{val y<-2;behave @pick(x,y)<-7;behave @print(y);}};behave @print(x);', 0, '7\n3\n', ''),
    'owner_dependent_selection_unproved': ('proc @pick(ref x:i64 in A,ref y:i64 in B)->u64 where {owner A,owner B}{return 1u64;}proc @pick(ref x:i64 in O,ref y:i64 in O)->u64 where {owner O}{return 7u64;}effect @pick;', 2, '', 'unproved symbolic family'),
    'borrowed_nullable_return': (IO + 'proc @id(ptr x:i64 in O)->ptr i64 in O where {owner O}{return x;}effect @id;ptr p:i64=null in here;behave @print(isnull(behave @id(p)));', 0, 'true\n', ''),
    'borrowed_writer_initializes': (IO + NPE + 'proc @set(ptr x:i64 in O)->void throws NullPointerException where {owner O}{x<-7;}effect @set;val x<-make local i64;try{behave @set(x);behave @print(x);}catch(e:const NullPointerException){};', 0, '7\n', ''),
    'borrowed_reader_requires_initialization': ('proc @read(ref x:i64 in O)->i64 where {owner O}{return x;}effect @read;val x<-make local i64;behave @read(x);', 2, '', 'before definite initialization'),
    'borrowed_address_before_argument_effect': (IO + NPE + 'proc @read(ptr x:i64 in O,unused:i64)->i64 throws NullPointerException where {owner O}{return x;}effect @read;val x<-1,y<-7;ptr p=x;try{behave @print(behave @read(p,{p=y;0}));}catch(e:const NullPointerException){};', 0, '1\n', ''),
    'borrowed_deferred_late_payload': (IO + 'symbol @bias:i64;proc @show(ref x:i64 in O)->void where {owner O}{behave @print(x+@bias);}effect @bias<-1;effect @show;val x<-2;defer perform @show(x);x<-7;effect @bias<-9;', 0, '8\n', ''),
    'borrowed_callable_holder_write': (IO + 'func one()->u64{return 1u64;}func two()->u64{return 7u64;}proc @replace(ref f:func()->u64 in O)->void where {owner O}{f<-two;}'
        'effect @replace;func held=one;behave @replace(held);behave @print(held());', 0, '7\n', ''),
}
for name, (source, status, output, diagnostic) in list(runner.CASES.items()):
    if ERROR in source and status == 0:
        runner.CASES[name] = (source, status, output, '[-Wmissing-return]')
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
