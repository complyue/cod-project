#!/usr/bin/env python3
"""Exception bindings preserve immutable bits, causes and checked obligations."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
TYPES = 'using "std:exceptions" MissingReturnException;using "std:exceptions" SystemException;using "std:exceptions" NullPointerException;using "std:exceptions" OutOfMemory;'
ERROR = 'func missing()->u64{}func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}'
CHAIN = 'func chain(e:MissingReturnException,c:MissingReturnException)->const MissingReturnException{return try{throw e from c;}catch(x:const MissingReturnException){x};}'
NULL_ERROR = 'func npe()->const NullPointerException{ptr p:i64=null in here;return try{val v<-p;throw error();}catch(e:const NullPointerException){e};}'
S = 'symbol @saved:MissingReturnException;'
COMMON = 'symbol @saved:SystemException;'
P = 'symbol @p:proc()->u64;proc depth()->u64{return @saved.cause_count;}'
runner.CASES = {
    'payload': (IO+TYPES+ERROR+S+'effect @saved<-error();behave @print(@saved.category);behave @print(@saved.site>0u64);behave @print(@saved.requested);behave @print(@saved.available);behave @print(@saved.cause_count);', 0, '5\ntrue\n0\n0\n0\n', ''),
    'common_slot': (IO+TYPES+ERROR+COMMON+'effect @saved<-error();try{throw @saved;}catch(e:const MissingReturnException){behave @print(e.category);}catch(e){behave @print(99u64);};', 0, '5\n', ''),
    'copied_bits': (IO+TYPES+ERROR+CHAIN+S+'effect @saved<-chain(error(),error());val kept<-@saved;effect @saved<-error();behave @print(kept.cause_count);behave @print(@saved.cause_count);', 0, '1\n0\n', ''),
    'retained_cause': (IO+TYPES+ERROR+CHAIN+S+'proc install()->const MissingReturnException{return chain(error(),chain(error(),error()));}'
        'effect @saved<-install();behave @print(@saved.cause_count);behave @print(cause_at(@saved,1u64).category);', 0, '2\n5\n', ''),
    'scope_restore': (IO+TYPES+ERROR+CHAIN+S+'effect @saved<-error();{{effect @saved<-chain(error(),error());behave @print(@saved.cause_count);}};behave @print(@saved.cause_count);', 0, '1\n0\n', ''),
    'exception_restore': (IO+TYPES+ERROR+CHAIN+S+'effect @saved<-error();try{{{effect @saved<-chain(error(),error());throw @saved;}}}catch(e:const MissingReturnException){behave @print(e.cause_count);behave @print(@saved.cause_count);};', 0, '1\n0\n', ''),
    'return_restore': (IO+TYPES+ERROR+CHAIN+S+'proc get()->const MissingReturnException{effect @saved<-chain(error(),error());return @saved;}'
        'effect @saved<-error();val kept<-get();behave @print(kept.cause_count);behave @print(@saved.cause_count);', 0, '1\n0\n', ''),
    'iteration_restore': (IO+TYPES+ERROR+CHAIN+S+'effect @saved<-error();for(i in 0u64..2u64){effect @saved<-chain(error(),error());behave @print(@saved.cause_count);};behave @print(@saved.cause_count);', 0, '1\n1\n0\n', ''),
    'prepared_defer_snapshot': (IO+TYPES+ERROR+CHAIN+S+'proc show(e:MissingReturnException)->void{behave @print(e.cause_count);}'
        'effect @saved<-chain(error(),error());defer show(@saved);effect @saved<-error();', 0, '1\n', ''),
    'delayed_defer_current': (IO+TYPES+ERROR+CHAIN+S+'effect @saved<-chain(error(),error());defer{behave @print(@saved.cause_count);};effect @saved<-error();', 0, '0\n', ''),
    'cleanup_replaces_exception': (IO+TYPES+ERROR+CHAIN+S+'proc fail()->void{effect @saved<-chain(error(),error());defer{throw @saved;};throw error();}'
        'try{fail();}catch(e:const MissingReturnException){behave @print(e.cause_count);};', 0, '1\n', ''),
    'current_and_captured': (IO+TYPES+ERROR+CHAIN+S+P+'effect @saved<-chain(error(),error());effect @p=depth;effect @saved<-error();'
        'behave @print(behave @p());behave @print(perform @p());behave @print(@saved.cause_count);', 0, '0\n1\n0\n', ''),
    'capture_outlives_binding_scope': (IO+TYPES+ERROR+CHAIN+COMMON+P+'proc build()->proc()->u64{effect @saved<-chain(error(),error());return depth;}'
        'effect @p=build();behave @print(perform @p());', 0, '1\n', ''),
    'nested_capture': (IO+TYPES+ERROR+CHAIN+COMMON+P+'symbol @q:proc()->u64;proc replay()->u64{return perform @p();}'
        'proc build()->proc()->u64{effect @saved<-chain(error(),error());effect @p=depth;return replay;}'
        'effect @q=build();behave @print(perform @q());', 0, '1\n', ''),
    'touch_capture': (IO+TYPES+ERROR+S+'symbol @p:proc()->u64;proc touched()->u64{touch @saved;return 9u64;}'
        'proc build()->proc()->u64{effect @saved<-error();return touched;}effect @p=build();behave @print(perform @p());', 0, '9\n', ''),
    'generic_transport': (IO+TYPES+ERROR+S+'func keep(x:T)->const T where T{return x;}effect @saved<-keep(error());val result<-keep(@saved);behave @print(result.category);', 0, '5\n', ''),
    'declaration_query': (IO+TYPES+S+'proc read()->const MissingReturnException{return @saved;}val n<-make local typeof(read().category){7u64};behave @print(n);', 0, '7\n', ''),
    'failed_rhs_preserves_binding': (IO+TYPES+ERROR+CHAIN+S+'effect @saved<-chain(error(),error());try{effect @saved<-{throw error();};}catch(e:const MissingReturnException){behave @print(@saved.cause_count);};', 0, '1\n', ''),
    'rhs_effect_survives_failure': (IO+TYPES+ERROR+CHAIN+S+'effect @saved<-chain(error(),error());try{effect @saved<-{effect @saved<-error();throw error();};}catch(e:const MissingReturnException){behave @print(@saved.cause_count);};', 0, '0\n', ''),
    'chain_preserves_original': (IO+TYPES+ERROR+CHAIN+S+'effect @saved<-error();val kept<-@saved;try{throw @saved from @saved;}catch(e:const MissingReturnException){behave @print(e.cause_count);};behave @print(kept.cause_count);behave @print(@saved.cause_count);', 0, '1\n0\n0\n', ''),
    'unbound_read': (TYPES+S+'@saved;', 2, '', 'cannot prove an installed binding'),
    'unbound_touch': (TYPES+S+'touch @saved;', 2, '', 'cannot prove an installed binding'),
    'unbound_capture': (TYPES+S+P+'effect @p=depth;', 2, '', 'cannot prove an installed binding'),
    'wrong_operator': (TYPES+ERROR+S+'effect @saved=error();', 2, '', "bits symbols require effect '<-'"),
    'wrong_category': (TYPES+ERROR+'symbol @saved:OutOfMemory;effect @saved<-error();', 2, '', 'effect value does not satisfy'),
    'ordinary_write_rejects': (TYPES+ERROR+S+'effect @saved<-error();@saved<-error();', 2, '', 'assignment requires a storage destination'),
    'function_read_rejects': (TYPES+S+'func get()->const MissingReturnException{return @saved;}', 2, '', 'function cannot access dynamic bindings'),
    'checked_throw_rejects': (TYPES+'symbol @saved:NullPointerException;proc fail()->void{throw @saved;}', 2, '', 'requires a covering catch or throws declaration'),
    'checked_common_rejects': (TYPES+COMMON+'proc fail()->void{throw @saved;}', 2, '', 'requires a covering catch or throws declaration'),
    'checked_dynamic_body_rejects': (TYPES+'symbol @saved:NullPointerException;proc build()->void{proc fail=proc()->void{throw @saved;};}', 2, '', 'requires a covering catch or throws declaration'),
    'checked_defer_rejects': (TYPES+'symbol @saved:NullPointerException;proc fail()->void{defer{throw @saved;};}', 2, '', 'requires a covering catch or throws declaration'),
    'checked_declaration_accepts': (TYPES+'symbol @saved:NullPointerException;proc fail()->void throws NullPointerException{throw @saved;}', 0, '', ''),
    'checked_catch_accepts': (TYPES+'symbol @saved:NullPointerException;proc fail()->void{try{throw @saved;}catch(e:const NullPointerException){};}', 0, '', ''),
    'checked_native_delivery': (IO+TYPES+ERROR+NULL_ERROR+'symbol @saved:NullPointerException;proc fail()->void throws NullPointerException{throw @saved;}'
        'effect @saved<-npe();try{fail();}catch(e:const NullPointerException){behave @print(e.category);};', 0, '4\n', ''),
    'checked_captured_delivery': (IO+TYPES+ERROR+NULL_ERROR+'symbol @saved:NullPointerException,@p:proc()->void throws NullPointerException;'
        'proc fail()->void throws NullPointerException{throw @saved;}effect @saved<-npe();effect @p=fail;'
        'try{perform @p();}catch(e:const NullPointerException){behave @print(e.category);};', 0, '4\n', ''),
    'checked_perform_requires_handler': (TYPES+ERROR+NULL_ERROR+'symbol @saved:NullPointerException,@p:proc()->void throws NullPointerException;'
        'proc fail()->void throws NullPointerException{throw @saved;}effect @saved<-npe();effect @p=fail;perform @p();', 2, '', 'requires a covering catch or throws declaration'),
    'common_preserves_actual_category': (IO+TYPES+ERROR+NULL_ERROR+COMMON+'effect @saved<-error();behave @print(@saved.category);effect @saved<-npe();'
        'try{throw @saved;}catch(e:const NullPointerException){behave @print(e.category);}catch(e){behave @print(99u64);};', 0, '5\n4\n', ''),
    'deferred_checked_recovery': (IO+TYPES+ERROR+NULL_ERROR+'symbol @saved:NullPointerException;'
        'proc recovered()->u64{defer catch(e:const NullPointerException){return e.category;};throw @saved;}'
        'effect @saved<-npe();behave @print(recovered());', 0, '4\n', ''),
}
for name, (source, status, output, diagnostic) in list(runner.CASES.items()):
    if ERROR in source and status == 0:
        runner.CASES[name] = (source, status, output, '[-Wmissing-return]')
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
