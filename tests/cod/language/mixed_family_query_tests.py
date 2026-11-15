#!/usr/bin/env python3
"""Declarations inspect uncovered inputs without overriding applicable methods."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
DECL = 'func target(input:T)->const T where T;'
METHOD = 'func target(input:bool)->bool{return input;}'
INSPECT = 'func inspect(callback:func(T)->const T,input:T)->const T where T;'
NPE = 'using "std:exceptions" NullPointerException;'
BASE = DECL+METHOD+'func query(input:T)->const T where T{val result<-make local typeof(target(input)){input};return result;}export target;export query;'

def query(expression='target(1)', value='7', prefix=DECL+METHOD):
    return IO+prefix+'val result<-make local typeof('+expression+'){'+value+'};behave @print(result);'

runner.CASES = {
    'uncovered_call': (query(),0,'7\n',''),
    'covered_call': (query('target(true)','false'),0,'false\n',''),
    'nested_calls': (query('target(target(1))'),0,'7\n',''),
    'uninitialized_operand': (query('target(slot)',prefix=DECL+METHOD+'val slot<-make local i64;'),0,'7\n',''),
    'null_operand': (query('target(slot)',prefix=DECL+METHOD+'ptr slot:i64=null in here;'),0,'7\n',''),
    'extra_arity': (query('target(1,2)',prefix='func target(inputs:i64...)->i64;func target(input:bool)->bool{return input;}'),0,'7\n',''),
    'zero_arity': (query('target()',prefix='func target(inputs:i64...)->i64;func target(input:bool)->bool{return input;}'),0,'7\n',''),
    'contextual_outer_call': (query('inspect(identity,1)',prefix='func identity(input:T)->const T where T{return input;}'+INSPECT+'func inspect(callback:func(bool)->bool,input:bool)->bool{return callback(input);}'),0,'7\n',''),
    'contextual_inner_family': (query('inspect(target,1)',prefix=DECL+METHOD+INSPECT),0,'7\n',''),
    'contextual_covered_family': (query('inspect(target,true)','false',DECL+METHOD+INSPECT),0,'false\n',''),
    'both_partial_families': (query('inspect(target,1)',prefix=DECL+METHOD+INSPECT+'func inspect(callback:func(bool)->bool,input:bool)->bool{return callback(input);}'),0,'7\n',''),
    'grouped_family': (query('inspect({target},1)',prefix=DECL+METHOD+INSPECT),0,'7\n',''),
    'conditional_family': (query('inspect(if(true){target}else{target},1)',prefix=DECL+METHOD+INSPECT),0,'7\n',''),
    'mixed_numeric_coverage': (query('inspect(target,1u64)','7u64','func target(input:u64)->u64;func target(input:u8)->u8{return input;}'+INSPECT),0,'7\n',''),
    'method_result_retains_type': (IO+'func target(input:u64)->u64;func target(input:u8)->u8{return input;}func tag(input:u8)->u64{return 8u64;}func tag(input:u64)->u64{return 64u64;}val result<-make local typeof(target(1u8)){7u8};behave @print(tag(result));',0,'8\n',''),
    'borrowed_uncovered_target': (query('target(slot)',prefix='func target(ref input:T in O)->ref T in O where {T,owner O};func target(ref input:bool in O)->ref bool in O where {owner O}{return input;}val slot<-make local i64;'),0,'7\n',''),
    'readonly_declaration_coverage': (query('target(slot)',prefix='func target(ref input:const i64 in O)->i64 where {owner O};func target(ref input:i64 in O)->i64 where {owner O}{return input;}val slot:const i64<-1;'),0,'7\n',''),
    'readonly_real_call_rejects': ('func target(ref input:const i64 in O)->i64 where {owner O};func target(ref input:i64 in O)->i64 where {owner O}{return input;}val slot:const i64<-1;target(slot);',2,'','readonly argument'),
    'distinct_owners_use_declaration': (IO+'func target(ref a:i64 in A,ref b:i64 in B)->i64 where {owner A,owner B};func target(ref a:i64 in O,ref b:i64 in O)->i64 where {owner O}{return a;}val first<-1;{{val second<-2;val result<-make local typeof(target(first,second)){7};behave @print(result);}};',0,'7\n',''),
    'unknown_method_owner_not_hidden': ('func target(ref a:i64 in A,ref b:i64 in B)->i64 where {owner A,owner B};func target(ref a:i64 in O,ref b:i64 in O)->i64 where {owner O}{return a;}func outer(ref first:i64 in A,ref second:i64 in B)->i64 where {owner A,owner B}{val result<-make local typeof(target(first,second)){7};return result;}',2,'','method selection requires an unproved owner equality'),
    'missing_runtime_method': (DECL+METHOD+'target(1);',2,'','no applicable method'),
    'missing_runtime_family_coverage': (DECL+METHOD+'func selected:func(i64)->const i64=target;',2,'','no applicable method'),
    'missing_runtime_callback': (DECL+METHOD+'func inspect(callback:func(i64)->i64)->i64{return callback(1);}inspect(target);',2,'','no applicable method'),
    'missing_signature': (query(prefix=METHOD),2,'','no applicable method'),
    'inapplicable_declaration': (query(prefix='func target(input:u64)->u64;'+METHOD),2,'','no applicable declared signature'),
    'conflicting_uncovered_results': (query(prefix=DECL+'func target(input:i64)->bool;'+METHOD),2,'','declarations do not establish one inspected result contract'),
    'selected_method_result_not_hidden': (query(prefix=DECL+'func target(input:T)->bool where T{return true;}'),2,'','does not satisfy callable declaration'),
    'selected_method_kind_not_hidden': (query(prefix=DECL+'proc target(input:bool)->bool{return input;}'),2,'','implementation kind'),
    'unresolved_method_result_not_hidden': (query(prefix='func target(first:T,second:T)->const T where T;func target(input:T) where T{return target(input);}'),2,'','known declaration or operand type'),
    'method_body_not_demanded': (query('target(true)','false',DECL+'func target(input:T)->const T where T{return input+input;}'),0,'false\n',''),
    'real_method_body_still_checked': (DECL+'func target(input:T)->const T where T{return input+input;}target(true);',2,'','no applicable numeric operation'),
    'unselected_generic_body_not_demanded': (query(prefix=DECL+'func target(first:T,second:T)->const T where T{return first+second;}'),0,'7\n',''),
    'covered_checked_contract': (query('inspect(target,1u64)','7u64',NPE+'func target(input:u64)->u64 throws NullPointerException;func target(input:u8)->u64{return 7u64;}func inspect(callback:func(T)->const T throws NullPointerException,input:T)->const T where T;'),0,'7\n',''),
    'checked_contract_not_hidden': (query('inspect(target,1u64)','7u64',NPE+'func target(input:u64)->u64 throws NullPointerException;func target(input:u8)->u64{return 7u64;}'+INSPECT),2,'','required checked exception'),
    'retained_context': (IO+DECL+METHOD+'func outer(input:T)->const T where T{func inner(item:T)->const T{val result<-make local typeof(target(item)){item};return result;};return inner(input);}behave @print(outer(8));behave @print(outer(true));',0,'8\ntrue\n',''),
    'imported_contract': ({'main.cod':query(prefix='using "base.cod" target;'), 'base.cod':BASE},0,'7\n',''),
    'imported_helper': ({'main.cod':IO+'using "base.cod" query;func target(input:i64)->bool;behave @print(query(9));', 'base.cod':BASE},0,'9\n',''),
    'receiver_context': ({'base.cod':BASE,'extension.cod':'import "base.cod" target;func target(input:u64)->u64{return input;}export import "base.cod" query;using "base.cod" query as frozen;export frozen;',
        'main.cod':IO+'using "extension.cod" query;using "extension.cod" frozen;behave @print(query(10u64));behave @print(frozen(11u64));'},0,'10\n11\n',''),
    'receiver_declaration_conflict': ({'base.cod':BASE,'extension.cod':'import "base.cod" target;func target(input:i64)->bool;export import "base.cod" query;',
        'main.cod':IO+'using "extension.cod" query;behave @print(query(7));'},2,'','declarations do not establish one inspected result contract'),
    'source_context_isolated': ({'base.cod':BASE,'extension.cod':'import "base.cod" target;func target(input:i64)->bool;using "base.cod" query;export query;',
        'main.cod':IO+'using "extension.cod" query;behave @print(query(12));'},0,'12\n',''),
}
AMBIGUOUS = 'func target(a:u64,b:u64)->u64;func target(a:u64,b:T)->u64 where T{return a;}func target(a:T,b:u64)->u64 where T{return b;}'
runner.CASES['method_ambiguity_not_hidden'] = (query('target(1u64,2u64)','7u64',AMBIGUOUS),2,'','ambiguous methods')
runner.CASES['family_ambiguity_not_hidden'] = (query('inspect(target)','7u64',AMBIGUOUS+'func inspect(callback:func(u64,u64)->u64)->u64;'),2,'','ambiguous methods')
runner.CASES['readonly_family_declaration_coverage'] = (query('inspect(target,held)',prefix='func target(ref input:const i64 in O)->i64 where {owner O};func target(ref input:i64 in O)->i64 where {owner O}{return input;}func model(ref input:const i64 in P)->i64 where {owner P}{return input;}func held=model;func inspect(callback:F,model:F)->i64 where F;'),0,'7\n','')
runner.CASES['unknown_family_owner_not_hidden'] = (query('inspect(target,held)',prefix='func target(ref a:i64 in A,ref b:i64 in B)->i64 where {owner A,owner B};func target(ref a:i64 in O,ref b:i64 in O)->i64 where {owner O}{return a;}func model(ref a:i64 in A,ref b:i64 in B)->i64 where {owner A,owner B}{return a;}func held=model;func inspect(callback:F,model:F)->i64 where F;'),2,'','method selection requires an unproved owner equality')
CALLBACK_BASE = DECL+METHOD+INSPECT+'func query(input:T)->const T where T{val result<-make local typeof(inspect(target,input)){input};return result;}export target;export query;'
runner.CASES['receiver_callable_declaration_conflict'] = ({'base.cod':CALLBACK_BASE,'extension.cod':'import "base.cod" target;func target(input:i64)->bool;export import "base.cod" query;',
    'main.cod':IO+'using "extension.cod" query;behave @print(query(7));'},2,'','declarations do not establish one inspected result contract')
runner.CASES['source_callable_context_isolated'] = ({'base.cod':CALLBACK_BASE,'extension.cod':'import "base.cod" target;func target(input:i64)->bool;using "base.cod" query;export query;',
    'main.cod':IO+'using "extension.cod" query;behave @print(query(13));'},0,'13\n','')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
