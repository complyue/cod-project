#!/usr/bin/env python3
"""Definition activation and current or replayed symbolic invocation."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
X = 'symbol @x:u64;proc read()->u64{return @x;}'
P = 'symbol @p:proc()->u64;'
DEF = 'effect @p()->u64{return read();};'

runner.CASES = {
    'current_and_replay': (
        IO + X + P + 'effect @x<-1u64;' + DEF + 'effect @x<-2u64;'
        'behave @print(read());behave @print(behave @p());behave @print(perform @p());behave @print(@x);',
        0, '2\n2\n1\n2\n', ''),
    'nested_symbol_replay': (
        IO + X + P + 'symbol @q:proc()->u64;effect @x<-3u64;' + DEF +
        'effect @q()->u64{return behave @p();};effect @x<-4u64;'
        'behave @print(perform @q());behave @print(behave @q());behave @print(perform @p());',
        0, '3\n4\n3\n', ''),
    'direct_environment_calls': (
        IO + X + P + 'effect @x<-3u64;' + DEF + 'effect @x<-4u64;'
        'proc relay()->u64{proc helper()->u64{return behave @p();};return {{helper()}};}'
        'behave @print(relay());', 0, '4\n', ''),
    'procedure_holder_rejected': (
        X + 'proc saved=read;', 2, '', 'procedure signatures cannot occupy ordinary value or reference storage'),
    'handler_extraction_rejected': (
        X + P + 'effect @x<-1u64;' + DEF + 'proc saved=@p;', 2, '', 'an installed procedure can only be invoked'),
    'handler_assignment_rejected': (
        X + P + 'effect @x<-1u64;effect @p=read;', 2, '', 'procedure symbols require definition activation'),
    'handler_assignment_from_symbol_rejected': (
        X + P + 'symbol @q:proc()->u64;effect @x<-1u64;' + DEF + 'effect @q=@p;',
        2, '', 'procedure symbols require definition activation'),
    'unbound_activation_dependency': (X + P + DEF, 2, '', 'cannot prove an installed binding'),
    'unbound_callee': (P + 'perform @p();', 2, '', 'cannot prove an installed binding'),
    'wrong_call_arity': (X + P + 'effect @x<-1u64;' + DEF + 'perform @p(2u64);', 2, '', 'argument count'),
    'not_a_procedure_symbol': (
        'symbol @x:u64;effect @x<-1u64;perform @x();', 2, '', 'dynamic invocation requires a procedure symbol'),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
