#!/usr/bin/env python3
"""Recursive caller-ref preservation uses one prepared native method."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from language_test_support import ROOT, MODE, COD
from callable_value_staging_tests import inventory
from caller_ref_preservation_tests import IO, NPE, RECURSIVE, COUNTERS, SHOW


def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name, passed=bool(passed), **details))
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed:
            print(json.dumps(details, indent=2), flush=True)
    source = IO + NPE + RECURSIVE + COUNTERS + 'try{p<=>q;}catch(e:const NullPointerException){};' + SHOW
    with tempfile.TemporaryDirectory(prefix='cod-caller-ref-staging-') as directory:
        scratch = Path(directory)
        for general in (False, True):
            mode = 'general' if general else 'default'
            inventories = []
            for depth in (40, 400):
                entry = scratch / f'depth-{depth}.cod'
                ir = scratch / f'depth-{depth}-{mode}.ll'
                entry.write_text(source.replace('40u64', f'{depth}u64'))
                command = [str(COD), '--run', str(entry), '--emit-llvm', str(ir)]
                if general:
                    command.append('--no-specialize')
                run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
                record(f'{mode}/depth_{depth}', run.returncode == 0 and run.stdout == '0\n0\n' and not run.stderr,
                       command=command, exit=run.returncode, stdout=run.stdout, stderr=run.stderr)
                methods = inventory(ir.read_text()) if ir.exists() else {}
                inventories.append(methods)
                record(f'{mode}/one_method_{depth}', len(methods) == 1, methods=list(methods))
            record(mode + '/recursion_reuses_implementation', inventories[0] == inventories[1], methods=list(inventories[1]))
            command[1] = '--check'
            checked = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
            record(mode + '/inspection_does_not_execute', checked.returncode == 0 and not checked.stdout and not checked.stderr,
                   command=command, exit=checked.returncode, stdout=checked.stdout, stderr=checked.stderr)
    passed = sum(row['passed'] for row in results)
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [ROOT / 'cod/main.cc', *sorted((ROOT / 'cod').glob('lang_*.hh')), Path(__file__),
                   Path(__file__).with_name('caller_ref_preservation_tests.py'), Path(__file__).with_name('foundation_tests.py'),
                   Path(__file__).with_name('callable_value_staging_tests.py'), Path(__file__).with_name('language_test_support.py')]
        digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE, passed=passed, total=len(results), results=results, executable_sha256=digest(COD),
            sources={str(path.relative_to(ROOT)): digest(path) for path in sources}), indent=2) + '\n')
    return passed != len(results)


if __name__ == '__main__':
    raise SystemExit(main())
