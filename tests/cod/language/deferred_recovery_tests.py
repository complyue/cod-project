#!/usr/bin/env python3
"""Scope-owned queues preserve completion identity through deferred recovery."""
from pathlib import Path
import sys
import foundation_tests as runner

PRELUDE = '''using symbol "std:io" @print;
using "std:exceptions" MissingReturnError;
using "std:exceptions" OutOfMemoryError;
using "std:exceptions" NullPointerException;
func missing()->const i64 {}
func error()->const MissingReturnError {try{missing():void;}catch(e:const MissingReturnError){return e;};}
proc observe(x:i64)->void{behave @print(x);return;}
'''
WARN = '[-Wmissing-return]'
CHECKED_READ = '''using "std:exceptions" NullPointerException;
using symbol "std:io" @print;

proc read(ptr value:const u64 in O) -> void throws NullPointerException where {const owner O} {
    behave @print(value);
    return;
}
'''
FIFO_FORWARDING = '''using "std:exceptions" { MissingReturnError, NullPointerException };
using symbol "std:io" @print;

func fail_missing_result() -> u64 {}
proc fail_first(ptr value:const u64 in O) -> void throws NullPointerException where {const owner O} {
    behave @print(value);
    return;
}
proc fail_last(ptr value:const u64 in O) -> void throws NullPointerException where {const owner O} {
    behave @print(value);
    return;
}

proc run() -> void {
    ptr missing:u64 = null in here;
    val first_site <- 0u64;
    val last_site <- 0u64;
    try { fail_first(missing); } catch (e:const NullPointerException) { (first_site <- e.site):void; };
    try { fail_last(missing); } catch (e:const NullPointerException) { (last_site <- e.site):void; };

    defer catch (e:const NullPointerException) {
              if (e.site == first_site) { behave @print(104u64); }
              else { if (e.site == last_site) { behave @print(106u64); } else { behave @print(999u64); }; };
          }
          catch (e:const MissingReturnError) { behave @print(105u64); }
          finally { behave @print(109u64); };
    defer catch (e:const NullPointerException) {
              if (e.site == first_site) { behave @print(204u64); }
              else { if (e.site == last_site) { behave @print(206u64); } else { behave @print(998u64); }; };
              throw e;
          } finally { behave @print(209u64); };
    defer fail_last(missing);
    defer fail_missing_result();
    defer fail_first(missing);
    return;
}

run();
'''
runner.CASES = {
    "attached_catch_keeps_deferred_failure_pending": (CHECKED_READ + '''
proc run() -> void {
    ptr missing:u64 = null in here;
    defer catch (failure:const NullPointerException) { behave @print(30u64); }
          finally { behave @print(31u64); };
    defer { behave @print(10u64); read(missing); }
          catch (failure:const NullPointerException) { behave @print(20u64); }
          finally { behave @print(21u64); };
    defer read(missing);
    return;
}
run();
''', 0, '10\n20\n21\n30\n31\n', ''),
    "ordinary_failure_reaches_caller": (CHECKED_READ + '''
proc run() -> void throws NullPointerException {
    ptr missing:u64 = null in here;
    defer catch (failure:const NullPointerException) { behave @print(99u64); }
          finally { behave @print(1u64); };
    defer { behave @print(2u64); read(missing); }
          catch (failure:const NullPointerException) { behave @print(3u64); }
          finally { behave @print(4u64); };
    read(missing);
    return;
}
try { run(); } catch (failure:const NullPointerException) { behave @print(5u64); };
''', 0, '2\n3\n4\n1\n5\n', ''),
    "handled_deferred_failure_resumes_body_failure": (CHECKED_READ + '''
proc run() -> void throws NullPointerException {
    ptr missing:u64 = null in here;
    defer catch (failure:const NullPointerException) { behave @print(1u64); }
          finally { behave @print(2u64); };
    defer read(missing);
    read(missing);
    return;
}
try { run(); } catch (failure:const NullPointerException) { behave @print(3u64); };
''', 0, '1\n2\n3\n', ''),
    "deferred_handler_can_replace_body_failure_with_return": (CHECKED_READ + '''
proc run() -> u64 throws NullPointerException {
    ptr missing:u64 = null in here;
    defer { behave @print(3u64); };
    defer catch (failure:const NullPointerException) { behave @print(1u64); return 42u64; }
          finally { behave @print(2u64); };
    defer read(missing);
    read(missing);
    return 0u64;
}
try { behave @print(run()); } catch (failure:const NullPointerException) { behave @print(99u64); };
''', 0, '1\n2\n3\n42\n', ''),
    "deferred_handler_writes_visible_to_body_failure_catch": (CHECKED_READ + '''
proc run() -> void {
    val result <- make u64;
    ptr missing:u64 = null in here;
    try { read(missing); } catch (caught:const NullPointerException) {
        try {{
            defer catch (failure:const NullPointerException) { result <- 42u64; };
            defer { throw caught; };
            throw caught;
        }} catch (failure:const NullPointerException) { behave @print(result); };
    };
    return;
}
run();
''', 0, '42\n', ''),
    "unrelated_return_preserves_checked_deferred_failure": (CHECKED_READ + '''
proc run() -> u64 {
    ptr missing:u64 = null in here;
    defer catch (failure:const NullPointerException) { behave @print(1u64); }
          finally { behave @print(2u64); };
    defer { return 77u64; };
    defer read(missing);
    return 99u64;
}
behave @print(run());
''', 0, '1\n2\n77\n', ''),
    "bodyless_finalizer_return_preserves_forwarded_failure": (CHECKED_READ + '''
proc run() -> u64 {
    ptr missing:u64 = null in here;
    defer catch (failure:const NullPointerException) { behave @print(30u64); }
          finally { behave @print(31u64); };
    defer catch (failure:const NullPointerException) { behave @print(10u64); throw failure; }
          finally { behave @print(20u64); return 77u64; };
    defer read(missing);
    return 99u64;
}
behave @print(run());
''', 0, '10\n20\n30\n31\n77\n', ''),
    "separate_handler_and_finalizer_failures_reach_next_handler": (CHECKED_READ + '''
proc run() -> void {
    ptr missing:u64 = null in here;
    defer catch (failure:const NullPointerException) { behave @print(30u64); };
    defer catch (failure:const NullPointerException) { throw failure; }
          finally { read(missing); };
    defer read(missing);
    return;
}
run();
''', 0, '30\n30\n', ''),
    "bodyless_handler_visits_each_failure_before_finalizer": (CHECKED_READ + '''
proc run() -> void {
    ptr missing:u64 = null in here;
    defer catch (failure:const NullPointerException) { behave @print(1u64); }
          finally { behave @print(2u64); };
    defer read(missing);
    defer read(missing);
    return;
}
run();
''', 0, '1\n1\n2\n', ''),
    "pending_return": (PRELUDE + 'proc f()->const i64{defer observe(1);defer catch(e:const MissingReturnError){observe(2);};defer missing();return 7;}observe(f());', 0, '2\n1\n7\n', WARN),
    "attached_body_runs_during_failure": (PRELUDE + 'try{{defer{observe(1);}catch(e:const MissingReturnError){observe(99);}finally{observe(2);};missing();{}}}catch(e:const MissingReturnError){observe(3);};', 0, '1\n2\n3\n', WARN),
    "own_body_failure": (PRELUDE + '{{defer{observe(1);missing();}catch(e:const MissingReturnError){observe(2);}finally{observe(3);};}};observe(4);', 0, '1\n2\n3\n4\n', WARN),
    "body_normal": (PRELUDE + '{{defer{observe(1);}catch(e){observe(99);}finally{observe(2);};}};', 0, '1\n2\n', WARN),
    "recover_original_value": (PRELUDE + 'observe({{defer catch(e:const MissingReturnError){};defer missing();3}});', 0, '3\n', WARN),
    "value_before_any_failure": (PRELUDE + 'observe({{defer catch(e){};3}});', 0, '3\n', WARN),
    "prepared_scope_value": (PRELUDE + 'observe({{defer catch(e){};defer missing();3}});', 0, '3\n', WARN),
    "void_body_failure": (PRELUDE + 'proc f()->void{defer catch(e:const MissingReturnError){observe(99);return;};missing();return;}try{f();}catch(e:const MissingReturnError){observe(1);};observe(2);', 0, '1\n2\n', WARN),
    "explicit_handler_return": (PRELUDE + 'proc f()->const i64{defer observe(1);defer catch(e:const MissingReturnError){return 8;};defer missing();}observe(f());', 0, '1\n8\n', WARN),
    "handler_value_is_not_return": (PRELUDE + 'proc f()->const i64{defer catch(e:const MissingReturnError){8};defer missing();}f();', 70, '', 'uncaught MissingReturnError'),
    "handler_replaces_return": (PRELUDE + 'proc f()->const i64{defer catch(e:const MissingReturnError){return 8;};defer missing();return 3;}observe(f());', 0, '8\n', WARN),
    "finally_replaces_handler": (PRELUDE + 'proc f()->const i64{defer catch(e:const MissingReturnError){return 8;}finally{return 9;};defer missing();return 3;}observe(f());', 0, '9\n', WARN),
    "return_through_locally_recovered_finally": (PRELUDE + 'proc f()->const i64{try{return 6;}finally{try{missing():void;}catch(e:const MissingReturnError){};}}observe(f());', 0, '6\n', WARN),
    "ordinary_finally_cancels_deferred_replacement": (PRELUDE + 'proc f()->const i64{defer catch(e:const MissingReturnError){};defer{try{return 6;}finally{missing():void;}};return 2;}observe(f());', 0, '2\n', WARN),
    "attached_finally_failure_cancels_local_return": (PRELUDE + 'proc f()->const i64{defer catch(e:const MissingReturnError){};defer{return 6;}finally{missing():void;};return 2;}observe(f());', 0, '2\n', WARN),
    "repeated_replacements": (PRELUDE + 'proc f()->const i64{defer catch(e:const MissingReturnError){};defer missing();defer{return 7;};defer{return 6;};return 2;}observe(f());', 0, '7\n', WARN),
    "ordinary_catch_cancels_return": (PRELUDE + 'proc f()->const i64{try{try{return 1;}finally{missing():void;};}catch(e:const MissingReturnError){observe(8);};return 2;}observe(f());', 0, '8\n2\n', WARN),
    "nested_registration": (PRELUDE + 'proc f()->const i64{defer catch(e:const MissingReturnError){observe(1);};defer catch(e:const MissingReturnError){defer missing();observe(2);};missing();}try{f();{}}catch(e:const MissingReturnError){observe(3);};', 0, '2\n1\n3\n', WARN),
    "nested_handler_snapshot": (PRELUDE + '{{defer catch(e:const MissingReturnError){defer{behave @print(e.category);};};missing();{}}};', 0, '5\n', WARN),
    "explicit_chain": (PRELUDE + 'val original<-error();{{defer catch(e:const MissingReturnError){behave @print(e.cause_count);};defer catch(e:const MissingReturnError){throw original from e;};defer{throw original;};}};', 0, '1\n', WARN),
    "bare_rethrow": (PRELUDE + '{{defer catch(e:const MissingReturnError){observe(1);};defer catch(e:const MissingReturnError){throw;};defer missing();}};', 0, '1\n', WARN),
    "named_rethrow": (PRELUDE + '{{defer catch(e:const MissingReturnError){observe(1);};defer catch(e:const MissingReturnError){throw e;};defer missing();}};', 0, '1\n', WARN),
    "sibling_exclusion": (PRELUDE + '{{defer catch(e:const MissingReturnError){observe(1);};defer catch(e:const MissingReturnError){throw;}catch(e){observe(99);};defer missing();}};', 0, '1\n', WARN),
    "conditional_registration": (PRELUDE + 'proc f(flag:bool)->void{if(flag){defer catch(e:const MissingReturnError){observe(1);return;};}else{};defer missing();return;}f(true);try{f(false);}catch(e:const MissingReturnError){observe(2);};', 0, '1\n2\n', WARN),
    "loop_handlers": (PRELUDE + 'try{{for(i in 0..3){defer{observe(2);}catch(e:const MissingReturnError){observe(99);};};missing();{}}}catch(e:const MissingReturnError){observe(3);};', 0, '2\n2\n2\n3\n', WARN),
    "loop_zero": (PRELUDE + 'try{{for(i in 0..0){defer catch(e:const MissingReturnError){observe(99);};};missing();{}}}catch(e:const MissingReturnError){observe(1);};', 0, '1\n', WARN),
    "loop_explicit_scope": (PRELUDE + 'for(i in 0..3){{defer catch(e:const MissingReturnError){observe(1);};defer missing();}};observe(2);', 0, '1\n1\n1\n2\n', WARN),
    "recursive_recovery": (PRELUDE + 'proc f(n:i64)->const i64{defer catch(e:const MissingReturnError){return n;};if(n==0){defer missing();return 99;}else{return f(n-1)+1;}}observe(f(3));', 0, '3\n', WARN),
    "generic_recovery": (PRELUDE + 'func f(x:T)->const T where T{defer catch(e:const MissingReturnError){};defer missing();return x;}behave @print(f(2u64));behave @print(f(true));', 0, '2\ntrue\n', WARN),
    "terminal_failure_not_caught": (PRELUDE + '{{defer catch(e){observe(99);}finally{observe(1);};3/0;{}}};', 70, '1\n', 'division by zero'),
    "write_before_ordinary_failure": (PRELUDE + 'val n<-0;try{{defer catch(e:const MissingReturnError){};n<-2;missing();{}}}catch(e:const MissingReturnError){observe(n);};', 0, '2\n', WARN),
    "uninitialized_after_recovery": (PRELUDE + 'val n<-make i64;{{defer catch(e:const MissingReturnError){};defer{missing();n<-2;};}};observe(n);', 2, '', 'before definite initialization'),
    "initialized_before_deferred_failure": (PRELUDE + 'val n<-make i64;{{defer catch(e:const MissingReturnError){};defer{n<-2;missing();};}};observe(n);', 0, '2\n', WARN),
    "checked_body_requires_ordinary_coverage": (PRELUDE + 'func f(e:NullPointerException)->void{defer catch(x:const NullPointerException){};throw e;}', 2, '', 'requires a covering catch or throws declaration'),
    "checked_call_covered": (PRELUDE + 'func bad(e:NullPointerException)->void throws NullPointerException{throw e;}func f(e:NullPointerException)->void{defer catch(x:const NullPointerException){};defer bad(e);}', 0, '', WARN),
    "checked_block_covered": (PRELUDE + 'func f(e:NullPointerException)->void{defer catch(x:const NullPointerException){};defer{throw e;};}', 0, '', WARN),
    "checked_wrong_order": (PRELUDE + 'func f(e:NullPointerException)->void{defer{throw e;};defer catch(x:const NullPointerException){};}', 2, '', 'coverage within the owning pass'),
    "checked_conditional_gap": (PRELUDE + 'func f(e:NullPointerException,flag:bool)->void{if(flag){defer catch(x:const NullPointerException){};}else{};throw e;}', 2, '', 'requires a covering catch or throws declaration'),
    "checked_after_return_unreachable": (PRELUDE + 'func f(e:NullPointerException)->void{return;throw e;}', 0, '', WARN),
    "checked_registration_catch_wrong_scope": (PRELUDE + 'func f(e:NullPointerException)->void{try{defer{throw e;};}catch(x:const NullPointerException){};}', 2, '', 'coverage within the owning pass'),
    "checked_full_scope_catch": (PRELUDE + 'func f(e:NullPointerException)->void{try{{defer{throw e;};}}catch(x:const NullPointerException){};}', 2, '', 'coverage within the owning pass'),
    "checked_one_iteration": (PRELUDE + 'func f(e:NullPointerException)->void{for(i in 0..1){defer catch(x:const NullPointerException){};};defer{throw e;};}', 0, '', WARN),
    "checked_zero_iterations": (PRELUDE + 'func f(e:NullPointerException)->void{for(i in 0..0){defer catch(x:const NullPointerException){};};throw e;}', 2, '', 'requires a covering catch or throws declaration'),
    "checked_per_iteration": (PRELUDE + 'func f(e:NullPointerException)->void{for(i in 0..3){ {{defer catch(x:const NullPointerException){};defer{throw e;};}}; };}', 0, '', WARN),
    "selective_forwarding_keeps_unmatched_failure_ahead": (
        FIFO_FORWARDING, 0, '204\n206\n209\n105\n104\n106\n109\n', WARN),
    "finalizer_failure_follows_forwarded_failure": (
        FIFO_FORWARDING.replace('} finally { behave @print(209u64); };',
                                '} finally { behave @print(209u64); fail_last(missing); };')
                       .replace('    defer fail_last(missing);\n', ''),
        0, '204\n209\n105\n104\n106\n109\n', WARN),
    "forwarded_owned_failures_keep_their_payloads": ('''
using "std:exceptions" Exception;
using symbol "std:io" @print;

data DeferredFailure <: Exception { code:u64; }

proc fail(code:u64) -> void throws DeferredFailure {
    throw DeferredFailure { code <- code; };
}

proc run() -> void {
    defer catch (e:DeferredFailure) { behave @print(e.code); }
          catch (e) { behave @print(99u64); }
          finally { behave @print(30u64); };
    defer catch (e:DeferredFailure) {
        if (e.code == 1u64) { (e.code <- 10u64):void; }
        else { (e.code <- 20u64):void; };
        throw;
    };
    defer fail(1u64);
    defer fail(2u64);
    return;
}

run();
''', 0, '20\n10\n30\n', ''),
    "checked_loop_failure_count_is_unproved": (PRELUDE + 'func f(e:NullPointerException)->void{defer catch(x:const NullPointerException){};for(i in 0..3){defer{throw e;};};}', 2, '', 'coverage within the owning pass'),
    "checked_finally_does_not_recover": (PRELUDE + 'func f(e:NullPointerException)->void{defer{}finally{};throw e;}', 2, '', 'requires a covering catch or throws declaration'),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]


from language_test_support import expect_source_rejections
expect_source_rejections(runner, (
    'nested_registration',
    'nested_handler_snapshot',
), 'currently draining scope')

if __name__ == '__main__':
    sys.exit(runner.main())
