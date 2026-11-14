#!/usr/bin/env python3
"""Common exception families cover concrete categories with shared native methods."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from foundation_tests import ROOT, MODE, COD
from callable_value_staging_tests import inventory
from family_contract_tests import ERROR

SOURCE = ERROR + '''
symbol @pick:proc(SystemException)->u64;
proc @pick(e:MissingReturnException)->u64{return 7u64;}
proc @pick(e:T)->u64 where T{return e.category;}
effect @pick;val e:SystemException<-error();
behave @print(behave @pick(e));e<-null_error();behave @print(behave @pick(e));
for(i in 0u64..10u64){effect @pick;behave @pick(e);};
'''

def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name, passed=passed, **details))
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed: print(json.dumps(details, indent=2), flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-family-contract-') as directory:
        scratch = Path(directory)
        def run(text, general, execute=True):
            source, ir = scratch / 'main.cod', scratch / 'main.ll'
            source.write_text(text)
            command = [str(COD), '--run' if execute else '--check', str(source), '--emit-llvm', str(ir)]
            if general: command.append('--no-specialize')
            completed = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
            return completed, ir.read_text() if completed.returncode == 0 else ''
        for general in (False, True):
            mode = 'general' if general else 'default'
            output, ir = run(SOURCE, general)
            bodies = inventory(ir)
            family = [name for name in bodies if '.pick.' in name]
            record(mode + '/actual_categories', output.returncode == 0 and output.stdout == '7\n4\n' and '[-Wmissing-return]' in output.stderr,
                   exit=output.returncode, stdout=output.stdout, stderr=output.stderr)
            record(mode + '/shared_family_methods', len(family) == (2 if general else 8) and
                   sum(name.endswith('.general') for name in family) == 1, methods=family)
            record(mode + '/unobserved_concrete_category', general or any('OutOfMemory' in name for name in family), methods=family)
            repeated, repeated_ir = run(SOURCE.replace('10u64', '1000u64'), general)
            record(mode + '/runtime_count_preserves_inventory', repeated.returncode == 0 and repeated.stdout == '7\n4\n' and set(inventory(repeated_ir)) == set(bodies))
            checked, _ = run(SOURCE, general, False)
            record(mode + '/checking_does_not_execute', checked.returncode == 0 and checked.stdout == '')
    count = sum(result['passed'] for result in results)
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [ROOT / 'cod/main.cc', *sorted((ROOT / 'cod').glob('lang_*.hh')), Path(__file__),
                   Path(__file__).with_name('foundation_tests.py'), Path(__file__).with_name('callable_value_staging_tests.py'),
                   Path(__file__).with_name('family_contract_tests.py')]
        digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE, passed=count, total=len(results), results=results, executable_sha256=digest(COD),
            sources={str(path.relative_to(ROOT)): digest(path) for path in sources}), indent=2) + '\n')
    return count != len(results)

if __name__ == '__main__':
    raise SystemExit(main())
