#!/usr/bin/env python3
"""Callable target/value receivers preserve shared recursive native bodies."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE = runner.SOURCE.replace('func step(', '''
func identity(value:T)->const T where T{return value;}
func step(''').replace(' if(n==0u64){', '''
 func local:func(T)->const T=identity;
 ptr callback:func(T)->const T=if(n%2u64==0u64){local}else{identity};
 val received<-callback(next);
 if(n==0u64){''')
runner.SPELLINGS = {name:(write.replace('next','received'),rebind)
                   for name,(write,rebind) in runner.SPELLINGS.items()}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
