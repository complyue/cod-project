#!/usr/bin/env python3
"""Rebinding queries check slots and target evidence without changing either."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
OUT = lambda value: 'behave @print('+value+');'
QUERY = lambda expression, value='9': 'val result<-make local typeof('+expression+'){'+value+'};'
BASE = 'val x<-1,y<-7;ptr p=x,q=y;'
runner.CASES = {}
for name, expression in [('infix','p=y'),('explicit','(=)(p,y)'),('intrinsic','intrinsic (=)(p,y)')]:
    runner.CASES[name] = (IO+BASE+QUERY(expression)+OUT('result')+OUT('p')+OUT('q'),0,'9\n1\n7\n','')
runner.CASES.update({
 'right_associative': (IO+BASE+QUERY('p=q=y')+OUT('p')+OUT('q'),0,'1\n7\n',''),
 'target_write': (IO+BASE+QUERY('(p=y)<-9')+OUT('p')+OUT('y'),0,'1\n7\n',''),
 'bits_swap_result': (IO+BASE+QUERY('{(p=y)<->x;1}')+OUT('p')+OUT('y'),0,'1\n7\n',''),
 'uninitialized_target': ('val x<-make local i64;ptr p:i64=null in here;'+QUERY('p=x'),0,'',''),
 'null_source': (BASE+QUERY('p=null'),0,'',''),
 'explicit_null_owner': (BASE+QUERY('p=null in here'),0,'',''),
 'null_destination': ('val x<-1;ptr p:i64=null in here;'+QUERY('p=x'),0,'',''),
 'physical_readonly_slot': ('val x:const i64<-1;ptr p=x;'+QUERY('p=x'),0,'',''),
 'readonly_alias_view': (BASE+'ptr var alias:const->p;'+QUERY('alias=y'),0,'',''),
 'selected_slot': (BASE+QUERY('{if(true){p}else{q}}=x'),0,'',''),
 'borrowed_result_source': ('func get(ref x:i64 in O)->ref i64 in O where {owner O}{return x;}'+BASE+QUERY('p=get(y)'),0,'',''),
 'operands_not_called': (IO+'proc source(ref value:i64 in O)->ref i64 in O where {owner O}{'+OUT('99')+'return value;}'+BASE+QUERY('p=source(y)')+OUT('p'),0,'1\n',''),
 'result_access_selection': ('func select(ref value:i64 in O)->i64 where {owner O}{return 1;}func select(ref value:const i64 in O)->bool where {owner O}{return true;}'+BASE+QUERY('select(p=y)'),0,'',''),
 'readonly_result_selection': ('func select(ref value:i64 in O)->i64 where {owner O}{return 1;}func select(ref value:const i64 in O)->bool where {owner O}{return true;}'+BASE+'ptr var alias:const->p;'+QUERY('select(alias=y)','true'),0,'',''),
 'result_identity': (BASE+QUERY('(p=y) is y','true'),0,'',''),
 'result_owner': ('func same(ref a:i64 in O,ref b:i64 in O)->i64 where {owner O}{return 1;}'+BASE+QUERY('same(p=y,x)'),0,'',''),
 'generic_query': (IO+'func query(value:T)->const T where T{val a<-value,b<-value;ptr target=a;val result<-make local typeof(target=b){value};return result;}'+OUT('query(7)')+OUT('query(true)'),0,'7\ntrue\n',''),
 'outer_query_without_capture': (IO+'func outer()->i64{'+BASE+'func inner()->i64{'+QUERY('p=y')+'return result;};return inner();}'+OUT('outer()'),0,'9\n',''),
 'callee_body_not_demanded': ('func invalid(ref value:T in O)->ref T in O where {T,owner O}{value<-value+value;return value;}val x<-true;ptr p=x;'+QUERY('p=invalid(x)','false'),0,'',''),
 'callable_target': (IO+'func one()->u64{return 1u64;}func two()->u64{return 2u64;}func a=one;func b=two;ptr p=a;'+QUERY('(p=b)()','9u64')+OUT('result')+OUT('p()'),0,'9\n1\n',''),
 'fixed_binding': (BASE+'const ptr fixed=x;'+QUERY('fixed=y'),2,'','const reference binding'),
 'value_binding': (BASE+QUERY('x=y'),2,'','val storage has no reference binding'),
 'fresh_source': (BASE+QUERY('p=7'),2,'','fresh value without an existing target'),
 'source_type': ('val x<-1,y<-7u64;ptr p=x;'+QUERY('p=y'),2,'','exact target type'),
 'source_readonly': ('val x<-1;const val y<-7;ptr p=x;'+QUERY('p=y'),2,'','readonly source cannot rebind'),
 'alias_keeps_physical_permission': ('val x<-1;const val y<-7;ptr p=x;ptr var alias:const->p;'+QUERY('alias=y'),2,'','readonly source cannot rebind'),
 'owner_change': ('val x<-1;ptr p=x;{{val y<-7;'+QUERY('p=y')+'}};',2,'','initializer-fixed owner'),
 'null_owner_change': ('val x<-1;ptr p=x;{{'+QUERY('p=null in here')+'}};',2,'','initializer-fixed owner'),
 'result_not_slot': (BASE+QUERY('(p=y)=x'),2,'','requires a binding slot'),
 'joined_result_not_slot': (BASE+QUERY('{if(true){p=y}else{q}}=x'),2,'','requires a binding slot'),
 'result_not_reference_swap': (BASE+QUERY('{(p=y)<=>q;1}'),2,'','requires a binding slot'),
 'readonly_result_not_writable': (BASE+'ptr var alias:const->p;'+QUERY('(alias=y)<-9'),2,'','readonly destination'),
 'borrowed_target_not_slot': ('func get(ref x:i64 in O)->ref i64 in O where {owner O}{return x;}'+BASE+QUERY('get(x)=y'),2,'','requires a binding slot'),
 'query_does_not_initialize': ('val x<-make local i64,y<-7;ptr p=x;'+QUERY('p=y')+'p;',2,'','before definite initialization'),
})
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {'borrowed_result_source':['x']}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
