#!/usr/bin/env python3
"""Checked unknown-type contracts remain explicit across calls and cleanup."""
from pathlib import Path
import foundation_tests as runner
IO='using symbol "std:io" @print;'
UNKNOWN='using "std:exceptions" StaticallyUnknownTypeException;'
TYPES=UNKNOWN+'using "std:exceptions" NullPointerException;using "std:exceptions" Exception;using "std:exceptions" UnsoundRuntimeTypingError;'
TWICE='func twice(x:T)->const T throws StaticallyUnknownTypeException where T{return x+x;}'
PLAIN='func twice(x:T)->const T where T{return x+x;}'
ONE='func one()->const i64 throws StaticallyUnknownTypeException{return 1;}'
OBS=lambda x:'behave @print('+x+');'
CATCH=lambda x:'try{0;'+x+'{}}catch(e:const StaticallyUnknownTypeException){};'
runner.CASES={
 'known_generic_needs_no_declaration':(IO+PLAIN+OBS('twice(3u64)'),0,'6\n',''),
 'known_operation_error_is_static':(PLAIN+'twice(true);',2,'','no applicable numeric operation'),
 'same_t_mismatch_is_static':('func same(a:T,b:T)->const T where T{return a;}same(1u64,true);',2,'','same-T relation'),
 'selected_category':(UNKNOWN+'try{}catch(e:const StaticallyUnknownTypeException){};',0,'',''),
 'selected_alias':('using "std:exceptions" StaticallyUnknownTypeException as Unknown;func one()->const i64 throws Unknown{return 1;}try{one();{}}catch(e:const Unknown){};',0,'',''),
 'imported_category':('import "std:exceptions" StaticallyUnknownTypeException;func one()->const i64 throws StaticallyUnknownTypeException{return 1;}try{one();{}}catch(e:const StaticallyUnknownTypeException){};',0,'',''),
 'voluntary_declaration_covered':(IO+UNKNOWN+TWICE+CATCH(OBS('twice(3u64)')),0,'6\n',''),
 'voluntary_declaration_not_waived':(UNKNOWN+TWICE+'twice(3u64);',2,'',"checked exception 'StaticallyUnknownTypeException'"),
 'wrong_category_does_not_cover':(TYPES+ONE+'try{one();{}}catch(e:const NullPointerException){};',2,'',"checked exception 'StaticallyUnknownTypeException'"),
 'catch_all_covers':(IO+UNKNOWN+ONE+'try{'+OBS('one()')+'}catch(e){};',0,'1\n',''),
 'recursive_contract':(IO+UNKNOWN+TWICE+'func run(x:T,n:i64)->const T throws StaticallyUnknownTypeException where T{if(n==0){return twice(x);}else{return run(x,n-1);};}'+CATCH(OBS('run(3u64,40)')),0,'6\n',''),
 'mutual_contract':(IO+UNKNOWN+'func even(n:i64)->const bool throws StaticallyUnknownTypeException{if(n==0){return true;}else{return odd(n-1);};}func odd(n:i64)->const bool throws StaticallyUnknownTypeException{if(n==0){return false;}else{return even(n-1);};}'+CATCH(OBS('even(40)')),0,'true\n',''),
 'inferred_callable_retains_throws':(IO+UNKNOWN+ONE+'func f=one;'+CATCH(OBS('f()')),0,'1\n',''),
 'inferred_callable_requires_cover':(UNKNOWN+ONE+'func f=one;f();',2,'',"checked exception 'StaticallyUnknownTypeException'"),
 'explicit_callable_view':(IO+UNKNOWN+ONE+'func f:func()->const i64 throws StaticallyUnknownTypeException=one;'+CATCH(OBS('f()')),0,'1\n',''),
 'declared_view_adds_obligation':(UNKNOWN+'func one()->const i64{return 1;}func f:func()->const i64 throws StaticallyUnknownTypeException=one;f();',2,'',"checked exception 'StaticallyUnknownTypeException'"),
 'declared_view_adds_covered_obligation':(IO+UNKNOWN+'func one()->const i64{return 1;}func f:func()->const i64 throws StaticallyUnknownTypeException=one;'+CATCH(OBS('f()')),0,'1\n',''),
 'declared_view_cannot_erase':(UNKNOWN+ONE+'func f:func()->const i64=one;',2,'','required checked exception'),
 'captured_callable':(IO+UNKNOWN+ONE+'func wrap(f:func()->const i64 throws StaticallyUnknownTypeException)->const (func()->const i64 throws StaticallyUnknownTypeException){return func[f]()->const i64 throws StaticallyUnknownTypeException{return f();};}func f=wrap(one);func g=f;'+CATCH(OBS('g()')),0,'1\n',''),
 'generic_callable':(IO+UNKNOWN+ONE+'func apply(f:F)->const i64 throws StaticallyUnknownTypeException where F{return f();}'+CATCH(OBS('apply(one)')),0,'1\n',''),
 'generic_caller_missing_declaration':(UNKNOWN+ONE+'func apply(f:F)->const i64 where F{return f();}try{apply(one);{}}catch(e:const StaticallyUnknownTypeException){};',2,'',"checked exception 'StaticallyUnknownTypeException'"),
 'deferred_declared_call':(IO+UNKNOWN+'proc show(x:i64)->void throws StaticallyUnknownTypeException{'+OBS('x')+'}'+CATCH('{{defer show(1);defer show(2);}};'),0,'2\n1\n',''),
 'registration_catch_not_invocation_cover':(UNKNOWN+'proc f()->void throws StaticallyUnknownTypeException{}try{defer f();}catch(e:const StaticallyUnknownTypeException){};',2,'',"checked exception 'StaticallyUnknownTypeException'"),
 'deferred_handler_covers':(IO+UNKNOWN+'proc show(x:i64)->void throws StaticallyUnknownTypeException{'+OBS('x')+'}{{defer catch(e:const StaticallyUnknownTypeException){}finally{};defer show(3);}};',0,'3\n',''),
 'deferred_selected_callable':(IO+UNKNOWN+'proc show(x:i64)->void throws StaticallyUnknownTypeException{'+OBS('x')+'}proc f=show;'+CATCH('{{defer f(4);}};'),0,'4\n',''),
 'exact_throw_requires_declaration':(UNKNOWN+'func f(e:StaticallyUnknownTypeException)->void{throw e;}',2,'',"checked exception 'StaticallyUnknownTypeException'"),
 'exact_throw_declaration':(UNKNOWN+'func f(e:StaticallyUnknownTypeException)->void throws StaticallyUnknownTypeException{throw e;}',0,'',''),
 'exact_throw_caught':(UNKNOWN+'func f(e:StaticallyUnknownTypeException)->const u64{try{throw e;}catch(x:const StaticallyUnknownTypeException){return x.category;};}',0,'',''),
 'bare_rethrow_requires_cover':(UNKNOWN+'func f(e:StaticallyUnknownTypeException)->void{try{throw e;}catch(x:const StaticallyUnknownTypeException){throw;};}',2,'',"checked exception 'StaticallyUnknownTypeException'"),
 'deferred_exact_throw_recovery':(UNKNOWN+'func f(e:StaticallyUnknownTypeException)->const i64{defer catch(x:const StaticallyUnknownTypeException){return 9;}finally{};defer{throw e;};return 1;}',0,'',''),
 'common_throw_requires_declared_root':(TYPES+'func f(e:Exception)->void throws NullPointerException{throw e;}',2,'',"declared checked exception 'Exception'"),
 'common_throw_declares_all':(TYPES+'func f(e:Exception)->void throws Exception{throw e;}',0,'',''),
 'common_declaration_requires_root_cover':(TYPES+'func f()->void throws Exception{}try{f();}catch(e:const NullPointerException){};',2,'',"declared checked exception 'Exception'"),
 'unchecked_typing_needs_no_cover':(TYPES+'func f(e:UnsoundRuntimeTypingError)->void{throw e;}',0,'',''),
 'unknown_is_distinct_from_typing':(TYPES+ONE+'try{one();{}}catch(e:const UnsoundRuntimeTypingError){};',2,'',"checked exception 'StaticallyUnknownTypeException'"),
 'alias_duplicate_handler':(UNKNOWN+'using "std:exceptions" StaticallyUnknownTypeException as Unknown;try{}catch(e:const StaticallyUnknownTypeException){}catch(e:const Unknown){};',2,'','duplicate exception category'),
}
runner.CASE_ARGUMENTS={}
runner.CASE_SHADOWS = {
    'captured_callable': ['f'],
}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
