#!/usr/bin/env python3
"""Generic loop invariants use shared methods independent of runtime depth."""
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
func copy(ref value:const T in O)->const T where {T,owner O}{return value;}
func repeat(ref value:const T in O,n:u64)->const T where {T,owner O}{
  ptr selected:const T=null in O;selected=value;
  val total<-value;
  for(i in 0u64..n){total<-total+copy(selected);};
  selected=null;
  return total;
}
val x<-7u64;behave @print(repeat(x,COUNTu64));
'''


def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name, passed=passed, **details))
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed:
            print(json.dumps(details, indent=2), flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-loop-invariant-') as directory:
        root = Path(directory)
        entry, ir = root / 'main.cod', root / 'main.ll'
        for general in (False, True):
            mode = 'general' if general else 'default'
            shallow = None
            for count in (30, 300):
                entry.write_text(SOURCE.replace('COUNT', str(count)))
                command = [str(COD), '--run', str(entry), '--emit-llvm', str(ir)]
                if general:
                    command.append('--no-specialize')
                executed = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
                output = dict(exit=executed.returncode, stdout=executed.stdout, stderr=executed.stderr)
                record(f'{mode}/{count}/native_result', output == dict(exit=0, stdout=f'{7*(count+1)}\n', stderr=''), output=output)
                bodies = inventory(ir.read_text()) if executed.returncode == 0 else {}
                record(f'{mode}/{count}/shared_methods', len({name.split('.')[2] for name in bodies}) == 2 and (not general or len(bodies) == 2) and (shallow is None or bodies == shallow), methods=list(bodies))
                shallow = bodies
                command[1] = '--check'
                checked = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
                record(f'{mode}/{count}/no_execution_during_checking', checked.returncode == 0 and not checked.stdout and not checked.stderr,
                       exit=checked.returncode, stdout=checked.stdout, stderr=checked.stderr)
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
