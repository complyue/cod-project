#!/usr/bin/env python3
"""Source definitions, selected interpretations and native coverage stay distinct."""
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

IO = 'using symbol "std:io" @print;'
BASE = IO + '''
func score(x:T)->u64 where T{return 1u64;}
func twice(x:u64){return score(x)+score(x);}
func relay(n:u64){if(n==0u64){return twice(n);}else{return relay(n-1u64);};}
export score;export twice;export relay;
behave @print(100u64);
'''
EXT = '''
import "base.cod" score;func score(x:u64)->u64{return 7u64;}export score;
using "base.cod" twice as frozen;export frozen;
export import "base.cod" twice as adapted;
export import "base.cod" relay;
'''
ENTRY = IO + '''
using "base.cod" score as base;using "ext.cod" score as extended;
using "ext.cod" frozen;using "ext.cod" adapted;import "ext.cod" relay;
behave @print(base(1u64));behave @print(extended(1u64));behave @print(base(1u64));
behave @print(frozen(1u64));behave @print(adapted(1u64));behave @print(relay(10u64));
'''

def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name, passed=passed, **details))
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed:
            print(json.dumps(details, indent=2), flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-import-staging-') as directory:
        root = Path(directory)
        for name, source in {'base.cod': BASE, 'ext.cod': EXT}.items():
            (root / name).write_text(source)
        def run(source, general, execute=True):
            entry, ir = root / 'main.cod', root / 'main.ll'
            entry.write_text(source)
            command = [str(COD), '--run' if execute else '--check', str(entry), '--emit-llvm', str(ir)]
            if general:
                command.append('--no-specialize')
            completed = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
            return dict(exit=completed.returncode, stdout=completed.stdout, stderr=completed.stderr), ir.read_text() if completed.returncode == 0 else ''
        for general in (False, True):
            mode = 'general' if general else 'default'
            expected = dict(exit=0, stdout='100\n1\n7\n1\n2\n14\n14\n', stderr='')
            output, ir = run(ENTRY, general)
            bodies = inventory(ir)
            record(mode + '/source_and_receiver_contexts', output == expected, output=output)
            identities = {re.match(r'cod.method.(\d+)\.', name).group(1) for name in bodies}
            record(mode + '/interpretations_retain_source_definitions', len(identities) == 4 and
                   any('.twice.interpretation.' in name for name in bodies) and any('.twice.native' in name for name in bodies), methods=list(bodies))
            repeated, repeated_ir = run('import "./ext.cod" relay as again;' + ENTRY, general)
            record(mode + '/import_aliases_share_coverage', repeated == expected and inventory(repeated_ir) == bodies, output=repeated)
            deeper, deeper_ir = run(ENTRY.replace('10u64', '1000u64'), general)
            record(mode + '/recursion_depth_adds_no_methods', deeper == expected and set(inventory(deeper_ir)) == set(bodies), output=deeper)
            record(mode + '/static_interpretation_adds_no_runtime_storage', bool(ir) and not any(name in ir for name in
                   ('@malloc', '@realloc', '@cod_lang_defer_allocate', '@cod_lang_callable_create', '@cod_lang_callable_copy')))
            checked, _ = run(ENTRY, general, False)
            record(mode + '/checking_executes_no_initializers', checked == dict(exit=0, stdout='', stderr=''), output=checked)
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
