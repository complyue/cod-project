#!/usr/bin/env python3
"""Family coverage is independent of observed calls and native specialization."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from foundation_tests import ROOT, MODE, COD
from callable_value_staging_tests import inventory

SOURCE = '''
using symbol "std:io" @print;
symbol @f:proc(u64)->u64;
proc @f(x:u64)->u64{return 1u64;}
proc @f(x:T)->u64 where T{return 7u64;}
effect @f;
behave @print(behave @f(1u8));behave @print(behave @f(1u64));
for(i in 0u64..10u64){effect @f;behave @f(1u8);};
'''

def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name, passed=passed, **details))
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed: print(json.dumps(details, indent=2), flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-family-staging-') as directory:
        root = Path(directory)
        def run(text, general, execute=True):
            source, ir = root / 'main.cod', root / 'main.ll'
            source.write_text(text)
            command = [str(COD), '--run' if execute else '--check', str(source), '--emit-llvm', str(ir)]
            if general: command.append('--no-specialize')
            completed = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
            return dict(exit=completed.returncode, stdout=completed.stdout, stderr=completed.stderr), ir.read_text() if completed.returncode == 0 else ''
        for general in (False, True):
            mode = 'general' if general else 'default'
            output, ir = run(SOURCE, general)
            expected = dict(exit=0, stdout='7\n1\n', stderr='')
            bodies = inventory(ir)
            record(mode + '/native', output == expected, output=output)
            record(mode + '/shared_semantic_methods', len(bodies) == (2 if general else 4) and
                   sum(name.endswith('.general') for name in bodies) == 1, methods=list(bodies))
            record(mode + '/unused_admitted_input', general or any(name.endswith('.specialized.u32') for name in bodies), methods=list(bodies))
            repeated, repeated_ir = run(SOURCE.replace('10u64', '1000u64'), general)
            record(mode + '/installations_do_not_register_methods', repeated == expected and set(inventory(repeated_ir)) == set(bodies), output=repeated)
            checked, _ = run(SOURCE, general, False)
            record(mode + '/preparation_has_no_effects', checked == dict(exit=0, stdout='', stderr=''), output=checked)
    count = sum(result['passed'] for result in results)
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [ROOT / 'cod/main.cc', *sorted((ROOT / 'cod').glob('lang_*.hh')), Path(__file__),
                   Path(__file__).with_name('foundation_tests.py'), Path(__file__).with_name('callable_value_staging_tests.py')]
        digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE, passed=count, total=len(results), results=results, executable_sha256=digest(COD),
            sources={str(path.relative_to(ROOT)): digest(path) for path in sources}), indent=2) + '\n')
    return count != len(results)

if __name__ == '__main__':
    raise SystemExit(main())
