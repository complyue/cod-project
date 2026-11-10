#!/usr/bin/env python3
"""Check allocation/root-construction cleanup with the selected CoD toolchain."""

import os
from pathlib import Path
import subprocess
import tempfile


SOURCE = r'''
#include <shilos/region.hh>
#include <cassert>
#include <cstdlib>
#include <new>

// Observe only the region block; exception machinery and library allocations
// are deliberately outside this counter.
static void *watched = nullptr;
static unsigned released = 0;
static bool reject_next = false;
void *operator new(std::size_t size) {
  if (reject_next) { reject_next = false; throw std::bad_alloc(); }
  if (void *p = std::malloc(size ? size : 1)) return p;
  throw std::bad_alloc();
}
void operator delete(void *p) noexcept {
  if (p && p == watched) { ++released; watched = nullptr; }
  std::free(p);
}
void operator delete(void *p, std::size_t) noexcept { ::operator delete(p); }

struct Root {
  static constexpr shilos::UUID TYPE_UUID{"43d883d2-edf8-47f0-ab62-d1eb445e3597"};
  static inline unsigned entered = 0;
  explicit Root(shilos::memory_region<Root> &region, bool fail = true) {
    ++entered;
    watched = &region;
    if (fail) throw 17;
  }
  Root(const Root &) = delete;
  Root(Root &&) = delete;
};

int main() {
  using Region = shilos::memory_region<Root>;
  for (bool explicit_argument : {false, true}) {
    const auto before = released;
    try {
      if (explicit_argument) Region::alloc_region(128, true);
      else Region::alloc_region(128);
      assert(false);
    } catch (int error) { assert(error == 17); }
    assert(released == before + 1 && watched == nullptr);
  }
  const auto entered = Root::entered;
  reject_next = true;
  try { Region::alloc_region(128); assert(false); }
  catch (const std::bad_alloc &) {}
  assert(Root::entered == entered && released == 2);
  auto *region = Region::alloc_region(128, false);
  assert(region == watched && released == 2);
  Region::free_region(region);
  assert(released == 3 && watched == nullptr);
}
'''


def main():
    root = Path(__file__).resolve().parents[2]
    mode = os.environ.get("COD_TEST_TOOLCHAIN", "build")
    if mode not in ("build", "built"):
        raise SystemExit("COD_TEST_TOOLCHAIN must be build or built")
    with tempfile.TemporaryDirectory(prefix="cod-region-construction-") as directory:
        work = Path(directory)
        source, executable = work / "construction.cc", work / "construction"
        source.write_text(SOURCE)
        subprocess.run([str(root / mode / "bin/clang++"), "-std=c++20", "-O0",
                        "-UNDEBUG", "-I" + str(root / "include"), str(source),
                        "-o", str(executable)], check=True)
        subprocess.run([str(executable)], check=True)
    print("PASS: both throwing-root overloads release once; allocation failure "
          "never enters the root; successful construction retains ownership until release")


if __name__ == "__main__":
    main()
