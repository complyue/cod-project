#!/usr/bin/env python3
"""A prepared CoD call preserves the native loan through deferred cleanup."""
from pathlib import Path
import subprocess
import tempfile

from language_test_support import ROOT, MODE, COD, checked, runtime_archive

SOURCE = r'''
handle type Session;
handle type View in Session;
proc "C" Session(val id:u64);
proc "C" View(val id:u64);
proc "C" observe_session(handle session:Session in O, val code:u64) -> void where {const owner O};
proc "C" observe_view(handle selected:View in O, val code:u64) -> void where {const owner O};

proc forward(handle selected:View in O) -> handle View in O where {const owner O} {
    return selected;
}

proc observe_prepared(handle selected:const View in O, code:u64) -> void where {const owner O} {
    defer observe_view(selected, code);
    return;
}

proc run() -> void {
    own handle session:Session <- make own handle Session(11u64);
    proc observe_lexical(code:u64) -> void {
        observe_session(session, code);
        return;
    };
    defer observe_lexical(9u64);
    for (i in 1u64 .. 4u64) {
        defer observe_prepared(forward(make handle View(i) in session), i + 10u64);
    };
    observe_session(session, 0u64);
    return;
}
run();
'''

PROVIDER = r'''
#include "cod_providers.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

static bool live;

cod_native_status cod_native_Session_construct_impl(uint64_t id, uint64_t *resource) {
    assert(id == 11 && !live);
    live = true;
    *resource = id;
    puts("open");
    return COD_NATIVE_OK;
}

void cod_native_Session_destroy_impl(uint64_t resource) {
    assert(resource == 11 && live);
    live = false;
    puts("close");
}

cod_native_status cod_native_View_construct_impl(cod_native_Session_borrow owner,
                                                 uint64_t id, uint64_t *resource) {
    assert(live && owner.payload == 11 && owner.owner == 11 && id >= 1 && id <= 3);
    *resource = 20 + id;
    return COD_NATIVE_OK;
}

cod_native_status cod_native_observe_session_impl(cod_native_Session_borrow session, uint64_t code) {
    assert(live && session.payload == 11 && session.owner == 11);
    printf("session %u\n", (unsigned)code);
    return COD_NATIVE_OK;
}

cod_native_status cod_native_observe_view_impl(cod_native_View_borrow view, uint64_t code) {
    assert(live && view.owner == 11 && view.payload == code + 10);
    printf("view %u\n", (unsigned)code);
    return COD_NATIVE_OK;
}
'''

EXPIRED_LOAN = r'''
handle type Session;
proc "C" Session(val id:u64);
symbol @current :own handle Session;

proc observe(handle value:Session in O) -> void where {const owner O} { return; }
proc run() -> void {
    for (i in 1u64 .. 4u64) {
        effect @current <- make own handle Session(i);
        defer observe(@current);
    };
    return;
}
run();
'''

EXPECTED = "open\nsession 0\nview 13\nview 12\nview 11\nsession 9\nclose\n"


def main():
    toolchain = ROOT / MODE
    clang = toolchain / "bin/clang"
    with tempfile.TemporaryDirectory(prefix="cod-deferred-native-loan-") as directory:
        scratch = Path(directory)
        source = scratch / "main.cod"
        source.write_text(SOURCE)
        rejected_source = scratch / "expired.cod"
        rejected_source.write_text(EXPIRED_LOAN)
        provider = scratch / "provider.c"
        provider.write_text(PROVIDER)
        for mode, flags in (("specialized", ()), ("shared", ("--no-specialize",))):
            output = scratch / mode
            output.mkdir()
            ir = output / "driver.ll"
            executable = output / "driver"
            output_text, _ = checked(
                [COD, *flags, "--no-cache", "--check", source, "--emit-c-provider", output, "--emit-llvm", ir], ROOT
            )
            assert not output_text, "Checking executed a native provider operation"
            checked([clang, "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic-errors", "-I", output,
                     output / "cod_providers.c", ir, provider, runtime_archive(toolchain), "-o", executable], ROOT)
            actual, _ = checked([executable], ROOT)
            assert actual == EXPECTED, (mode, actual)
            unpublished = output / "unpublished.ll"
            unpublished.write_text("previous output\n")
            rejected = subprocess.run([COD, *flags, "--no-cache", "--check", rejected_source, "--emit-llvm", unpublished],
                                      cwd=ROOT, capture_output=True, text=True, timeout=90)
            assert rejected.returncode == 2 and rejected.stdout == "", rejected
            assert "does not outlive its cleanup scope" in rejected.stderr, rejected
            assert unpublished.read_text() == "previous output\n"
            print(f"PASS {mode}", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
