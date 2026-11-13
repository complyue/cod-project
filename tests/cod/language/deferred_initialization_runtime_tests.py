#!/usr/bin/env python3
"""Faulted registrations cannot supply initialization to later cleanup reads."""
from pathlib import Path
import exception_value_runtime_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemory;
using "std:exceptions" MissingReturnException;
func missing()->const i64 {}
proc observe(x:u64)->void{behave @print(x);}
proc normal()->void {
  val x=make local u64;
  {{defer{observe(x);}finally{};defer{x<-41u64;};}};
  observe(x);
}
proc failed_registration()->void {
  val x=make local u64;
  defer{observe(x);}finally{observe(9002u64);};
  observe(9001u64);
  defer{x<-42u64;};
}
proc failed_action()->void {
  val x=make local u64;
  defer{observe(x);}catch(e:const MissingReturnException){observe(x);};
  defer{x<-43u64;missing();};
}
proc nested()->void {
  val x=make local u64;
  defer{
    x<-44u64;observe(9001u64);
    try{defer{observe(x);};}catch(e:const OutOfMemory){observe(x);observe(9002u64);};
  };
}
normal();
try{failed_registration();}catch(e:const OutOfMemory){observe(1u64);};
failed_action();nested();
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
  assert(bytes==16);
  if(deny_record){++failures;status[0]=1;return nullptr;}
  auto *record=cod_record_allocate(bytes,status);assert(record && live.insert(record).second);++created;
  if(live.size()>peak)peak=live.size();
  return record;
}
extern "C" void cod_lang_defer_free(void *record){assert(live.erase(record)==1);++released;cod_record_free(record);}
extern "C" void cod_lang_print_u64(std::uint64_t value){
  if(value==9001){assert(!deny_record && live.size()<=1);deny_record=true;return;}
  if(value==9002){assert(deny_record && live.empty());deny_record=false;return;}
  cod_trace(value);
}
extern "C" void cod_lang_exception_finish() noexcept {
  assert(!deny_record && live.empty() && created==released && created==6 && peak==2 && failures==2);
  cod_finish();
  std::printf("{\"records\":%llu,\"released\":%llu,\"peak\":%llu,\"failures\":%llu}\n",
      (unsigned long long)created,(unsigned long long)released,(unsigned long long)peak,(unsigned long long)failures);
}
'''
runner.EXPECTED_VALUES = ['41', '41', '1', '43', '44']
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
