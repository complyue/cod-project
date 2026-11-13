#!/usr/bin/env python3
"""Faulted registrations preserve returns and distinguish result-less recovery."""
from pathlib import Path
import exception_value_runtime_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemory;
using "std:exceptions" MissingReturnException;
proc observe(x:u64)->void{behave @print(x);}
proc retained()->const u64 {
  defer catch(e:const OutOfMemory){observe(1u64);observe(9002u64);};
  defer{observe(9001u64);defer observe(99u64);};
  return 7u64;
}
proc replacement()->const u64 {
  defer catch(e:const OutOfMemory){observe(2u64);observe(9002u64);return 8u64;};
  observe(9001u64);defer observe(99u64);return 5u64;
}
proc absent()->const u64 {
  defer catch(e:const OutOfMemory){observe(3u64);observe(9002u64);};
  observe(9001u64);defer observe(99u64);return 5u64;
}
observe(retained());observe(replacement());
try{absent();{}}catch(e:const MissingReturnException){observe(4u64);};
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
#include <set>
static bool deny_record=false;
static std::uint64_t created=0,released=0,failures=0,peak=0;
static std::set<void *> live;
extern "C" void *cod_lang_defer_allocate(std::uint64_t bytes,std::uint64_t *status){
  if(deny_record){++failures;assert(bytes==72);status[0]=1;return nullptr;}
  assert(bytes==16);
  auto *record=cod_record_allocate(bytes,status);assert(record && live.insert(record).second);++created;
  if(live.size()>peak)peak=live.size();
  return record;
}
extern "C" void cod_lang_defer_free(void *record){assert(live.erase(record)==1);++released;cod_record_free(record);}
extern "C" void cod_lang_print_u64(std::uint64_t value){
  if(value==9001){assert(live.size()==1);deny_record=true;return;}
  if(value==9002){assert(live.empty() && created==released);deny_record=false;return;}
  cod_trace(value);
}
extern "C" void cod_lang_exception_finish() noexcept {
  assert(!deny_record && live.empty() && created==released && created==4 && peak==2 && failures==3);
  cod_finish();
  std::printf("{\"records\":%llu,\"released\":%llu,\"peak\":%llu,\"failures\":%llu}\n",
      (unsigned long long)created,(unsigned long long)released,(unsigned long long)peak,(unsigned long long)failures);
}
'''
runner.EXPECTED_VALUES = ['1', '7', '2', '8', '3', '4']
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
