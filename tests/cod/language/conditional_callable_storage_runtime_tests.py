#!/usr/bin/env python3
"""Mixed callable reception releases complete images on success and failure."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
for(mode in 0..2){{
  func holder=wrap(base);
  ptr picked=if(mode==0){holder}else{wrap(wrap(other))};
  func saved=wrap(picked);
  holder<-other;
  observe(picked(1u64));observe(saved(1u64));
  picked<-base;observe(holder(1u64));
}};
observe(9000u64);
observe(9060u64);
try{ {{
  func holder=wrap(base);
  ptr picked=if(false){holder}else{try{wrap(other)}finally{observe(9061u64);{}}};
  observe(9999u64);
}}; }catch(e:const OutOfMemoryError){observe(9062u64);};
observe(9000u64);
{{
  func holder=wrap(base);observe(9020u64);
  try{
    ptr picked=if(false){holder}else{wrap(other)};
    observe(9999u64);
  }catch(e:const OutOfMemoryError){observe(9021u64);};
  observe(holder(1u64));
}};
observe(9000u64);
'''
runner.HARNESS = runner.HARNESS.replace(
    '  assert(value!=9999);',
    '''  if(value==9060){live_baseline=images.size();return;}
  if(value==9061){deny_copy=true;return;}
  if(value==9062){assert(deny_copy && images.size()==live_baseline);deny_copy=false;return;}
  assert(value!=9999);''').replace(
    'checkpoints==5 && creation_failures==1 && copy_failures==3',
    'checkpoints==8 && creation_failures==2 && copy_failures==4')
runner.EXPECTED_VALUES += ['11','3','1','13','14','11','2']
runner.EXTRA_SOURCES += [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
