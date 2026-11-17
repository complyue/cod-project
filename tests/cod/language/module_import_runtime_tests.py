#!/usr/bin/env python3
"""Imported complete callables retain native ownership and failure cleanup."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
definitions, execution = FIXTURE.source.split('{{func f=wrap(wrap(base));', 1)
exports = ['observe', 'base', 'other', 'wrap', 'apply', 'missing', 'one', 'two', 'wrapped', 'replaced', 'copy_recovery', 'retained']
FIXTURE = FIXTURE.updated(source_files={'library.cod': definitions + ''.join('export ' + name + ';' for name in exports)})
FIXTURE = FIXTURE.updated(source='using "std:exceptions" OutOfMemoryError;using "std:exceptions" MissingReturnError;'
                 + ''.join('import "library.cod" ' + name + ';' for name in exports)
                 + '{{func f=wrap(wrap(base));' + execution)
FIXTURE = FIXTURE.updated(expected_shadows=[])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
