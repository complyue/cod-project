#!/usr/bin/env python3
"""Definition identity, fresh captures, stored snapshots and nonexecuting queries."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
BITS = 'func one()->u64{return 1u64;}func two()->u64{return 2u64;}'
SYMBOLS = 'symbol @bias:u64;symbol @invoke:proc()->u64;'
def run(body):
    return 'proc demo()->void{' + body + '}demo();'
def observe(value):
    return 'behave @print(' + value + ');'
def perform(value):
    return 'effect @invoke=' + value + ';' + observe('perform @invoke()')

runner.CASES = {
    'local_symbolic_snapshots': (IO+SYMBOLS+run('proc read()->u64{return @bias;};effect @bias<-10u64;proc a=read;effect @bias<-20u64;proc b=read;proc c=a;'+perform('a')+perform('b')+perform('c')),0,'10\n20\n10\n',''),
    'definition_before_installation': (IO+SYMBOLS+run('proc read()->u64{return @bias;};effect @bias<-20u64;'+observe('read()')),0,'20\n',''),
    'unused_symbolic_definition': (SYMBOLS+run('proc read()->u64{return @bias;};'),0,'',''),
    'live_lexical_dependency': (IO+BITS+run('func source=one;func read[source]()->u64{return source();};func a=read;source<-two;func b=read;'+observe('a()')+observe('b()')+observe('read()')),0,'1\n2\n2\n',''),
    'inferred_lexical_dependency': (IO+BITS+run('func source=one;func read()->u64{return source();};func a=read;source<-two;func b=read;'+observe('a()')+observe('b()')),0,'1\n2\n',''),
    'definition_bound_shadow': (IO+BITS+run('func source=one;func read[source]()->u64{return source();};{{func source=two;func value=read;'+observe('value()')+observe('read()')+'}};'),0,'1\n1\n',''),
    'direct_target_prepared_before_operand': (IO+BITS+run('func source=one;func read[source](ignored:u64)->u64{return source();};'+observe('read({{source<-two;0u64}})')+observe('read(0u64)')),0,'1\n2\n',''),
    'deferred_target_snapshot': (IO+BITS+run('func source=one;proc read[source]()->void{'+observe('source()')+'};{{defer read();source<-two;}};read();'),0,'1\n2\n',''),
    'captured_definition_refreshes': (IO+BITS+run('func source=one;func first[source]()->u64{return source();};func second[first]()->u64{return first();};func a=second;source<-two;func b=second;'+observe('a()')+observe('b()')),0,'1\n2\n',''),
    'escaped_recursive_environment': (IO+BITS+'func factory(source:func()->u64)->func(u64)->u64{func recur[source](n:u64)->u64{if(n==0u64){return source();}else{};func next=recur;return next(n-1u64);};return recur;}func escaped=factory(one);'+observe('escaped(4u64)'),0,'1\n',''),
    'recursive_symbolic_refresh': (IO+'symbol @bias:u64;symbol @call:proc(u64)->u64;proc factory()->u64{effect @bias<-10u64;proc recur(n:u64)->u64{if(n==0u64){return @bias;}else{};effect @bias<-20u64;proc next=recur;effect @call=next;return perform @call(0u64);};effect @call=recur;return perform @call(1u64);}'+observe('factory()'),0,'20\n',''),
    'self_capture_ignores_inner_shadow': (IO+BITS+'func factory(source:func()->u64)->func(u64)->u64{func recur[source](n:u64)->u64{if(n==0u64){return source();}else{};{{func source=two;func next=recur;return next(0u64);}};};return recur;}func escaped=factory(one);'+observe('escaped(1u64)'),0,'1\n',''),
    'nested_self_definition_capture': (IO+BITS+'func factory(source:func()->u64)->func(u64)->u64{func recur[source](n:u64)->u64{if(n==0u64){return source();}else{};func step[recur]()->u64{return recur(0u64);};return step();};return recur;}func escaped=factory(one);'+observe('escaped(1u64)'),0,'1\n',''),
    'forwarded_self_definition': (IO+BITS+'func apply(fn:func(u64)->u64)->u64{return fn(0u64);}func factory(source:func()->u64)->func(u64)->u64{func recur[source](n:u64)->u64{if(n==0u64){return source();}else{return apply(recur);};};return recur;}func escaped=factory(two);'+observe('escaped(1u64)'),0,'2\n',''),
    'local_generic_definition': (IO+run('func identity(x:T)->const T where T{return x;};func value:func(u64)->const u64=identity;'+observe('identity(true)')+observe('value(7u64)')),0,'true\n7\n',''),
    'retained_generic_definition': (IO+'proc outer(x:T)->void where T{func identity(y:T)->const T{return y;};func value=identity;'+observe('value(x)')+'}outer(7u64);outer(true);',0,'7\ntrue\n',''),
    'inferred_result_context': (IO+BITS+run('func source=one;func read[source](){return source();};{{func source=func()->bool{return true;};func value=read;'+observe('value()')+'}};'),0,'1\n',''),
    'type_query_without_capture': (IO+SYMBOLS+run('proc read()->u64{return @bias;};val value<-make local typeof(read()){7u64};'+observe('value')),0,'7\n',''),
    'self_parameter_shadow': (IO+BITS+'func factory()->func(func()->u64)->u64{func recur(recur:func()->u64)->u64{return recur();};return recur;}func value=factory();'+observe('value(two)'),0,'2\n',''),
    'definition_has_no_slot': (BITS+run('func read()->u64{return 1u64;};read<-two;'),2,'','assignment requires a storage destination'),
    'definition_missing_symbol_at_use': (SYMBOLS+run('proc read()->u64{return @bias;};proc value=read;'),2,'','cannot prove an installed binding'),
    'unused_definition_body_checked': (run('func bad()->bool{return 1u64;};'),2,'','return type'),
    'explicit_capture_completeness': (BITS+run('func source=one;func read[]()->u64{return source();};'),2,'',"explicit capture list omits 'source'"),
    'ordinary_cell_capture_rejected': (run('val source<-1u64;func read[source]()->u64{return source;};'),2,'','capture of ordinary lexical cells'),
    'function_symbol_capture_rejected': (SYMBOLS+'func bad()->void{proc read()->u64{return @bias;};proc value=read;}bad();',2,'','a function cannot capture dynamic bindings'),
}
runner.CASES.update({
    'module_capture_identity': (IO+BITS+run('func read[one]()->u64{return one();};{{func one=two;func value=read;'+observe('value()')+observe('read()')+'}};'),0,'1\n1\n',''),
    'definition_capture_keeps_module_identity': (IO+BITS+run('func first[one]()->u64{return one();};func second[first]()->u64{return first();};{{func one=two;func value=second;'+observe('value()')+'}};'),0,'1\n',''),
    'loop_control_shadows_definition_in_query': (IO+'func test(){func item()->bool{return true;};for(item in 0u64..2u64){{val value<-make local typeof(item){3u64};return value;}};return 7u64;}'+observe('test()'),0,'3\n',''),
    'catch_shadows_definition_in_query': (IO+'using "std:exceptions" MissingReturnError;func missing()->u64{}proc test(){func item()->bool{return true;};try{missing();}catch(item:const MissingReturnError){val copy<-make local typeof(item){item};return copy.category;};return 99u64;}'+observe('test()'),0,'5\n','[-Wmissing-return]'),
})
runner.CASES.update({
    'unavailable_definition_direct_call': (IO+'proc test(b:bool)->void{if(b){func value()->u64{return 1u64;};}else{};'+observe('value()')+'}test(true);',2,'','not available on every reaching path'),
    'unavailable_definition_query': ('proc test(b:bool)->void{if(b){func value()->u64{return 1u64;};}else{};val result<-make local typeof(value()){7u64};}test(true);',2,'','not available on every reaching path'),
})
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {
    'module_capture_identity': ['one'],
    'definition_capture_keeps_module_identity': ['one'],
    'loop_control_shadows_definition_in_query': ['item'],
    'catch_shadows_definition_in_query': ['item'],
    'definition_bound_shadow': ['source'],
    'self_capture_ignores_inner_shadow': ['source'],
    'inferred_result_context': ['source'],
    'self_parameter_shadow': ['recur'],
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
