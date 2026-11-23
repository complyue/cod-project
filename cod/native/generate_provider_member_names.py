"""Project the canonical C member reservations into the marked CoD source block."""

import argparse
import json
from pathlib import Path
import re


NATIVE = Path(__file__).resolve().parent
SOURCE = NATIVE / "provider_member_names.def"
DEFAULT_COD = NATIVE.parents[1] / "applications/cod-booter/src/ProviderNames.cod"
START = "    # BEGIN GENERATED MEMBER RESERVATIONS\n"
END = "    # END GENERATED MEMBER RESERVATIONS\n"
ROW = re.compile(r'COD_PROVIDER_(EXACT|PREFIX)\("([A-Za-z_][A-Za-z_0-9]*)"\)')


def cod_block() -> str:
    entries: dict[str, list[str]] = {"EXACT": [], "PREFIX": []}
    for number, source_line in enumerate(SOURCE.read_text(encoding="ascii").splitlines(), 1):
        line = source_line.strip()
        if not line or (line.startswith("/*") and line.endswith("*/")):
            continue
        match = ROW.fullmatch(line)
        if match is None:
            raise SystemExit(f"{SOURCE}:{number}: malformed member reservation")
        mode, spelling = match.groups()
        if spelling in entries[mode]:
            raise SystemExit(f"{SOURCE}:{number}: duplicate {mode.lower()} reservation")
        entries[mode].append(spelling)

    if not all(entries.values()):
        raise SystemExit(f"{SOURCE}: exact and prefix reservations are both required")
    exact = json.dumps("\n".join(entries["EXACT"]) + "\n")
    prefix = json.dumps("\n".join(entries["PREFIX"]) + "\n")
    return (
        f"    return matches_reservation(spellings, first, past, {exact}, false) ||\n"
        f"        matches_reservation(spellings, first, past, {prefix}, true);\n"
    )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true", help="report drift without editing files")
    mode.add_argument("--write", action="store_true", help="replace the marked CoD block")
    parser.add_argument("--cod-file", type=Path, default=DEFAULT_COD)
    args = parser.parse_args()

    source = args.cod_file.read_bytes().decode("utf-8")
    if source.count(START) != 1 or source.count(END) != 1 or source.index(START) >= source.index(END):
        raise SystemExit(f"{args.cod_file}: expected one marked member reservation block")
    head, rest = source.split(START, 1)
    original, tail = rest.split(END, 1)
    replacement = cod_block()
    if original == replacement:
        return
    if args.check:
        raise SystemExit(f"{args.cod_file}: generated member reservations differ; run with --write")
    args.cod_file.write_bytes((head + START + replacement + END + tail).encode("utf-8"))


if __name__ == "__main__":
    main()
