#!/usr/bin/env python3
"""Callable snapshots, failed preparation and allocation-free empty environments."""
from pathlib import Path
import exception_value_runtime_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemory;
using "std:exceptions" MissingReturnException;
proc observe(x:u64)->void{behave @print(x);}
proc one(x:u64)->void{observe(x+1u64);}
proc two(x:u64)->void{observe(x+2u64);}
func missing()->const u64{}
func plus(x:u64)->const u64{return x+1u64;}
func minus(x:u64)->const u64{return x-1u64;}
func factory(b:bool)->func(u64)->const u64{return if(b){plus}else{minus};}
func replaced()->func(u64)->const u64{defer{return minus;};return plus;}
proc maker()->proc(u64)->void{return one;}
proc failed()->void{
  proc f=one;defer{f(30u64);observe(9002u64);};
  observe(9001u64);defer f(99u64);
}
proc preparation()->void{proc f=one;defer{observe(4u64);};defer f(missing());}
proc f=one;
{{defer f(10u64);defer f(11u64);f<-two;}};f(10u64);
{{defer f({{f<-one;20u64}});}};f(20u64);
try{failed();}catch(e:const OutOfMemory){observe(1u64);};
try{preparation();}catch(e:const MissingReturnException){observe(3u64);};
observe(replaced()(40u64));{{defer (maker())(40u64);}};
observe(9003u64);
val total<-0u64;
for(i in 0u64..10000u64){{func a=factory(true);func b=a;a<-minus;total<-total+b(i);}};
observe(9004u64);observe(total);
'''
runner.WRAPPERS = '''
#define cod_lang_defer_allocate cod_record_allocate
#define cod_lang_defer_free cod_record_free
#define cod_lang_exception_finish cod_finish
#define cod_lang_print_u64 cod_trace
#define cod_lang_callable_create cod_image_create
#define cod_lang_callable_copy cod_image_copy
#define cod_lang_callable_free cod_image_free
'''
runner.HARNESS = r'''
#undef cod_lang_defer_allocate
#undef cod_lang_defer_free
#undef cod_lang_exception_finish
#undef cod_lang_print_u64
#undef cod_lang_callable_create
#undef cod_lang_callable_copy
#undef cod_lang_callable_free
#include <cassert>
#include <set>
static bool denied=false,measuring=false;
static std::uint64_t created=0,released=0,failures=0,peak=0,prepared=0,baseline=0;
static std::uint64_t images=0,copies=0,image_releases=0;
static std::set<void *> live;
extern "C" void *cod_lang_callable_create(std::uint64_t fixed,std::uint64_t count,void *const *children,
                                         const std::uint64_t *offsets,std::uint64_t *status){
  ++images;return cod_image_create(fixed,count,children,offsets,status);
}
extern "C" void *cod_lang_callable_copy(const void *source,std::uint64_t *status){++copies;return cod_image_copy(source,status);}
extern "C" void cod_lang_callable_free(void *image){++image_releases;cod_image_free(image);}
extern "C" void *cod_lang_defer_allocate(std::uint64_t bytes,std::uint64_t *status){
  assert(!measuring && (bytes==16 || bytes==128));
  if(denied){assert(bytes==128);++failures;status[0]=1;return nullptr;}
  if(bytes==128)++prepared;
  auto *record=cod_record_allocate(bytes,status);assert(record && live.insert(record).second);++created;
  if(live.size()>peak)peak=live.size();return record;
}
extern "C" void cod_lang_defer_free(void *record){assert(live.erase(record)==1);++released;cod_record_free(record);}
extern "C" void cod_lang_print_u64(std::uint64_t value){
  if(value==9001){assert(!denied && live.size()==1);denied=true;return;}
  if(value==9002){assert(denied && live.empty());denied=false;return;}
  if(value==9003){assert(!measuring && live.empty());baseline=created;measuring=true;return;}
  if(value==9004){assert(measuring && created==baseline && live.empty());measuring=false;return;}
  cod_trace(value);
}
extern "C" void cod_lang_exception_finish() noexcept{
  assert(!denied && !measuring && live.empty() && created==released && created==7 && peak==2 && failures==1 && prepared==4);
  assert(images==0 && copies==0 && image_releases==0);
  cod_finish();
  std::printf("{\"records\":%llu,\"released\":%llu,\"peak\":%llu,\"failures\":%llu,\"images\":%llu,\"copies\":%llu}\n",
      (unsigned long long)created,(unsigned long long)released,(unsigned long long)peak,(unsigned long long)failures,
      (unsigned long long)images,(unsigned long long)copies);
}
'''
runner.EXPECTED_VALUES = ['12','11','12','22','21','31','1','4','3','39','41','50005000']
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
