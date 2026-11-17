#!/usr/bin/env python3
"""Uniform scalar coverage retains complete images and registration cleanup."""
from pathlib import Path
from native_test_support import run_native
import ordinary_family_runtime_tests as fixture
from uniform_family_tests import PARAMETERS, INPUTS, NAMES, ARGUMENTS, VIEW

SOURCE = f'''
func highbase({PARAMETERS})->u64{{return n0+n31;}}
func highother({PARAMETERS})->u64{{return n0+n31+10u64;}}
func highwrap(callback:{VIEW})->{VIEW}{{
 return func[callback]({PARAMETERS})->u64{{return callback({NAMES})+1u64;}};
}}
proc highshow({PARAMETERS})->void{{observe(n0+n31+@x);}}
proc otherhighshow({PARAMETERS})->void{{observe(n0+n31+10u64);}}
{{{{
 func held=highwrap(highwrap(highbase));func saved=held;held<-highother;
 observe(saved({ARGUMENTS}));observe(held({ARGUMENTS}));
 observe(9010u64);val total<-0u64;
 for(i in 0u64..1000u64){{total<-total+saved({ARGUMENTS});}};
 observe(9011u64);observe(total);
 observe(9020u64);
 try{{held<-highwrap(highbase);{{}}}}catch(e:const OutOfMemoryError){{observe(9021u64);}};
 observe(held({ARGUMENTS}));
 proc show=highshow;
 observe(9030u64);
 try{{defer show({ARGUMENTS});}}catch(e:const OutOfMemoryError){{observe(9031u64);}};
 observe(9050u64);
 try{{defer show({ARGUMENTS});}}catch(e:const OutOfMemoryError){{observe(9051u64);}};
 defer show({ARGUMENTS});show<-otherhighshow;
}}}};
observe(9000u64);
'''
FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__), Path(__file__).with_name('uniform_family_tests.py')])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + SOURCE,
    expected_values=[*FIXTURE.expected_values, '35', '43', '35000', '43', '34'],
    harness=FIXTURE.harness.replace('checkpoints==6', 'checkpoints==7')
        .replace('creation_failures==2', 'creation_failures==3')
        .replace('copy_failures==2', 'copy_failures==3')
        .replace('record_failures==1', 'record_failures==2'))

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
