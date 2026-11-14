#!/usr/bin/env python3
"""Concurrent native entries retain independent bindings and scope restoration."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from foundation_tests import ROOT, MODE, COD
from regional_runtime_tests import checked

TLS_SLOTS = 1
EXTRA_SOURCES = []
SOURCE = '''
using symbol "std:io" @print;
symbol @context:u64;
symbol @inspected:u64;
val independent<-make local typeof(@inspected){1u64};
effect @context<-1u64;
behave @print(100u64);
{{
 effect @context<-2u64;
 behave @print(101u64);
 behave @print(@context);
}};
behave @print(102u64);
behave @print(@context);
'''
WRAPPERS = '#define cod_lang_print_u64 cod_default_print\n#define cod_lang_exception_finish cod_default_finish\n'
HARNESS = r'''
#undef cod_lang_print_u64
#undef cod_lang_exception_finish
#include <atomic>
#include <cassert>
#include <thread>
static std::atomic<unsigned> phase{0}, completed{0};
static thread_local unsigned role=0, seen=0;
static void wait_for(unsigned target){while(phase.load()<target)std::this_thread::yield();}
extern "C" int cod_test_entry();
extern "C" void cod_lang_print_u64(std::uint64_t value){
  if(value==100){if(role==1){wait_for(1);phase.store(2);wait_for(3);}return;}
  if(value==101){if(role==0){phase.store(1);wait_for(2);}else{phase.store(4);wait_for(5);}return;}
  if(value==102){if(role==0){phase.store(3);wait_for(4);}return;}
  assert(value==(seen==0?2:1));++seen;
}
extern "C" void cod_lang_exception_finish() noexcept {++completed;}
int main(){
  std::thread first([]{role=0;assert(cod_test_entry()==0 && seen==2);phase.store(5);});
  std::thread second([]{role=1;assert(cod_test_entry()==0 && seen==2);});
  first.join();second.join();assert(completed==2);std::puts("isolated bindings");
}
'''

def main():
    toolchain = ROOT / MODE
    results = []
    with tempfile.TemporaryDirectory(prefix='cod-dynamic-threads-') as directory:
        scratch = Path(directory)
        entry = scratch / 'main.cod'
        entry.write_text(SOURCE)
        for general in (False, True):
            label = 'general' if general else 'default'
            ir, runtime = scratch / 'main.ll', scratch / 'runtime.cc'
            command = [COD, '--check', entry, '--emit-llvm', ir, '--emit-runtime', runtime]
            if general:
                command.append('--no-specialize')
            output, _ = checked(command, ROOT)
            assert not output
            code = ir.read_text()
            assert code.count('define i32 @main()') == 1
            assert code.count('thread_local global %cod.scalar') == TLS_SLOTS
            code = code.replace('define i32 @main()', 'define i32 @cod_test_entry()')
            source = scratch / 'instrumented.cc'
            source.write_text(WRAPPERS + runtime.read_text() + HARNESS)
            for isolated in (True, False):
                candidate = scratch / 'candidate.ll'
                candidate.write_text(code if isolated else code.replace('thread_local ', ''))
                executable = scratch / 'native'
                compile_command = [toolchain / 'bin/clang++', '-std=c++20', '-O2', '-g', '-UNDEBUG',
                    '-stdlib=libc++', '-fuse-ld=lld', '-pthread', '-I'+str(ROOT/'include'), candidate, source,
                    '-o', executable, '-L'+str(toolchain/'lib'), '-Wl,-rpath,'+str(toolchain/'lib')]
                checked(compile_command, ROOT)
                run = subprocess.run([str(executable)], cwd=scratch, capture_output=True, text=True, timeout=30)
                passed = (run.returncode == 0 and run.stdout == 'isolated bindings\n' and not run.stderr) if isolated else (
                    run.returncode != 0 and 'Assertion' in run.stderr)
                results.append(dict(name=label+('/tls' if isolated else '/shared-storage-negative-control'),
                    passed=passed, exit=run.returncode, stdout=run.stdout, stderr=run.stderr,
                    check_command=list(map(str, command)), compile_command=list(map(str, compile_command))))
                print(('PASS ' if passed else 'FAIL ')+results[-1]['name'], flush=True)
    count = sum(result['passed'] for result in results)
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [ROOT/'cod/main.cc', *sorted((ROOT/'cod').glob('lang_*.hh')), Path(__file__),
                   Path(__file__).with_name('foundation_tests.py'), Path(__file__).with_name('regional_runtime_tests.py'), *EXTRA_SOURCES]
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE, passed=count, total=len(results), results=results,
            executable_sha256=hashlib.sha256(COD.read_bytes()).hexdigest(),
            sources={str(path.relative_to(ROOT)):hashlib.sha256(path.read_bytes()).hexdigest() for path in sources}), indent=2)+'\n')
    return 0 if count == len(results) else 1

if __name__ == '__main__':
    raise SystemExit(main())
