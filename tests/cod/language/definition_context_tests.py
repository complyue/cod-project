#!/usr/bin/env python3
"""Check wide lexical definition sets without imposing a performance budget."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
import time
from foundation_tests import ROOT, MODE, COD


def program(count, explicit, execute=False):
    result = '->u64' if explicit else ''
    definitions = ''.join(f'func local_fn_{i}(){result}{{return {i}u64;}};' for i in range(count))
    body = definitions + (f'behave @print(local_fn_{count - 1}());' if execute else '')
    return ('using symbol "std:io" @print;' if execute else '') + 'proc test()->void{' + body + '}test();'


def main():
    results = []
    with tempfile.TemporaryDirectory(prefix='cod-definition-context-') as directory:
        path = Path(directory) / 'main.cod'
        for explicit in (False, True):
            for count in (64, 256, 1024):
                path.write_text(program(count, explicit))
                command = [str(COD), '--check', str(path)]
                start = time.monotonic()
                run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
                passed = run.returncode == 0 and not run.stdout and not run.stderr
                results.append(dict(name=f'{"declared" if explicit else "inferred"}/{count}', passed=passed,
                    command=command, seconds=time.monotonic() - start, exit=run.returncode,
                    stdout=run.stdout, stderr=run.stderr, source_sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
        for general in (False, True):
            path.write_text(program(256, False, execute=True))
            command = [str(COD), '--run', str(path)] + (['--no-specialize'] if general else [])
            run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
            results.append(dict(name='general' if general else 'default',
                passed=run.returncode == 0 and run.stdout == '255\n' and not run.stderr,
                command=command, exit=run.returncode, stdout=run.stdout, stderr=run.stderr))
    for result in results:
        print(('PASS ' if result['passed'] else 'FAIL ') + result['name'])
        if not result['passed']:
            print(json.dumps(result, indent=2))
    passed = sum(result['passed'] for result in results)
    print(f'Definition contexts: {passed}/{len(results)} passed')
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [ROOT / 'cod/main.cc', *sorted((ROOT / 'cod').glob('lang_*.hh')), Path(__file__),
                   Path(__file__).with_name('foundation_tests.py')]
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE, passed=passed, total=len(results), results=results,
            executable_sha256=hashlib.sha256(COD.read_bytes()).hexdigest(),
            sources={str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in sources}), indent=2) + '\n')
    return passed != len(results)


if __name__ == '__main__':
    raise SystemExit(main())
