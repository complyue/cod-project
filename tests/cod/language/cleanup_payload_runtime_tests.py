#!/usr/bin/env python3
"""Composed cleanup writes retain native ordering, snapshots and resource counts."""
from pathlib import Path
from native_test_support import run_native
import exception_writeback_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func copy_later(ref target:Exception in O,ref source:const Exception in P)->void where {owner O,owner P}{target<-source;}
func saved_cleanup_result(ref target:Exception in O)->const Exception where {owner O}{
 defer{defer{target<-null_value();{}};return target;};
 return null_value();
}
{{
 val payload:Exception<-null_value(),source:Exception<-null_value();
 {{defer copy_later(payload,source);source<-missing_value();}};
 try{throw payload;}catch(e:const Exception){observe(e.category);};
 {{defer write_payload(payload,source,20u64);source<-null_value();}};
 try{throw payload;}catch(e:const Exception){observe(e.category);};
 {{defer{payload<-missing_value();{}};defer{payload<-null_value();{}};}};
 try{throw payload;}catch(e:const Exception){observe(e.category);};
 {{defer (<->)(payload,source);}};
 observe(payload.category);observe(source.category);
 payload<-missing_value();
 try{throw saved_cleanup_result(payload);}catch(e:const Exception){observe(e.category);};
 observe(payload.category);
 {{defer{ {{defer (<-)(payload,missing_value());}};};}};
 try{throw payload;}catch(e:const Exception){observe(e.category);};
}};
''')
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['5','5','5','4','5','5','4','5'])
if __name__=='__main__':
    raise SystemExit(run_native(FIXTURE))
