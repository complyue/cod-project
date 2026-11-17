#!/usr/bin/env python3
"""Declared contextual queries emit the same native code as their inspected type."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from foundation_tests import ROOT, MODE, COD
from declaration_query_tests import IO, ID, DECL

EXTRA_SOURCES = []
CASES = (
    ('generic', ID+DECL, 'absent(identity,1)', 'i64', '7', '7\n'),
    ('nested', ID+DECL, 'absent(identity,absent(identity,1))', 'i64', '8', '8\n'),
    ('variadic', ID+'func absent(callback:func(T)->const T,inputs:T...)->const T where T;',
     'absent(identity,1,2,3)', 'i64', '9', '9\n'),
    ('unproved_body', 'func broken(input:T)->const T where T{return input+input;}'+DECL,
     'absent(broken,true)', 'bool', 'false', 'false\n'),
    ('procedure', 'proc observe(input:T)->const T where T{behave @print(99);return input;}proc absent(callback:proc(T)->const T,input:T)->const T where T;',
     'absent(observe,1)', 'i64', '10', '10\n'),
    ('borrowed', ID+'func absent(callback:func(T)->const T,ref input:T in O)->ref T in O where {T,owner O};val slot<-make local i64;',
     'absent(identity,slot)', 'i64', '11', '11\n'),
)

def main():
    results = []
    with tempfile.TemporaryDirectory(prefix='cod-declaration-query-') as directory:
        source, output = Path(directory)/'main.cod', Path(directory)/'main.ll'
        for general in (False, True):
            for name, prefix, query, target, value, expected in CASES:
                runs = []
                spelling = 'typeof('+query+')'
                for inspected in (spelling, target+' '*(len(spelling)-len(target))):
                    source.write_text(IO+prefix+'val result<-make local '+inspected+'{'+value+'};behave @print(result);')
                    command = [str(COD), '--run', str(source), '--emit-llvm', str(output)]
                    if general:
                        command.append('--no-specialize')
                    run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
                    runs.append(dict(exit=run.returncode, stdout=run.stdout, stderr=run.stderr,
                                     ir=output.read_text() if run.returncode == 0 and output.exists() else ''))
                left, right = runs
                identical_ir = bool(left['ir']) and left['ir'] == right['ir']
                passed = left == right and left['exit'] == 0 and left['stdout'] == expected and not left['stderr'] and identical_ir
                for run in runs:
                    run.pop('ir')
                result = dict(name=('general/' if general else 'default/')+name, passed=passed,
                              queried=left, explicit=right, identical_ir=identical_ir)
                results.append(result)
                print(('PASS ' if passed else 'FAIL ')+result['name'], flush=True)
                if not passed:
                    print(json.dumps(result, indent=2), flush=True)
    count = sum(item['passed'] for item in results)
    print(f'Declaration query staging: {count}/{len(results)} passed')
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [Path(__file__).with_name("language_test_support.py"), ROOT/'cod/main.cc', *sorted((ROOT/'cod').glob('lang_*.hh')), Path(__file__),
                   Path(__file__).with_name('foundation_tests.py'), Path(__file__).with_name('declaration_query_tests.py'), *EXTRA_SOURCES]
        digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE, passed=count, total=len(results), results=results, executable_sha256=digest(COD),
            sources={str(path.relative_to(ROOT)):digest(path) for path in sources}), indent=2)+'\n')
    return count != len(results)

if __name__ == '__main__':
    raise SystemExit(main())
