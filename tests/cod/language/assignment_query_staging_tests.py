#!/usr/bin/env python3
"""Assignment queries emit exactly the independently declared result type's code."""
from pathlib import Path
import declaration_query_staging_tests as runner

BASE = 'val x<-1,y<-7;ptr p=x,q=y;'
runner.CASES = (
    ('transfer', BASE, 'x<-y', 'i64', '9', '9\n'),
    ('rebind', BASE, 'p=y', 'i64', '9', '9\n'),
    ('intrinsic_transfer', BASE, 'intrinsic (<-)(x,y)', 'i64', '9', '9\n'),
    ('intrinsic_rebind', BASE, 'intrinsic (=)(p,y)', 'i64', '9', '9\n'),
    ('uninitialized', 'val x<-make local i64,y<-make local i64;', 'x<-y', 'i64', '9', '9\n'),
    ('nullable', 'ptr p:i64=null in here;', 'p<-7', 'i64', '9', '9\n'),
    ('right_chain', BASE, 'p=q=y', 'i64', '9', '9\n'),
    ('result_write', BASE, '(p=y)<-9', 'i64', '9', '9\n'),
    ('selected_slot', BASE, '{if(true){p}else{q}}=y', 'i64', '9', '9\n'),
    ('source_not_called', 'proc source()->i64{behave @print(99);return 7;}'+BASE, 'x<-source()', 'i64', '9', '9\n'),
    ('body_not_demanded', 'func invalid(value:T)->const T where T{return value+value;}val x<-true;', 'x<-invalid(true)', 'bool', 'true', 'true\n'),
    ('callable_not_copied', 'func one()->u64{return 1u64;}func f=one;func g=one;', '(f<-g)()', 'u64', '9u64', '9\n'),
)
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
