#!/usr/bin/env python3
"""Account for complete images through growth, preparation and cleanup failures."""
from pathlib import Path
import exception_value_runtime_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemory;
using "std:exceptions" MissingReturnException;
proc observe(x:u64)->void{behave @print(x);}
func base(x:u64)->const u64{return x;}
func other(x:u64)->const u64{return x+10u64;}
func wrap(f:func(u64)->const u64)->func(u64)->const u64{
  return func[f](x:u64)->const u64{return f(x)+1u64;};
}
func apply(f:func(u64)->const u64,x:u64)->const u64{return f(x);}
func missing()->const u64{}
proc one(x:u64)->void{observe(x);}
proc two(x:u64)->void{observe(x+10u64);}
proc wrapped(p:proc(u64)->void)->proc(u64)->void{return proc[p](x:u64)->void{p(x);};}
func replaced()->func(u64)->const u64{defer{return wrap(other);};return wrap(base);}
proc copy_recovery()->func(u64)->const u64{
  func f=wrap(base);func g=wrap(other);
  defer catch(e:const OutOfMemory){observe(9031u64);};
  defer{observe(9030u64);return g;};
  return f;
}
func retained()->func(u64)->const u64{
  defer catch(e:const MissingReturnException){};
  defer{missing();{}};
  return wrap(base);
}
{{func f=wrap(wrap(base));func g=f;f<-wrap(other);observe(g(1u64));observe(f(1u64));}};
observe(9000u64);
{{
  func f=base;for(i in 0u64..80u64){{f<-wrap(f);}};
  observe(9010u64);val total<-0u64;
  for(i in 0u64..10000u64){total<-total+f(i);};
  observe(9011u64);observe(total);
}};
observe(9000u64);
{{
  func f=wrap(base);observe(9020u64);
  try{f<-wrap(f);{}}catch(e:const OutOfMemory){observe(9021u64);};
  observe(f(2u64));
  func g=wrap(other);observe(9030u64);
  try{f<-g;{}}catch(e:const OutOfMemory){observe(9031u64);};
  observe(f(3u64));
  observe(9030u64);
  try{f({{observe(9999u64);4u64}});{}}catch(e:const OutOfMemory){observe(9031u64);};
  observe(9040u64);
  try{apply(f,missing());{}}catch(e:const MissingReturnException){observe(9041u64);};
}};
observe(9000u64);
{{
  proc f=wrapped(one);
  {{defer f(5u64);observe(9050u64);try{defer f(99u64);}catch(e:const OutOfMemory){observe(9051u64);};f<-two;}};
}};
observe(9000u64);
{{observe(replaced()(6u64));observe(retained()(7u64));observe(copy_recovery()(8u64));}};
observe(9000u64);
'''
runner.WRAPPERS = '''
#define cod_lang_callable_create cod_image_create
#define cod_lang_callable_copy cod_image_copy
#define cod_lang_callable_free cod_image_free
#define cod_lang_defer_allocate cod_record_allocate
#define cod_lang_defer_free cod_record_free
#define cod_lang_exception_finish cod_finish
#define cod_lang_print_u64 cod_trace
'''
runner.HARNESS = r'''
#undef cod_lang_callable_create
#undef cod_lang_callable_copy
#undef cod_lang_callable_free
#undef cod_lang_defer_allocate
#undef cod_lang_defer_free
#undef cod_lang_exception_finish
#undef cod_lang_print_u64
#include <cassert>
#include <map>
#include <set>
static std::map<void *,std::uint64_t> images;
static std::set<void *> records;
static bool deny_create=false,deny_copy=false,deny_record=false,measuring=false;
static std::uint64_t created=0,copied=0,released=0,creation_failures=0,copy_failures=0,record_failures=0;
static std::uint64_t bytes=0,peak_bytes=0,largest=0,baseline=0,live_baseline=0,checkpoints=0;
static void track(void *image){
  assert(image);auto size=*static_cast<std::uint64_t *>(image);assert(images.emplace(image,size).second);
  bytes+=size;if(bytes>peak_bytes)peak_bytes=bytes;if(size>largest)largest=size;
}
extern "C" void *cod_lang_callable_create(std::uint64_t fixed,std::uint64_t count,void *const *children,
                                         const std::uint64_t *offsets,std::uint64_t *status){
  assert(!measuring);
  if(deny_create){++creation_failures;status[0]=1;return nullptr;}
  auto *image=cod_image_create(fixed,count,children,offsets,status);track(image);++created;return image;
}
extern "C" void *cod_lang_callable_copy(const void *source,std::uint64_t *status){
  assert(!measuring);
  if(deny_copy){++copy_failures;status[0]=1;return nullptr;}
  auto *image=cod_image_copy(source,status);track(image);++copied;return image;
}
extern "C" void cod_lang_callable_free(void *image){
  auto found=images.find(image);assert(found!=images.end());bytes-=found->second;images.erase(found);++released;cod_image_free(image);
}
extern "C" void *cod_lang_defer_allocate(std::uint64_t size,std::uint64_t *status){
  if(deny_record){++record_failures;status[0]=1;return nullptr;}
  auto *record=cod_record_allocate(size,status);assert(record && records.insert(record).second);return record;
}
extern "C" void cod_lang_defer_free(void *record){assert(records.erase(record)==1);cod_record_free(record);}
extern "C" void cod_lang_print_u64(std::uint64_t value){
  if(value==9000){assert(images.empty() && records.empty() && bytes==0);++checkpoints;return;}
  if(value==9010){assert(!measuring && !images.empty());baseline=created+copied;measuring=true;return;}
  if(value==9011){assert(measuring && created+copied==baseline);measuring=false;return;}
  if(value==9020){deny_create=true;live_baseline=images.size();return;}
  if(value==9021){assert(deny_create && images.size()==live_baseline);deny_create=false;return;}
  if(value==9030){deny_copy=true;live_baseline=images.size();return;}
  if(value==9031){assert(deny_copy && images.size()==live_baseline);deny_copy=false;return;}
  if(value==9040){live_baseline=images.size();return;}
  if(value==9041){assert(images.size()==live_baseline);return;}
  if(value==9050){deny_record=true;live_baseline=images.size();return;}
  if(value==9051){assert(deny_record && images.size()==live_baseline);deny_record=false;return;}
  assert(value!=9999);cod_trace(value);
}
extern "C" void cod_lang_exception_finish() noexcept{
  assert(images.empty() && records.empty() && bytes==0 && !measuring && !deny_create && !deny_copy && !deny_record);
  assert(created+copied==released && checkpoints==5 && creation_failures==1 && copy_failures==3 && record_failures==1 && largest>4000);
  cod_finish();
  std::printf("{\"created\":%llu,\"copied\":%llu,\"released\":%llu,\"peak_bytes\":%llu,\"largest\":%llu,\"creation_failures\":%llu,\"copy_failures\":%llu,\"record_failures\":%llu}\n",
      (unsigned long long)created,(unsigned long long)copied,(unsigned long long)released,(unsigned long long)peak_bytes,
      (unsigned long long)largest,(unsigned long long)creation_failures,(unsigned long long)copy_failures,(unsigned long long)record_failures);
}
'''
runner.EXPECTED_VALUES = ['3','12','50795000','3','4','5','17','8','9']
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
