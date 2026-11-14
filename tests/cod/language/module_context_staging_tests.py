#!/usr/bin/env python3
"""Module selection preserves semantic identities without runtime registration."""
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
symbol @score:proc(u64)->u64;
proc @score(n:u64)->u64{return 1u64;}
proc @score(n:T)->u64 where T{return 7u64;}
export @score;
behave @print(100u64);
'''
ADAPTER = 'using symbol "base.cod" @score as @selected;export @selected;'
OTHER = 'proc @score(n:T)->u64 where T{return 9u64;}export @score;'
# A generic symbolic definition needs its independently established slot contract.
OTHER = 'symbol @score:proc(u64)->u64;' + OTHER
ENTRY = IO + '''
using symbol "base.cod" @score as @first;
using symbol "adapter.cod" @selected as @second;
using symbol "other.cod" @score as @third;
effect @first;effect @third;
behave @print(behave @first(1u8));
behave @print(behave @second(1u64));
behave @print(behave @third(1u8));
for(i in 0u64..10u64){effect @second;behave @second(1u32);};
'''

def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name, passed=passed, **details))
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed:
            print(json.dumps(details, indent=2), flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-module-staging-') as directory:
        root = Path(directory)
        for name, source in {'base.cod': BASE, 'adapter.cod': ADAPTER, 'other.cod': OTHER}.items():
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
            expected = dict(exit=0, stdout='100\n7\n1\n9\n', stderr='')
            output, ir = run(ENTRY, general)
            bodies = inventory(ir)
            record(mode + '/selected_contexts_and_once_only_initialization', output == expected, output=output)
            identities = {re.match(r'cod.method.(\d+)\.', name).group(1) for name in bodies}
            record(mode + '/aliases_share_semantic_methods', len(identities) == 3 and (not general or len(bodies) == 3), methods=list(bodies))
            repeated, repeated_ir = run('using symbol "./base.cod" @score as @again;' + ENTRY, general)
            record(mode + '/repeated_selection_adds_no_execution_or_bodies', repeated == expected and inventory(repeated_ir) == bodies, output=repeated)
            deeper, deeper_ir = run(ENTRY.replace('10u64', '1000u64'), general)
            record(mode + '/runtime_count_adds_no_methods', deeper == expected and set(inventory(deeper_ir)) == set(bodies), output=deeper)
            checked, _ = run(ENTRY, general, False)
            record(mode + '/source_scan_executes_no_initializers', checked == dict(exit=0, stdout='', stderr=''), output=checked)
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
