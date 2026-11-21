"""Immutable native fixtures and shared LLVM/native execution evidence."""
from dataclasses import dataclass, field, replace
import datetime
import hashlib
import json
import os
from pathlib import Path
import tempfile
from types import MappingProxyType
from typing import Callable, Mapping

from language_test_support import ROOT, MODE, COD, RUNTIME_SOURCE, checked, shadow_diagnostics, write_instrumented_runtime


@dataclass(frozen=True)
class NativeFixture:
    source: str
    wrappers: str
    harness: str
    expected_values: tuple[str, ...]
    expected_shadows: tuple[str, ...] = ()
    source_files: Mapping[str, str] = field(default_factory=dict)
    sources: tuple[Path, ...] = ()
    transform_ir: Callable[[str], tuple[str, dict]] | None = None

    def __post_init__(self):
        object.__setattr__(self, 'expected_values', tuple(self.expected_values))
        object.__setattr__(self, 'expected_shadows', tuple(self.expected_shadows))
        object.__setattr__(self, 'source_files', MappingProxyType(dict(self.source_files)))
        object.__setattr__(self, 'sources', tuple(dict.fromkeys(Path(path).resolve() for path in self.sources)))

    def updated(self, **changes):
        if 'sources' in changes:
            changes['sources'] = (*self.sources, *changes['sources'])
        return replace(self, **changes)


def run_native(fixture, report_path=None):
    toolchain = ROOT / MODE
    results = []
    with tempfile.TemporaryDirectory(prefix="cod-native-runtime-") as directory:
        scratch = Path(directory)
        entry = scratch / "main.cod"
        for name, contents in fixture.source_files.items():
            path = scratch / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(contents)
        entry.write_text(fixture.source)
        for general in (False, True):
            ir, bc = [scratch / name for name in ("main.ll", "main.bc")]
            command = [COD, "--check", entry, "--emit-llvm", ir]
            if fixture.transform_ir is None:
                command.extend(["--emit-bc", bc])
            if general:
                command.append("--no-specialize")
            diagnostics = []
            output, frontend_seconds = checked(command, ROOT, ('[-Wmissing-return]', '[-Wshadow]'), diagnostics)
            if not shadow_diagnostics(''.join(diagnostics), fixture.expected_shadows)[0]:
                raise RuntimeError(f"Unexpected shadow diagnostics: {diagnostics}")
            if output:
                raise RuntimeError("Checking executed runtime effects")
            evidence = {}
            if fixture.transform_ir is not None:
                original = ir.read_text()
                modified, evidence = fixture.transform_ir(original)
                ir.write_text(modified)
                evidence = {**evidence, "original_ir_sha256": hashlib.sha256(original.encode()).hexdigest(),
                            "injected_ir_sha256": hashlib.sha256(modified.encode()).hexdigest()}
            assembled = bc if fixture.transform_ir is not None else scratch / "roundtrip.bc"
            checked([toolchain / "bin/llvm-as", ir, "-o", assembled], ROOT)
            checked([toolchain / "bin/opt", "-passes=verify", "-disable-output", bc], ROOT)
            source = scratch / "instrumented.cc"
            write_instrumented_runtime(source, fixture.wrappers, fixture.harness)
            executable = scratch / "native"
            compile_command = [toolchain / "bin/clang++", "-std=c++20", "-O2", "-g", "-UNDEBUG", "-stdlib=libc++",
                               "-fuse-ld=lld", "-I" + str(ROOT / "include"), bc, source, "-o", executable,
                               "-L" + str(toolchain / "lib"), "-Wl,-rpath," + str(toolchain / "lib")]
            _, compile_seconds = checked(compile_command, ROOT)
            output, execution_seconds = checked([executable], ROOT)
            *values, metrics = output.splitlines()
            if values != list(fixture.expected_values):
                raise RuntimeError(f"Unexpected native output: {values}")
            results.append({**evidence, "name": "general" if general else "default", "passed": True, "metrics": json.loads(metrics),
                            "stdout": values, "diagnostics": diagnostics, "frontend_seconds": frontend_seconds, "compile_seconds": compile_seconds,
                            "execution_seconds": execution_seconds, "check_command": list(map(str, command)),
                            "compile_command": list(map(str, compile_command))})
            print(f"PASS {results[-1]['name']}: {metrics}", flush=True)
    if report := report_path or os.environ.get("COD_LANGUAGE_REPORT"):
        sources = [ROOT / "cod/main.cc", RUNTIME_SOURCE, RUNTIME_SOURCE.with_suffix(".h"),
                   RUNTIME_SOURCE.parent.parent / "lang_intrinsic_catalog.h",
                   *sorted((ROOT / "cod").glob("lang_*.hh"))]
        sources += [Path(__file__), Path(__file__).with_name("language_test_support.py"), *fixture.sources]
        Path(report).write_text(json.dumps({"recorded_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(), "toolchain": MODE,
                                         "passed": len(results), "total": len(results), "results": results,
                                         "executable_sha256": hashlib.sha256(COD.read_bytes()).hexdigest(),
                                         "sources": {str(path.relative_to(ROOT)): hashlib.sha256(path.read_bytes()).hexdigest() for path in sources}}, indent=2) + "\n")
    return 0
