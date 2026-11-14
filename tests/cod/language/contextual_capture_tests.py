#!/usr/bin/env python3
"""Shared deferred actions retain each checked context's caught-value storage."""
from pathlib import Path
import foundation_tests as runner
import deferred_capture_tests as fixture

ERROR = fixture.ERROR
PICK = '''
func pick(tag:bool,ref x:const MissingReturnException in O)->ref const MissingReturnException in O where {owner O}{return x;}
func pick(tag:i64,ref x:const MissingReturnException in O)->const MissingReturnException where {owner O}{return x;}
'''
TWO = '''
func pick(tag:bool,ref x:const MissingReturnException in O,ref y:const MissingReturnException in P)->ref const MissingReturnException in O where {owner O,owner P}{return x;}
func pick(tag:i64,ref x:const MissingReturnException in O,ref y:const MissingReturnException in P)->ref const MissingReturnException in P where {owner O,owner P}{return y;}
'''
def program(body, calls='run(false);run(0);', setup='ref y=pick(tag,e);'):
    return ERROR + PICK + 'proc run(tag:T)->void where T{try{throw chain(error());}catch(e:const MissingReturnException){' + setup + body + '};}' + calls

runner.CASES = {
    'borrow_or_copy': (program('defer{behave @print(y.cause_count);};'), 0, '1\n1\n', fixture.WARN),
    'reverse_demand_order': (program('defer{behave @print(y.cause_count);};', 'run(0);run(false);'), 0, '1\n1\n', fixture.WARN),
    'same_record_different_address': (program('defer{behave @print(e.category);behave @print(y.cause_count);};'), 0, '5\n1\n5\n1\n', fixture.WARN),
    'different_captured_binding': (ERROR + TWO + 'proc run(tag:T)->void where T{try{throw error();}catch(a:const MissingReturnException){try{throw chain(a);}catch(b:const MissingReturnException){ref y=pick(tag,a,b);defer{behave @print(y.cause_count);};};};}run(false);run(0);', 0, '0\n1\n', fixture.WARN),
    'same_record_different_captured_binding': (ERROR + TWO + 'proc run(tag:T)->void where T{try{throw error();}catch(a:const MissingReturnException){try{throw chain(a);}catch(b:const MissingReturnException){ref y=pick(tag,a,b);defer{behave @print(a.cause_count);behave @print(b.cause_count);behave @print(y.cause_count);};};};}run(false);run(0);', 0, '0\n1\n0\n0\n1\n1\n', fixture.WARN),
    'nested_registration': (program('defer{defer{behave @print(y.cause_count);};};'), 0, '1\n1\n', fixture.WARN),
    'nested_with_direct_capture': (program('defer{behave @print(e.category);defer{behave @print(y.cause_count);};};'), 0, '5\n1\n5\n1\n', fixture.WARN),
    'nested_full_scope': (program('defer{{defer{behave @print(y.cause_count);};behave @print(e.category);}};'), 0, '5\n1\n5\n1\n', fixture.WARN),
    'finally_snapshot': (program('defer{behave @print(y.cause_count);}finally{behave @print(e.category);};'), 0, '1\n5\n1\n5\n', fixture.WARN),
    'bodyless_finally': (program('defer finally{behave @print(y.cause_count);};'), 0, '1\n1\n', fixture.WARN),
    'named_rethrow': (program('defer{try{throw y;}catch(x){behave @print(x.cause_count);};};'), 0, '1\n1\n', fixture.WARN),
    'pending_failure_skips_body': (program('defer{behave @print(99);}catch(x:const MissingReturnException){behave @print(y.cause_count);}finally{behave @print(e.category);};defer{throw e;};'), 0, '1\n5\n1\n5\n', fixture.WARN),
    'returned_snapshot': (ERROR + PICK + 'func run(tag:T)->const MissingReturnException where T{try{throw chain(error());}catch(e:const MissingReturnException){ref y=pick(tag,e);defer{return y;};};return error();}behave @print(run(false).cause_count);behave @print(run(0).cause_count);', 0, '1\n1\n', fixture.WARN),
    'aliased_reference': (program('const ref var alias->y;defer{behave @print(alias.cause_count);};'), 0, '1\n1\n', fixture.WARN),
    'borrowed_snapshot_address': (ERROR + PICK + 'func read(ref x:const MissingReturnException in O)->const u64 where {owner O}{return x.cause_count;}proc run(tag:T)->void where T{try{throw chain(error());}catch(e:const MissingReturnException){ref y=pick(tag,e);defer{behave @print(e.category);behave @print(read(y));};};}run(false);run(0);', 0, '5\n1\n5\n1\n', fixture.WARN),
    'later_lexical_slots': (program('val z=7;val var alias->z;defer{behave @print(y.cause_count);behave @print(alias);};'), 0, '1\n7\n1\n7\n', fixture.WARN),
    'per_iteration_scopes': (ERROR + PICK + 'proc run(tag:T)->void where T{for(i in 0..3){{try{throw chain(error());}catch(e:const MissingReturnException){ref y=pick(tag,e);defer{behave @print(y.cause_count);};};}};}run(false);run(0);', 0, '1\n1\n1\n1\n1\n1\n', fixture.WARN),
    'recursive_frames': (ERROR + PICK + 'proc run(tag:T,n:i64,e:MissingReturnException)->void where T{try{throw e;}catch(x:const MissingReturnException){ref y=pick(tag,x);defer{behave @print(y.cause_count);};};if(n>0){run(tag,n-1,chain(e));}else{};}run(false,2,error());run(0,2,error());', 0, '2\n1\n0\n2\n1\n0\n', fixture.WARN),
    'cleared_pointer_stays_absent': (program('defer{behave @print(e.category);try{behave @print(y.cause_count);}catch(x:const NullPointerException){behave @print(8);};};y=null;', setup='ptr y=pick(tag,e);'), 0, '5\n8\n5\n8\n', fixture.WARN),
    'registration_order': (program('defer{behave @print(1);};defer{behave @print(y.cause_count);};defer{behave @print(3);};'), 0, '3\n1\n1\n3\n1\n1\n', fixture.WARN),
    'readonly_borrow_preserved': (program('defer{y<-error();};'), 2, '', 'readonly destination'),
    'catch_local_borrow_cannot_escape': (ERROR + PICK + 'func run(tag:T,ref x:const MissingReturnException in O)->ref const MissingReturnException in O where {T,owner O}{try{throw x;}catch(e:const MissingReturnException){ref y=pick(tag,e);return y;};return x;}val x=error();run(false,x);run(0,x);', 2, '', 'declared external owner'),
    'loop_declaration_still_rejected': (ERROR + PICK + 'proc run(tag:T)->void where T{for(i in 0..2){try{throw error();}catch(e:const MissingReturnException){ref y=pick(tag,e);defer{behave @print(y.category);};};};}run(false);run(0);', 2, '', 'per-iteration {{'),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
