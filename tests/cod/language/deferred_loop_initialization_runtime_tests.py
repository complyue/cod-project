#!/usr/bin/env python3
"""Loops in deferred actions preserve exact release through nested registration failure."""
from pathlib import Path
from native_test_support import run_native
import selected_loop_initialization_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
assert FIXTURE.source.count(' for(i in 0u64..3u64){')==1
assert FIXTURE.source.count(' observe(selected);\n}\nverify(false);')==1
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(' for(i in 0u64..3u64){',' {{defer{for(i in 0u64..3u64){'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(' observe(selected);\n}\nverify(false);',' observe(selected);\n };}};\n}\nverify(false);'))
assert 'created==8 && peak==1 && failures==4' in FIXTURE.harness
# Block records are released before their bodies invoke nested registrations.
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('created==8 && peak==1 && failures==4','created==10 && peak==1 && failures==4'))
if __name__=='__main__':
    raise SystemExit(run_native(FIXTURE))
