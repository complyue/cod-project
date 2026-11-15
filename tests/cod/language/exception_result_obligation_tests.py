#!/usr/bin/env python3
"""Dependent declaration results require category evidence beyond input coverage."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
TYPES = 'using "std:exceptions" SystemException;using "std:exceptions" NullPointerException;using "std:exceptions" MissingReturnException;'
ERROR = TYPES+'func missing()->u64{}func missing_value()->const MissingReturnException{try{missing();}catch(caught:const MissingReturnException){return caught;};}'
DECL = 'func f(input:T)->const T where T;'
GAP = 'declared result type parameter is not determined'
OUT = lambda text: 'behave @print('+text+');'
runner.CASES = {
 'broad_explicit_result': (TYPES+DECL+'func f(input:SystemException)->const SystemException{return input;}',2,'',GAP),
 'broad_inferred_result': (TYPES+DECL+'func f(input:SystemException){return input;}',2,'',GAP),
 'broad_fixed_category_result': (ERROR+DECL+'func f(input:SystemException)->const MissingReturnException{return missing_value();}',2,'',GAP),
 'later_declaration': (TYPES+'func f(input:SystemException)->const SystemException{return input;}'+DECL,2,'',GAP),
 'generic_result_cannot_replace_category': (TYPES+DECL+'func f(input:U)->const U where U{return input;}func relay(input:SystemException)->const SystemException{return f(input);}',2,'',GAP),
 'generic_body_cannot_change_category': (ERROR+DECL+'func f(input:U)->const U where U{return missing_value();}func relay(input:SystemException)->const SystemException{return f(input);}',2,'',GAP),
 'generic_inferred_result': (TYPES+DECL+'func f(input:U) where U{return input;}func relay(input:SystemException)->const SystemException{return f(input);}',2,'',GAP),
 'common_copy_cannot_establish_category': (ERROR+DECL+'func f(input:U)->const U where U{return input;}val common:SystemException<-missing_value();f(common);',2,'',GAP),
 'untyped_catch_cannot_establish_category': (ERROR+DECL+'func f(input:U)->const U where U{return input;}try{missing();{}}catch(caught){f(caught);{}};',2,'',GAP),
 'recursive_contract_cannot_assume_category': (TYPES+DECL+'func f(input:U) where U{return f(input);}func relay(input:SystemException)->const SystemException{return f(input);}',2,'',GAP),
 'deferred_preparation_cannot_assume_category': (TYPES+DECL+'func f(input:U)->const U where U{return input;}proc relay(input:SystemException)->void{defer f(input);}',2,'',GAP),
 'operator_result_cannot_assume_category': (TYPES+'func (+)(input:T,flag:bool)->const T where T;func (+)(input:U,flag:bool)->const U where U{return input;}func relay(input:SystemException)->const SystemException{return input+true;}',2,'',GAP),
 'procedure_result_cannot_assume_category': (TYPES+DECL.replace('func','proc')+'proc f(input:SystemException)->const SystemException{return input;}',2,'',GAP),
 'nested_result_cannot_assume_category': (TYPES+'func f(input:T)->func()->const T where T;func f(input:SystemException)->func()->const SystemException{return func()->const SystemException{return input;};}',2,'',GAP),
 'fixed_callback_input_preserves_unknown_result': (TYPES+'func f(input:T,callback:func()->bool)->const T where T;func f(input:SystemException,callback:func()->bool)->const SystemException{return input;}',2,'',GAP),
 'independent_binder_result_cannot_assume_category': (TYPES+'func f(first:T,second:U)->const T where {T,U};func f(first:SystemException,second:SystemException)->const SystemException{return first;}',2,'',GAP),
 'variadic_result_cannot_assume_category': (TYPES+'func f(inputs:T...)->const T where T;func f(first:SystemException,second:SystemException)->const SystemException{return first;}',2,'',GAP),
 'concrete_generic_result': (IO+ERROR+DECL+'func f(input:U)->const U where U{return input;}'+OUT('f(missing_value()).category'),0,'5\n','[-Wmissing-return]'),
 'concrete_repeated_generic_result': (IO+ERROR+'func f(first:T,second:T)->const T where T;func f(first:U,second:U)->const U where U{return second;}'+OUT('f(missing_value(),missing_value()).category'),0,'5\n','[-Wmissing-return]'),
 'concrete_typed_catch_result': (IO+ERROR+DECL+'func f(input:U)->const U where U{return input;}try{missing();{}}catch(caught:const MissingReturnException){'+OUT('f(caught).category')+'};',0,'5\n','[-Wmissing-return]'),
 'broad_fixed_result': (IO+ERROR+'func f(input:T)->const SystemException where T;func f(input:SystemException)->const SystemException{return input;}'+OUT('f(missing_value()).category'),0,'5\n','[-Wmissing-return]'),
 'fixed_result_anchors_recursive_body': (TYPES+'func f(input:T)->bool where T;func f(input:SystemException){return f(input);}func selected=f;',0,'',''),
 'borrowed_common_target_type': (IO+ERROR+'func f(ref input:const T in O)->ref const T in O where {T,owner O};func f(ref input:const SystemException in P)->ref const SystemException in P where {owner P}{return input;}val common:SystemException<-missing_value();'+OUT('f(common).category'),0,'5\n','[-Wmissing-return]'),
 'concrete_other_input': (IO+ERROR+'func f(first:T,second:T)->const T where T;func f(first:SystemException,second:MissingReturnException)->const MissingReturnException{return second;}val common:SystemException<-missing_value();'+OUT('f(common,missing_value()).category'),0,'5\n','[-Wmissing-return]'),
 'imported_result_promise': ({'main.cod':TYPES+'import "./promise.cod" f;func f(input:SystemException)->const SystemException{return input;}','promise.cod':DECL+'export f;'},2,'',GAP),
}
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
