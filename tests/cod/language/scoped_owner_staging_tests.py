#!/usr/bin/env python3
"""Fresh scope evidence preserves inferred contracts and shared native methods."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from callable_value_staging_tests import inventory
from foundation_tests import ROOT, MODE, COD

SOURCE = '''
using symbol "std:io" @print;
func first(ref a:i64 in A,ref b:i64 in B)->ref i64 in A where {owner A,owner B}{return a;}
func second(ref a:i64 in A,ref b:i64 in B)->ref i64 in B where {owner A,owner B}{return b;}
func tag(ref a:i64 in O,ref b:i64 in O)->bool where {owner O}{return true;}
func tag(ref a:i64 in O,ref b:i64 in P)->i64 where {owner O,owner P}{return 0;}
func code(kind:bool)->u64{return 100u64;}
func code(kind:i64)->u64{return 200u64;}
func inspect(select:F,ref input:i64 in O,n:u64) where {F,owner O}{
 if(n==0u64){return {{val local<-2;ref selected=select(input,local);code(tag(input,selected))}};}
 else{return inspect(select,input,n-1u64);};
}
val slot<-1;
behave @print(inspect(first,slot,30u64));behave @print(inspect(second,slot,30u64));
'''

def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name, passed=passed, **details))
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed: print(json.dumps(details, indent=2), flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-scoped-owner-staging-') as directory:
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
            expected = dict(exit=0, stdout='100\n200\n', stderr='')
            methods = inventory(ir)
            record(mode + '/native', output == expected, output=output)
            record(mode + '/shared_methods', len(methods) == (7 if general else 9) and
                sum(name.endswith('.general') for name in methods) == 1, methods=list(methods))
            record(mode + '/owner_contexts_share_body', sum('.inspect.' in name for name in methods) == (1 if general else 3), methods=list(methods))
            deep, deep_ir = run(SOURCE.replace('30u64', '300u64'), general)
            record(mode + '/recursion_preserves_inventory', deep == expected and inventory(deep_ir) == methods, output=deep)
            checked, _ = run(SOURCE, general, False)
            record(mode + '/checking_has_no_effects', checked == dict(exit=0, stdout='', stderr=''), output=checked)
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
