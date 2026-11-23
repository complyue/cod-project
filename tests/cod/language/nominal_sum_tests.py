#!/usr/bin/env python3
"""Exercise nominal construction and case reception through native execution."""
from pathlib import Path
import subprocess
import tempfile

from language_test_support import COD, ROOT, checked


SYNTAX = """
export data SyntaxPayload =
    ByteLiteral { value:u8; }
  | Call { callee:u64; arguments:u64; }
  | Empty {};
"""

SAVED_SUBJECT = """
using symbol "std:io" @print;
using "./syntax.cod" SyntaxPayload as Payload { ByteLiteral, Call, Empty };

proc inspect() -> u64 {
    ref original:Payload = make ByteLiteral { value <- 7u8; };
    ref replacement:Payload = make Call { callee <- 19u64; arguments <- 3u64; };
    ref selected:Payload = original;
    return case selected of {
        (ByteLiteral { val value:u64 <- value; }) |
            { selected = replacement; value == 9u64 } -> { 99u64 };
        (ByteLiteral { val value:u64 <- value; }) -> { value };
        (Call { val count <- arguments; }) -> { count };
        (Empty) -> { 0u64 };
    };
}
behave @print(inspect());
"""

FOREIGN_CONSTRUCTOR = """
using "./syntax.cod" SyntaxPayload { Empty };
using "./foreign.cod" Foreign { Call };

proc inspect() -> u64 {
    ref node:SyntaxPayload = make Empty {};
    return case node of {
        (Call { val count <- arguments; }) -> { count };
        (_) -> { 0u64 };
    };
}
"""


def main():
    with tempfile.TemporaryDirectory(prefix="cod-nominal-sum-") as directory:
        scratch = Path(directory)
        (scratch / "syntax.cod").write_text(SYNTAX)
        (scratch / "foreign.cod").write_text("export data Foreign = Call { arguments:u64; };\n")
        accepted = scratch / "accepted.cod"
        accepted.write_text(SAVED_SUBJECT)
        rejected = scratch / "rejected.cod"
        rejected.write_text(FOREIGN_CONSTRUCTOR)
        artifact = scratch / "existing.ll"
        for mode, flags in (("specialized", ()), ("shared", ("--no-specialize",))):
            actual, _ = checked([COD, *flags, "--no-cache", "--run", accepted], ROOT)
            assert actual == "7\n", (mode, actual)
            artifact.write_text("existing output\n")
            result = subprocess.run(
                [COD, *flags, "--no-cache", "--check", rejected, "--emit-llvm", artifact],
                cwd=ROOT, capture_output=True, text=True, timeout=90,
            )
            assert result.returncode == 2 and result.stdout == "", result
            assert "'Call' is not a selected data constructor of 'SyntaxPayload'" in result.stderr, result
            assert artifact.read_text() == "existing output\n"
            print(f"PASS {mode}", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
