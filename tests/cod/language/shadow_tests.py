#!/usr/bin/env python3
"""Source declarations share shadow diagnostics without changing lexical identity."""
from collections import Counter
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
from foundation_tests import ROOT, MODE, COD

IO = 'using symbol "std:io" @print;\n'
NPE = 'using "std:exceptions" NullPointerException;\n'
MODULE = 'func target(x:u64)->u64{return x+1u64;}\n'
CASES = {
    'module_callable_parameter': (IO + MODULE +
        'func apply(target:func(u64)->u64)->u64{return target(6u64);}behave @print(apply(target));', '7\n', ['target']),
    'module_callable_local': (IO + MODULE +
        'proc run()->void{val target<-7u64;behave @print(target);}run();', '7\n', ['target']),
    'module_callable_alias': (IO + MODULE +
        'proc run()->void{val x<-7u64;var target->x;behave @print(target);}run();', '7\n', ['target']),
    'named_local_definition': (IO + MODULE +
        'proc run()->void{func target(x:u64)->u64{return x+2u64;};behave @print(target(5u64));}run();', '7\n', ['target']),
    'initializer_before_binding': (IO + MODULE +
        'proc run()->void{func target=target;behave @print(target(6u64));}run();', '7\n', ['target']),
    'module_symbol_parameter': (IO + 'symbol named:u64;'
        'func f(named:u64)->u64{return named;}behave @print(f(7u64));', '7\n', ['named']),
    'module_type_parameter': (IO + NPE +
        'func f(NullPointerException:u64)->u64{return NullPointerException;}behave @print(f(7u64));', '7\n', ['NullPointerException']),
    'module_value_parameter': (IO + 'val outer<-1u64;'
        'func f(outer:u64)->u64{return outer;}behave @print(f(7u64));', '7\n', ['outer']),
    'uncaptured_outer_parameter': (IO + 'func f(x:u64)->u64{'
        'func inner(x:u64)->u64{return x;};return inner(7u64);}behave @print(f(1u64));', '7\n', ['x']),
    'uncaptured_outer_local': (IO + 'func f()->u64{val x<-1u64;'
        'func inner()->u64{val x<-7u64;return x;};return inner();}behave @print(f());', '7\n', ['x']),
    'uncaptured_ancestor': (IO + 'func f(x:u64)->u64{'
        'func middle()->u64{func inner(x:u64)->u64{return x;};return inner(7u64);};return middle();}behave @print(f(1u64));', '7\n', ['x']),
    'uncaptured_ancestor_alias': (IO + 'func f(x:u64)->u64{'
        'func middle()->u64{func inner()->u64{val y<-7u64;var x->y;return x;};return inner();};return middle();}behave @print(f(1u64));', '7\n', ['x']),
    'uncaptured_ancestor_definition': (IO + 'func f(x:u64)->u64{'
        'func middle()->u64{func inner()->u64{func x()->u64{return 7u64;};return x();};return inner();};return middle();}behave @print(f(1u64));', '7\n', ['x']),
    'uncaptured_ancestor_loop': (IO + 'proc f(i:u64)->void{'
        'proc middle()->void{proc inner()->void{for(i in 0u64..2u64){behave @print(i);};};inner();};middle();}f(7u64);', '0\n1\n', ['i']),
    'uncaptured_ancestor_fold': (IO + 'func f(i:u64,s:u64)->u64{'
        'func middle()->u64{func inner()->u64{return fold(i in 0u64..3u64,s=0u64){s+i};};return inner();};return middle();}behave @print(f(7u64,9u64));', '3\n', ['i', 's']),
    'uncaptured_ancestor_catch': (IO + NPE + 'proc f(e:u64)->void{'
        'proc middle()->void{proc inner()->void{ptr empty:u64=null in here;try{empty;{}}'
        'catch(e:const NullPointerException){behave @print(7u64);};};inner();};middle();}f(1u64);', '7\n', ['e']),
    'uncaptured_ancestor_owner': (IO + 'func f(O:u64)->u64{'
        'func middle()->u64{func inner(ref x:u64 in O)->u64 where {owner O}{return x;};val y<-7u64;return inner(y);};return middle();}behave @print(f(1u64));', '7\n', ['O']),
    'shadow_self': (IO + 'proc run()->void{func recur(recur:u64)->u64{return recur;};behave @print(recur(7u64));}run();', '7\n', ['recur']),
    'shadow_enclosing_callable': (IO + 'proc run()->void{func outer()->u64{'
        'func inner(outer:u64)->u64{return outer;};return inner(7u64);};behave @print(outer());}run();', '7\n', ['outer']),
    'owner_binding': (IO + 'symbol O:u64;'
        'func f(ref x:u64 in O)->u64 where {owner O}{return x;}val x<-7u64;behave @print(f(x));', '7\n', ['O', 'x']),
    'loop_index': (IO + 'val i<-7;for(i in 0..2){behave @print(i);};behave @print(i);', '0\n1\n7\n', ['i']),
    'fold_index_and_accumulator': (IO + 'val i<-7,s<-9;behave @print(fold(i in 0..3,s=0){s+i});'
        'behave @print(i);behave @print(s);', '3\n7\n9\n', ['i', 's']),
    'catch_parameter': (IO + NPE + 'val e<-7u64;ptr empty:u64=null in here;try{empty;{}}'
        'catch(e:const NullPointerException){behave @print(1u64);};behave @print(e);', '1\n7\n', ['e']),
    'deferred_catch_parameter': (IO + NPE + 'val e<-7u64;{{defer catch(e:const NullPointerException)'
        '{behave @print(1u64);};ptr empty:u64=null in here;empty;}};behave @print(e);', '1\n7\n', ['e']),
    'two_source_declarations': (IO + 'val x<-1u64;{{val x<-2u64;behave @print(x);}};'
        '{{val x<-3u64;behave @print(x);}};', '2\n3\n', ['x', 'x']),
    'generic_instances_once': (IO + 'func f(x:T)->const T where T{{{val x<-x;return x;}}}'
        'behave @print(f(7u64));behave @print(f(true));behave @print(f(8u8));', '7\ntrue\n8\n', ['x']),
    'callable_domain_once': (IO + 'func f(x:T)->const T where T{{{val x<-x;return x;}}}'
        'func apply(g:func(u64)->u64)->u64{return g(7u64);}behave @print(apply(f));', '7\n', ['x']),
    'recursive_context_once': (IO + 'func f(x:u64)->u64{if(x==0u64){return 7u64;}'
        'else{{{val x<-x-1u64;return f(x);}}};}behave @print(f(20u64));', '7\n', ['x']),
    'imported_interpretations_once': ({
        'library.cod': 'func f(x:u64)->u64{{{val x<-x;return x;}}}export f;',
        'main.cod': IO + 'using "library.cod" f as a;import "library.cod" f as b;'
                    'behave @print(a(7u64));behave @print(b(8u64));',
    }, '7\n8\n', ['x']),
    'captured_slots_are_not_declarations': (IO + 'func constant()->u64{return 7u64;}func f(x:func()->u64)->u64{'
        'func inner=func[x]()->u64{return x();};return inner();}behave @print(f(constant));', '7\n', []),
    'module_capture_is_not_declaration': (IO + MODULE + 'func f()->u64{'
        'func inner=func[target]()->u64{return target(6u64);};return inner();}behave @print(f());', '7\n', []),
    'query_only_body_not_demanded': (IO + 'func f(x:u64)->u64{'
        'val y<-make local typeof((func(x:u64)->u64{return x;})(0u64)){7u64};return y;}behave @print(f(1u64));', '7\n', []),
    'overloads_are_not_shadow': (IO + 'func f(x:u64)->u64{return x;}func f(x:bool)->bool{return x;}'
        'behave @print(f(7u64));behave @print(f(true));', '7\ntrue\n', []),
    'new_names': (IO + 'func f(x:u64)->u64{func inner(y:u64)->u64{return y;};return inner(x);}'
        'behave @print(f(7u64));', '7\n', []),
}
ERRORS = {
    'duplicate_value': ('val x<-1u64;val x<-2u64;', "duplicate declaration of 'x'"),
    'duplicate_parameter': ('func f(x:u64,x:u64)->u64{return x;}f(1u64,2u64);', 'duplicate parameter name'),
    'duplicate_catch': (NPE + 'ptr empty:u64=null in here;try{empty;}catch(e:const NullPointerException){val e<-1u64;};',
                        "duplicate declaration of catch parameter 'e'"),
    'nearest_noncallable': (MODULE + 'func f(target:u64)->u64{return target(1u64);}f(2u64);',
                           "nearest binding 'target' is not callable"),
    'nearest_nonsymbol': ('symbol p:u64;func f(p:u64)->u64{return @p;}f(1u64);',
                         "nearest binding 'p' is not a symbol"),
}


def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name, passed=passed, **details))
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed:
            print(json.dumps(details, indent=2), flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-shadow-') as directory:
        path = Path(directory) / 'main.cod'
        for name, (source, stdout, names) in CASES.items():
            files = source if isinstance(source, dict) else {'main.cod': source}
            for filename, contents in files.items():
                (Path(directory) / filename).write_text(contents)
            for mode, arguments in [('check', ['--check']), ('default', ['--run']), ('general', ['--run', '--no-specialize']),
                                    ('shadow_error', ['--run', '-Werror=shadow']), ('all_errors', ['--run', '--warnings-as-errors'])]:
                command = [str(COD), arguments[0], str(path), *arguments[1:]]
                run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
                promoted = bool(names) and mode in ('shadow_error', 'all_errors')
                warnings = re.findall(r"^(.+):(\d+):(\d+): warning: declaration shadows outer binding '([^']+)' \[-Wshadow\](.*)$",
                                      run.stderr, re.M)
                lines = [line for line in run.stderr.splitlines() if line]
                locations = [(file, line, column, binding) for file, line, column, binding, suffix in warnings]
                passed = (run.returncode == (2 if promoted else 0) and
                          run.stdout == ('' if promoted or mode == 'check' else stdout) and
                          (len(warnings) == 1 and warnings[0][3] in names if promoted else
                           Counter(binding for _, _, _, binding, _ in warnings) == Counter(names)) and
                          len(lines) == len(warnings) and len(set(locations)) == len(locations) and
                          all(Path(file) in {Path(directory) / name for name in files} and int(line) > 0 and int(column) > 0 and
                              suffix == (' (treated as error)' if promoted else '') for file, line, column, _, suffix in warnings))
                record(name + '/' + mode, passed, command=command, exit=run.returncode, stdout=run.stdout, stderr=run.stderr)
        for name, (source, diagnostic) in ERRORS.items():
            path.write_text(source)
            command = [str(COD), '--run', str(path)]
            run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
            record(name, run.returncode == 2 and not run.stdout and diagnostic in run.stderr,
                   command=command, exit=run.returncode, stdout=run.stdout, stderr=run.stderr)
    passed = sum(item['passed'] for item in results)
    print(f'Shadow diagnostics: {passed}/{len(results)} passed')
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [ROOT / 'cod/main.cc', *sorted((ROOT / 'cod').glob('lang_*.hh')), Path(__file__),
                   Path(__file__).with_name('foundation_tests.py')]
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(), toolchain=MODE,
            passed=passed, total=len(results), results=results, executable_sha256=hashlib.sha256(COD.read_bytes()).hexdigest(),
            sources={str(path.relative_to(ROOT)): hashlib.sha256(path.read_bytes()).hexdigest() for path in sources}), indent=2) + '\n')
    return passed != len(results)


if __name__ == '__main__':
    raise SystemExit(main())
