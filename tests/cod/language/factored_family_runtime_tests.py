#!/usr/bin/env python3
"""Projected family coverage retains dispatch, complete images and failure cleanup."""
from pathlib import Path
from native_test_support import run_native
import uniform_family_runtime_tests as fixture
from factored_family_tests import COUNT, PARAMETERS, NAMES, ARGUMENTS, VIEW, family, phase_family

SHOW = ''.join('proc factored_show(' + PARAMETERS.replace('n0:u64', 'n0:' + first) +
    ')->void{observe(' + str(tag) + 'u64+' + '+'.join(f'n{i}' for i in range(COUNT)) + '+@x);return;}'
    for first, tag in (('u32', 32000), ('u64', 64000)))
PHASE_METHODS = phase_family(COUNT).replace('func choose(', 'func phase_choose(')
PHASE_EXACT = ','.join(['1u8'] + ['2u64'] * (COUNT - 1))
PHASE_WIDENED = ','.join(['1u8'] + ['2u64'] * (COUNT - 2) + ['2u8'])
SOURCE = family(name='factored_other', extra=1000) + PHASE_METHODS + f'''
func factored_wrap(callback:{VIEW})->{VIEW}{{
 return func[callback]({PARAMETERS})->u64{{return callback({NAMES})+1u64;}};
}}
{SHOW}
proc factored_other_show({PARAMETERS})->void{{observe(99u64);return;}}
{{{{
 func held=factored_wrap(factored_wrap(factored_choose));func saved=held;held<-factored_other;
 observe(saved({ARGUMENTS}));observe(held({ARGUMENTS}));
 func phase:{VIEW}=phase_choose;
 observe(phase({PHASE_EXACT}));observe(phase({PHASE_WIDENED}));
 observe(9010u64);val total<-0u64;
 for(i in 0u64..1000u64){{total<-total+saved({ARGUMENTS});}};
 observe(9011u64);observe(total);
 observe(9020u64);
 try{{held<-factored_wrap(factored_choose);{{}}}}catch(e:const OutOfMemoryError){{observe(9021u64);}};
 observe(held({ARGUMENTS}));
 proc show:proc({','.join(['u64'] * COUNT)})->void=factored_show;
 observe(9030u64);
 try{{defer show({ARGUMENTS});}}catch(e:const OutOfMemoryError){{observe(9031u64);}};
 observe(9050u64);
 try{{defer show({ARGUMENTS});}}catch(e:const OutOfMemoryError){{observe(9051u64);}};
 defer show({ARGUMENTS});show<-factored_other_show:void;
}}}};
observe(9000u64);
'''
FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__), Path(__file__).with_name('factored_family_tests.py'),
    Path(__file__).with_name('foundation_tests.py')])
for original, expected in (('checkpoints==7', 'checkpoints==8'), ('creation_failures==3', 'creation_failures==4'),
                           ('copy_failures==3', 'copy_failures==4'), ('record_failures==2', 'record_failures==3')):
    assert FIXTURE.harness.count(original) == 1
    FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace(original, expected))
FIXTURE = FIXTURE.updated(source_files={**FIXTURE.source_files,
    'factored-choices.cod': family(name='factored_choose') + 'export factored_choose;'})
FIXTURE = FIXTURE.updated(source='using "factored-choices.cod" factored_choose;' + FIXTURE.source + SOURCE,
    expected_values=[*FIXTURE.expected_values, '64138', '33136', '101', '202', '64138000', '33136', '32137'])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
