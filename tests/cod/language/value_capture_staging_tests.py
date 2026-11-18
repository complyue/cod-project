#!/usr/bin/env python3
"""Scalar capture activations share native bodies across types and depth."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

from language_test_support import ROOT, COD
from callable_value_staging_tests import inventory

SOURCE = '''
using symbol "std:io" @print;
func factory(val seed:T)->func(T,u64)->const T where T{
 func step[seed](val delta:T,depth:u64)->const T{
  seed<-seed+delta;
  if(depth==0u64){return seed;}else{return step(delta,depth-1u64)+seed;};
 };
 return step;
}
func signed=factory(1);func unsigned=factory(1u64);func floating=factory(1f32);
behave @print(signed(1,DEPTHu64));behave @print(signed(1,DEPTHu64));
behave @print(unsigned(1u64,DEPTHu64));behave @print(floating(1f32,DEPTHu64));
'''


def main():
    results = []
    def record(name, passed, **details):
        results.append({'name': name, 'passed': passed, **details})
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed:
            print(json.dumps(details, indent=2), flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-value-capture-staging-') as directory:
        scratch = Path(directory)
        for general in (False, True):
            mode = 'general' if general else 'default'
            baseline = None
            for depth in (3, 30, 300):
                source, ir = scratch / 'main.cod', scratch / 'main.ll'
                source.write_text(SOURCE.replace('DEPTH', str(depth)))
                command = [str(COD), '--run', str(source), '--emit-llvm', str(ir)]
                if general:
                    command.append('--no-specialize')
                run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
                expected = str((depth + 2) * (depth + 3) // 2 - 1) + '\n'
                record(mode + f'/depth{depth}', run.returncode == 0 and run.stdout == expected * 4 and not run.stderr,
                       command=command, exit=run.returncode, stdout=run.stdout, stderr=run.stderr)
                methods = inventory(ir.read_text()) if run.returncode == 0 and ir.exists() else {}
                steps = [name for name in methods if '.step.' in name]
                record(mode + f'/shared_capture_body{depth}', sum(name.endswith('.general') for name in steps) == 1,
                       steps=steps)
                if baseline is None:
                    baseline = methods
                else:
                    record(mode + f'/stable_inventory{depth}', bool(methods) and methods == baseline,
                           methods=methods)
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [ROOT / 'cod/main.cc', *sorted((ROOT / 'cod').glob('lang_*.hh')), Path(__file__),
                   Path(__file__).with_name('language_test_support.py'), Path(__file__).with_name('callable_value_staging_tests.py')]
        Path(report).write_text(json.dumps({'passed': sum(r['passed'] for r in results), 'total': len(results),
                                         'executable_sha256': hashlib.sha256(COD.read_bytes()).hexdigest(),
                                         'sources': {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in sources},
                                         'results': results}, indent=2) + '\n')
    return int(not all(r['passed'] for r in results))


if __name__ == '__main__':
    raise SystemExit(main())
