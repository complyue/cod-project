#!/usr/bin/env python3
"""Bodyless signatures supply contextual type evidence without executable demand."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
ID = 'func identity(input:T)->const T where T{return input;}'
DECL = 'func absent(callback:func(T)->const T,input:T)->const T where T;'
NPE = 'using "std:exceptions" NullPointerException;'

def query(expression='absent(identity,1)', value='7', prefix=ID+DECL):
    return IO+prefix+'val result<-make local typeof('+expression+'){'+value+'};behave @print(result);'

runner.CASES = {
    'later_type': (query(),0,'7\n',''),
    'boolean_type': (query('absent(identity,true)','false'),0,'false\n',''),
    'earlier_type': (query('absent(1,identity)',prefix=ID+'func absent(input:T,callback:func(T)->const T)->const T where T;'),0,'7\n',''),
    'overloaded_operand': (query('absent(pick,1)',prefix='func pick(input:i64)->const i64{return input;}func pick(input:bool)->const bool{return input;}'+DECL),0,'7\n',''),
    'grouped_operand': (query('absent({identity},1)'),0,'7\n',''),
    'scoped_operand': (query('absent({{identity}},1)'),0,'7\n',''),
    'conditional_operand': (query('absent(if(true){identity}else{identity},1)'),0,'7\n',''),
    'anonymous_operand': (query('absent(func(input:T)->const T where T{return input;},1)'),0,'7\n',''),
    'multiple_operands': (query('absent(identity,identity,1)',prefix=ID+'func absent(first:func(T)->const T,second:func(T)->const T,input:T)->const T where T;'),0,'7\n',''),
    'nested_queries': (query('absent(identity,absent(identity,1))'),0,'7\n',''),
    'variadic_context': (query('absent(identity,1,2,3)',prefix=ID+'func absent(callback:func(T)->const T,inputs:T...)->const T where T;'),0,'7\n',''),
    'variadic_callbacks': (query('absent(1,identity,identity)',prefix=ID+'func absent(input:T,callbacks:func(T)->const T...)->const T where T;'),0,'7\n',''),
    'duplicate_contracts': (query(prefix=ID+DECL+DECL),0,'7\n',''),
    'disjoint_contracts': (query(prefix=ID+'func absent(callback:func(i64)->const i64,input:i64)->i64;func absent(callback:func(bool)->const bool,input:bool)->bool;'),0,'7\n',''),
    'uninitialized_input': (query('absent(identity,slot)',prefix=ID+DECL+'val slot<-make local i64;'),0,'7\n',''),
    'null_input': (query('absent(identity,slot)',prefix=ID+DECL+'ptr slot:i64=null in here;'),0,'7\n',''),
    'borrowed_input': (query('absent(identity,slot)',prefix=ID+'func absent(callback:func(T)->const T,ref input:T in O)->ref T in O where {T,owner O};val slot<-make local i64;'),0,'7\n',''),
    'invalid_body_not_demanded': (query('absent(broken,true)','false','func broken(input:T)->const T where T{return input+input;}'+DECL),0,'false\n',''),
    'recursive_body_not_demanded': (query('absent(recursive,true)','false','func recursive(input:T)->const T where T{return recursive(input);}'+DECL),0,'false\n',''),
    'procedure_not_invoked': (query(prefix='proc identity(input:T)->const T where T{behave @print(99);return input;}proc absent(callback:proc(T)->const T,input:T)->const T where T;'),0,'7\n',''),
    'checked_contract': (query(prefix=NPE+'func identity(input:T)->const T throws NullPointerException where T{return input;}func absent(callback:func(T)->const T throws NullPointerException,input:T)->const T throws NullPointerException where T;'),0,'7\n',''),
    'checked_contract_preserved': (query(prefix=NPE+'func identity(input:T)->const T throws NullPointerException where T{return input;}'+DECL),2,'','required checked exception'),
    'wrong_callable_kind': (query(prefix='proc identity(input:T)->const T where T{return input;}'+DECL),2,'','callable view contract'),
    'result_conflict': (query(prefix=ID+DECL+'func absent(callback:func(T)->const T,input:T)->bool where T;'),2,'','declarations do not establish one inspected result contract'),
    'context_conflict': (query(prefix=ID+'func absent(callback:func(i64)->const i64,input:i64)->i64;func absent(callback:func(bool)->const bool,input:i64)->i64;'),2,'','known declaration or operand type'),
    'widening_contract_not_discarded': (query('absent(identity,1u32)',prefix=ID+'func absent(callback:func(i64)->const i64,input:u32)->i64;func absent(callback:func(bool)->const bool,input:u64)->i64;'),2,'','known declaration or operand type'),
    'inconsistent_inputs': (query('absent(identity,1,true)',prefix=ID+'func absent(callback:func(T)->const T,inputs:T...)->const T where T;'),2,'','known declaration or operand type'),
    'unknown_tail': (query('absent(identity)',prefix=ID+'func absent(callback:func(T)->const T,inputs:T...)->const T where T;'),2,'','known declaration or operand type'),
    'borrow_needs_view': (query(prefix=ID+'func absent(callback:func(T)->const T,ref input:T in O)->const T where {T,owner O};'),2,'','borrowed parameter requires an existing view'),
    'nearest_binding': (query(prefix=ID+DECL+'val identity<-1;'),2,'','no applicable declared signature'),
    'real_call_requires_implementation': (ID+DECL+'func chosen:func(i64)->const i64=identity;absent(chosen,1);',2,'','no applicable implementation for declared callable'),
    'nested_real_call_requires_implementation': (ID+DECL+'func chosen:func(i64)->const i64=identity;func apply(callback:func(T)->const T,input:T)->const T where T{return callback(input);}apply(identity,absent(chosen,1));',2,'','no applicable implementation for declared callable'),
    'query_does_not_initialize': (ID+DECL+'val slot<-make local i64;val result<-make local typeof(absent(identity,slot)){7};slot;',2,'','before definite initialization'),
    'retained_context': (IO+ID+DECL+'func outer(input:T)->const T where T{func inner(item:T)->const T{val result<-make local typeof(absent(identity,item)){item};return result;};return inner(input);}behave @print(outer(8));behave @print(outer(true));',0,'8\ntrue\n',''),
    'self_query_context': (IO+DECL+'func outer(input:T)->const T where T{func self(item:T)->const T{val result<-make local typeof(absent(self,item)){item};return result;};return self(input);}behave @print(outer(9));behave @print(outer(true));',0,'9\ntrue\n',''),
    'imported_declaration': ({'main.cod':query(prefix='using "./contract.cod" absent;'+ID), 'contract.cod':DECL+'export absent;'},0,'7\n',''),
    'imported_helper_context': ({'main.cod':IO+'using "./contract.cod" outer;func absent(input:bool)->bool;behave @print(outer(12));',
        'contract.cod':ID+DECL+'func outer(input:T)->const T where T{val result<-make local typeof(absent(identity,input)){input};return result;}export outer;'},0,'12\n',''),
    'readonly_filters_context': (query('absent(identity,slot)',prefix=ID+'func absent(callback:func(i64)->const i64,ref input:const i64 in O)->i64 where {owner O};func absent(callback:func(bool)->const bool,ref input:i64 in O)->i64 where {owner O};val slot:const i64<-1;'),0,'7\n',''),
    'distinct_owners_filter_context': (IO+ID+'func absent(callback:func(i64)->const i64,ref first:i64 in A,ref second:i64 in B)->i64 where {owner A,owner B};func absent(callback:func(bool)->const bool,ref first:i64 in O,ref second:i64 in O)->i64 where {owner O};val first<-1;{{val second<-2;val result<-make local typeof(absent(identity,first,second)){13};behave @print(result);}};',0,'13\n',''),
    'unknown_owner_equality': (ID+'func absent(callback:func(i64)->const i64,ref first:i64 in O,ref second:i64 in O)->i64 where {owner O};func outer(ref a:i64 in A,ref b:i64 in B)->i64 where {owner A,owner B}{val result<-make local typeof(absent(identity,a,b)){14};return result;}',2,'','unproved owner equality'),
}
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
