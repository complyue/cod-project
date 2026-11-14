#!/usr/bin/env python3
"""Measure immutable catch snapshots and inject failure in nested registration."""
from pathlib import Path
import exception_value_runtime_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" MissingReturnException;
using "std:exceptions" OutOfMemory;
func missing()->const i64 {}
func error()->const MissingReturnException {
  try{missing();}catch(e:const MissingReturnException){return e;};
}
func chain(e:MissingReturnException)->const MissingReturnException {
  return try{throw e from e;}catch(e:const MissingReturnException){e};
}
proc snapshots()->void {
  val saved<-error();
  for(i in 0..4){
    saved<-chain(saved);
    try{throw saved;}catch(e){defer{defer{behave @print(e.cause_count);};};};
  };
}
snapshots();
{{try{throw error();}catch(e){defer{behave @print(10u64);};};}};
{{try{throw error();}catch(e){defer{behave @print(e.category);behave @print(e.category);};};}};
{{try{throw error();}catch(a:const MissingReturnException){try{throw chain(a);}catch(b){defer{behave @print(a.cause_count);behave @print(b.cause_count);};};};}};
proc failing()->void {
  try{throw chain(error());}catch(e){
    defer{behave @print(e.cause_count);};
    defer{behave @print(9001u64);defer{behave @print(e.cause_count);};};
  };
}
try{failing();}catch(e:const OutOfMemory){behave @print(e.cause_count);behave @print(9002u64);};
'''
runner.WRAPPERS = '''
#define cod_lang_defer_allocate cod_record_allocate
#define cod_lang_defer_free cod_record_free
#define cod_lang_exception_finish cod_finish
#define cod_lang_print_u64 cod_trace
'''
runner.HARNESS = r'''
#undef cod_lang_defer_allocate
#undef cod_lang_defer_free
#undef cod_lang_exception_finish
#undef cod_lang_print_u64
#include <cassert>
#include <map>
static bool deny_record=false;
static std::uint64_t created=0,released=0,failures=0,peak=0;
static std::map<void *,std::uint64_t> live;
static std::map<std::uint64_t,std::uint64_t> sizes;
extern "C" void *cod_lang_defer_allocate(std::uint64_t bytes,std::uint64_t *status){
  if(deny_record){++failures;assert(bytes==72);status[0]=1;return nullptr;}
  auto *record=cod_record_allocate(bytes,status);assert(record && !live.contains(record));
  live.emplace(record,bytes);++sizes[bytes];++created;
  if(live.size()>peak)peak=live.size();
  return record;
}
extern "C" void cod_lang_defer_free(void *record){assert(live.erase(record)==1);++released;cod_record_free(record);}
extern "C" void cod_lang_print_u64(std::uint64_t value){
  if(value==9001){assert(live.size()==1);deny_record=true;return;}
  if(value==9002){assert(live.empty() && failures==1);deny_record=false;return;}
  cod_trace(value);
}
extern "C" void cod_lang_exception_finish() noexcept {
  assert(!deny_record && live.empty() && created==released && created==13 && peak==4 && failures==1);
  assert(sizes.size()==3 && sizes[16]==1 && sizes[72]==11 && sizes[128]==1);
  cod_finish();
  std::printf("{\"records\":%llu,\"released\":%llu,\"peak\":%llu,\"failures\":%llu,\"zero_capture_bytes\":16,\"one_capture_bytes\":72,\"two_capture_bytes\":128}\n",
      (unsigned long long)created,(unsigned long long)released,(unsigned long long)peak,(unsigned long long)failures);
}
'''
runner.EXPECTED_VALUES = ['4', '3', '2', '1', '10', '5', '5', '0', '1', '1', '0']
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == "__main__":
    raise SystemExit(runner.main())
