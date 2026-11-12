#!/usr/bin/env python3
"""Check owning-region cleanup, root relocation and compile-time invalid access."""

import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

from regional_runtime_tests import run_case

HARNESS = r'''
#undef cod_lang_region_new
#undef cod_lang_region_free
#undef cod_lang_print_i64
#include <cassert>
#include <cstring>
#include <map>

static std::map<void *, std::uint64_t> live;
static unsigned acquired = 0, released = 0, relocated = 0;
struct Node { std::uint64_t value; shilos::regional_ptr<Node> next; };
struct Packet { std::uint64_t sequence; shilos::regional_ptr<Node> head; };
extern "C" void *cod_lang_region_new(std::uint64_t bytes, std::uint64_t *status) {
  void *p = cod_lang_region_acquire(bytes, status);
  if (p) { assert(live.emplace(p, bytes).second); ++acquired; }
  return p;
}
extern "C" void cod_lang_region_free(void *owner) {
  if (!owner) return;
  const auto found = live.find(owner);
  assert(found != live.end());
  const auto capacity = found->second;
  live.erase(found);
  ++released;
  if (capacity != 256) { cod_lang_region_release(owner); return; }
  auto &original = *static_cast<CoDRegion *>(owner);
  CoDRegionOwner copy(capacity);
  const auto used = original.occupation();
  std::memcpy(static_cast<void *>(&*copy), owner, used);
  cod_lang_region_release(owner);
  const auto *root = reinterpret_cast<const Packet *>(copy->root().get());
  assert(root->sequence == 42 && root->head->value == 7);
  assert(root->head->next.get() == root->head.get());
  ++relocated;
}
extern "C" void cod_lang_print_i64(std::int64_t step) {
  if (step == 101 || step == 102 || step == 1 || step == 5 || step == 6 || step == 7 || step == 8 || step == 9 || step == 10 || step == 11)
    assert(live.size() == 1);
  else if (step == 2) assert(live.size() == 2);
  else { assert(step == 0 || step == 3 || step == 4); assert(live.empty()); }
  cod_lang_trace_i64(step);
  if (step == 0) {
    assert(acquired == released && acquired == 19 && relocated == 1);
    std::printf("{\"owners_acquired\":%u,\"owners_released\":%u,\"root_relocations\":%u,"
                "\"pending_result_cleanup\":true,\"callee_entry_timing\":true}\n", acquired, released, relocated);
  }
}
'''

BASE = '''data Packet {sequence:u64;} region symbol state:Packet;
function fresh()-> owned region :Packet {make Packet(capacity=8u64){sequence<-1u64}}
function drop(owned region p:Packet) -> void {}
function read(region p:Packet) -> u64 {root(p).sequence}
'''

REJECTS = {
    "stale-borrow-argument": ('data Node {value:u64;} function match(node:Node in O, value:u64) -> bool where {owner O} {node.value == value} function compare() -> bool {'
                                   'owned region p:Packet <- make Packet(capacity=64u64){sequence<-1u64};'
                                   'ref n=alloc p Node(2u64); match(n, {drop(p); 0u64})} compare();',
                                   "requires a live owning region"),
    "recursive-replacement-borrow": ('function replace(owned region p:Packet,n:u64)->owned region:Packet {'
                                     'if(n==0u64){p}else{replace(fresh(),n-1u64)}} '
                                     'owned region p<-fresh(); ref v=root(p); '
                                     'owned region q<-replace(p,3u64); v.sequence;',
                                     "requires a live owning region"),
    "recursive-owner-alias": ('function next(ref v:Packet in p,owned region p:Packet,n:u64)->owned region:Packet {'
                              'if(n==0u64){p}else{next(v,fresh(),n-1u64)}} '
                              'owned region p<-fresh(); next(root(p),p,3u64);',
                              "requires the same actual owner"),
    "callable-owner-argument": ('function callback(region r:Packet)->func()->const u64 with {region r} {'
                                'val var sequence:u64 in r -> r.sequence; func()->const u64 {sequence}} '
                                'function apply(owned region r:Packet,cb:func()->const u64 with {region r})->const u64 {cb()} '
                                'owned region first<-fresh(); owned region second<-fresh(); '
                                'apply(second,callback(first));', "callable environment owner mismatch"),
    "recursive-callback-readonly": ('function writable(region r:Packet)->func()->const u64 with {region r} {'
                                    'val var value:u64 in r -> r.sequence; func()->const u64 {value}} '
                                    'function readonly(region r:Packet)->func()->const u64 with {region r} {'
                                    'val var value:const u64 in r -> r.sequence; func()->const u64 {value}} '
                                    'function step(owned region r:Packet,cb:func()->const u64 with {region r},n:u64)->const u64 {'
                                    'region projected=cb -> r; projected.sequence<-cb(); '
                                    'if(n==0u64){cb()}else{owned region next<-fresh(); step(next,readonly(next),n-1u64)}} '
                                    'owned region seed<-fresh(); step(seed,writable(seed),3u64);',
                                    "cannot transfer into a read-only destination view"),
    "consumed-name": ('proc test()->void {owned region p:Packet <- fresh(); drop(p); read(p); {}} test();',
                      "owner has been consumed"),
    "escaping-view": ('function bad() -> Packet {owned region p:Packet <- fresh(); root(p)} bad();',
                      "result would outlive its owning scope"),
    "ended-effect-owner": ('proc test()->void {owned region p:Packet <- fresh(); ref v=root(p); '
                           '{effect @state <- p; {}}; v.sequence; {}} test();', "requires a live owning region"),
    "conditional-consumption": ('proc test(flag:bool)->void {owned region p:Packet <- fresh(); '
                                'if(flag){drop(p)}else{}; read(p); {}} test(true);', "owner has been consumed"),
    "duplicate-move": ('function both(owned region a:Packet,owned region b:Packet) -> void{} '
                       'proc test()->void {owned region p:Packet <- fresh(); both(p,p)} test();',
                       "cannot be consumed by multiple parameters"),
    "later-argument-move": ('function both(region a:Packet,b:u64) -> void{} '
                            'proc test()->void {owned region p:Packet <- fresh(); both(p,{drop(p);1u64})} test();',
                            "owner has been consumed"),
    "borrow-extraction": ('function bad(region p:Packet)-> owned region :Packet{p} bad(fresh());',
                          "expected owned region :"),
    "dynamic-extraction": ('effect @state <- fresh(); drop(@state);', "expected owned region :"),
    "repeated-move": ('proc test()->void {owned region p:Packet <- fresh(); for(i in 0..2){drop(p)}} test();',
                      "repeating loop cannot consume an owner"),
    "mode-overload": ('function take(region p:Packet) -> void{} function take(owned region p:Packet) -> void{} '
                      'take(fresh());', "overloads differing only in owning versus borrowed mode"),
    "exceptional-join": ('using "std:exceptions" InvalidCapacity; proc test()->void {owned region p:Packet <- fresh(); '
                         'try {make Packet(capacity=0u64){sequence<-1u64}; drop(p); '
                         'make Packet(capacity=0u64){sequence<-1u64}; {}} '
                         'catch(e:const InvalidCapacity){read(p);{}}} test();', "owner has been consumed"),
    "owner-field": ('data Bad {owned region owner:Packet;} '
                    'make Bad(capacity=64u64){owner<-fresh()};', "region handles cannot be stored in user data fields"),
    "missing-field": ('make Packet(capacity=8u64){};', "initialize every root field exactly once"),
    "duplicate-field": ('make Packet(capacity=8u64){sequence<-1u64; sequence<-2u64};', "duplicate root initializer"),
}

# Both native entry calls and contextual procedure adapters must preserve these
# owner/access checks when a recursive frame constructs a new callback image.
for _name in ("callable-owner-argument", "recursive-callback-readonly"):
    _body, _diagnostic = REJECTS[_name]
    REJECTS["procedure-" + _name] = (_body.replace("function ", "proc ").replace("func()", "proc()"), _diagnostic)


def main():
    directory = Path(__file__).resolve().parent
    root = directory.parents[2]
    mode = os.environ.get("COD_TEST_TOOLCHAIN", "build")
    if mode not in ("build", "built"):
        raise SystemExit("COD_TEST_TOOLCHAIN must be build or built")
    toolchain = root / mode
    with tempfile.TemporaryDirectory(prefix="cod-owned-regions-") as temporary:
        scratch = Path(temporary)
        output, evidence = run_case(root, toolchain, directory / "owned-region-lifecycle.cod", scratch, HARNESS,
            extra_wrappers="#define cod_lang_region_new cod_lang_region_acquire\n"
                           "#define cod_lang_print_i64 cod_lang_trace_i64\n")
        assert output == ["9", "9", "9", "11", "10", "101", "102", "1", "2", "2", "3", "4", "5", "6", "7", "8", "0"], output
        print("PASS: initialized roots, ownership calls/joins, exceptional cleanup and relocation")
        for name, (body, diagnostic) in REJECTS.items():
            source = scratch / (name + ".cod")
            source.write_text(BASE + body)
            result = subprocess.run([toolchain / "bin/cod", "--check", source], capture_output=True, text=True, timeout=30)
            assert result.returncode == 2 and diagnostic in result.stderr, (name, result.returncode, result.stderr)
            print("PASS: " + name)
        evidence.update(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                        toolchain=mode, trace=output, compile_rejections=list(REJECTS))
        evidence["compiler_sha256"] = hashlib.sha256((toolchain / "bin/cod").read_bytes()).hexdigest()
        paths = [root / "cod" / name for name in ("lang_syntax.hh", "lang_frontend.hh", "lang_llvm.hh", "lang_native_runtime.hh")]
        paths += [directory / name for name in ("owned-region-library.cod", "owned-region-lifecycle.cod", "owned_region_tests.py")]
        evidence["source_sha256"] = {str(p.relative_to(root)): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
        if report := os.environ.get("COD_OWNERSHIP_REPORT"):
            Path(report).write_text(json.dumps(evidence, indent=2) + "\n")


if __name__ == "__main__":
    main()
