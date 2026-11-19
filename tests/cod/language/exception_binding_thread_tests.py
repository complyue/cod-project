#!/usr/bin/env python3
"""Concurrent exception captures retain independent cause tables and bindings."""
from pathlib import Path
import dynamic_binding_thread_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" MissingReturnError;
func missing()->u64{}
func error()->const MissingReturnError{try{missing():void;}catch(e:const MissingReturnError){return e;};}
func chain(e:MissingReturnError)->const MissingReturnError{return try{throw e from e;}catch(x:const MissingReturnError){x};}
symbol @context:MissingReturnError;
symbol @selected:proc()->u64;
symbol @inspected:u64;
val independent<-make local typeof(@inspected){1u64};
proc depth()->u64{return @context.cause_count;}
effect @context<-chain(error());effect @selected=depth;
behave @print(100u64);
{{
 effect @context<-chain(@context);effect @selected=depth;
 behave @print(101u64);
 behave @print(perform @selected());
}};
behave @print(102u64);
behave @print(perform @selected());
'''
runner.HARNESS = runner.HARNESS.replace('{++completed;}', '{assert(cod_causes && cod_causes->size()==2);cod_default_finish();assert(!cod_causes);++completed;}')
runner.TLS_SLOTS = 2
runner.ALLOWED_DIAGNOSTICS = ('[-Wmissing-return]',)
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
