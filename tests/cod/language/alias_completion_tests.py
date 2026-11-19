#!/usr/bin/env python3
"""Alias selectors preserve completion and publish only selected storage."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
OBS = IO+'proc observe(x:u64)->void{behave @print(x);return;}'
ERROR = NPE+'func error()->const NullPointerException{ptr p:u64=null in here;try{p;{}}catch(e:const NullPointerException){return e;};}'
runner.CASES = {}
for kind in ('var', 'val var', 'ref var', 'ptr var', 'const var'):
    for annotation in ('', ':u64'):
        alias = kind+' ignored'+annotation+'->'
        label = kind.replace(' ', '_')+('_typed' if annotation else '')
        runner.CASES[label+'_return'] = (OBS+'proc f()->u64{defer observe(1u64);'+alias+'{{defer observe(2u64);return 7u64;}};observe(99u64);return 0u64;}behave @print(f());',0,'2\n1\n7\n','')
        runner.CASES[label+'_throw'] = (OBS+ERROR+'proc f()->void throws NullPointerException{defer observe(1u64);'+alias+'{{throw error();}};observe(99u64);}try{f();}catch(e:const NullPointerException){observe(2u64);};',0,'1\n2\n','')

runner.CASES.update({
    'inferred_result': (IO+'func f(){var ignored->{{return 7u64;}};return 0u64;}behave @print(f());',0,'7\n',''),
    'unreachable_anchor_cannot_prove_recursion': ('func f(){var ignored->{{return f();}};return 1u64;}',2,'','unproved callable result'),
    'throw_only_has_no_result_anchor': (NPE+'func f(e:NullPointerException){var ignored->{{throw e;}};return 1u64;}',2,'','unproved callable result'),
    'void_return': (OBS+'proc f()->void{defer observe(1u64);var ignored->{{return;}};observe(99u64);}f();',0,'1\n',''),
    'finally_replaces_return': (IO+'func f()->u64{var ignored->try{return 7u64;}finally{return 8u64;};return 0u64;}behave @print(f());',0,'8\n',''),
    'mixed_selector_finally_returns': (IO+'func f(flag:bool)->u64{val x<-1u64;var ignored->try{if(flag){x}else{2u64}}finally{return 7u64;};return 0u64;}behave @print(f(true));behave @print(f(false));',0,'7\n7\n',''),
    'normal_val_alias_survives': (IO+'func f(flag:bool)->u64{val x<-7u64;val var selected->if(flag){return 9u64;}else{x};selected<-8u64;return x;}behave @print(f(true));behave @print(f(false));',0,'9\n8\n',''),
    'normal_ptr_alias_survives': (IO+'func f(flag:bool)->bool{val x<-7u64;ptr p=x;ptr var selected->if(flag){return false;}else{p};selected=null;return p is null;}behave @print(f(true));behave @print(f(false));',0,'false\ntrue\n',''),
    'generic_selector': (IO+'func f(val x:T,flag:bool)->const T where T{var selected->if(flag){return x;}else{x};return selected;}behave @print(f(7u64,true));behave @print(f(8u64,false));behave @print(f(true,true));behave @print(f(false,false));',0,'7\n8\ntrue\nfalse\n',''),
    'enclosing_call_preparation': (IO+'func first(x:u64,y:u64)->u64{return x;}func f()->u64{return first({var ignored->{{return 7u64;}};1u64},2u64);}behave @print(f());',0,'7\n',''),
    'nested_deferred_return': (OBS+'proc f()->u64{defer observe(1u64);defer{{var ignored->{{return 8u64;}};observe(99u64);}};return 7u64;}behave @print(f());',0,'1\n8\n',''),
    'iteration_return': (IO+'func f()->u64{for(i in 0..2){{var ignored->{{return 7u64;}};}};return 0u64;}behave @print(f());',0,'7\n',''),
    'no_binding_after_caught_exit': (ERROR+'func f()->u64{try{var missing->{{throw error();}};}catch(e:const NullPointerException){};return missing;}f();',2,'','unknown scalar value'),
    'normal_fresh_value_rejected': ('func f(flag:bool)->u64{var selected->if(flag){return 7u64;}else{1u64+2u64};return 0u64;}f(true);',2,'','alias requires an existing storage place'),
    'normal_kind_mismatch_rejected': ('func f(flag:bool)->u64{val x<-1u64;ptr var selected->if(flag){return 7u64;}else{x};return 0u64;}f(true);',2,'','alias kind must match'),
    'normal_readonly_target_rejected': ('func f(flag:bool)->u64{const val x<-1u64;var selected->if(flag){return 7u64;}else{x};selected<-2u64;return 0u64;}f(true);',2,'','readonly destination'),
    'normal_assignment_result_has_no_slot': ('func f(flag:bool)->u64{val x<-1u64;ptr p=x,q=x;var selected->if(flag){return 7u64;}else{p=q};return 0u64;}f(true);',2,'','alias of a non-binding storage place'),
    'annotation_name_still_checked': ('func f()->u64{var ignored:Unknown->{{return 7u64;}};return 0u64;}f();',2,'','non-value local storage type'),
    'owner_name_still_checked': ('func f()->u64{var ignored in Missing->{{return 7u64;}};return 0u64;}f();',2,'','owner'),
    'duplicate_name_still_checked': ('func f()->u64{val x<-1u64;var x->{{return 7u64;}};return 0u64;}f();',2,'',"duplicate declaration of 'x'"),
    'catch_name_still_checked': (ERROR+'func f()->u64{try{throw error();}catch(e:const NullPointerException){var e->{{return 7u64;}};};return 0u64;}f();',2,'',"duplicate declaration of catch parameter 'e'"),
    'deferred_scope_rule': ('func f()->u64{defer{var ignored->{{return 7u64;}};};return 0u64;}f();',2,'','requires a nested explicit'),
    'iteration_scope_rule': ('func f()->u64{for(i in 0..2){var ignored->{{return 7u64;}};};return 0u64;}f();',2,'','requires a nested per-iteration'),
})
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
