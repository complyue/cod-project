#!/usr/bin/env python3
"""Explicit owner aliases preserve native borrows, absent results and cleanup."""
from pathlib import Path
import borrowed_call_runtime_tests as fixture

runner=fixture.runner
runner.SOURCE=runner.SOURCE.replace(
 'proc show(x:u64 in O)->void where {owner O}{observe(x);}',
 'proc show(x:u64 in O)->void where {owner O}{ref p:u64 in O=x;observe(p);}')
runner.SOURCE=runner.SOURCE.replace(
 'proc set(x:u64 in O)->void where {owner O}{x<-33u64;}',
 'proc set(x:u64 in O)->void where {owner O}{ptr p:u64 in O=null;p=x;p<-33u64;}')
runner.SOURCE=runner.SOURCE.replace(
 'throws NullPointerException where {owner O}{observe(x);}',
 'throws NullPointerException where {owner O}{ptr p:u64 in O=x;observe(p);}')
runner.SOURCE=runner.SOURCE.replace('defer{x<-39u64;};return x;',
 'defer{{ref p:u64 in O=x;p<-39u64;}};return x;')
runner.SOURCE=runner.SOURCE.replace(
 'if(n==0u64){return x;}else{return recursive(x,n-1u64);};',
 'ref p:T in O=x;if(n==0u64){return p;}else{return recursive(p,n-1u64);};')
runner.SOURCE+='''
func absent(ptr x:T in O)->ptr T in O where {T,owner O}{ptr p:T in O=null;return p;}
ptr p=absent(a);behave @print(isnull(p));p=a;p<-41u64;observe(a);
'''
runner.EXPECTED_VALUES += ['true','41']
runner.EXTRA_SOURCES += [Path(__file__)]
if __name__=='__main__':
 raise SystemExit(runner.main())
