#!/usr/bin/env python3
"""Observe relocation and cleanup of emitted regional construction."""

import tempfile
from pathlib import Path

from language_test_support import COD, DIRECTORY, MODE, ROOT, RUNTIME_SOURCE, checked


CASES = (
    ("regional-sum-placement-init", 1, ["true", "true", "1", "2", "relocated Link cycle"]),
    ("regional-sum-initializer-abrupt", 0, ["7", "released Graph", "42"]),
)


def main():
    tools = ROOT / MODE / "bin"
    clang = tools / "clang"
    observer = DIRECTORY / "regional_placement_probe.c"
    hooks = ("acquire", "reserve", "release")
    defines = [f"-Dcod_lang_region_{hook}=cod_test_region_{hook}" for hook in hooks]
    with tempfile.TemporaryDirectory(prefix="cod-regional-placement-") as directory:
        work = Path(directory)
        for name, relocate, expected in CASES:
            runtime = work / "runtime.o"
            checked([clang, "-std=c11", *defines, "-c", RUNTIME_SOURCE, "-o", runtime], ROOT)
            probe = work / "observer.o"
            checked([clang, "-std=c11", "-Wall", "-Wextra", "-Werror", "-UNDEBUG",
                     f"-DRELOCATE_GRAPH={relocate}", "-I", RUNTIME_SOURCE.parent,
                     "-I", ROOT / "include", "-c", observer, "-o", probe], ROOT)
            for flags in ([], ["--no-specialize"]):
                bitcode = work / "program.bc"
                checked([COD, *flags, "--check", DIRECTORY / (name + ".cod"), "--emit-bc", bitcode], ROOT)
                checked([tools / "opt", "-passes=verify", "-disable-output", bitcode], ROOT)
                executable = work / "program"
                checked([clang, "-Wno-override-module", bitcode, probe, runtime, "-o", executable], ROOT)
                output, _ = checked([executable], ROOT)
                if output.splitlines() != expected:
                    raise RuntimeError(f"{name} {flags}: {output!r}")
                print(f"PASS {name} ({'shared' if flags else 'specialized'})")


if __name__ == "__main__":
    main()
