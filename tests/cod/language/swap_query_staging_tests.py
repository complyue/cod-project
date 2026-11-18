#!/usr/bin/env python3
"""Exchange type inspection emits exactly the independently declared type's code."""
from pathlib import Path
import declaration_query_staging_tests as runner

BASE = 'val x<-1,y<-7;ptr p=x,q=y;'
runner.CASES = (
    ('reference', BASE, '{p<=>q;1}', 'i64', '9', '9\n'),
    ('bits', BASE, '{p<->q;1}', 'i64', '9', '9\n'),
    ('intrinsic_reference', BASE, '{intrinsic (<=>)(p,q);1}', 'i64', '9', '9\n'),
    ('uninitialized_bits', 'val p<-make local i64,q<-make local i64;', '{p<->q;1}', 'i64', '9', '9\n'),
    ('null_bits', 'ptr p:i64=null in here,q:i64=null in here;', '{p<->q;1}', 'i64', '9', '9\n'),
    ('selected_slots', BASE, '{ {if(true){p}else{q}}<=>q;1}', 'i64', '9', '9\n'),
    ('procedure_method', 'proc (<->)(ref a:i64 in O,ref b:i64 in P)->void where {owner O,owner P}{behave @print(99);}'+BASE,
     '{p<->q;1}', 'i64', '9', '9\n'),
    ('generic_body_not_demanded', 'func (<=>)(ptr a:T in O,ptr b:T in O)->void where {T,owner O}{a<-true;}'+BASE,
     '{p<=>q;1}', 'i64', '9', '9\n'),
    ('operand_not_called', 'proc target(ref a:i64 in O)->ref i64 in O where {owner O}{behave @print(99);return a;}'+BASE,
     '{target(x)<->target(y);1}', 'i64', '9', '9\n'),
    ('callable_targets', 'func one()->u64{return 1u64;}func f=one;ptr p=f,q=f;',
     '{p<=>q;1}', 'i64', '9', '9\n'),
)
READONLY_METHOD='func (<=>)(ptr lhs_view:const T in O,ptr rhs_view:const T in O)->void where {T,owner O}{lhs_view<-true;}'
ALIASES=BASE+'ptr var ro:const->p;ptr var other:const->q;'
runner.CASES += (
    ('readonly_alias',READONLY_METHOD+ALIASES,'{ro<=>other;1}','i64','9','9\n'),
    ('readonly_alias_explicit',READONLY_METHOD+ALIASES,'{(<=>)(ro,other);1}','i64','9','9\n'),
    ('readonly_alias_selected',READONLY_METHOD+ALIASES,'{ {if(true){ro}else{other}}<=>other;1}','i64','9','9\n'),
)

runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
