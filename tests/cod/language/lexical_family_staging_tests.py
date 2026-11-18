#!/usr/bin/env python3
"""Lexical families cover unused inputs without expanding runtime captures."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from callable_value_staging_tests import inventory
from foundation_tests import ROOT, MODE, COD

SOURCE = '''
using symbol "std:io" @print;
func tag(x:u8)->u64{return 8u64;}
func tag(x:u32)->u64{return 32u64;}
func tag(x:u64)->u64{return 64u64;}
func wrap(f:func(u64)->u64)->func(u64)->u64{
  return func[f](x:T)->u64 where T{return f(x);};
}
func held:func(u64)->u64=tag;
for(i in 0u64..10u64){held<-wrap(held);};
behave @print(held(1u8));behave @print(held(1u64));
'''


def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name, passed=passed, **details))
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed: print(json.dumps(details, indent=2), flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-lexical-family-staging-') as directory:
        source, ir = Path(directory) / 'main.cod', Path(directory) / 'main.ll'
        def run(text, general, execute=True):
            source.write_text(text)
            command = [str(COD), '--run' if execute else '--check', str(source), '--emit-llvm', str(ir)]
            if general: command.append('--no-specialize')
            completed = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
            return dict(exit=completed.returncode, stdout=completed.stdout, stderr=completed.stderr), ir.read_text() if completed.returncode == 0 else ''
        for general in (False, True):
            mode = 'general' if general else 'default'
            output, text = run(SOURCE, general)
            expected = dict(exit=0, stdout='8\n64\n', stderr='')
            bodies = inventory(text)
            record(mode+'/native', output == expected, output=output)
            record(mode+'/one_shared_generic_body', len(bodies) == (5 if general else 8)
                   and sum(name.endswith('.general') for name in bodies) == 1, methods=list(bodies))
            record(mode+'/unused_input_covered', general or any('.specialized.u32.' in name for name in bodies), methods=list(bodies))
            deeper, deep_ir = run(SOURCE.replace('10u64', '1000u64'), general)
            record(mode+'/runtime_capture_growth_reuses_methods', deeper == expected and inventory(deep_ir) == bodies, output=deeper)
            checked, checked_ir = run(SOURCE, general, False)
            record(mode+'/preparation_has_no_effects', checked == dict(exit=0, stdout='', stderr='') and inventory(checked_ir) == bodies, output=checked)
    count = sum(result['passed'] for result in results)
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [Path(__file__).with_name("language_test_support.py"), ROOT/'cod/main.cc', *sorted((ROOT/'cod').glob('lang_*.hh')), Path(__file__),
                   Path(__file__).with_name('foundation_tests.py'), Path(__file__).with_name('callable_value_staging_tests.py')]
        digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE, passed=count, total=len(results), results=results, executable_sha256=digest(COD),
            sources={str(path.relative_to(ROOT)): digest(path) for path in sources}), indent=2)+'\n')
    return count != len(results)

if __name__ == '__main__':
    raise SystemExit(main())
