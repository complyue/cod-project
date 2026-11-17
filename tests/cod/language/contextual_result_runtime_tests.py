#!/usr/bin/env python3
"""Expected results retain complete-image cleanup and allocation-free context dispatch."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
for signature, head in (
 ('func[f](x:u64)->const u64', 'func[f](x:u64)'),
 ('proc[p](x:u64)->void', 'proc[p](x:u64)'),
):
 assert signature in FIXTURE.source
 FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(signature, head))
FIXTURE = FIXTURE.updated(source='''
func zero()->u64{return 0u64;}
func floating()->f64{return 0f64;}
func expected(f:F)->const F where F{f<-func(){return 1u32;};return f;}
''' + FIXTURE.source)
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(
 '  observe(9010u64);val total<-0u64;',
 '  func a=expected(zero);func b=expected(floating);val real_total<-0f64;\n'
 '  observe(9010u64);val total<-0u64;'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(
 'total<-total+f(i);', 'total<-total+f(i)+a();real_total<-real_total+b();'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(
 '  observe(9011u64);observe(total);',
 '  if(real_total==10000f64){}else{observe(9999u64);};observe(9011u64);observe(total);'))
FIXTURE = FIXTURE.updated(expected_values=[value.replace('50795000', '50805000') for value in list(FIXTURE.expected_values)])
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
 raise SystemExit(run_native(FIXTURE))
