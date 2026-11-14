#!/usr/bin/env python3
"""Checked unknown-type contracts remain explicit across calls and cleanup."""
from pathlib import Path
import foundation_tests as runner
IO='using symbol "std:io" @print;'
UNKNOWN='using "std:exceptions" StaticallyUnknownType;'
TYPES=UNKNOWN+'using "std:exceptions" NullPointerException;using "std:exceptions" SystemException;using "std:exceptions" UnsoundRuntimeTyping;'
TWICE='func twice(x:T)->const T throws StaticallyUnknownType where T{return x+x;}'
PLAIN='func twice(x:T)->const T where T{return x+x;}'
ONE='func one()->const i64 throws StaticallyUnknownType{return 1;}'
OBS=lambda x:'behave @print('+x+');'
CATCH=lambda x:'try{0;'+x+'{}}catch(e:const StaticallyUnknownType){};'
runner.CASES={
 'known_generic_needs_no_declaration':(IO+PLAIN+OBS('twice(3u64)'),0,'6\n',''),
 'known_operation_error_is_static':(PLAIN+'twice(true);',2,'','no applicable numeric operation'),
 'same_t_mismatch_is_static':('func same(a:T,b:T)->const T where T{return a;}same(1u64,true);',2,'','same-T relation'),
 'selected_category':(UNKNOWN+'try{}catch(e:const StaticallyUnknownType){};',0,'',''),
 'selected_alias':('using "std:exceptions" StaticallyUnknownType as Unknown;func one()->const i64 throws Unknown{return 1;}try{one();{}}catch(e:const Unknown){};',0,'',''),
 'imported_category':('import "std:exceptions" StaticallyUnknownType;func one()->const i64 throws StaticallyUnknownType{return 1;}try{one();{}}catch(e:const StaticallyUnknownType){};',0,'',''),
 'voluntary_declaration_covered':(IO+UNKNOWN+TWICE+CATCH(OBS('twice(3u64)')),0,'6\n',''),
 'voluntary_declaration_not_waived':(UNKNOWN+TWICE+'twice(3u64);',2,'',"checked exception 'StaticallyUnknownType'"),
 'wrong_category_does_not_cover':(TYPES+ONE+'try{one();{}}catch(e:const NullPointerException){};',2,'',"checked exception 'StaticallyUnknownType'"),
 'catch_all_covers':(IO+UNKNOWN+ONE+'try{'+OBS('one()')+'}catch(e){};',0,'1\n',''),
 'recursive_contract':(IO+UNKNOWN+TWICE+'func run(x:T,n:i64)->const T throws StaticallyUnknownType where T{if(n==0){return twice(x);}else{return run(x,n-1);};}'+CATCH(OBS('run(3u64,40)')),0,'6\n',''),
 'mutual_contract':(IO+UNKNOWN+'func even(n:i64)->const bool throws StaticallyUnknownType{if(n==0){return true;}else{return odd(n-1);};}func odd(n:i64)->const bool throws StaticallyUnknownType{if(n==0){return false;}else{return even(n-1);};}'+CATCH(OBS('even(40)')),0,'true\n',''),
 'inferred_callable_retains_throws':(IO+UNKNOWN+ONE+'func f=one;'+CATCH(OBS('f()')),0,'1\n',''),
 'inferred_callable_requires_cover':(UNKNOWN+ONE+'func f=one;f();',2,'',"checked exception 'StaticallyUnknownType'"),
 'explicit_callable_view':(IO+UNKNOWN+ONE+'func f:func()->const i64 throws StaticallyUnknownType=one;'+CATCH(OBS('f()')),0,'1\n',''),
 'declared_view_adds_obligation':(UNKNOWN+'func one()->const i64{return 1;}func f:func()->const i64 throws StaticallyUnknownType=one;f();',2,'',"checked exception 'StaticallyUnknownType'"),
 'declared_view_adds_covered_obligation':(IO+UNKNOWN+'func one()->const i64{return 1;}func f:func()->const i64 throws StaticallyUnknownType=one;'+CATCH(OBS('f()')),0,'1\n',''),
 'declared_view_cannot_erase':(UNKNOWN+ONE+'func f:func()->const i64=one;',2,'','required checked exception'),
 'captured_callable':(IO+UNKNOWN+ONE+'func wrap(f:func()->const i64 throws StaticallyUnknownType)->const (func()->const i64 throws StaticallyUnknownType){return func[f]()->const i64 throws StaticallyUnknownType{return f();};}func f=wrap(one);func g=f;'+CATCH(OBS('g()')),0,'1\n',''),
 'generic_callable':(IO+UNKNOWN+ONE+'func apply(f:F)->const i64 throws StaticallyUnknownType where F{return f();}'+CATCH(OBS('apply(one)')),0,'1\n',''),
 'generic_caller_missing_declaration':(UNKNOWN+ONE+'func apply(f:F)->const i64 where F{return f();}try{apply(one);{}}catch(e:const StaticallyUnknownType){};',2,'',"checked exception 'StaticallyUnknownType'"),
 'deferred_declared_call':(IO+UNKNOWN+'proc show(x:i64)->void throws StaticallyUnknownType{'+OBS('x')+'}'+CATCH('{{defer show(1);defer show(2);}};'),0,'2\n1\n',''),
 'registration_catch_not_invocation_cover':(UNKNOWN+'proc f()->void throws StaticallyUnknownType{}try{defer f();}catch(e:const StaticallyUnknownType){};',2,'',"checked exception 'StaticallyUnknownType'"),
 'deferred_handler_covers':(IO+UNKNOWN+'proc show(x:i64)->void throws StaticallyUnknownType{'+OBS('x')+'}{{defer catch(e:const StaticallyUnknownType){}finally{};defer show(3);}};',0,'3\n',''),
 'deferred_selected_callable':(IO+UNKNOWN+'proc show(x:i64)->void throws StaticallyUnknownType{'+OBS('x')+'}proc f=show;'+CATCH('{{defer f(4);}};'),0,'4\n',''),
 'exact_throw_requires_declaration':(UNKNOWN+'func f(e:StaticallyUnknownType)->void{throw e;}',2,'',"checked exception 'StaticallyUnknownType'"),
 'exact_throw_declaration':(UNKNOWN+'func f(e:StaticallyUnknownType)->void throws StaticallyUnknownType{throw e;}',0,'',''),
 'exact_throw_caught':(UNKNOWN+'func f(e:StaticallyUnknownType)->const u64{try{throw e;}catch(x:const StaticallyUnknownType){return x.category;};}',0,'',''),
 'bare_rethrow_requires_cover':(UNKNOWN+'func f(e:StaticallyUnknownType)->void{try{throw e;}catch(x:const StaticallyUnknownType){throw;};}',2,'',"checked exception 'StaticallyUnknownType'"),
 'deferred_exact_throw_recovery':(UNKNOWN+'func f(e:StaticallyUnknownType)->const i64{defer catch(x:const StaticallyUnknownType){return 9;}finally{};defer{throw e;};return 1;}',0,'',''),
 'common_throw_requires_unknown_cover':(TYPES+'func f(e:SystemException)->void throws NullPointerException{throw e;}',2,'',"checked exception 'StaticallyUnknownType'"),
 'common_throw_declares_all':(TYPES+'func f(e:SystemException)->void throws SystemException{throw e;}',0,'',''),
 'common_declaration_requires_both':(TYPES+'func f()->void throws SystemException{}try{f();}catch(e:const NullPointerException){};',2,'',"checked exception 'StaticallyUnknownType'"),
 'unchecked_typing_needs_no_cover':(TYPES+'func f(e:UnsoundRuntimeTyping)->void{throw e;}',0,'',''),
 'unknown_is_distinct_from_typing':(TYPES+ONE+'try{one();{}}catch(e:const UnsoundRuntimeTyping){};',2,'',"checked exception 'StaticallyUnknownType'"),
 'alias_duplicate_handler':(UNKNOWN+'using "std:exceptions" StaticallyUnknownType as Unknown;try{}catch(e:const StaticallyUnknownType){}catch(e:const Unknown){};',2,'','duplicate exception category'),
}
runner.CASE_ARGUMENTS={}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
