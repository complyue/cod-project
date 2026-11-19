#!/usr/bin/env python3
"""Separate complete capture construction, duplication and readonly invocation."""
from pathlib import Path
from native_test_support import NativeFixture, run_native

SOURCE = '''
using symbol "std:io" @print;
proc observe(value:u64)->void{behave @print(value);return;}
func one()->const u64{return 1u64;}
func wrap(value:func()->const u64)->func()->const u64{
 return func[value]()->const u64{return value();};
}
func inspect(ref value:const func()->const u64 in O)->const u64 where {owner O}{return value();}
func factory(retained:const func()->const u64)->func()->const u64{
 return func[retained]()->const u64{return inspect(retained);};
}
proc stored()->void{
 observe(10001u64);
 func view=factory(wrap(one));
 observe(20001u64);
 observe(10002u64);
 func duplicate=view;
 observe(20002u64);
 observe(10003u64);
 val query<-make local typeof(view()){0u64};
 for(index in 0u64..100u64){view();duplicate();};
 observe(20003u64);
 observe(view());observe(duplicate());
 return;
}
stored();
proc direct(retained:const func()->const u64)->void{
 func read[retained](offset:u64)->const u64{return inspect(retained)+offset;};
 observe(10004u64);
 val query<-make local typeof(read(0u64)){0u64};
 for(index in 0u64..100u64){read(0u64);};
 observe(20004u64);
 observe(read(1u64));
 return;
}
direct(wrap(one));
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
#include <array>
#include <cassert>
#include <map>
struct Phase{
 std::uint64_t creates=0,copies=0,releases=0,records=0;
 std::uint64_t initial_images=0,final_images=0,initial_bytes=0,final_bytes=0;
 bool visited=false;
};
static std::array<Phase,4> phases;
static std::map<void *,std::uint64_t> images;
static std::uint64_t active=0,bytes=0,peak=0,created=0,copied=0,released=0;
static void track(void *image){
 assert(image);const auto size=*static_cast<std::uint64_t *>(image);
 assert(images.emplace(image,size).second);bytes+=size;if(bytes>peak)peak=bytes;
}
extern "C" void *cod_lang_callable_create(std::uint64_t fixed,std::uint64_t count,
 void *const *children,const std::uint64_t *offsets,std::uint64_t *status){
 auto *image=cod_image_create(fixed,count,children,offsets,status);track(image);++created;
 if(active)++phases[active-1].creates;return image;
}
extern "C" void *cod_lang_callable_copy(const void *source,std::uint64_t *status){
 auto *image=cod_image_copy(source,status);track(image);++copied;
 if(active)++phases[active-1].copies;return image;
}
extern "C" void cod_lang_callable_free(void *image){
 const auto found=images.find(image);assert(found!=images.end());bytes-=found->second;images.erase(found);
 ++released;if(active)++phases[active-1].releases;cod_image_free(image);
}
extern "C" void *cod_lang_defer_allocate(std::uint64_t size,std::uint64_t *status){
 if(active)++phases[active-1].records;return cod_record_allocate(size,status);
}
extern "C" void cod_lang_print_u64(std::uint64_t value){
 if(value>10000 && value<=10004){
  assert(!active);active=value-10000;auto &phase=phases[active-1];assert(!phase.visited);
  phase.visited=true;phase.initial_images=images.size();phase.initial_bytes=bytes;return;
 }
 if(value>20000 && value<=20004){
  assert(active==value-20000);auto &phase=phases[active-1];phase.final_images=images.size();phase.final_bytes=bytes;
  if(active>=3){
   assert(phase.creates==0 && phase.copies==0 && phase.releases==0 && phase.records==0);
   assert(phase.initial_images==phase.final_images && phase.initial_bytes==phase.final_bytes);
  }
  active=0;return;
 }
 cod_trace(value);
}
extern "C" void cod_lang_exception_finish() noexcept{
 assert(!active && images.empty() && bytes==0 && created+copied==released);
 for(const auto &phase:phases)assert(phase.visited);
 cod_finish();
 std::printf("{\"created\":%llu,\"copied\":%llu,\"released\":%llu,\"peak_bytes\":%llu,\"phases\":[",
  (unsigned long long)created,(unsigned long long)copied,(unsigned long long)released,(unsigned long long)peak);
 const std::array<const char *,4> names={"construction","duplication","stored_invocation_and_query","direct_invocation_and_query"};
 for(std::size_t i=0;i<phases.size();++i){
  const auto &phase=phases[i];
  std::printf("%s{\"name\":\"%s\",\"creates\":%llu,\"copies\":%llu,\"releases\":%llu,\"records\":%llu,\"initial_images\":%llu,\"final_images\":%llu,\"initial_bytes\":%llu,\"final_bytes\":%llu}",
   i?",":"",names[i],(unsigned long long)phase.creates,(unsigned long long)phase.copies,(unsigned long long)phase.releases,
   (unsigned long long)phase.records,(unsigned long long)phase.initial_images,(unsigned long long)phase.final_images,
   (unsigned long long)phase.initial_bytes,(unsigned long long)phase.final_bytes);
 }
 std::puts("]}");
}
'''
FIXTURE = NativeFixture(SOURCE, WRAPPERS, HARNESS, ['1', '1', '2'], sources=(Path(__file__),))
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
