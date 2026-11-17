#!/usr/bin/env python3
"""Loops in deferred actions preserve exact release through nested registration failure."""
from pathlib import Path
import selected_loop_initialization_runtime_tests as fixture

runner=fixture.runner
assert runner.SOURCE.count(' for(i in 0u64..3u64){')==1
assert runner.SOURCE.count(' observe(selected);\n}\nverify(false);')==1
runner.SOURCE=runner.SOURCE.replace(' for(i in 0u64..3u64){',' {{defer{for(i in 0u64..3u64){')
runner.SOURCE=runner.SOURCE.replace(' observe(selected);\n}\nverify(false);',' observe(selected);\n };}};\n}\nverify(false);')
assert 'created==8 && peak==1 && failures==4' in runner.HARNESS
# Block records are released before their bodies invoke nested registrations.
runner.HARNESS=runner.HARNESS.replace('created==8 && peak==1 && failures==4','created==10 && peak==1 && failures==4')
runner.EXTRA_SOURCES+=[Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
