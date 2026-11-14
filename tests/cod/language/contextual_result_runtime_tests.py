#!/usr/bin/env python3
"""Expected results retain complete-image cleanup and allocation-free context dispatch."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
for signature, head in (
 ('func[f](x:u64)->const u64', 'func[f](x:u64)'),
 ('proc[p](x:u64)->void', 'proc[p](x:u64)'),
):
 assert signature in runner.SOURCE
 runner.SOURCE = runner.SOURCE.replace(signature, head)
runner.SOURCE = '''
func zero()->u64{return 0u64;}
func floating()->f64{return 0f64;}
func expected(f:F)->const F where F{f<-func(){return 1u32;};return f;}
''' + runner.SOURCE
runner.SOURCE = runner.SOURCE.replace(
 '  observe(9010u64);val total<-0u64;',
 '  func a=expected(zero);func b=expected(floating);val real_total<-0f64;\n'
 '  observe(9010u64);val total<-0u64;')
runner.SOURCE = runner.SOURCE.replace(
 'total<-total+f(i);', 'total<-total+f(i)+a();real_total<-real_total+b();')
runner.SOURCE = runner.SOURCE.replace(
 '  observe(9011u64);observe(total);',
 '  if(real_total==10000f64){}else{observe(9999u64);};observe(9011u64);observe(total);')
runner.EXPECTED_VALUES = [value.replace('50795000', '50805000') for value in runner.EXPECTED_VALUES]
runner.EXTRA_SOURCES += [Path(__file__)]
if __name__ == '__main__':
 raise SystemExit(runner.main())
