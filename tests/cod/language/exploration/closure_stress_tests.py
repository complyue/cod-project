#!/usr/bin/env python3
"""Exercise closure composition and bounded frame reuse on each experiment branch."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time


def main():
    variant = sys.argv[1] if len(sys.argv) > 1 else "owner"
    assert variant in ("owner", "slot")
    root = Path(__file__).resolve().parents[4]
    compiler = root / os.environ.get("COD_TEST_TOOLCHAIN", "build") / "bin/cod"
    results = []
    with tempfile.TemporaryDirectory(prefix="cod-closure-composition-") as temporary:
        scratch = Path(temporary)
        capture = " [state = state]" if variant == "slot" else ""
        (scratch / "base.cod").write_text('region Shape { val value:u64; }\n'
            'export Shape;\nfunction reader(region state:Shape) -> func()->const u64 {\n'
            f'  func{capture} () -> const u64 {{ state.value }}\n}}\nexport reader;\n')
        previous = "base.cod"
        levels = 64
        for index in range(levels):
            name = f"layer-{index}.cod"
            capture = " [prior = prior]" if variant == "slot" else ""
            (scratch / name).write_text(f'using "{previous}" reader as previous;\n'
                'using "base.cod" Shape;\n'
                'function reader(region state:Shape) -> func()->const u64 {\n'
                '  const ref prior = previous(state);\n'
                f'  func{capture} () -> const u64 {{ prior() + 1u64 }}\n}}\nexport reader;\n')
            previous = name
        entry = scratch / "entry.cod"
        entry.write_text(f'using "{previous}" reader;\nusing symbol "std:io" @print;\n'
            'region Concrete { tag:u8; value:u64; spare:u64; }\n'
            'owned region state:Concrete <- make Concrete(capacity = 256u64) { tag <- 1u8; value <- 5u64; spare <- 7u64 };\n'
            'const ref callback = reader(state);\nbehave @print(callback());\n'
            'state.value <- 21u64;\nbehave @print(callback());\n')
        start = time.monotonic()
        run = subprocess.run([compiler, "--run", entry, "--emit-llvm", scratch / "deep.ll"],
                             capture_output=True, text=True, timeout=120)
        elapsed = time.monotonic() - start
        assert run.returncode == 0 and run.stdout == f"{5 + levels}\n{21 + levels}\n" and not run.stderr, run
        sources = {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(scratch.glob("*.cod"))}
        results.append({"case": "64-module-returned-nested-closures", "seconds": elapsed,
                        "stdout": run.stdout, "llvm_bytes": (scratch / "deep.ll").stat().st_size,
                        "sources": sources})
        frame = Path(__file__).with_name("closure-frames.cod").read_text()
        frame = frame.replace('using symbol "std:io" @print;',
                              'using symbol "std:io" @print;\nusing symbol "std:io" @assert;')
        frame = frame.replace('const ref step = updater(state);',
                              'const val occupied = used_bytes(state);\nconst ref step = updater(state);')
        frame = frame.replace('frame in 0 .. 3', 'frame in 0 .. 10000')
        frame += '\nbehave @assert(used_bytes(state) == occupied);\n'
        entry.write_text(frame)
        start = time.monotonic()
        run = subprocess.run([compiler, "--run", entry, "--emit-llvm", scratch / "frames.ll"],
                             capture_output=True, text=True, timeout=120)
        elapsed = time.monotonic() - start
        assert run.returncode == 0 and run.stdout == '10001\n10000\n' and not run.stderr, run
        ir = (scratch / "frames.ll").read_text()
        results.append({"case": "10000-two-frame-updates", "seconds": elapsed,
                        "stdout": run.stdout, "bounded_occupation": True, "llvm_bytes": len(ir.encode()),
                        "region_acquisition_calls": sum('call ' in l and '@cod_lang_region_new(' in l for l in ir.splitlines()),
                        "source_sha256": hashlib.sha256(frame.encode()).hexdigest()})
    evidence = {"variant": variant, "compiler_sha256": hashlib.sha256(compiler.read_bytes()).hexdigest(),
                "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(), "passed": True,
                "results": results, "timing_scope": "frontend, native compile/link and process combined; not a simulation throughput benchmark"}
    print(json.dumps(evidence, indent=2))
    if output := os.environ.get("COD_CLOSURE_REPORT"):
        Path(output).write_text(json.dumps(evidence, indent=2) + "\n")


if __name__ == "__main__":
    main()
