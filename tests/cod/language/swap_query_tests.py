#!/usr/bin/env python3
"""Storage-exchange queries preserve destination contracts without execution."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
BASE = 'val x<-1,y<-7;ptr p=x,q=y;'
REF = 'func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void where {owner O}{intrinsic (<=>)(a,b);}'
BITS = 'func (<->)(ref a:i64 in O,ref b:i64 in P)->void where {owner O,owner P}{intrinsic (<->)(a,b);}'
OUT = lambda value:'behave @print('+value+');'
QUERY = lambda expression:'val result<-make local typeof({ '+expression+';1 }){9};'

runner.CASES = {}
for name, spelling in [('infix','p OP q'),('explicit','(OP)(p,q)'),('intrinsic','intrinsic (OP)(p,q)')]:
    for operation in ('<=>','<->'):
        runner.CASES[name+operation] = (IO+BASE+QUERY(spelling.replace('OP',operation))+OUT('result')+OUT('x')+OUT('y')+OUT('(p is x)'),0,'9\n1\n7\ntrue\n','')
runner.CASES.update({
 'uninitialized_bits': ('val x<-make local i64,y<-make local i64;'+QUERY('x<->y'),0,'',''),
 'null_bits_targets': ('ptr p:i64=null in here,q:i64=null in here;'+QUERY('p<->q'),0,'',''),
 'null_reference_targets': ('ptr p:i64=null in here,q:i64=null in here;'+QUERY('p<=>q'),0,'',''),
 'fixed_bits_slots': ('const ptr p=make local i64,q=make local i64;'+QUERY('p<->q'),0,'',''),
 'readonly_reference_targets': ('val x<-1,y<-7;ptr p:const i64=x,q:const i64=y;'+QUERY('p<=>q'),0,'',''),
 'different_bits_owners': ('val x<-1;{{val y<-7;'+QUERY('x<->y')+'}};',0,'',''),
 'selected_slots': (BASE+QUERY('{if(true){p}else{q}}<=>q'),0,'',''),
 'ordinary_alias': (BASE+'ptr var alias->p;'+QUERY('alias<=>q'),0,'',''),
 'method_body_not_demanded': ('func (<=>)(ptr a:T in O,ptr b:T in O)->void where {T,owner O}{a<-true;}'+BASE+QUERY('p<=>q'),0,'',''),
 'bits_method_body_not_demanded': ('func (<->)(ref a:T in O,ref b:T in P)->void where {T,owner O,owner P}{a<-true;}'+BASE+QUERY('p<->q'),0,'',''),
 'method_different_types': ('func (<=>)(ptr a:i64 in O,ptr b:bool in P)->void where {owner O,owner P}{}val x<-1,y<-true;ptr p=x,q=y;'+QUERY('p<=>q'),0,'',''),
 'declared_failure_not_raised': ('using "std:exceptions" NullPointerException;func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void throws NullPointerException where {owner O};'+BASE+QUERY('p<=>q'),0,'',''),
 'procedure_not_invoked': (IO+'proc (<->)(ref a:i64 in O,ref b:i64 in P)->void where {owner O,owner P}{'+OUT('99')+'return;}func run()->i64{'+BASE+QUERY('p<->q')+'return result;}'+OUT('run()'),0,'9\n',''),
 'operand_not_invoked': (IO+'proc target(ref a:i64 in O)->ref i64 in O where {owner O}{'+OUT('99')+'return a;}'+BASE+QUERY('target(x)<->target(y)')+OUT('x')+OUT('y'),0,'1\n7\n',''),
 'outer_query_no_capture': (IO+'func run()->i64{'+BASE+'func inner()->i64{'+QUERY('p<=>q')+'return result;};return inner();}'+OUT('run()'),0,'9\n',''),
 'generic_query': (IO+'func run(value:T)->i64 where T{val x<-value,y<-value;ptr p=x,q=y;'+QUERY('p<=>q')+'return result;}'+OUT('run(1)')+OUT('run(true)'),0,'9\n9\n',''),
 'method_distinct_owners_and_access': ('func (<=>)(ptr a:const i64 in O,ptr b:i64 in P)->void where {owner O,owner P}{}val x<-1;ptr p:const i64=x;{{val y<-7;ptr q=y;'+QUERY('p<=>q')+'}};',0,'',''),
 'ambiguous_methods': ('func (<->)(ref a:T in O,ref b:i64 in P)->void where {T,owner O,owner P}{}func (<->)(ref a:i64 in O,ref b:T in P)->void where {T,owner O,owner P}{}'+BASE+QUERY('p<->q'),2,'','ambiguous methods'),
 'retained_fixed_slot': ('func run()->void{val x<-1,y<-7;const ptr p=x;ptr q=y;func inner()->void{'+QUERY('p<=>q')+'};inner();}run();',2,'','const reference binding'),
 'retained_alias_access': ('func run()->void{'+BASE+'ptr var a:const->p;ptr var b:const->q;func inner()->void{'+QUERY('a<=>b')+'};inner();}run();',2,'','readonly source'),
 'nested_different_owners': ('func run()->void{val x<-1;ptr p=x;{{val y<-7;ptr q=y;func inner()->void{'+QUERY('p<=>q')+'};inner();}};}run();',2,'','initializer-fixed owner'),
 'query_establishes_no_initialization': ('val x<-make local i64,y<-1;'+QUERY('x<->y')+'x;',2,'','before definite initialization'),
 'wrong_arity': (BASE+QUERY('(<=>)(p)'),2,'','swap requires two destinations'),
 'bits_value_operand': (BASE+QUERY('x<->1'),2,'','storage destinations'),
 'reference_val_operand': (BASE+QUERY('x<=>q'),2,'','val storage has no reference binding'),
 'reference_borrowed_result': ('func target(ptr a:i64 in O)->ptr i64 in O where {owner O}{return a;}'+BASE+QUERY('target(p)<=>q'),2,'','requires a binding slot'),
 'fixed_reference_slot': ('val x<-1,y<-7;const ptr p=x;ptr q=y;'+QUERY('p<=>q'),2,'','const reference binding'),
 'fixed_slot_alias': (BASE+'const ptr var alias->p;'+QUERY('alias<=>q'),2,'','const reference binding'),
 'conditional_fixed_slot': ('val x<-1,y<-7;const ptr p=x;ptr q=y;'+QUERY('{if(true){p}else{q}}<=>q'),2,'','const reference binding'),
 'readonly_bits': ('const val x<-1;val y<-7;'+QUERY('x<->y'),2,'','readonly destination'),
 'different_bits_types': ('val x<-1,y<-7u64;'+QUERY('x<->y'),2,'','exact same type'),
 'different_reference_types': ('val x<-1,y<-7u64;ptr p=x,q=y;'+QUERY('p<=>q'),2,'','exact target type'),
 'different_reference_owners': ('val x<-1;ptr p=x;{{val y<-7;ptr q=y;'+QUERY('p<=>q')+'}};',2,'','initializer-fixed owner'),
 'readonly_reference_source': ('val x<-1,y<-7;ptr p:const i64=x,q=y;'+QUERY('p<=>q'),2,'','readonly source'),
 'alias_permission_laundering': (BASE+'ptr var a:const->p;ptr var b:const->q;'+QUERY('a<=>b'),2,'','readonly source'),
 'method_alias_contract': ('func (<=>)(ptr a:const i64 in O,ptr b:const i64 in O)->void where {owner O}{}'+BASE+'ptr var left:const->p;ptr var right:const->q;'+QUERY('left<=>right'),0,'',''),
 'mixed_view': (BASE+QUERY('{if(true){x}else{1}}<->y'),2,'','storage destinations'),
 'callable_bits': ('func one()->u64{return 1u64;}func x=one;func y=one;'+QUERY('x<->y'),2,'','bits storage destinations'),
 'method_owner_constraint': (REF+'val x<-1;ptr p=x;{{val y<-7;ptr q=y;'+QUERY('p<=>q')+'}};',2,'','initializer-fixed owner'),
})
for name, operation, definitions in [('reference','<=>',REF),('bits','<->',BITS)]:
    runner.CASES['imported_'+name] = ({'ops.cod':definitions+'export ('+operation+');',
        'main.cod':IO+'using "./ops.cod" ('+operation+') as exchange;'+BASE+QUERY('exchange(p,q)')+OUT('x')+OUT('y')},0,'1\n7\n','')
runner.CASES['selected_operation_identity'] = ({'ops.cod':REF+'export (<=>);',
    'main.cod':IO+'using "./ops.cod" (<=>) as (<->);'+BASE+QUERY('p<->q')+OUT('(p is x)')},0,'true\n','')
READONLY_METHOD='func (<=>)(ptr lhs_view:const T in O,ptr rhs_view:const T in O)->void where {T,owner O}{lhs_view<-true;}'
ALIASES=BASE+'ptr var ro:const->p;ptr var other:const->q;'
for name,operation in [('infix','ro<=>other'),('explicit','(<=>)(ro,other)'),('selected','{if(true){ro}else{other}}<=>other')]:
    runner.CASES['readonly_alias_'+name]=(IO+READONLY_METHOD+ALIASES+QUERY(operation)+OUT('result')+OUT('p')+OUT('q'),0,'9\n1\n7\n','')
runner.CASES['readonly_alias_generic_context']=(IO+READONLY_METHOD+'func run(value:T)->i64 where T{val x<-value,y<-value;ptr p=x,q=y;ptr var ro:const->p;ptr var other:const->q;'+QUERY('ro<=>other')+'return result;}'+OUT('run(1)')+OUT('run(true)'),0,'9\n9\n','')
runner.CASES['readonly_alias_retained_query']=(IO+READONLY_METHOD+'func outer()->i64{'+ALIASES+'func inner()->i64{'+QUERY('ro<=>other')+'return result;};return inner();}'+OUT('outer()'),0,'9\n','')
runner.CASES['readonly_alias_procedure_query']=(IO+READONLY_METHOD.replace('func (<=>)','proc (<=>)').replace('lhs_view<-true;','behave @print(99);')+'func run()->i64{'+ALIASES+QUERY('ro<=>other')+'return result;}'+OUT('run()'),0,'9\n','')
runner.CASES['query_does_not_supply_invocation_proof']=(READONLY_METHOD.replace('lhs_view<-true;','lhs_view=rhs_view:void;')+BASE.replace('ptr p=x,q=y;','ptr p=x;ptr q:const i64=y;')+'ptr var ro:const->p;ptr var other:const->q;'+QUERY('ro<=>other')+'ro<=>other;',2,'','permission preservation')
runner.CASES['readonly_alias_intrinsic_contract']=(READONLY_METHOD+ALIASES+QUERY('intrinsic (<=>)(ro,other)'),2,'','readonly source')
runner.CASES['readonly_alias_declaration_has_no_method']=(READONLY_METHOD.replace('{lhs_view<-true;}',';')+ALIASES+QUERY('ro<=>other'),2,'','readonly source')
runner.CASES['readonly_alias_fixed_slot']=(READONLY_METHOD+ALIASES.replace('ptr var ro','const ptr var ro')+QUERY('ro<=>other'),2,'','const reference binding')
runner.CASES['readonly_alias_owner_mismatch']=(READONLY_METHOD+'val x<-1;ptr p:const i64=x;ptr var ro:const->p;{{val y<-7;ptr q:const i64=y;ptr var other:const->q;'+QUERY('ro<=>other')+'}};',2,'','initializer-fixed owner')
runner.CASES['readonly_alias_selected_family']=({'ops.cod':READONLY_METHOD+'export (<=>);','main.cod':IO+'using "./ops.cod" (<=>) as exchange;'+ALIASES+QUERY('exchange(ro,other)')+OUT('result')},0,'9\n','')

runner.CASES['readonly_method_query_has_no_write']=(READONLY_METHOD+ALIASES.replace('x<-1','x<-make local i64')+QUERY('ro<=>other')+'x;',2,'','before definite initialization')
runner.CASES['query_does_not_supply_ref_preservation']=(READONLY_METHOD.replace('lhs_view<-true;','lhs_view=rhs_view:void;')+BASE.replace('ptr p=x,q=y;','ref p=x,q=y;')+'ref var ro:const->p;ref var other:const->q;'+QUERY('ro<=>other')+'ro<=>other;',0,'','')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
