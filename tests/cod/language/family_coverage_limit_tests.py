#!/usr/bin/env python3
"""Family coverage exhaustion is distinct from unsupported or invalid source."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile

from language_test_support import ROOT, MODE, COD

IO = 'using symbol "std:io" @print;'


def named(kind, count, wrong_result=False):
    names = 'abcdefgh'[:count]
    parameters = ','.join(name + ':' + kind for name in names)
    signature = ','.join([kind] * count)
    arguments = ','.join(str(i + 1) + kind for i in range(count))
    result, value = ('bool', 'true') if wrong_result else (kind, 'a')
    return (IO + f'func target({parameters})->const {result}{{return {value};}}'
            f'func selected:func({signature})->const {kind}=target;'
            f'behave @print(selected({arguments}));')


def queried(kind):
    parameters = ','.join(name + ':' + kind for name in 'abcdefgh')
    signature = ','.join([kind] * 8)
    return (IO + f'func target({parameters})->const {kind};'
            f'func inspect(callback:func({signature})->const {kind})->u64;'
            'val result<-make local typeof(inspect(target)){7u64};behave @print(result);')


def symbolic(kind):
    parameters = ','.join(name + ':' + kind for name in 'abcdefgh')
    arguments = ','.join(str(i + 1) + kind for i in range(8))
    return (IO + f'proc @target({parameters})->const {kind}{{return a;}}'
            f'effect @target;behave @print(behave @target({arguments}));')


CASES = {
    'named_limit': (named('u64', 8), 'limit', ''),
    'declaration_query_limit': (queried('u64'), 'limit', ''),
    'symbolic_limit': (symbolic('u64'), 'limit', ''),
    'same_arity_singleton_domain': (named('u8', 8), 'success', '1\n'),
    'smaller_widening_domain': (named('u64', 7), 'success', '1\n'),
    'declaration_query_without_implementation': (queried('u8'), 'success', '7\n'),
    'symbolic_singleton_domain': (symbolic('u8'), 'success', '1\n'),
    'incompatible_result_precedes_exhaustion': (named('u64', 8, True), 'contract', ''),
}


def main():
    results = []
    with tempfile.TemporaryDirectory(prefix='cod-family-coverage-limit-') as directory:
        scratch = Path(directory)
        for name, (source, expectation, output) in CASES.items():
            entry = scratch / (name + '.cod')
            entry.write_text(source)
            for general in (False, True):
                mode = 'general' if general else 'default'
                ir = scratch / (name + '-' + mode + '.ll')
                command = [str(COD), '--run', str(entry), '--emit-llvm', str(ir)]
                if general:
                    command.append('--no-specialize')
                run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=120)
                passed = run.stdout == output
                if expectation == 'limit':
                    pattern = (re.escape(str(entry)) + r':\d+:\d+: compiler resource limit: 4096 callable family '
                               r"coverage steps while covering '(?:func|proc)\([^\n]+\) -> [^\n]+'; coverage remains unproved\n")
                    passed = passed and run.returncode == 2 and re.fullmatch(pattern, run.stderr) is not None and not ir.exists()
                elif expectation == 'contract':
                    passed = passed and run.returncode == 2 and 'callable view contract' in run.stderr
                    passed = passed and 'compiler resource limit' not in run.stderr and not ir.exists()
                else:
                    passed = passed and run.returncode == 0 and run.stderr == '' and ir.is_file()
                result = dict(name=name + '/' + mode, passed=bool(passed), expectation=expectation,
                              command=command, exit=run.returncode, stdout=run.stdout, stderr=run.stderr)
                results.append(result)
                print(('PASS ' if passed else 'FAIL ') + result['name'], flush=True)
                if not passed:
                    print(json.dumps(result, indent=2), flush=True)
    passed = sum(result['passed'] for result in results)
    print(f'Family coverage limits: {passed}/{len(results)} passed')
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [ROOT / 'cod/main.cc', *sorted((ROOT / 'cod').glob('lang_*.hh')),
                   Path(__file__), ROOT / 'tests/cod/language/language_test_support.py']
        digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE, passed=passed, total=len(results), results=results, executable_sha256=digest(COD),
            sources={str(path.relative_to(ROOT)): digest(path) for path in sources}), indent=2) + '\n')
    return 0 if passed == len(results) else 1


if __name__ == '__main__':
    raise SystemExit(main())
