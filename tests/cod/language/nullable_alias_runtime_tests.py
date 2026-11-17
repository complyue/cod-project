#!/usr/bin/env python3
"""Nullable reference copies preserve exact deferred resource release."""
from pathlib import Path
from native_test_support import run_native
import nullable_predicate_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
setup = 'ptr selected=if(choose){payload}else{null in here};ptr observed=selected;ptr saved=observed;selected=null;'
for name, parameters, value in (
    ('cleanup', 'ptr x:u64 in O', '74u64'),
    ('alias', 'ptr x:u64 in O,ptr y:u64 in O', '75u64'),
):
    before = 'proc '+name+'('+parameters+',payload:Exception)->void throws NullPointerException where {owner O}{'
    after = 'proc '+name+'('+parameters+',payload:Exception,choose:bool)->void throws NullPointerException where {owner O}{'+setup
    assert before in FIXTURE.source
    FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(before, after))
    before = 'if(classify(payload)){x<-'+value+';{}}else{};try{throw payload;}catch(e:const MissingReturnError){}catch(e:const Exception){x<-'+value+';{}};'
    after = 'if(classify(observed)){x<-'+value+';{}}else{};if(saved is null){x<-'+value+';{}}else{try{throw payload;}catch(e:const MissingReturnError){}catch(e:const Exception){x<-'+value+';{}};};'
    assert before in FIXTURE.source
    FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(before, after))
for before, after in (('cleanup(b,failure())','cleanup(b,failure(),false)'), ('alias(c,c,failure())','alias(c,c,failure(),true)')):
    assert before in FIXTURE.source
    FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(before, after))
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
