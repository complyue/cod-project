#!/usr/bin/env python3
"""Check fixture isolation, dependency evidence and native-provider injection."""
import ast
from dataclasses import FrozenInstanceError
import datetime
import hashlib
import json
import os
from pathlib import Path
import random
import subprocess
import sys
import tempfile

from language_test_support import DIRECTORY, ROOT, MODE, COD, checked, diagnostic_matches, shadow_diagnostics
from native_test_support import NativeFixture, run_native


def main():
    modules = {path.stem: path for path in DIRECTORY.glob('*.py')}
    dependencies = {}
    fixtures = []
    for name, path in modules.items():
        tree = ast.parse(path.read_text())
        dependencies[name] = set()
        for node in ast.walk(tree):
            if isinstance(node, ast.Import):
                dependencies[name].update(alias.name for alias in node.names if alias.name in modules)
            elif isinstance(node, ast.ImportFrom) and node.module in modules:
                dependencies[name].add(node.module)
        if any(isinstance(node, ast.Assign) and any(isinstance(target, ast.Name) and target.id == 'FIXTURE'
                                                  for target in node.targets) for node in tree.body):
            fixtures.append(name)
    fixtures.sort()
    snapshot = '''
import importlib, json, sys
from pathlib import Path
sys.path.insert(0, str(Path.cwd() / 'tests/cod/language'))
def describe(f):
    return dict(source=f.source, wrappers=f.wrappers, harness=f.harness, expected_values=f.expected_values,
                expected_shadows=f.expected_shadows, source_files=dict(f.source_files),
                sources=sorted(str(p.relative_to(Path.cwd())) for p in f.sources),
                transform=None if f.transform_ir is None else f.transform_ir.__module__+'.'+f.transform_ir.__qualname__)
loaded = {}; initial = {}
for name in json.loads(sys.argv[1]):
    loaded[name] = importlib.import_module(name)
    initial[name] = describe(loaded[name].FIXTURE)
print(json.dumps(dict(initial=initial, final={n: describe(m.FIXTURE) for n,m in loaded.items()})))
'''
    shuffled = fixtures.copy()
    random.Random(17).shuffle(shuffled)
    snapshots = []
    for order in (fixtures, list(reversed(fixtures)), shuffled):
        result = subprocess.run([sys.executable, '-c', snapshot, json.dumps(order)], cwd=ROOT,
                                capture_output=True, text=True, check=True, timeout=120)
        snapshots.append(json.loads(result.stdout))
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name, passed=bool(passed), **details))
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed:
            print(json.dumps(details, indent=2), flush=True)
    def closure(name, found=None):
        found = set() if found is None else found
        for dependency in dependencies[name]:
            if dependency not in found:
                found.add(dependency)
                closure(dependency, found)
        return found
    for name in fixtures:
        expected = snapshots[0]['initial'][name]
        inputs = {str(modules[n].relative_to(ROOT)) for n in closure(name) | {name}}
        recorded = set(expected['sources']) | {'tests/cod/language/native_test_support.py',
                                              'tests/cod/language/language_test_support.py'}
        record(name + '/independent_inputs',
               all(run['initial'][name] == expected == run['final'][name] for run in snapshots)
               and inputs <= recorded, missing_sources=sorted(inputs - recorded))
    values, files = ['1'], {'helper.cod': 'source'}
    base = NativeFixture('source', 'wrappers', 'harness', values, source_files=files)
    values.append('2')
    files['helper.cod'] = 'changed'
    record('copies_mutable_inputs', base.expected_values == ('1',) and base.source_files['helper.cod'] == 'source')
    try:
        base.source = 'changed'
    except FrozenInstanceError:
        record('immutable_fields', True)
    else:
        record('immutable_fields', False)
    try:
        base.source_files['helper.cod'] = 'changed'
    except TypeError:
        record('immutable_source_files', True)
    else:
        record('immutable_source_files', False)
    child = base.updated(expected_values=['3'], sources=[Path(__file__)])
    record('independent_derived_inputs', base.expected_values == ('1',) and child.expected_values == ('3',)
           and not base.sources and child.sources == (Path(__file__).resolve(),))
    diagnostics = []
    stdout, _ = checked([sys.executable, '-c', 'print("ok")'], ROOT)
    record('checked_stdout', stdout == 'ok\n')
    warning = 'warning: expected [fixture]\n'
    checked([sys.executable, '-c', 'import sys;sys.stderr.write(' + repr(warning) + ')'], ROOT, ('[fixture]',), diagnostics)
    record('checked_allowed_diagnostic', diagnostics == [warning])
    for name, code in [('nonzero_exit', 'raise SystemExit(9)'), ('unexpected_stderr', 'import sys;sys.stderr.write("unexpected\\n")')]:
        try:
            checked([sys.executable, '-c', code], ROOT)
        except RuntimeError:
            record(name, True)
        else:
            record(name, False)
    record('diagnostic_text_excludes_path', not diagnostic_matches('/tmp/expected.cod:1:2: different\n', 'expected', Path('/tmp/expected.cod')))
    record('diagnostic_location', diagnostic_matches('/tmp/source.cod:1:2: expected\n', 'expected', Path('/tmp/source.cod'), (1, 2)))
    record('shadow_diagnostic_multiplicity', shadow_diagnostics("/tmp/a:1:2: warning: declaration shadows outer binding 'x' [-Wshadow]\n", ['x'])[0]
           and not shadow_diagnostics("/tmp/a:1:2: warning: declaration shadows outer binding 'x' [-Wshadow]\n", ['x', 'x'])[0])
    # Exercise the provider transformation through the same native runner used
    # by derived exception fixtures, with small independently checked consumers.
    from unknown_exception_runtime_tests import FIXTURE as provider
    source = '''
using symbol "std:io" @print;
using "std:exceptions" StaticallyUnknownTypeException;
using "std:exceptions" UnsoundRuntimeTypingError;
func unknown_boundary()->const u64 throws StaticallyUnknownTypeException{return 0u64;}
func typing_boundary()->const u64{return 0u64;}
try{behave @print(unknown_boundary());}catch(e:const StaticallyUnknownTypeException){behave @print(e.category);};
try{behave @print(typing_boundary());}catch(e:const UnsoundRuntimeTypingError){behave @print(e.category);};
'''
    probe = provider.updated(source=source, wrappers='#define cod_lang_exception_finish cod_finish\n',
                             harness='''
#undef cod_lang_exception_finish
extern "C" void cod_lang_exception_finish() noexcept { cod_finish(); std::printf("{}\\n"); }
''', expected_values=['7', '6'], expected_shadows=[], source_files={}, sources=[Path(__file__)])
    with tempfile.TemporaryDirectory(prefix='cod-native-fixture-proof-') as directory:
        report = Path(directory) / 'native.json'
        run_native(probe, report)
        evidence = json.loads(report.read_text())
        for result in evidence['results']:
            record('provider_injection/' + result['name'], result['passed'] and len(result['injected']) == 2
                   and result['original_ir_sha256'] != result['injected_ir_sha256'], evidence=result)
    passed = sum(result['passed'] for result in results)
    print(f'Native fixture support: {passed}/{len(results)} passed')
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        names = {Path(__file__).stem}
        for name in fixtures:
            names.update(closure(name) | {name})
        sources = [ROOT / 'cod/main.cc', *sorted((ROOT / 'cod').glob('lang_*.hh')), *(modules[n] for n in sorted(names))]
        digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE, passed=passed, total=len(results), results=results, executable_sha256=digest(COD),
            sources={str(path.relative_to(ROOT)): digest(path) for path in sources}), indent=2) + '\n')
    return 0 if passed == len(results) else 1


if __name__ == '__main__':
    raise SystemExit(main())
