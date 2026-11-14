#!/usr/bin/env python3
"""Imported complete callables retain native ownership and failure cleanup."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
definitions, execution = runner.SOURCE.split('{{func f=wrap(wrap(base));', 1)
exports = ['observe', 'base', 'other', 'wrap', 'apply', 'missing', 'one', 'two', 'wrapped', 'replaced', 'copy_recovery', 'retained']
runner.SOURCE_FILES = {'library.cod': definitions + ''.join('export ' + name + ';' for name in exports)}
runner.SOURCE = ('using "std:exceptions" OutOfMemory;using "std:exceptions" MissingReturnException;'
                 + ''.join('import "library.cod" ' + name + ';' for name in exports)
                 + '{{func f=wrap(wrap(base));' + execution)
runner.EXPECTED_SHADOWS = []
runner.EXTRA_SOURCES += [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
