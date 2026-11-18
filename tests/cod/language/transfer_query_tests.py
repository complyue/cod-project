#!/usr/bin/env python3
"""Transfer type queries retain destination type/access without mutation or copying."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
OUT = lambda value:'behave @print('+value+');'
QUERY = lambda expression, value='9':'val result<-make local typeof('+expression+'){'+value+'};'
BASE = 'val x<-1,y<-7;ptr p=x;'
ID = 'func identity(value:T)->const T where T{return value;}'
runner.CASES = {}
for name, call in [('infix','x<-y'),('explicit','(<-)(x,y)'),('intrinsic','intrinsic (<-)(x,y)')]:
    runner.CASES[name] = (IO+BASE+QUERY(call)+OUT('result')+OUT('x')+OUT('y'),0,'9\n1\n7\n','')
runner.CASES.update({
 'uninitialized_destination': ('val x<-make local i64;'+QUERY('x<-7'),0,'',''),
 'uninitialized_source': ('val x<-1,y<-make local i64;'+QUERY('x<-y'),0,'',''),
 'null_destination': ('ptr p:i64=null in here;'+QUERY('p<-7'),0,'',''),
 'null_source': ('val x<-1;ptr p:i64=null in here;'+QUERY('x<-p'),0,'',''),
 'readonly_source': ('val x<-1;const val y<-7;'+QUERY('x<-y'),0,'',''),
 'fixed_pointer_destination': ('val x<-1;const ptr p=x;'+QUERY('p<-7'),0,'',''),
 'cross_owner_transfer': ('val x<-1;{{val y<-7;'+QUERY('x<-y')+'}};',0,'',''),
 'result_destination_type': (IO+'val x<-1u64;'+QUERY('x<-7u8','4294967296u64')+OUT('result'),0,'4294967296\n',''),
 'nested_transfer_types': (IO+'val x<-1u64,y<-2u32;'+QUERY('x<-y<-7u8','4294967296u64')+OUT('result')+OUT('x')+OUT('y'),0,'4294967296\n1\n2\n',''),
 'selected_destination': (IO+BASE+QUERY('{if(true){x}else{y}}<-7')+OUT('x')+OUT('y'),0,'1\n7\n',''),
 'destination_not_called': (IO+'proc target(ref value:i64 in O)->ref i64 in O where {owner O}{'+OUT('99')+'return value;}'+BASE+QUERY('target(x)<-7')+OUT('x'),0,'1\n',''),
 'source_not_called': (IO+'proc source()->i64{'+OUT('99')+'return 7;}'+BASE+QUERY('x<-source()')+OUT('x'),0,'1\n',''),
 'initializer_runs_once': (IO+'proc next(ref counter:i64 in O)->i64 where {owner O}{counter<-counter+1;return counter;}val counter<-0,x<-7;'+QUERY('x<-next(counter)','next(counter)')+OUT('counter')+OUT('result')+OUT('x'),0,'1\n1\n7\n',''),
 'result_readonly_selection': (IO+'func select(ref value:i64 in O)->u64 where {owner O}{return 1u64;}func select(ref value:const i64 in O)->bool where {owner O}{return true;}'+BASE+QUERY('select(x<-7)','true')+OUT('result'),0,'true\n',''),
 'result_owner': ('func same(ref a:const i64 in O,ref b:i64 in O)->i64 where {owner O}{return 1;}'+BASE+QUERY('same(x<-7,y)'),0,'',''),
 'result_identity_view': (IO+BASE+QUERY('{x<-7} is x','true')+OUT('result')+OUT('x'),0,'true\n1\n',''),
 'generic_query': (IO+'func query(value:T)->const T where T{val x<-value;val result<-make local typeof(x<-value){value};return result;}'+OUT('query(7)')+OUT('query(true)'),0,'7\ntrue\n',''),
 'outer_query_without_capture': (IO+'func outer()->i64{'+BASE+'func inner()->i64{'+QUERY('x<-y')+'return result;};return inner();}'+OUT('outer()'),0,'9\n',''),
 'recursive_query': (IO+'func query(value:T,n:u64)->const T where T{val x<-value;val result<-make local typeof(x<-value){value};if(n==0u64){return result;}else{return query(result,n-1u64);};}'+OUT('query(7,30u64)'),0,'7\n',''),
 'generic_body_not_demanded': ('func invalid(value:T)->const T where T{return value+value;}val x<-true;'+QUERY('x<-invalid(true)','false'),0,'',''),
 'exception_copy_not_allocated': ('using "std:exceptions" Exception;val x<-make local Exception,y<-make local Exception;val result<-make local typeof(x<-y);',0,'',''),
 'callable_name_context': (IO+ID+'func known(value:i64)->i64{return value+1;}func target=known;'+QUERY('(target<-identity)(7)')+OUT('result')+OUT('target(7)'),0,'9\n8\n',''),
 'callable_anonymous_context': (IO+'func known(value:i64)->i64{return value+1;}func target=known;'+QUERY('(target<-func(value:i64){return value;})(7)')+OUT('result')+OUT('target(7)'),0,'9\n8\n',''),
 'callable_result_invocation': (IO+'func known(value:i64)->i64{return value+1;}func target=known;func other=known;'+QUERY('(target<-other)(1)')+OUT('result'),0,'9\n',''),
 'wrong_destination': (QUERY('1<-7'),2,'','storage destination'),
 'readonly_destination': ('const val x<-1;'+QUERY('x<-7'),2,'','readonly destination'),
 'readonly_alias': (BASE+'ptr var alias:const->p;'+QUERY('alias<-7'),2,'','readonly destination'),
 'wrong_type': (BASE+QUERY('x<-true'),2,'','source does not satisfy destination type'),
 'narrowing': ('val x<-1u8;'+QUERY('x<-7u64','1u8'),2,'','source does not satisfy destination type'),
 'null_not_bits': (BASE+QUERY('x<-null'),2,'','source does not satisfy destination type'),
 'result_remains_readonly': (BASE+QUERY('{x<-7}<-9'),2,'','readonly destination'),
 'result_cannot_swap_bits': (BASE+QUERY('{ {x<-7}<->y;1}'),2,'','readonly destination'),
 'result_has_no_binding_slot': (BASE+QUERY('{ {x<-7}<=>p;1}'),2,'','requires a binding slot'),
 'result_cannot_supply_writable_view': ('func take(ref value:i64 in O)->i64 where {owner O}{return value;}'+BASE+QUERY('take(x<-7)'),2,'','readonly argument'),
 'result_preserves_owner': ('func same(ref a:const i64 in O,ref b:i64 in O)->i64 where {owner O}{return 1;}val x<-1;{{val y<-7;'+QUERY('same(x<-7,y)')+'}};',2,'','different actual owners'),
 'mixed_destination': (BASE+QUERY('{if(true){x}else{1}}<-7'),2,'','storage destination'),
 'query_does_not_initialize': ('val x<-make local i64;'+QUERY('x<-7')+'x;',2,'','before definite initialization'),
 'query_does_not_satisfy_callee_write': ('func query(ptr value:i64 in O)->void where {owner O}{'+QUERY('value<-7')+'}val x<-make local i64;query(x);x;',2,'','before definite initialization'),
 'query_does_not_validate_bad_signature': ('func bad(value:bool)->i64{return 1;}'+BASE+QUERY('x<-bad(1)'),2,'','no applicable method'),
})
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {'initializer_runs_once':['counter']}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
