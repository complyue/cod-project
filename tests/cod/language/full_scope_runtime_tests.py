#!/usr/bin/env python3
"""Immediate full-scope capture preparation, completion and exact cleanup."""
from pathlib import Path
from native_test_support import NativeFixture, run_native

SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemoryError;
proc observe(value:u64)->void{behave @print(value);}
func one()->const u64{return 1u64;}
func two()->const u64{return 2u64;}
func wrap(value:func()->const u64)->func()->const u64{
 return func[value]()->const u64{return value();};
}
proc attempt(val failure:u64)->void{
 func left=wrap(one);func right=wrap(two);val seed<-1u64;
 observe(10000u64+failure);
 try{
  {{
   observe(70000u64);
   defer{observe(80000u64);observe(left());observe(right());observe(seed);};
   left<-wrap(two);right<-wrap(one);seed<-9u64;
  }};
 }catch(e:const OutOfMemoryError){observe(70001u64);};
 observe(left());observe(right());observe(seed);observe(20000u64);
}
for(index in 1u64..9u64){attempt(index);};
proc result_failure()->void{
 func original=wrap(one);observe(30000u64);
 try{
  func result={{original<-wrap(two);observe(30001u64);return original;}};
  observe(99999u64);
 }catch(e:const OutOfMemoryError){observe(30002u64);};
 observe(original());observe(30003u64);
}
result_failure();
'''
WRAPPERS = '''
#define cod_lang_callable_create cod_image_create
#define cod_lang_callable_copy cod_image_copy
#define cod_lang_callable_free cod_image_free
#define cod_lang_defer_allocate cod_record_allocate
#define cod_lang_defer_free cod_record_free
#define cod_lang_exception_finish cod_finish
#define cod_lang_print_u64 cod_trace
'''
HARNESS = r'''
#undef cod_lang_callable_create
#undef cod_lang_callable_copy
#undef cod_lang_callable_free
#undef cod_lang_defer_allocate
#undef cod_lang_defer_free
#undef cod_lang_exception_finish
#undef cod_lang_print_u64
#include <array>
#include <cassert>
#include <map>
#include <set>
static std::map<void *,std::uint64_t> images;
static std::set<void *> records;
static std::uint64_t acquisitions=0,released=0,failures=0,budget=0,bytes=0,peak=0;
static std::uint64_t baseline_count=0,baseline_bytes=0,failed_attempts=0,completed_attempts=0;
static unsigned local_values=0;
static bool active=false,entered=false,cleaned=false,result_failed=false;
static bool deny(){if(budget && --budget==0){++failures;return true;}return false;}
static void track(void *image){
 assert(image);const auto size=*static_cast<std::uint64_t *>(image);
 assert(images.emplace(image,size).second);++acquisitions;bytes+=size;if(bytes>peak)peak=bytes;
}
extern "C" void *cod_lang_callable_create(std::uint64_t fixed,std::uint64_t count,
 void *const *children,const std::uint64_t *offsets,std::uint64_t *status){
 if(deny()){status[0]=1;return nullptr;}
 auto *image=cod_image_create(fixed,count,children,offsets,status);track(image);return image;
}
extern "C" void *cod_lang_callable_copy(const void *source,std::uint64_t *status){
 if(deny()){status[0]=1;return nullptr;}
 auto *image=cod_image_copy(source,status);track(image);return image;
}
extern "C" void cod_lang_callable_free(void *image){
 const auto found=images.find(image);assert(found!=images.end());
 bytes-=found->second;images.erase(found);++released;cod_image_free(image);
}
extern "C" void *cod_lang_defer_allocate(std::uint64_t size,std::uint64_t *status){
 auto *record=cod_record_allocate(size,status);assert(record && records.insert(record).second);return record;
}
extern "C" void cod_lang_defer_free(void *record){assert(records.erase(record)==1);cod_record_free(record);}
extern "C" void cod_lang_print_u64(std::uint64_t value){
 if(value>10000 && value<10009){
  assert(!active && !budget && records.empty());active=true;entered=cleaned=false;local_values=0;
  budget=value-10000;baseline_count=images.size();baseline_bytes=bytes;return;
 }
 if(value==70000){assert(active && !entered);entered=true;budget=0;return;}
 if(value==70001){assert(active && !entered && !budget);++failed_attempts;return;}
 if(value==80000){assert(active && entered && !cleaned);cleaned=true;local_values=3;return;}
 if(local_values){
  constexpr std::array<std::uint64_t,3> expected{2,1,9};
  assert(value==expected[3-local_values]);--local_values;return;
 }
 if(value==20000){
  assert(active && records.empty() && images.size()==baseline_count && bytes==baseline_bytes);
  assert(!local_values && entered==cleaned);if(entered)++completed_attempts;
  active=false;budget=0;return;
 }
 if(value==30000){assert(!active);baseline_count=images.size();baseline_bytes=bytes;return;}
 if(value==30001){assert(!budget);budget=1;return;}
 if(value==30002){
  assert(!budget && !result_failed && images.size()==baseline_count && bytes==baseline_bytes);
  result_failed=true;return;
 }
 if(value==30003){assert(result_failed);return;}
 assert(value!=99999);cod_trace(value);
}
extern "C" void cod_lang_exception_finish() noexcept{
 assert(!active && !budget && !local_values && records.empty() && images.empty() && bytes==0);
 assert(acquisitions==released && failed_attempts>=2 && completed_attempts>=1 && result_failed);
 assert(failures==failed_attempts+1);cod_finish();
 std::printf("{\"acquisitions\":%llu,\"released\":%llu,\"preparation_failures\":%llu,"
             "\"completed_attempts\":%llu,\"result_failures\":1,\"peak_bytes\":%llu}\n",
  (unsigned long long)acquisitions,(unsigned long long)released,(unsigned long long)failed_attempts,
  (unsigned long long)completed_attempts,(unsigned long long)peak);
}
'''
FIXTURE = NativeFixture(SOURCE, WRAPPERS, HARNESS, ['1', '2', '1'] * 8 + ['1'], sources=(Path(__file__),))
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
