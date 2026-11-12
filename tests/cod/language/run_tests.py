#!/usr/bin/env python3
"""Execute CoD source fixtures and compare observable results and diagnostics."""

import datetime
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import sys
import tempfile


def digest(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def main():
    directory = Path(__file__).resolve().parent
    root = directory.parents[2]
    mode = os.environ.get("COD_TEST_TOOLCHAIN", "build")
    if mode not in ("build", "built"):
        raise SystemExit("COD_TEST_TOOLCHAIN must be build or built")
    executable = root / mode / "bin" / ("cod.exe" if os.name == "nt" else "cod")
    cases = json.loads((directory / "cases.json").read_text())
    if sys.argv[1:]:
        requested = set(sys.argv[1:])
        cases = [case for case in cases if Path(case["file"]).stem in requested]
        if {Path(case["file"]).stem for case in cases} != requested:
            raise SystemExit("Unknown or duplicate fixture selection")
    results = []
    for case in cases:
        command = [str(executable), "--run", str(directory / case["file"]), *case.get("args", [])]
        result = subprocess.run(command, cwd=root, capture_output=True, text=True, timeout=60)
        diagnostics = case.get("stderr_contains", "")
        if isinstance(diagnostics, str):
            diagnostics = [diagnostics]
        expected_warnings = bool(case.get("stderr_contains")) and all(
            "[-Wunit-conversion]" in line or "[-Wmulti-alias-extension]" in line or "[-Wshadow]" in line
            or "[-Wunused-capture]" in line
            or (": note:" in line and any(message in line for message in diagnostics if message))
            for line in result.stderr.splitlines())
        passed = (result.returncode == case["exit"] and result.stdout == case["stdout"]
                  and all(message in result.stderr for message in diagnostics)
                  and (case["exit"] != 0 or result.stderr == "" or expected_warnings))
        results.append({"fixture": case["file"], "command": command, "expected": case,
                        "exit_code": result.returncode, "stdout": result.stdout,
                        "stderr": result.stderr, "passed": passed})
        print(f"{'PASS' if passed else 'FAIL'} {case['file']}", flush=True)
        if not passed:
            print(json.dumps(results[-1], indent=2))

    with tempfile.TemporaryDirectory(prefix="cod-language-check-") as scratch:
        generated = Path(scratch) / "program.ll"
        bitcode = Path(scratch) / "program.bc"
        command = [str(executable), "--check", str(directory / "import-mapping.cod"),
                   "--emit-llvm", str(generated), "--emit-bc", str(bitcode)]
        check = subprocess.run(command, cwd=root, capture_output=True, text=True, timeout=60)
        passed = (check.returncode == 0 and check.stdout == "" and check.stderr == ""
                  and generated.is_file() and "define i32 @main()" in generated.read_text()
                  and bitcode.read_bytes().startswith(b"BC\xc0\xde"))
        results.append({"fixture": "frontend-only", "command": command, "exit_code": check.returncode,
                        "stdout": check.stdout, "stderr": check.stderr, "passed": passed})
        print(f"{'PASS' if passed else 'FAIL'} frontend-only", flush=True)

    passed_count = sum(case["passed"] for case in results)
    print(f"CoD language: {passed_count}/{len(results)} passed ({mode} toolchain, LLVM IR)")
    if report := os.environ.get("COD_LANGUAGE_REPORT"):
        sources = [root / "cod/main.cc", root / "cod/lang_exceptions.hh", root / "cod/lang_syntax.hh", root / "cod/lang_frontend.hh",
                   root / "cod/lang_llvm.hh", root / "cod/lang_native_runtime.hh",
                   root / "include/shilos/region.hh",
                   root / "tests/cod/test_language.sh", directory / "run_tests.py", directory / "cases.json",
                   *sorted(directory.rglob("*.cod"))]
        evidence = {"schema_version": 2, "recorded_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                    "platform": platform.platform(), "toolchain": mode, "backend": "llvm", "passed": passed_count,
                    "total": len(results), "results": results,
                    "executable": {"path": str(executable.relative_to(root)), "sha256": digest(executable)},
                    "source_sha256": {str(path.relative_to(root)): digest(path) for path in sources}}
        Path(report).write_text(json.dumps(evidence, indent=2) + "\n")
    return 0 if passed_count == len(results) else 1


if __name__ == "__main__":
    sys.exit(main())
