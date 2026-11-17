#!/usr/bin/env python3
"""Explicit assignments retain selected slots, complete images and failure cleanup."""
from pathlib import Path
from native_test_support import run_native
import selected_slot_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
for source, explicit in (
    ('(if(flag){b}else{a})=c;', 'intrinsic (=)((if(flag){b}else{a}),c);'),
    ('((if(flag){a}else{b})=g)<-f;', '(<-)((=)((if(flag){a}else{b}),g),f);'),
    ('defer{(if(flag){a}else{b})=g;}', 'defer{intrinsic (=)((if(flag){a}else{b}),g);}'),
):
    assert source in FIXTURE.source
    FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(source, explicit))
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
