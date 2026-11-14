#!/usr/bin/env python3
"""Fold conversion stays local to one shared native method."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
from foundation_tests import ROOT, MODE, COD
from callable_value_staging_tests import inventory

SOURCE = '''
using symbol "std:io" @print;
func folded(seed:T,x:U,n:i64)->const T where {T,U} {
    return fold(i in 0..n,a=seed){x};
}
behave @print(folded(0u8,255u8,30));
behave @print(folded(0f64,255u8,30));
behave @print(folded(0f64,4294967295u32,30));
behave @print(folded(0f64,0.5f32,30));
'''


def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name, passed=passed, **details))
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed:
            print(json.dumps(details, indent=2), flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-fold-conversion-') as directory:
        root = Path(directory)
        def run(text, general):
            source, ir = root/'main.cod', root/'main.ll'
            source.write_text(text)
            command = [str(COD), '--run', str(source), '--emit-llvm', str(ir)]
            if general:
                command.append('--no-specialize')
            completed = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
            return dict(exit=completed.returncode, stdout=completed.stdout, stderr=completed.stderr), (
                ir.read_text() if completed.returncode == 0 and ir.exists() else '')
        for general in (False, True):
            mode = 'general' if general else 'default'
            base, ir = run(SOURCE, general)
            expected = dict(exit=0, stdout='255\n255\n4294967295\n0.5\n', stderr='')
            bodies = inventory(ir)
            record(mode+'/native_results', base == expected, output=base)
            methods = [name for name in bodies if 'cod.method.0.folded.' in name]
            record(mode+'/shared_method', len(methods) == (1 if general else 5)
                   and sum(name.endswith('.general') for name in methods) == 1, methods=methods)
            record(mode+'/native_conversions', bool(re.search(r'uitofp i32 [^\n]+ to double', ir))
                   and bool(re.search(r'fpext float [^\n]+ to double', ir)))
            repeated, repeated_ir = run(SOURCE.replace(',30)', ',300)'), general)
            record(mode+'/iteration_count', repeated == expected and set(inventory(repeated_ir)) == set(bodies), output=repeated)
            prefix, calls = SOURCE.split('behave @print', 1)
            calls = ['behave @print' + line for line in calls.split('behave @print')]
            reordered, reordered_ir = run(prefix + ''.join(reversed(calls)), general)
            record(mode+'/demand_order', reordered == dict(exit=0, stdout='0.5\n4294967295\n255\n255\n', stderr='')
                   and set(inventory(reordered_ir)) == set(bodies), output=reordered)
    count = sum(item['passed'] for item in results)
    print(f'Fold conversion staging: {count}/{len(results)} passed')
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [ROOT/'cod/main.cc', *sorted((ROOT/'cod').glob('lang_*.hh')), Path(__file__),
                   Path(__file__).with_name('foundation_tests.py'), Path(__file__).with_name('callable_value_staging_tests.py')]
        digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE, passed=count, total=len(results), results=results, executable_sha256=digest(COD),
            sources={str(path.relative_to(ROOT)): digest(path) for path in sources}), indent=2) + '\n')
    return count != len(results)


if __name__ == '__main__':
    raise SystemExit(main())
