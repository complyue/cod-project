#!/usr/bin/env python3
"""Whole-domain scalar proofs share native bodies at every recursion depth."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from language_test_support import ROOT, MODE, COD
from callable_value_staging_tests import inventory
from uniform_family_tests import IO, BASE, PARAMETERS, NAMES, ARGUMENTS, VIEW

SOURCE = IO + BASE + BASE.replace('base(', 'other(').replace('n0+n31', 'n0+n31+10u64') + f'''
func wrap(callback:{VIEW})->{VIEW}{{return func[callback]({PARAMETERS})->u64{{return callback({NAMES})+1u64;}};}}
func relay(callback:{VIEW},depth:u64)->u64{{
 if(depth==0u64){{return callback({ARGUMENTS});}}else{{return relay(callback,depth-1u64);}};
}}
func first=wrap(base);func second=wrap(other);
behave @print(relay(first,3u64));behave @print(relay(second,300u64));
'''


def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name, passed=bool(passed), **details))
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed:
            print(json.dumps(details, indent=2), flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-uniform-family-staging-') as directory:
        scratch = Path(directory)
        for general in (False, True):
            mode = 'general' if general else 'default'
            inventories = []
            for depth in (300, 3000):
                entry = scratch / f'depth-{depth}.cod'
                ir = scratch / f'depth-{depth}-{mode}.ll'
                entry.write_text(SOURCE.replace('300u64', f'{depth}u64'))
                command = [str(COD), '--run', str(entry), '--emit-llvm', str(ir)]
                if general:
                    command.append('--no-specialize')
                run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
                record(f'{mode}/depth_{depth}', run.returncode == 0 and run.stdout == '34\n44\n' and run.stderr == '',
                       command=command, exit=run.returncode, stdout=run.stdout, stderr=run.stderr)
                bodies = inventory(ir.read_text()) if ir.exists() else {}
                inventories.append(bodies)
                record(f'{mode}/finite_prepared_methods_{depth}', len(bodies) == (5 if general else 6) and
                       sum('.base.' in name for name in bodies) == 1 and sum('.other.' in name for name in bodies) == 1,
                       methods=list(bodies))
            record(mode + '/recursion_reuses_implementations', inventories[0] == inventories[1], methods=list(inventories[1]))
            command[1] = '--check'
            checked = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
            record(mode + '/inspection_does_not_execute', checked.returncode == 0 and checked.stdout == '' and checked.stderr == '',
                   command=command, exit=checked.returncode, stdout=checked.stdout, stderr=checked.stderr)
    passed = sum(row['passed'] for row in results)
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [ROOT / 'cod/main.cc', *sorted((ROOT / 'cod').glob('lang_*.hh')), Path(__file__),
                   Path(__file__).with_name('uniform_family_tests.py'), Path(__file__).with_name('foundation_tests.py'),
                   Path(__file__).with_name('callable_value_staging_tests.py'), Path(__file__).with_name('language_test_support.py')]
        digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE, passed=passed, total=len(results), results=results, executable_sha256=digest(COD),
            sources={str(path.relative_to(ROOT)): digest(path) for path in sources}), indent=2) + '\n')
    return passed != len(results)


if __name__ == '__main__':
    raise SystemExit(main())
