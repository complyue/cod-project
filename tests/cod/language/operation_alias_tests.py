#!/usr/bin/env python3
"""Selected operation identities preserve destination contracts through aliases."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
OUT = lambda value: 'behave @print('+value+');'
BASE = 'val x<-1,y<-7,z<-9;ptr p=x,q=y;'
PTR = 'func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void where {owner O}{intrinsic (<=>)(a,b);}'
REF = PTR.replace('ptr a:', 'ref a:').replace('ptr b:', 'ref b:')
BITS = REF.replace('<=>', '<->')
READ = 'proc (<=>)(a:i64 in O,b:i64 in O)->void where {owner O}{'+OUT('a')+OUT('b')+'return;}'

def source(method, body, *, operation='<=>', selection='using', extra=''):
    return {'ops.cod': IO+NPE+method+'export ('+operation+');',
            'main.cod': IO+NPE+selection+' "ops.cod" ('+operation+') as exchange;'+extra+body}

runner.CASES = {}
for name, method, body, output in (
    ('caller_bindings', REF, 'exchange(p,q);'+OUT('p')+OUT('q')+OUT('x'), '7\n1\n1\n'),
    ('read_after_operands', REF, 'exchange(p,{p=z;q});'+OUT('p')+OUT('q'), '7\n9\n'),
    ('ordered_once', REF, 'exchange({'+OUT('3')+'p},{'+OUT('4')+'q});'+OUT('p'), '3\n4\n7\n'),
    ('entry_read_after_restore', READ, 'p=null;exchange(p,{p=z;q});', '9\n7\n'),
    ('entry_read_after_clear', READ, 'try{exchange(p,{p=null;q});}catch(e:const NullPointerException){'+OUT('8')+'};', '8\n'),
    ('null_exchange', PTR, 'q=null;exchange(p,q);'+OUT('(p is null)')+OUT('q'), 'true\n1\n'),
    ('self_binding', PTR, 'exchange(p,p);'+OUT('p'), '1\n'),
    ('selected_alias', REF, 'ptr var chosen->p;exchange(chosen,q);'+OUT('p'), '7\n'),
    ('builtin_fallback', REF, 'val a<-false,b<-true;ptr u=a,v=b;exchange(u,v);'+OUT('u')+OUT('v'), 'true\nfalse\n'),
    ('deferred_current_targets', PTR, '{{defer exchange(p,q);q=z:void;}};'+OUT('p')+OUT('q'), '9\n1\n'),
    ('deferred_ref_restore', REF, '{{defer exchange(p,q);defer (=)(q,z);q=null:void;}};'+OUT('p'), '9\n'),
    ('deferred_read_null', READ, 'try{ {{defer exchange(p,q);p=null:void;}};{}}catch(e:const NullPointerException){'+OUT('8')+'};', '8\n'),
    ('deferred_read_registration', READ, 'p=null;try{ {{defer exchange(p,q);p=z:void;}};{}}catch(e:const NullPointerException){'+OUT('8')+'};', '8\n'),
    ('deferred_builtin', REF, 'val a<-false,b<-true;ptr u=a,v=b;{{defer exchange(u,v);}};'+OUT('u'), 'true\n'),
    ('failed_rhs', REF, 'ptr absent:i64=null in here;try{exchange({p=z;p},{absent;q});}catch(e:const NullPointerException){'+OUT('p')+OUT('q')+'};', '9\n7\n'),
    ('failed_registration', PTR, 'ptr absent:i64=null in here;try{ {{defer exchange(p,{absent;q});}};{}}catch(e:const NullPointerException){'+OUT('p')+OUT('q')+'};', '1\n7\n'),
):
    runner.CASES[name] = (source(method, BASE+body), 0, output, '')

for name, body, output in (
    ('bits_saved_target', 'exchange(p,{p=z;q});'+OUT('x')+OUT('y')+OUT('p')+OUT('(q is y)'), '7\n1\n9\ntrue\n'),
    ('bits_deferred_saved_targets', '{{defer exchange(p,q);p=z:void;}};'+OUT('x')+OUT('y')+OUT('p'), '7\n1\n9\n'),
    ('bits_builtin_fallback', 'val a<-false,b<-true;exchange(a,b);'+OUT('a')+OUT('b'), 'true\nfalse\n'),
):
    runner.CASES[name] = (source(BITS, BASE+body, operation='<->'), 0, output, '')

runner.CASES['post_operand_initialization'] = (source(REF, 'val x<-make local i64,y<-7;ptr p=x,q=y;exchange(p,{x<-1;q});'+OUT('p')+OUT('q')), 0, '7\n1\n', '')
runner.CASES['deferred_initialization'] = (source(REF, BASE+'val empty<-make local i64;{{defer exchange(p,q);defer (<-)(empty,9);q=empty:void;}};'+OUT('p')), 0, '9\n', '')
runner.CASES['extension_inherits_slots'] = (source(REF, 'val x<-false,y<-true;ptr p=x,q=y;exchange(p,q);'+OUT('p'), selection='import', extra='func exchange(ref a:bool in O,ref b:bool in O)->void where {owner O}{intrinsic (<=>)(a,b);}'), 0, 'true\n', '')
runner.CASES['mixed_callable_kinds'] = (source(REF, 'val x<-false,y<-true;ptr p=x,q=y;exchange(p,q);'+OUT('p'), selection='import', extra='proc exchange(ref a:bool in O,ref b:bool in O)->void where {owner O}{'+OUT('4')+'intrinsic (<=>)(a,b);return;}'), 0, '4\ntrue\n', '')
runner.CASES['double_rename'] = ({'ops.cod': REF+'export (<=>);', 'middle.cod':'import "ops.cod" (<=>) as exchange;func exchange(ref a:bool in O,ref b:bool in O)->void where {owner O}{intrinsic (<=>)(a,b);}export exchange;', 'main.cod':IO+'using "middle.cod" exchange as rotate;'+BASE+'rotate(p,q);'+OUT('p')+'val a<-false,b<-true;ptr u=a,v=b;rotate(u,v);'+OUT('u')},0,'7\ntrue\n','')
runner.CASES['ordinary_alias_keeps_local_bindings'] = ({'ops.cod':REF.replace('(<=>)(ref','helper(ref')+'export helper;', 'main.cod':IO+'using "ops.cod" helper as (<=>);'+BASE+'p<=>q;'+OUT('p')+OUT('q')},0,'1\n7\n','')
runner.CASES['reference_alias_at_bits_token'] = ({'ops.cod':REF+'export (<=>);','main.cod':IO+'using "ops.cod" (<=>) as (<->);'+BASE+'p<->q;'+OUT('(p is y)')+OUT('x')},0,'true\n1\n','')
runner.CASES['intrinsic_uses_builtin_spelling'] = ({'ops.cod':REF+'export (<=>);','main.cod':IO+'using "ops.cod" (<=>) as (<->);'+BASE+'intrinsic (<->)(p,q);'+OUT('(p is x)')+OUT('x')},0,'true\n7\n','')
runner.CASES['lexical_shadow'] = (source(REF,'proc run()->void{func exchange(a:i64,b:i64)->i64{return a+b;};'+OUT('exchange(1,7)')+'return;}run();'),0,'8\n','[-Wshadow]')
runner.CASES['pending_return'] = (source(REF,'proc run()->i64{'+BASE+'defer{'+OUT('p')+OUT('q')+'};defer exchange(p,q);return p;}'+OUT('run()')),0,'7\n1\n1\n','')

for name, method, body, diagnostic in (
    ('fixed_binding', REF, 'val x<-1,y<-7;const ptr p=x;ptr q=y;exchange(p,q);', 'const reference binding'),
    ('value_is_not_binding', REF, 'val x<-1,y<-7;exchange(x,y);', 'val storage has no reference binding'),
    ('late_ref_null', REF, BASE+'{{defer exchange(p,q);q=null:void;}};', 'cannot prove a non-null target'),
    ('late_ref_uninitialized', REF, BASE+'val empty<-make local i64;{{defer exchange(p,q);q=empty:void;}};', 'cannot prove initialization'),
    ('ended_slot', PTR, BASE+'defer exchange(({{ptr r=x;r}}),q);', 'lexical scope has ended'),
    ('callable_slot_abi', REF, 'func f=exchange;', 'unproved callable slot contract'),
    ('nearest_noncallable', REF, 'proc run(exchange:i64)->void{exchange(1,7);}run(2);', "nearest binding 'exchange' is not callable"),
):
    runner.CASES[name] = (source(method,body),2,'',diagnostic)

runner.CASES['ref_not_weakened'] = (source(PTR, 'val x<-1,y<-7;ref p=x,q=y;exchange(p,q);'),0,'','')

runner.CASES['type_query'] = (source(REF,BASE+'val result<-make local typeof({exchange(p,q);1}){9};'+OUT('result')+OUT('(p is x)')),0,'9\ntrue\n','')

for name, declaration, diagnostic in (
    ('extension_requires_owners','func exchange(a:bool,b:bool)->void{}','explicit external owners'),
    ('extension_requires_void','func exchange(ref a:bool in O,ref b:bool in O)->bool where {owner O}{return true;}','swap method must return void'),
    ('extension_requires_two','func exchange(ref a:bool in O)->void where {owner O}{}','operator method arity'),
    ('extension_inferred_result','func exchange(ref a:bool in O,ref b:bool in O) where {owner O}{return true;}','swap method must return void'),
):
    runner.CASES[name] = (source(REF,'',selection='import',extra=declaration),2,'',diagnostic)
runner.CASES['using_has_no_extension_permission'] = (source(REF,'',extra='func exchange(ref a:bool in O,ref b:bool in O)->void where {owner O}{}'),2,'','using does not grant callable extension')

for order in ('method_first','builtin_first'):
    calls = 'run(false,true);run(false,1);run(true,1);' if order=='builtin_first' else 'run(false,1);run(true,1);run(false,true);'
    body = 'proc run(flag:bool,value:T)->void where T{val x<-value,y<-value;ptr p=x,q=y;if(flag){p=null;{}}else{};try{exchange(p,q);}catch(e:const NullPointerException){'+OUT('8')+'};return;}'+calls
    runner.CASES['shared_context_'+order] = (source(READ,body),0,'1\n1\n8\n','')

for selection, result in (('using','1\n'),('import','7\n')):
    runner.CASES['helper_context_'+selection] = ({
        'ops.cod':IO+'func score(x:T)->i64 where T{return 1;}proc (<=>)(ref a:i64 in O,ref b:i64 in O)->void where {owner O}{'+OUT('score(a)')+'intrinsic (<=>)(a,b);return;}export score;export (<=>);',
        'main.cod':IO+'import "ops.cod" score;func score(input:i64)->i64{return 7;}'+selection+' "ops.cod" (<=>) as exchange;'+BASE+'exchange(p,q);'+OUT('p')},0,result+'7\n','')
runner.CASES['deferred_alias_uses_current_tls'] = ({
    'ops.cod':IO+'symbol @amount:i64;export @amount;proc (<=>)(ref a:i64 in O,ref b:i64 in O)->void where {owner O}{'+OUT('@amount')+'intrinsic (<=>)(a,b);return;}export (<=>);',
    'main.cod':IO+'symbol from "ops.cod" @amount;using "ops.cod" (<=>) as exchange;'+BASE+'effect @amount<-1;{{defer exchange(p,q);effect @amount<-7;}};'+OUT('p')},0,'7\n7\n','')

DECL = 'func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void throws NullPointerException where {owner O};'
runner.CASES['builtin_declaration_obligation'] = (source(DECL,BASE+'exchange(p,q);'),2,'','requires a covering catch')
runner.CASES['builtin_declaration_execution'] = (source(DECL,BASE+'try{exchange(p,q);}catch(e:const NullPointerException){};'+OUT('p')),0,'7\n','')
runner.CASES['intrinsic_bypasses_alias_declaration'] = (source(DECL,BASE+'intrinsic (<=>)(p,q);'+OUT('p')),0,'7\n','')
runner.CASES['intrinsic_does_not_resolve_alias'] = (source(REF,BASE+'intrinsic (exchange)(p,q);'),2,'','expected an operator or intrinsic name')

runner.CASES['operand_shadow_preserves_declaration'] = (source(DECL,BASE+'exchange({val exchange<-1;p},q);'),2,'','requires a covering catch')
runner.CASES['operand_shadow_preserves_builtin'] = (source(DECL,BASE+'try{exchange({val exchange<-1;p},q);}catch(e:const NullPointerException){};'+OUT('p')),0,'7\n','[-Wshadow]')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
