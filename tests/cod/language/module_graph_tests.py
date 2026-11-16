#!/usr/bin/env python3
"""Deep source graphs preserve ordered incorporation, sharing and diagnostics."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from foundation_tests import ROOT, MODE, COD
from callable_value_staging_tests import inventory

IO = 'using symbol "std:io" @print;'
SHARED = IO + 'func g()->u64{return 9u64;}export g;behave @print(99999u64);'
LEAF = 'func f()->u64{return 7u64;}export f;'


def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name, passed=passed, **details))
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed:
            print(json.dumps(details, indent=2), flush=True)

    with tempfile.TemporaryDirectory(prefix='cod-module-graph-') as directory:
        root = Path(directory)
        (root / 'shared.cod').write_text(SHARED)
        entry, ir = root / 'main.cod', root / 'main.ll'
        entry.write_text(IO + 'using "m0.cod" f;using "./shared.cod" g;behave @print(f());')
        def run(general, execute=False):
            command = [str(COD), '--run' if execute else '--check', str(entry), '--emit-llvm', str(ir)]
            if general:
                command.append('--no-specialize')
            completed = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=120)
            return dict(exit=completed.returncode, stdout=completed.stdout, stderr=completed.stderr)

        shallow_bodies = {}
        for depth in (64, 20000):
            for i in range(depth):
                source = IO + f'using "m{i+1}.cod" f;' if i + 1 < depth else IO + LEAF + f'behave @print({depth}u64);'
                if i % 1000 == 0:
                    source = IO + f'using "m{i+1}.cod" f;using "shared.cod" g;using "./shared.cod" g as again;behave @print({i}u64);'
                (root / f'm{i}.cod').write_text(source + 'export f;')
            expected = ''.join(f'{n}\n' for n in [depth, 99999, *reversed(range(0, depth, 1000)), 7])
            for general in (False, True):
                mode = 'general' if general else 'default'
                label = f'{mode}/depth_{depth}'
                checked = run(general)
                record(label + '/checking_executes_no_initializers', checked == dict(exit=0, stdout='', stderr=''), output=checked)
                executed = run(general, True)
                record(label + '/ordered_once_only_initialization', executed == dict(exit=0, stdout=expected, stderr=''), output=executed)
                bodies = inventory(ir.read_text()) if executed['exit'] == 0 else {}
                record(label + '/shared_native_methods', len(bodies) == 2 and
                       (depth == 64 or bodies == shallow_bodies.get(general)), methods=list(bodies))
                if depth == 64:
                    shallow_bodies[general] = bodies

        # These failures occur after traversing the full chain, not at its root.
        leaf = root / 'm19999.cod'
        failures = {
            'cycle': ('using "m0.cod" f;export f;', 'cyclic module construction remains unproved'),
            'missing_source': ('using "absent.cod" f;export f;', 'cannot read CoD source'),
            'unpublished_resource': ('func hidden()->u64{return 7u64;}export hidden;', "source module does not export 'f'"),
            'interleaved_selection': (IO + 'behave @print(1u64);using "shared.cod" g;' + LEAF, 'first-reference incorporation analysis'),
            'retained_scope': (LEAF + 'val x<-7u64;', 'retained dependency initialization scope remains unproved'),
        }
        for name, (source, diagnostic) in failures.items():
            leaf.write_text(source)
            for general in (False, True):
                output = run(general)
                record(('general' if general else 'default') + '/' + name,
                       output['exit'] == 2 and not output['stdout'] and diagnostic in output['stderr'], output=output)

        leaf.write_text(IO + LEAF + '1u64/0u64;')
        for general in (False, True):
            output = run(general, True)
            record(('general' if general else 'default') + '/initialization_failure_stops_later_modules',
                   output['exit'] == 70 and not output['stdout'] and 'division by zero' in output['stderr'], output=output)

    count = sum(result['passed'] for result in results)
    print(f'Module graphs: {count}/{len(results)} passed')
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
