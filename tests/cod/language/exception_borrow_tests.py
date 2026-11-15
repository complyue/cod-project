#!/usr/bin/env python3
"""Common exception payloads preserve exact borrowed storage contracts."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
TYPES = 'using "std:exceptions" SystemException;using "std:exceptions" MissingReturnException;using "std:exceptions" NullPointerException;'
ERROR = TYPES+'func missing()->u64{}func missing_value()->const MissingReturnException{try{missing();}catch(caught:const MissingReturnException){return caught;};}'
ERROR += 'func null_value()->const NullPointerException{ptr absent:u64=null in here;try{absent;{}}catch(caught:const NullPointerException){return caught;};return null_value();}'
READ = 'func category(ref input:const SystemException in O)->u64 where {owner O}{return input.category;}'
IDENTITY = 'func identity(ref input:SystemException in O)->ref SystemException in O where {owner O}{return input;}'
POINTER = 'func identity(ptr input:SystemException in O)->ptr SystemException in O where {owner O}{return input;}'
WRITE = 'proc write(ptr target:SystemException in O,value:SystemException)->void throws NullPointerException where {owner O}{target<-value;}'
PAIR = 'func pair(ref left:const T in A,ref right:const T in B)->u64 where {T,owner A,owner B};'
PAIR_BODY = 'func pair(ref left:const SystemException in A,ref right:const SystemException in B)->u64 where {owner A,owner B}{return left.category+right.category;}'
OUT = lambda text: 'behave @print('+text+');'
runner.CASES = {
 'readonly_callable': (IO+ERROR+READ+'func held=category;val common:SystemException<-missing_value();'+OUT('held(common)')+'common<-null_value();'+OUT('held(common)'),0,'5\n4\n',''),
 'returned_alias': (IO+ERROR+IDENTITY+'func select=identity;val common:SystemException<-missing_value();ref alias=select(common);alias<-null_value();'+OUT('common.category')+'common<-missing_value();'+OUT('alias.category'),0,'4\n5\n',''),
 'returned_readonly_access': (ERROR+IDENTITY.replace('input:SystemException','input:const SystemException').replace('->ref SystemException','->ref const SystemException')+'func select=identity;val common:SystemException<-missing_value();select(common)<-null_value();',2,'','readonly destination'),
 'readonly_input_cannot_be_writable': (ERROR+IDENTITY+'func select=identity;const val common:SystemException<-missing_value();select(common);',2,'','readonly argument'),
 'nullable_forwarding': (IO+ERROR+POINTER+'func select=identity;val common:SystemException<-missing_value();ptr handle=common;handle=null;'+OUT('((select(handle)) is null)'),0,'true\n',''),
 'nullable_read_caught': (IO+ERROR+POINTER+'func select=identity;ptr handle:SystemException=null in here;try{'+OUT('select(handle).category')+'}catch(caught:const NullPointerException){'+OUT('7u64')+'};',0,'7\n',''),
 'uninitialized_target_not_read': (IO+TYPES+POINTER+'func select=identity;val unread<-make local SystemException;ptr handle=select(unread);'+OUT('(handle is null)'),0,'false\n',''),
 'uninitialized_target_read_rejected': (TYPES+READ+'func held=category;val unread<-make local SystemException;held(unread);',2,'','before definite initialization'),
 'writer_initializes': (IO+ERROR+WRITE+'proc writer=write;val unread<-make local SystemException;try{writer(unread,missing_value());'+OUT('unread.category')+'}catch(caught:const NullPointerException){};',0,'5\n',''),
 'later_argument_mutates_borrow': (IO+ERROR+'func read(ref input:const SystemException in O,ignored:bool)->u64 where {owner O}{return input.category;}func held=read;val common:SystemException<-missing_value();'+OUT('held(common,{common<-null_value();true})'),0,'4\n',''),
 'selected_address_precedes_rebind': (IO+ERROR+'func read(ptr input:const SystemException in O,ignored:bool)->u64 throws NullPointerException where {owner O}{return input.category;}func held=read;val first:SystemException<-missing_value(),second:SystemException<-null_value();ptr handle=first;try{'+OUT('held(handle,{handle=second;true})')+'}catch(caught:const NullPointerException){};',0,'5\n',''),
 'deferred_snapshot_and_live_borrow': (IO+ERROR+'proc show(snapshot:SystemException,ref live:const SystemException in O)->void where {owner O}{'+OUT('snapshot.category')+OUT('live.category')+'}proc held=show;val common:SystemException<-missing_value();{{defer held(common,common);common<-null_value();}};',0,'5\n4\n',''),
 'repeated_target_type_declaration': (IO+ERROR+PAIR+PAIR_BODY+'func held=pair;val first:SystemException<-missing_value(),second:SystemException<-null_value();'+OUT('held(first,second)'),0,'9\n',''),
 'exact_borrowed_method_dominates_generic': (IO+ERROR+'func choose(ref left:const T in A,ref right:const T in B)->u64 where {T,owner A,owner B}{return 1u64;}func choose(ref left:const SystemException in A,ref right:const SystemException in B)->u64 where {owner A,owner B}{return 2u64;}val first:SystemException<-missing_value(),second:SystemException<-null_value();'+OUT('choose(first,second)'),0,'2\n',''),
 'symbolic_borrowed_method_dominates_generic': (IO+ERROR+'proc @choose(ref left:const SystemException in A,ref right:const SystemException in B)->u64 where {owner A,owner B}{return 2u64;}proc @choose(ref left:const T in A,ref right:const T in B)->u64 where {T,owner A,owner B}{return 1u64;}effect @choose;val first:SystemException<-missing_value(),second:SystemException<-null_value();'+OUT('behave @choose(first,second)'),0,'2\n',''),
 'declared_dependent_borrowed_result': (IO+ERROR+'func select(ref left:T in A,ref right:T in B)->ref T in B where {T,owner A,owner B};func select(ref left:SystemException in X,ref right:SystemException in Y) where {owner X,owner Y}{return right;}func held=select;val first:SystemException<-missing_value(),second:SystemException<-null_value();held(first,second)<-first;'+OUT('second.category'),0,'5\n',''),
 'borrowed_common_result_copy': (IO+ERROR+'func copy(ref input:const T in O)->const T where {T,owner O};func copy(ref input:const SystemException in O) where {owner O}{return input;}func held=copy;val common:SystemException<-missing_value();val saved<-held(common);common<-null_value();'+OUT('saved.category')+OUT('common.category'),0,'5\n4\n',''),
 'owner_mismatch': (ERROR+'func same(ref left:const SystemException in O,ref right:const SystemException in O)->u64 where {owner O}{return left.category;}func held=same;val first:SystemException<-missing_value();{{val second:SystemException<-null_value();held(first,second);}};',2,'','different actual owners'),
 'borrowed_result_escape': (ERROR+IDENTITY+'func select=identity;ptr escaped={{val local:SystemException<-missing_value();select(local)}};escaped;',2,'','owner has ended'),
 'deferred_catch_target_expires': (ERROR+'proc show(ref input:const SystemException in O)->void where {owner O}{}proc held=show;try{missing();{}}catch(caught){defer held(caught);};',2,'','does not outlive'),
 'direct_upcast_remains_unproved': (ERROR+READ+'val concrete<-missing_value();category(concrete);',2,'','borrowed exception upcasts'),
 'callable_upcast_rejected': (ERROR+READ+'func held=category;val concrete<-missing_value();held(concrete);',2,'','argument does not satisfy the callable view type'),
 'query_upcast_rejected': (ERROR+READ+'func held=category;val concrete<-missing_value();val result<-make local typeof(held(concrete)){1u64};',2,'','argument does not satisfy the callable view type'),
 'generic_higher_order_borrow': (IO+ERROR+IDENTITY+'func apply(f:F,ref input:SystemException in O)->ref SystemException in O where {F,owner O}{return f(input);}func held=identity;val common:SystemException<-missing_value();apply(held,common)<-null_value();'+OUT('common.category'),0,'4\n',''),
 'captured_borrowing_callable': (IO+ERROR+READ+'func held=category;func captured=func[held](ref input:const SystemException in O)->u64 where {owner O}{return held(input);};val common:SystemException<-missing_value();'+OUT('captured(common)'),0,'5\n',''),
 'symbolic_borrow_uses_current_payload': (IO+ERROR+'proc @category(ref input:const SystemException in O)->u64 where {owner O}{return input.category;}effect @category;val common:SystemException<-missing_value();'+OUT('behave @category(common)')+'common<-null_value();'+OUT('perform @category(common)'),0,'5\n4\n',''),
 'mixed_family_selection': (IO+ERROR+'proc @choose(value:SystemException,ref input:const SystemException in O)->u64 where {owner O}{return value.category+input.category;}proc @choose(value:MissingReturnException,ref input:const SystemException in O)->u64 where {owner O}{return 17u64;}effect @choose;val first:SystemException<-missing_value(),second:SystemException<-null_value();'+OUT('behave @choose(first,second)')+OUT('behave @choose(second,first)'),0,'17\n9\n',''),
 'borrow_target_not_payload_dispatch': (IO+ERROR+'proc @choose(ref input:const SystemException in O)->u64 where {owner O}{return 1u64;}proc @choose(ref input:const MissingReturnException in O)->u64 where {owner O}{return 2u64;}effect @choose;val common:SystemException<-missing_value();'+OUT('behave @choose(common)'),0,'1\n',''),
 'type_query_does_not_read_payload': (IO+TYPES+READ+'func held=category;val unread<-make local SystemException;val result<-make local typeof(held(unread)){7u64};'+OUT('result'),0,'7\n',''),
 'declared_checked_failure': (ERROR+'func category(ref input:const T in O)->u64 throws NullPointerException where {T,owner O};'+READ+'func held=category;val common:SystemException<-missing_value();held(common);',2,'','requires a covering catch'),
 'mixed_dependent_category_remains_unproved': (TYPES+'func f(value:T,ref input:const T in O)->u64 where {T,owner O};func f(value:SystemException,ref input:const SystemException in O)->u64 where {owner O}{return 7u64;}',2,'','borrowed or nested common type'),
}
parameters = [f'ref item{i}:const SystemException in O{i}' for i in range(12)]
owners = ','.join(f'owner O{i}' for i in range(12))
many = 'func many('+','.join(parameters)+')->u64 where {'+owners+'}{return item0.category+item11.category;}'
inputs = ','.join('first' if i % 2 == 0 else 'second' for i in range(12))
runner.CASES['many_borrows_need_no_category_product'] = (
    IO+ERROR+many+'func held=many;val first:SystemException<-missing_value(),second:SystemException<-null_value();'+OUT('held('+inputs+')'),0,'9\n','')
for name, (source, status, output, diagnostic) in list(runner.CASES.items()):
    if ERROR in source and status == 0:
        runner.CASES[name] = (source,status,output,'[-Wmissing-return]')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
