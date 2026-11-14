#!/usr/bin/env python3
"""Native and IR checks for the supported scalar source foundation."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile

DIRECTORY = Path(__file__).resolve().parent
ROOT = DIRECTORY.parents[2]
MODE = os.environ.get("COD_TEST_TOOLCHAIN", "build")
if MODE not in ("build", "built"):
    raise SystemExit("COD_TEST_TOOLCHAIN must be build or built")
COD = ROOT / MODE / "bin" / ("cod.exe" if os.name == "nt" else "cod")
PRELUDE = 'using symbol "std:io" @print;\n'
OBSERVER = PRELUDE + 'proc observe(x:u64)->void { behave @print(x); }\n'
CASES = {
    "scalars": (None, 0, "6\n3\n4\ntrue\n100\n1\n1\n2\n720\ntrue\nfalse\n7\nfalse\n4\n3\n3\n4\n4\n4\n3\n3\n5\n4\n-4\n-1\n100\n1\n", ""),
    "cleanup": (None, 0, "1\n2\n5\n4\n2\n1\n9\n1\n3\n5\n4\n3\n1\n9\n0\n1\n2\n3\n3\n7\n8\n6\n11\n10\nfalse\ntrue\n", ""),
    "failure": (None, 70, "2\n3\n1\n0\n", "uncaught MissingReturnException"),
    "preparation_failure": (OBSERVER + 'proc missing()->const u64 { defer observe(3u64); {} }\nproc go()->void { defer observe(1u64); defer observe(missing()); observe(99u64); }\ngo();', 70, "3\n1\n", "uncaught MissingReturnException"),
    "overflow_cleanup": (OBSERVER + 'proc go()->const u64 { defer observe(9u64); return 18446744073709551615u64 + 1u64; }\ngo();', 70, "9\n", "u64 addition overflow"),
    "division_cleanup": (OBSERVER + 'proc go()->const i64 { defer observe(8u64); return 3 / 0; }\ngo();', 70, "8\n", "division by zero"),
    "signed_boundaries": (PRELUDE + 'behave @print(-9223372036854775808); behave @print(-9223372036854775808 % -1); behave @print(1f32);', 0, "-9223372036854775808\n0\n1\n", ""),
    "signed_division_overflow": (OBSERVER + 'proc go()->const i64 { defer observe(8u64); return -9223372036854775808 / -1; } go();', 70, "8\n", "i64 division overflow"),
    "literal_overflow": ('9223372036854775808;', 2, "", "integer literal is outside i64"),
    "floating_literal_overflow": ('1e99f32;', 2, "", "floating literal is outside f32 range"),
    "unsigned_negation": ('-1u64;', 2, "", "numeric unary operation requires i64"),
    "same_t": ('func first(x:T,y:T)->const T where T { return x; } first(1u64,true);', 2, "", "same-T relation"),
    "ambiguous": ('func f(x:u64,y:T)->const u64 where T { return 1u64; } func f(x:T,y:u64)->const u64 where T { return 2u64; } f(1u64,2u64);', 2, "", "ambiguous methods"),
    "bad_generic_operation": ('func twice(x:T)->const T where T { return x+x; } twice(true);', 2, "", "no applicable numeric operation"),
    "recursive_result": ('func f(x:u64)->const bool { return g(x); } func g(x:u64)->const u64 { return if(x==0u64){1u64}else{f(x-1u64)}; } f(2u64);', 2, "", "return type"),
    "unreachable_definition": ('func unused(x:u64)->const bool { return x; }', 2, "", "return type"),
    "function_effect": (PRELUDE + 'func wrong()->void { behave @print(1u64); } wrong();', 2, "", "function cannot invoke a dynamic procedure"),
    "nearest_symbol": (PRELUDE + 'proc wrong(print:u64)->void { behave @print(1u64); } wrong(3u64);', 2, "", "nearest binding 'print' is not a symbol"),
    "nearest_callable": ('func f()->const u64 {return 1u64;} func g(f:u64)->const u64 {return f();} g(1u64);', 2, "", "nearest binding 'f' is not callable"),
    "local_storage": ('proc go()->void { val x=1u64; } go();', 0, "", ""),
    "unselected_throws_type": ('func f(x:T)->const T throws StaticallyUnknownType where T { return x; } f(1u64);', 2, "", "throws contracts"),
    "missing_return_error": ('func f()->const u64 { 3u64 } f();', 2, "", "treated as error"),
    "empty": ("", 0, "", ""),
}
CASE_ARGUMENTS = {"missing_return_error": ["-Werror=missing-return"]}
EXTRA_SOURCES = []


def main():
    selected = sys.argv[1:] or list(CASES)
    if any(name not in CASES for name in selected):
        raise SystemExit("Unknown foundation fixture")
    results = []
    def record(name, passed, **details):
        results.append({"name": name, "passed": passed, **details})
        print(f"{'PASS' if passed else 'FAIL'} {name}", flush=True)
        if not passed:
            print(json.dumps(details, indent=2), flush=True)
    with tempfile.TemporaryDirectory(prefix="cod-foundation-") as scratch_name:
        scratch = Path(scratch_name)
        scalar_ir = {}
        for name in selected:
            source, code, stdout, diagnostic = CASES[name]
            path = DIRECTORY / "foundation" / f"{name}.cod"
            if source is not None:
                path = scratch / f"{name}.cod"
                path.write_text(source)
            for general in (False, True):
                label = f"{name}/{'general' if general else 'default'}"
                ir = scratch / f"{name}-{general}.ll"
                command = [str(COD), "--run", str(path), "--emit-llvm", str(ir)]
                if general:
                    command.append("--no-specialize")
                command.extend(CASE_ARGUMENTS.get(name, []))
                run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
                passed = run.returncode == code and run.stdout == stdout and diagnostic in run.stderr
                if not diagnostic:
                    passed = passed and run.stderr == ""
                record(label, passed, command=command, exit=run.returncode, stdout=run.stdout, stderr=run.stderr)
                if name == "scalars" and ir.exists():
                    scalar_ir[general] = ir.read_text()
        if len(scalar_ir) == 2:
            def bodies(ir):
                return dict(re.findall(r'define internal %cod.outcome @([^ (]+)\([^\n]*\) \{\n(.*?)\n\}', ir, re.S))
            default, general = bodies(scalar_ir[False]), bodies(scalar_ir[True])
            record("general_inventory", len(general) == 12 and all(name.endswith((".general", ".native")) for name in general), names=list(general))
            known = default.get("cod.method.0.twice.specialized.u64", "")
            record("known_direct_specialization", bool(known) and "operation.selected" not in known
                   and "call %cod.outcome @cod.method.0.twice.specialized.u64" in scalar_ir[False])
            shared = general.get("cod.method.10.both.general", "")
            record("independent_local_operations", "fadd float" in shared and "llvm.uadd.with.overflow.i64" in shared
                   and shared.count("operation.join") >= 4)
            record("recursion_reuses_methods", "@cod.method.9.relay.general" in general.get("cod.method.9.relay.general", "")
                   and "@cod.method.8.odd.native" in general.get("cod.method.7.even.native", ""))
            record("specialization_reuse", sum(name.startswith("cod.method.9.relay.specialized") for name in default) == 2
                   and sum(name.startswith("cod.method.0.twice.specialized") for name in default) == 3)
            wrapped = general.get("cod.method.11.wrapped.general", "")
            record("semantic_selection_before_implementation", "@cod.method.2.chosen.general" in wrapped
                   and "@cod.method.3.chosen.native" in wrapped)
            bitcode = scratch / "checked.bc"
            checked = subprocess.run([str(COD), "--check", str(DIRECTORY / "foundation/cleanup.cod"), "--emit-bc", str(bitcode)],
                                     cwd=ROOT, capture_output=True, text=True, timeout=60)
            record("analysis_does_not_execute_effects", checked.returncode == 0 and checked.stdout == "" and checked.stderr == ""
                   and bitcode.exists() and bitcode.read_bytes().startswith(b"BC\xc0\xde"))
            # Reorder semantic definitions and increase runtime depth; checking/emission must remain finite and equivalent.
            original = (DIRECTORY / "foundation/scalars.cod").read_text()
            reordered = original.replace('func chosen(x:T) -> const u64 where T { return 1u64 }\nfunc chosen(x:u64) -> const u64 { return 100u64 }',
                'func chosen(x:u64) -> const u64 { return 100u64 }\nfunc chosen(x:T) -> const u64 where T { return 1u64 }').replace('relay(30u64,false)', 'relay(300u64,false)')
            path = scratch / "reordered.cod"
            path.write_text(reordered)
            ir = scratch / "reordered.ll"
            run = subprocess.run([str(COD), "--run", str(path), "--no-specialize", "--emit-llvm", str(ir)],
                                 cwd=ROOT, capture_output=True, text=True, timeout=60)
            record("order_and_runtime_depth", run.returncode == 0 and run.stdout == CASES["scalars"][2]
                   and run.stderr == "" and ir.exists() and len(bodies(ir.read_text())) == len(general),
                   exit=run.returncode, stdout=run.stdout, stderr=run.stderr)
    count = sum(result["passed"] for result in results)
    print(f"Scalar foundation: {count}/{len(results)} passed")
    if report := os.environ.get("COD_LANGUAGE_REPORT"):
        sources = [ROOT / "cod/main.cc", *sorted((ROOT / "cod").glob("lang_*.hh"))]
        sources += [Path(__file__), *EXTRA_SOURCES, *sorted((DIRECTORY / "foundation").glob("*.cod"))]
        evidence = {"recorded_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(), "toolchain": MODE,
                    "passed": count, "total": len(results), "results": results,
                    "executable_sha256": hashlib.sha256(COD.read_bytes()).hexdigest(),
                    "sources": {str(path.relative_to(ROOT)): hashlib.sha256(path.read_bytes()).hexdigest() for path in sources}}
        Path(report).write_text(json.dumps(evidence, indent=2) + "\n")
    return 0 if count == len(results) else 1


if __name__ == "__main__":
    sys.exit(main())
