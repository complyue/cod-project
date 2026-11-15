#!/usr/bin/env python3
"""Recursive result obligations retain declaration evidence across body shadows."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
BASE = 'func number(x:u64)->u64{return x+2u64;}func truth(x:bool)->bool{return x;}'
OUT = lambda value: 'behave @print('+value+');'
RUN = lambda body: IO+'proc run()->void{'+body+'}run();'
ANCHOR = 'val value<-make local typeof(sample){9u64};return value;'
STEP = '{{val sample<-false;return recur(n-1u64);}}'
runner.CASES = {}
runner.CASE_SHADOWS = {}
for kind in ('func','proc'):
    for reverse in (False,True):
        name = kind+('_recursive_first' if reverse else '_anchor_first')
        body = 'if(n!=0u64){'+STEP+'}else{'+ANCHOR+'};' if reverse else 'if(n==0u64){'+ANCHOR+'}else{'+STEP+'};'
        runner.CASES[name] = (RUN('val sample<-7u64;'+kind+' recur(n:u64){'+body+'};'+OUT('recur(20u64)')),0,'9\n','')
        runner.CASE_SHADOWS[name] = ['sample']

runner.CASES.update({
 'captured_callable_shadow': (IO+BASE+'proc run()->void{func f=number;func recur[f](n:u64){if(n==0u64){return f(5u64);}else{{{func f=truth;return recur(n-1u64);}}};};'+OUT('recur(20u64)')+'}run();',0,'7\n',''),
 'capture_and_query_shadow': (IO+BASE+'proc run()->void{val sample<-7u64;func f=number;func recur[f](n:u64){if(n==0u64){val value<-make local typeof(sample){5u64};return f(value);}else{{{val sample<-false;func f=truth;return recur(n-1u64);}}};};'+OUT('recur(20u64)')+'}run();',0,'7\n',''),
 'nested_definition_sees_body_scope': (RUN('val sample<-7u64;func recur(n:u64){if(n==0u64){'+ANCHOR+'}else{{{val sample<-false;func inner(){val value<-make local typeof(sample){true};return value;};if(inner()){return recur(n-1u64);}else{return 0u64;};}}};};'+OUT('recur(20u64)')),0,'9\n',''),
 'generic_declaration_contexts': (IO+'func factory(sample:T) where T{func recur(value:T,n:u64){if(n==0u64){val saved<-make local typeof(sample){value};return saved;}else{{{val sample<-0f64;return recur(value,n-1u64);}}};};return recur;}'
     +OUT('factory(1u64)(7u64,20u64)')+OUT('factory(false)(true,20u64)')+OUT('factory(1u64)(9u64,20u64)'),0,'7\ntrue\n9\n',''),
 'required_query_does_not_execute': (RUN('val sample<-7u64;proc recur(n:u64){'+OUT('99u64')+'if(n==0u64){'+ANCHOR+'}else{'+STEP+'};};val result<-make local typeof(recur(20u64)){8u64};'+OUT('result')),0,'8\n',''),
 'explicit_result_control': (RUN('val sample<-7u64;func recur(n:u64)->u64{if(n==0u64){'+ANCHOR+'}else{'+STEP+'};};'+OUT('recur(20u64)')),0,'9\n',''),
 'symbol_lookup_context': (IO+'symbol @sample:u64;effect @sample<-7u64;proc run()->void{proc recur(n:u64){if(n==0u64){return @sample;}else{{{val sample<-false;return recur(n-1u64);}}};};'+OUT('recur(20u64)')+'}run();',0,'7\n',''),
 'uninitialized_query_source': (RUN('val sample<-make local u64;func recur(n:u64){if(n==0u64){'+ANCHOR+'}else{'+STEP+'};};'+OUT('recur(20u64)')),0,'9\n',''),
 'query_does_not_initialize_source': ('proc run()->void{val sample<-make local u64;func recur(n:u64){if(n==0u64){'+ANCHOR+'}else{'+STEP+'};};recur(20u64);sample;}run();',2,'','before definite initialization'),
 'shadow_cannot_anchor_recursive_cycle': ('proc run()->void{val sample<-7u64;func recur(n:u64){val value<-make local typeof(sample){9u64};{{val sample<-false;return recur(n-1u64);}};};}',2,'','unproved callable result'),
 'dependent_return_still_checked': ('proc run()->void{val sample<-7u64;func recur(n:u64){if(n==0u64){'+ANCHOR+'}else{{{val sample<-false;return recur(n-1u64)&&sample;}}};};}',2,'','logical operation requires bool operands'),
 'recursive_runtime_value_still_unproved': ('proc run()->void{func recur(n:u64)->u64{func copy=recur;return n;};}',2,'','materialization of the current lexical callable'),
})
runner.CASE_SHADOWS.update({
 'captured_callable_shadow':['f'], 'capture_and_query_shadow':['sample','f'],
 'nested_definition_sees_body_scope':['sample'], 'generic_declaration_contexts':['sample'],
 'required_query_does_not_execute':['sample'], 'explicit_result_control':['sample'],
 'symbol_lookup_context':['sample'], 'uninitialized_query_source':['sample'],
})
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
