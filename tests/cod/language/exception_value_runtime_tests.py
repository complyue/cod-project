#!/usr/bin/env python3
"""Inject allocation failures at exception and deferred-record service boundaries."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import tempfile

from foundation_tests import ROOT, MODE, COD
from regional_runtime_tests import checked

SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemory;
using "std:exceptions" MissingReturnException;
func missing()->const i64 {}
func error()->const MissingReturnException {
  try{missing();}catch(e:const MissingReturnException){return e;};
}
proc observe(x:u64)->void{behave @print(x);}
proc records()->void {
  defer observe(1u64);
  behave @print(9001u64);
  defer observe(99u64);
}
try{records();}catch(e:const OutOfMemory){
  behave @print(e.category);
  behave @print(e.cause_count);
  behave @print(9002u64);
};
val original<-error();
val saved<-try{throw original from original;}catch(e:const MissingReturnException){e};
behave @print(saved.cause_count);
behave @print(9011u64);
try{throw saved from saved;}catch(e:const OutOfMemory){
  behave @print(e.category);
  behave @print(e.cause_count);
  behave @print(9012u64);
};
behave @print(original.cause_count);
behave @print(saved.cause_count);
behave @print(cause_at(saved,0u64).category);
'''
WRAPPERS = '''
#define cod_lang_defer_allocate cod_record_allocate
#define cod_lang_defer_free cod_record_free
#define cod_lang_exception_chain cod_chain
#define cod_lang_exception_finish cod_finish
#define cod_lang_print_u64 cod_trace
'''
HARNESS = r'''
#undef cod_lang_defer_allocate
#undef cod_lang_defer_free
#undef cod_lang_exception_chain
#undef cod_lang_exception_finish
#undef cod_lang_print_u64
#include <cassert>
static bool deny_record=false, deny_chain=false;
static std::uint64_t created=0, released=0, record_failures=0, chain_failures=0, chain_calls=0;
static std::uint64_t record_bytes=0;
extern "C" void *cod_lang_defer_allocate(std::uint64_t bytes,std::uint64_t *status) {
  if(deny_record){++record_failures;status[0]=1;return nullptr;}
  auto *record=cod_record_allocate(bytes,status);assert(record);record_bytes=bytes;++created;return record;
}
extern "C" void cod_lang_defer_free(void *record){++released;cod_record_free(record);}
extern "C" std::uint64_t cod_lang_exception_chain(std::uint64_t category,std::uint64_t site,
    std::uint64_t requested,std::uint64_t available,std::uint64_t cause,std::uint64_t *status) {
  ++chain_calls;
  if(deny_chain){++chain_failures;status[0]=1;return 0;}
  return cod_chain(category,site,requested,available,cause,status);
}
extern "C" void cod_lang_print_u64(std::uint64_t value){
  if(value==9001){deny_record=true;return;}
  if(value==9002){assert(record_failures==1 && created==released);deny_record=false;return;}
  if(value==9011){deny_chain=true;deny_record=true;return;}
  if(value==9012){assert(chain_failures==1 && record_failures==1);deny_chain=deny_record=false;return;}
  cod_trace(value);
}
extern "C" void cod_lang_exception_finish() noexcept {
  assert(!deny_record && !deny_chain && created==released && created==1);
  assert(record_failures==1 && chain_calls==2 && chain_failures==1);
  cod_finish();
  std::printf("{\"records\":%llu,\"released\":%llu,\"record_bytes\":%llu,\"record_failures\":%llu,\"chain_calls\":%llu,\"chain_failures\":%llu}\n",
      (unsigned long long)created,(unsigned long long)released,(unsigned long long)record_bytes,(unsigned long long)record_failures,
      (unsigned long long)chain_calls,(unsigned long long)chain_failures);
}
'''
EXPECTED_VALUES = ['1', '1', '0', '1', '1', '0', '0', '1', '5']
EXTRA_SOURCES = []
SOURCE_FILES = {}


def main():
    toolchain = ROOT / MODE
    results = []
    with tempfile.TemporaryDirectory(prefix="cod-exception-runtime-") as directory:
        scratch = Path(directory)
        entry = scratch / "main.cod"
        for name, contents in SOURCE_FILES.items():
            path = scratch / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(contents)
        entry.write_text(SOURCE)
        for general in (False, True):
            ir, bc, runtime = [scratch / name for name in ("main.ll", "main.bc", "runtime.cc")]
            command = [COD, "--check", entry, "--emit-llvm", ir, "--emit-bc", bc, "--emit-runtime", runtime]
            if general:
                command.append("--no-specialize")
            output, frontend_seconds = checked(command, ROOT, ('[-Wmissing-return]',))
            if output:
                raise RuntimeError("Checking executed runtime effects")
            checked([toolchain / "bin/llvm-as", ir, "-o", scratch / "roundtrip.bc"], ROOT)
            checked([toolchain / "bin/opt", "-passes=verify", "-disable-output", bc], ROOT)
            source = scratch / "instrumented.cc"
            source.write_text(WRAPPERS + runtime.read_text() + HARNESS)
            executable = scratch / "native"
            compile_command = [toolchain / "bin/clang++", "-std=c++20", "-O2", "-g", "-UNDEBUG", "-stdlib=libc++",
                               "-fuse-ld=lld", "-I" + str(ROOT / "include"), bc, source, "-o", executable,
                               "-L" + str(toolchain / "lib"), "-Wl,-rpath," + str(toolchain / "lib")]
            _, compile_seconds = checked(compile_command, ROOT)
            output, execution_seconds = checked([executable], ROOT)
            *values, metrics = output.splitlines()
            if values != EXPECTED_VALUES:
                raise RuntimeError(f"Unexpected exception trace: {values}")
            results.append({"name": "general" if general else "default", "passed": True, "metrics": json.loads(metrics),
                            "stdout": values, "frontend_seconds": frontend_seconds, "compile_seconds": compile_seconds,
                            "execution_seconds": execution_seconds, "check_command": list(map(str, command)),
                            "compile_command": list(map(str, compile_command))})
            print(f"PASS {results[-1]['name']}: {metrics}", flush=True)
    if report := os.environ.get("COD_LANGUAGE_REPORT"):
        sources = [ROOT / "cod/main.cc", *sorted((ROOT / "cod").glob("lang_*.hh"))]
        sources += [Path(__file__), Path(__file__).with_name("foundation_tests.py"), Path(__file__).with_name("regional_runtime_tests.py"), *EXTRA_SOURCES]
        Path(report).write_text(json.dumps({"recorded_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(), "toolchain": MODE,
                                         "passed": len(results), "total": len(results), "results": results,
                                         "executable_sha256": hashlib.sha256(COD.read_bytes()).hexdigest(),
                                         "sources": {str(path.relative_to(ROOT)): hashlib.sha256(path.read_bytes()).hexdigest() for path in sources}}, indent=2) + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
