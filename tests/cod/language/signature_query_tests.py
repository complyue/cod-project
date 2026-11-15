#!/usr/bin/env python3
"""Type-only callable operands retain declaration contracts without implementations."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
ID = 'func declared(input:T)->const T where T;'
APPLY = 'func inspect(callback:func(T)->const T,input:T)->const T where T;'
MODEL = 'func inspect(callback:F,model:F)->i64 where F;'
NPE = 'using "std:exceptions" NullPointerException;'
ERRORS = 'using "std:exceptions" Exception;using "std:exceptions" MissingReturnError;'

def query(expression='inspect(declared,1)', value='7', prefix=ID+APPLY):
    return IO+prefix+'val result<-make local typeof('+expression+'){'+value+'};behave @print(result);'

runner.CASES = {
    'generic_operand': (query(),0,'7\n',''),
    'boolean_operand': (query('inspect(declared,true)','false'),0,'false\n',''),
    'concrete_operand': (query(prefix='func declared(input:i64)->i64;'+APPLY),0,'7\n',''),
    'grouped_operand': (query('inspect({declared},1)'),0,'7\n',''),
    'scoped_operand': (query('inspect({{declared}},1)'),0,'7\n',''),
    'conditional_operand': (query('inspect(if(true){declared}else{declared},1)'),0,'7\n',''),
    'nested_operand': (query('inspect(declared,inspect(declared,1))'),0,'7\n',''),
    'implemented_consumer': (query(prefix=ID+'func inspect(callback:func(T)->const T,input:T)->const T where T{return callback(input);}'),0,'7\n',''),
    'variadic_declaration_operand': (query(prefix='func declared(inputs:T...)->const T where T;'+APPLY),0,'7\n',''),
    'variadic_pair_signature': (query('inspect(declared,1)',prefix='func declared(inputs:i64...)->i64;func inspect(callback:func(T,T)->const T,input:T)->const T where T;'),0,'7\n',''),
    'variadic_same_type_not_erased': (query('inspect(declared,1)',prefix='func declared(inputs:T...)->const T where T;func inspect(callback:func(T,T)->const T,input:T)->const T where T;'),2,'','no applicable declared signature'),
    'duplicate_declarations': (query(prefix=ID+ID+APPLY),0,'7\n',''),
    'context_selects_disjoint_signatures': (query(prefix='func declared(input:i64)->i64;func declared(input:bool)->bool;'+APPLY),0,'7\n',''),
    'concrete_signature_without_context': (query('inspect(declared)',prefix='func declared(input:u64)->u64;func inspect(callback:F)->i64 where F;'),0,'7\n',''),
    'duplicate_signature_without_context': (query('inspect(declared)',prefix='func declared(input:u64)->u64;func declared(input:u64)->u64;func inspect(callback:F)->i64 where F;'),0,'7\n',''),
    'complete_admitted_inputs': (query('inspect(declared,1u64)','7u64','func declared(input:u8)->u64;func declared(input:u32)->u64;func declared(input:u64)->u64;'+APPLY),0,'7\n',''),
    'result_widening': (query('inspect(declared,1u64)','7u64','func declared(input:u64)->u8;'+APPLY),0,'7\n',''),
    'uncovered_inputs': (query('inspect(declared,1u64)','7u64','func declared(input:u8)->u64;'+APPLY),2,'','no applicable declared signature'),
    'conflicting_results': (query(prefix=ID+'func declared(input:i64)->bool;'+APPLY),2,'','declarations do not establish one inspected result contract'),
    'dependent_result_undetermined': (query(prefix='func declared(input:T)->const U where {T,U};'+APPLY),2,'','declarations do not establish one inspected result contract'),
    'result_mismatch': (query(prefix='func declared(input:T)->bool where T;'+APPLY),2,'','callable view contract'),
    'wrong_kind': (query(prefix='proc declared(input:T)->const T where T;'+APPLY),2,'','callable view contract'),
    'checked_failure_preserved': (query(prefix=NPE+'func declared(input:T)->const T throws NullPointerException where T;'+APPLY),2,'','required checked exception'),
    'checked_failure_allowed': (query(prefix=NPE+'func declared(input:T)->const T throws NullPointerException where T;func inspect(callback:func(T)->const T throws NullPointerException,input:T)->const T where T;'),0,'7\n',''),
    'procedure_signature': (query(prefix='proc declared(input:T)->const T where T;func inspect(callback:proc(T)->const T,input:T)->const T where T;'),0,'7\n',''),
    'void_signature': (query('inspect(declared)','true','func declared()->void;func inspect(callback:func()->void)->bool;'),0,'true\n',''),
    'selected_signature_value_call': (query('(if(true){declared}else{declared})(1u64)','7u64','func declared(input:u64)->u64;'),0,'7\n',''),
    'unused_checked_input_obligation': (query('inspect(declared,1u64)','7u64',NPE+'func declared(input:u8)->u64 throws NullPointerException;func declared(input:u64)->u64;'+APPLY),2,'','required checked exception'),
    'no_overload_signature_guess': (query('inspect(declared)',prefix='func declared(input:u64)->u64;func declared(input:bool)->bool;func inspect(callback:F)->i64 where F;'),2,'','known declaration or operand type'),
    'borrowed_signature': (query('inspect(declared,model)',prefix='func declared(ref input:const i64 in O)->ref const i64 in O where {owner O};func model(ref input:const i64 in P)->ref const i64 in P where {owner P}{return input;}'+MODEL),0,'7\n',''),
    'nullable_signature': (query('inspect(declared,model)',prefix='func declared(ptr input:i64 in O)->ptr i64 in O where {owner O};func model(ptr input:i64 in P)->ptr i64 in P where {owner P}{return input;}'+MODEL),0,'7\n',''),
    'common_exception_borrow': (query('inspect(declared,model)',prefix=ERRORS+'func declared(ref input:Exception in O)->ref Exception in O where {owner O};func model(ref input:Exception in P)->ref Exception in P where {owner P}{return input;}'+MODEL),0,'7\n',''),
    'dependent_exception_signature_only': (query('inspect(declared,held)',prefix=ERRORS+ID+'func model(input:Exception)->const Exception{return input;}func held=model;'+MODEL),0,'7\n',''),
    'exception_domain_incomplete': (query('inspect(declared,held)',prefix=ERRORS+'func declared(input:MissingReturnError)->const Exception;func model(input:Exception)->const Exception{return input;}func held=model;'+MODEL),2,'','no applicable declared signature'),
    'borrowed_upcast_not_invented': (query('inspect(declared,held)',prefix=ERRORS+'func declared(ref input:Exception in O)->ref Exception in O where {owner O};func model(ref input:MissingReturnError in P)->ref MissingReturnError in P where {owner P}{return input;}func held=model;'+MODEL),2,'','borrowed target adaptation'),
    'owner_contract_mismatch': (query('inspect(declared,held)',prefix='func declared(ref a:i64 in A,ref b:i64 in B)->ref i64 in A where {owner A,owner B};func model(ref a:i64 in A,ref b:i64 in B)->ref i64 in B where {owner A,owner B}{return b;}func held=model;'+MODEL),2,'','callable view contract'),
    'unknown_owner_relation': (query('inspect(declared,held)',prefix='func declared(ref a:T in O,ref b:T in O)->const T where {T,owner O};func model(ref a:i64 in A,ref b:i64 in B)->i64 where {owner A,owner B}{return a;}func held=model;'+MODEL),2,'','unproved owner equality'),
    'missing_body_at_execution': (ID+'func inspect(callback:func(i64)->const i64,input:i64)->i64{return callback(input);}inspect(declared,1);',2,'','no applicable implementation'),
    'missing_body_at_materialization': ('func declared(input:i64)->i64;func selected=declared;',2,'','no applicable implementation'),
    'no_open_signature_guess': (query('inspect(declared)',prefix=ID+'func inspect(callback:F)->i64 where F;'),2,'','known declaration or operand type'),
    'nearest_binding_preserved': (query(prefix=ID+APPLY+'val declared<-true;'),2,'','no applicable declared signature'),
    'retained_context': (IO+ID+APPLY+'func outer(input:T)->const T where T{func inner(item:T)->const T{val result<-make local typeof(inspect(declared,item)){item};return result;};return inner(input);}behave @print(outer(8));behave @print(outer(true));',0,'8\ntrue\n',''),
    'imported_signature': ({'main.cod':query(prefix='using "contract.cod" published as declared;'+APPLY), 'contract.cod':ID.replace('declared','published')+'export published;'},0,'7\n',''),
    'imported_private_signature': ({'main.cod':IO+'using "contract.cod" outer;func declared(input:bool)->bool;behave @print(outer(9));',
        'contract.cod':ID+APPLY+'func outer(input:T)->const T where T{val result<-make local typeof(inspect(declared,input)){input};return result;}export outer;'},0,'9\n',''),
    'raw_construction_boundary': (IO+'func declared(input:u64)->u64;func increment(input:u64)->u64{return input+1u64;}func selected=make local typeof(declared){increment};behave @print(selected(7u64));',2,'','raw callable storage construction'),
}
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
