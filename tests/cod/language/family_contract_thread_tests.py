#!/usr/bin/env python3
"""Concrete exception dispatch preserves thread-local family captures."""
from pathlib import Path
import dynamic_binding_thread_tests as runner

runner.SOURCE = runner.SOURCE.replace('symbol @context:u64;', '''symbol @context:u64;
using "std:exceptions" Exception;
using "std:exceptions" MissingReturnError;
func missing()->u64{}
func error()->const MissingReturnError{try{missing();}catch(e:const MissingReturnError){return e;};}
val argument:Exception<-error();
symbol @selected:proc(Exception)->u64;
proc @selected(v:MissingReturnError)->u64{return @context;}
proc @selected(v:T)->u64 where T{return 99u64;}''')
runner.SOURCE = runner.SOURCE.replace('effect @context<-1u64;', 'effect @context<-1u64;effect @selected;')
runner.SOURCE = runner.SOURCE.replace('effect @context<-2u64;', 'effect @context<-2u64;effect @selected;')
runner.SOURCE = runner.SOURCE.replace('behave @print(@context);', 'behave @print(perform @selected(argument));')
runner.TLS_SLOTS = 2
runner.ALLOWED_DIAGNOSTICS = ('[-Wmissing-return]',)
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
