#!/usr/bin/env python3
"""Explicit assignments retain selected slots, complete images and failure cleanup."""
from pathlib import Path
import selected_slot_runtime_tests as fixture

runner = fixture.runner
for source, explicit in (
    ('(if(flag){b}else{a})=c;', 'intrinsic (=)((if(flag){b}else{a}),c);'),
    ('((if(flag){a}else{b})=g)<-f;', '(<-)((=)((if(flag){a}else{b}),g),f);'),
    ('defer{(if(flag){a}else{b})=g;}', 'defer{intrinsic (=)((if(flag){a}else{b}),g);}'),
):
    assert source in runner.SOURCE
    runner.SOURCE = runner.SOURCE.replace(source, explicit)
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
