#!/usr/bin/env python3
"""Exercise final checked-exception findings through independent source bodies."""
from pathlib import Path
import subprocess
import tempfile

from language_test_support import COD, ROOT


def main():
    with tempfile.TemporaryDirectory(prefix="cod-terminal-body-") as directory:
        scratch = Path(directory)
        entry = scratch / "entry.cod"
        entry.write_text('using "./first.cod" { forward }\nusing "./second.cod" { second }\nforward();\n')
        first = scratch / "first.cod"
        first.write_text('''\
using "std:exceptions" Exception;
data FirstFailure <: Exception { value:u64 }
export proc forward() -> void { first() }
proc first() -> void { throw FirstFailure { value <- 1u64 } }
''')
        second = scratch / "second.cod"
        second.write_text('''\
using "std:exceptions" Exception;
data SecondFailure <: Exception { value:u64 }
export proc second() -> void { throw SecondFailure { value <- 2u64 } }
''')
        pair = [scratch / "cod_providers.h", scratch / "cod_providers.c"]
        previous = "existing provider output\n"
        for file in pair:
            file.write_text(previous)
        for mode, flags in (("specialized", ()), ("shared", ("--no-specialize",))):
            for route in ((), ("--emit-c-provider", scratch)):
                for limit in (1, 2, None):
                    cap = () if limit is None else ("--max-diagnostics", str(limit))
                    result = subprocess.run(
                        [COD, *flags, "--no-cache", "--check", entry, *route, *cap],
                        cwd=ROOT, capture_output=True, text=True, timeout=90,
                    )
                    assert (result.returncode, result.stdout) == (2, ""), result
                    findings = [line for line in result.stderr.splitlines() if "checked exception" in line]
                    assert len(findings) == (1 if limit == 1 else 2), result
                    assert findings[0].startswith(f"{first}:4:24: checked exception 'FirstFailure"), result
                    if limit != 1:
                        assert findings[1].startswith(f"{second}:3:32: checked exception 'SecondFailure"), result
                    assert ("Coverage is incomplete." in result.stderr) == (limit == 1), result
                    assert all(file.read_text() == previous for file in pair), result
            print(f"PASS {mode}", flush=True)


if __name__ == "__main__":
    main()
