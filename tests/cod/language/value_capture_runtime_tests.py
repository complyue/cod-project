#!/usr/bin/env python3
"""Track capture-local ownership, preparation failure and replacement effects."""
from pathlib import Path
from native_test_support import NativeFixture, run_native

SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemoryError;
using "std:exceptions" MissingReturnError;
proc observe(x:u64)->void{behave @print(x);}
func one()->const u64{return 1u64;}
func two()->const u64{return 2u64;}
func three()->const u64{return 3u64;}
func missing()->const u64{}
func wrap(f:func()->const u64)->func()->const u64{return func[f]()->const u64{return f();};}
proc prefix_step(val failure:u64)->void{
 func left=wrap(one);func right=wrap(two);
 proc action=proc[left,right](ref output:u64 in O)->void where {owner O}{
  observe(70000u64);output<-1u64;left<-wrap(two);right<-wrap(one);
 };
 val output<-0u64;
 observe(10000u64+failure);
 try{action(output);}catch(e:const OutOfMemoryError){};
 if(output==0u64){observe(70001u64);}else{observe(70002u64);};
 observe(20000u64);observe(left());observe(right());
}
for(index in 1u64..13u64){prefix_step(index);};
proc boundary(val scenario:u64)->void{
 func left=wrap(one);func right=wrap(two);func last=wrap(three);
 proc action=proc[left,right,last,scenario]()->void{
  try{
   left<-{left<-right;observe(40000u64+scenario);if(scenario==1u64){missing();{}}else{};last};{}
  }catch(e:const MissingReturnError){observe(left());}
   catch(e:const OutOfMemoryError){observe(left());};
  observe(left());
 };
 action();observe(50000u64);observe(left());observe(right());observe(last());
}
boundary(1u64);boundary(2u64);boundary(3u64);
proc return_preparation()->void{
 func retained=wrap(one);
 proc produce=proc[retained](failing:bool)->func()->const u64{
  retained<-wrap(two);
  if(failing){observe(60001u64);}else{};
  return retained;
 };
 observe(60000u64);
 try{produce(true);{}}catch(e:const OutOfMemoryError){observe(60002u64);};
 observe(retained());observe(produce(false)());
}
return_preparation();
proc readonly_observation()->void{
 func retained=wrap(one);
 func read=func[retained]()->const u64{return retained();};
 observe(30000u64);
 for(index in 0u64..100u64){read();};
 observe(30001u64);observe(read());
}
readonly_observation();
'''
WRAPPERS = '''
#define cod_lang_callable_create cod_image_create
#define cod_lang_callable_copy cod_image_copy
#define cod_lang_callable_free cod_image_free
#define cod_lang_defer_allocate cod_record_allocate
#define cod_lang_exception_finish cod_finish
#define cod_lang_print_u64 cod_trace
'''
HARNESS = r'''
#undef cod_lang_callable_create
#undef cod_lang_callable_copy
#undef cod_lang_callable_free
#undef cod_lang_defer_allocate
#undef cod_lang_exception_finish
#undef cod_lang_print_u64
#include <cassert>
#include <map>
static std::map<void *,std::uint64_t> images;
static std::uint64_t acquisitions=0,released=0,failures=0,budget=0,bytes=0,peak=0;
static std::uint64_t baseline_count=0,baseline_bytes=0,readonly_acquisitions=0,records=0,readonly_records=0;
static bool readonly_phase=false,outer_failure=false,entered=false;
static bool deny(){
 assert(!readonly_phase);
 if(budget && --budget==0){++failures;return true;}
 return false;
}
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
 if(deny()){status[0]=1;outer_failure=false;return nullptr;}
 auto *image=cod_image_copy(source,status);track(image);return image;
}
extern "C" void cod_lang_callable_free(void *image){
 const auto found=images.find(image);assert(found!=images.end());
 bytes-=found->second;images.erase(found);++released;cod_image_free(image);
}
extern "C" void *cod_lang_defer_allocate(std::uint64_t size,std::uint64_t *status){
 ++records;return cod_record_allocate(size,status);
}
extern "C" void cod_lang_print_u64(std::uint64_t value){
 if(value>10000 && value<10013){
  assert(!budget);budget=value-10000;baseline_count=images.size();baseline_bytes=bytes;entered=false;return;
 }
 if(value==20000){assert(images.size()==baseline_count && bytes==baseline_bytes);budget=0;return;}
 if(value==40001 || value==40003)return;
 if(value==40002){assert(!budget);budget=1;outer_failure=true;return;}
 if(value==50000){assert(!budget && !outer_failure);return;}
 if(value==60000){baseline_count=images.size();baseline_bytes=bytes;return;}
 if(value==60001){assert(!budget);budget=1;return;}
 if(value==60002){assert(!budget && images.size()==baseline_count && bytes==baseline_bytes);return;}
 if(value==70000){entered=true;return;}
 if(value==70001){assert(!entered);return;}
 if(value==70002){assert(entered);return;}
 if(value==30000){readonly_phase=true;readonly_acquisitions=acquisitions;readonly_records=records;return;}
 if(value==30001){
  assert(readonly_phase && acquisitions==readonly_acquisitions && records==readonly_records);
  readonly_phase=false;return;
 }
 cod_trace(value);
}
extern "C" void cod_lang_exception_finish() noexcept{
 assert(images.empty() && bytes==0 && released==acquisitions && failures>=6);
 assert(!budget && !readonly_phase && !outer_failure);cod_finish();
 std::printf("{\"acquisitions\":%llu,\"released\":%llu,\"failures\":%llu,\"peak_bytes\":%llu,\"readonly_extra_acquisitions\":0,\"readonly_extra_records\":0}\n",
  (unsigned long long)acquisitions,(unsigned long long)released,(unsigned long long)failures,(unsigned long long)peak);
}
'''
EXPECTED = ['1', '2'] * 12 + ['2', '2', '1', '2', '3'] * 2 + ['3', '1', '2', '3', '1', '2', '1']
FIXTURE = NativeFixture(SOURCE, WRAPPERS, HARNESS, EXPECTED, sources=(Path(__file__),))
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
