"""Shared toolchain selection, diagnostics and checked subprocess execution."""
from collections import Counter
import os
from pathlib import Path
import re
import subprocess
import time

DIRECTORY = Path(__file__).resolve().parent
ROOT = DIRECTORY.parents[2]
MODE = os.environ.get("COD_TEST_TOOLCHAIN", "build")
if MODE not in ("build", "built"):
    raise SystemExit("COD_TEST_TOOLCHAIN must be build or built")


def seed_executable(toolchain):
    return toolchain / "bin" / ("cod-seed.exe" if os.name == "nt" else "cod-seed")


COD = seed_executable(ROOT / MODE)
RUNTIME_SOURCE = ROOT / "cod/runtime/cod_lang_runtime.c"


def runtime_archive(toolchain):
    if override := os.environ.get("COD_RUNTIME_ARCHIVE"):
        return Path(override).resolve()
    installed = toolchain / "lib/libcod-runtime.a"
    if installed.is_file() or not (toolchain / "CMakeCache.txt").is_file():
        return installed
    return toolchain.parent / "build-cod-runtime/libcod-runtime.a"


def write_instrumented_runtime(path, wrappers, harness):
    # Apply the fixture's symbol wrappers to the shared implementation.
    path.write_text(f'''#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <new>
{wrappers}
#include "{RUNTIME_SOURCE.as_posix()}"
{harness}''')


def expect_source_rejections(fixture, names, diagnostic):
    """Give explicitly identified invalid source cases their checking outcome."""
    for name in names:
        source, _, _, _ = fixture.CASES[name]
        fixture.CASES[name] = (source, 2, '', diagnostic)
        for field in ('CASE_SHADOWS', 'CASE_LOCATIONS'):
            getattr(fixture, field, {}).pop(name, None)


def observe_after_cleanup(work, observation):
    """Observe a callable's final state after normal preparation and newer actions."""
    return ('val observe_on_exit<-false;defer{if(observe_on_exit){' + observation + '}else{};};'
            + work + 'observe_on_exit<-true;')


def shadow_diagnostics(stderr, expected):
    pattern = r"^.+:\d+:\d+: warning: declaration shadows outer binding '([^']+)' \[-Wshadow\]\n"
    names = re.findall(pattern, stderr, re.M)
    return Counter(names) == Counter(expected), re.sub(pattern, '', stderr, flags=re.M)


def diagnostic_matches(stderr, expected, path, location=None):
    messages = re.sub(r'^.+:\d+:\d+: ', '', stderr, flags=re.M)
    if expected not in messages:
        return False
    if location is None:
        return True
    line, column = location
    return re.search(rf'^{re.escape(str(path))}:{line}:{column}: ', stderr, re.M) is not None


def checked(command, root, allowed_warnings=(), diagnostics=None):
    started = time.perf_counter()
    result = subprocess.run([str(x) for x in command], cwd=root, capture_output=True, text=True, timeout=120)
    seconds = time.perf_counter() - started
    unexpected = any(not ('warning:' in line or ': note:' in line) or
                     not any(line.endswith(warning) for warning in allowed_warnings)
                     for line in result.stderr.splitlines())
    if result.returncode or unexpected:
        raise RuntimeError(f"{command}\nexit={result.returncode}\n{result.stdout}\n{result.stderr}")
    if diagnostics is not None and result.stderr:
        diagnostics.append(result.stderr)
    return result.stdout, seconds
