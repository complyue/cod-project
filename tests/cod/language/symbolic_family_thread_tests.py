#!/usr/bin/env python3
"""Concurrent family values select actual types and retain independent captures."""
from pathlib import Path
import dynamic_binding_thread_tests as runner

runner.SOURCE = runner.SOURCE.replace('symbol @context:u64;', '''symbol @context:u64;
symbol @selected:proc(u64)->u64;
proc @selected(v:T)->u64 where T{return @context;}
proc @selected(v:u64)->u64{return 99u64;}''')
runner.SOURCE = runner.SOURCE.replace('effect @context<-1u64;', 'effect @context<-1u64;effect @selected;')
runner.SOURCE = runner.SOURCE.replace('effect @context<-2u64;', 'effect @context<-2u64;effect @selected;')
runner.SOURCE = runner.SOURCE.replace('behave @print(@context);', 'behave @print(perform @selected(0u8));')
runner.TLS_SLOTS = 2
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
