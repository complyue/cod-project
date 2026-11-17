#!/usr/bin/env python3
"""Fixed multi-method coverage shares bodies across widths and recursion."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from language_test_support import ROOT, MODE, COD
from callable_value_staging_tests import inventory
from factored_family_tests import IO, family, view, arguments


def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name, passed=bool(passed), **details))
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed:
            print(json.dumps(details, indent=2), flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-factored-family-staging-') as directory:
        scratch = Path(directory)
        for general in (False, True):
            mode = 'general' if general else 'default'
            for count in (16, 32):
                inventories = []
                source = IO + family(count) + 'func relay(callback:' + view(count) + ',depth:u64)->u64{'
                source += 'if(depth==0u64){return callback(' + arguments(count) + ');}else{return relay(callback,depth-1u64);};}'
                source += 'func held:' + view(count) + '=choose;behave @print(relay(held,300u64));'
                for depth in (300, 3000):
                    entry = scratch / f'width-{count}-depth-{depth}.cod'
                    ir = scratch / f'width-{count}-depth-{depth}-{mode}.ll'
                    entry.write_text(source.replace('300u64', f'{depth}u64'))
                    command = [str(COD), '--run', str(entry), '--emit-llvm', str(ir)]
                    if general:
                        command.append('--no-specialize')
                    run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
                    record(f'{mode}/width_{count}/depth_{depth}', run.returncode == 0 and
                           run.stdout == f'{32000 + count * (count + 1) // 2}\n' and not run.stderr,
                           command=command, exit=run.returncode, stdout=run.stdout, stderr=run.stderr)
                    methods = inventory(ir.read_text()) if ir.exists() else {}
                    inventories.append(methods)
                    record(f'{mode}/width_{count}/finite_methods_{depth}', len(methods) == 3 and
                           sum('.choose.' in name for name in methods) == 2 and sum('.relay.' in name for name in methods) == 1,
                           methods=list(methods))
                record(f'{mode}/width_{count}/recursion_reuses_implementations', inventories[0] == inventories[1],
                       methods=list(inventories[1]))
                command[1] = '--check'
                checked = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
                record(f'{mode}/width_{count}/inspection_does_not_execute', checked.returncode == 0 and
                       not checked.stdout and not checked.stderr,
                       command=command, exit=checked.returncode, stdout=checked.stdout, stderr=checked.stderr)
    passed = sum(row['passed'] for row in results)
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [ROOT / 'cod/main.cc', *sorted((ROOT / 'cod').glob('lang_*.hh')), Path(__file__),
                   Path(__file__).with_name('factored_family_tests.py'), Path(__file__).with_name('foundation_tests.py'),
                   Path(__file__).with_name('callable_value_staging_tests.py'), Path(__file__).with_name('language_test_support.py')]
        digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE, passed=passed, total=len(results), results=results, executable_sha256=digest(COD),
            sources={str(path.relative_to(ROOT)): digest(path) for path in sources}), indent=2) + '\n')
    return passed != len(results)


if __name__ == '__main__':
    raise SystemExit(main())
