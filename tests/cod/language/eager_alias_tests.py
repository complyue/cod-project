#!/usr/bin/env python3
"""Eager aliases retain selected operations, preparation and failure contracts."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
OUT = lambda value: 'behave @print('+value+');'
ADD = 'func (+)(a:bool,b:bool)->bool{return a||b;}'
NEG = 'func (-)(a:bool)->bool{return !a;}'
NOT = 'func (!)(a:i64)->bool{return a==0;}'

def source(body, method=ADD, operation='+', alias='calculate', selection='using'):
    return {'ops.cod':IO+NPE+method+'export ('+operation+');',
            'main.cod':IO+NPE+selection+' "ops.cod" ('+operation+') as '+alias+';'+body}

runner.CASES = {
 'named_method_and_builtin': (source(OUT('calculate(false,true)')+OUT('calculate(2,3)')+OUT('calculate(1u8,2u64)')),0,'true\n5\n3\n',''),
 'snapshot_before_later_write': (source('val x<-1;'+OUT('calculate(x,{x<-9;2})')+OUT('x')),0,'3\n9\n',''),
 'ordered_once': (source('proc next(x:i64)->i64{'+OUT('x')+'return x;}'+OUT('calculate(next(1),next(2))')),0,'1\n2\n3\n',''),
 'operand_shadow': (source(OUT('calculate({val calculate<-1;2},3)')),0,'5\n','[-Wshadow]'),
 'deferred_snapshot': (source('val x<-1;{{defer calculate(x,{'+OUT('2')+'x<-9;3});'+OUT('4')+'}};'+OUT('x')),0,'2\n4\n9\n',''),
 'builtin_negation': (source(OUT('calculate(3)')+OUT('calculate(2f32)'),NEG,'-'),0,'-3\n-2\n',''),
 'builtin_not': (source(OUT('calculate(false)')+OUT('calculate(1)'),NOT,'!'),0,'true\nfalse\n',''),
 'not_refines_null': (source('proc run(ptr p:i64 in O)->void where {owner O}{if(calculate((p is null))){'+OUT('p')+'}else{'+OUT('0')+'};return;}val x<-8;run(x);',NOT,'!'),0,'8\n',''),
 'method_not_does_not_refine': (source('proc run(ptr p:i64 in O)->void where {owner O}{if(calculate((p is null))){'+OUT('p')+'}else{};}', 'func (!)(a:bool)->bool{return a;}','!'),2,'','requires a covering catch'),
 'type_query_builtin': (source('val result<-make local typeof(calculate(1u8,2u64)){7u64};'+OUT('result')),0,'7\n',''),
 'inferred_result': (source('func result(){return calculate(1u8,2u64);}'+OUT('result()')),0,'3\n',''),
 'value_and_method_contexts': (source('func twice(a:T)->const T where T{return calculate(a,a);}'+OUT('twice(4)')+OUT('twice(true)')),0,'8\ntrue\n',''),
 'lexical_capture_free': (source('func f=func[](x:i64)->i64{return calculate(x,1);};'+OUT('f(2)')),0,'3\n',''),
 'lexical_shadow': (source('proc run()->void{func calculate(a:i64,b:i64)->i64{return a;};'+OUT('calculate(1,7)')+'return;}run();'),0,'1\n','[-Wshadow]'),
 'noncallable_shadow': (source('proc run(calculate:i64)->void{calculate(1,2);}run(3);'),2,'',"nearest binding 'calculate' is not callable"),
 'no_implicit_fallback': ({'ops.cod':'func helper(a:bool,b:bool)->bool{return a;}export helper;', 'main.cod':'using "./ops.cod" helper as (+);1+2;'},2,'','no applicable'),
 'ordinary_family_at_operator': ({'ops.cod':'func helper(a:i64,b:i64)->i64{return a;}export helper;', 'main.cod':IO+'using "./ops.cod" helper as (+);'+OUT('1+7')},0,'1\n',''),
 'renamed_operator': (source(OUT('8+3')+OUT('(+)(3)'),NEG+'func (-)(a:bool,b:bool)->bool{return a;}', '-', '(+)'),0,'5\n-3\n',''),
 'eager_at_swap_spelling': (source('val x<-1,y<-7;'+OUT('x<=>y')+'val result<-make local typeof(x<=>y){9};'+OUT('result'),ADD,'+','(<=>)'),0,'8\n9\n',''),
 'intrinsic_uses_spelling': (source(OUT('intrinsic (+)(8,3)'),NEG,'-','(+)'),0,'11\n',''),
 'negative_alias_proves_loop': (source('val x<-make local i64;for(i in (+)(1)..0){x<-i;};'+OUT('x'),NEG,'-','(+)'),0,'-1\n',''),
 'positive_alias_cannot_prove_loop': (source('val x<-make local i64;for(i in -1..0){x<-i;};x;',ADD,'+','(-)'),2,'','before definite initialization'),
 'signed_minimum_requires_negation': (source('-9223372036854775808;',ADD,'+','(-)'),2,'','integer literal is outside i64'),
 'wrong_arity': (source('calculate(1,2,3);'),2,'','builtin operator argument count'),
 'unsigned_unary_minus': (source('calculate(1u64);',NEG,'-'),2,'','numeric unary operation requires'),
 'inexact_common_type': (source('calculate(1,2u64);'),2,'','no lossless common type'),
 'selected_body_no_fallback': (source('calculate(1,2);','func (+)(a:T,b:T)->const T where T{return !a;}'),2,'','logical negation requires bool'),
}
for name, operation, left, right, diagnostic in (
    ('overflow','+','9223372036854775807','1','i64 addition overflow'),
    ('underflow','-','0u8','1u8','u8 subtraction overflow'),
    ('multiply','*','18446744073709551615u64','2u64','u64 multiplication overflow'),
    ('divide_zero','/','1','0','division by zero'),
    ('divide_overflow','/','-9223372036854775808','-1','i64 division overflow'),
):
    method='func ('+operation+')(a:bool,b:bool)->bool{return a;}'
    for deferred in (False,True):
        body='{{defer{'+OUT('9')+'};'+('defer ' if deferred else '')+'calculate('+left+','+right+');}};'
        runner.CASES[name+('_deferred' if deferred else '')]=(source(body,method,operation),70,'9\n',diagnostic)
runner.CASES['remainder_minimum']=(source(OUT('calculate(-9223372036854775808,-1)'), 'func (%)(a:bool,b:bool)->bool{return a;}','%'),0,'0\n','')
runner.CASES['borrowed_method_observes_current_target']=(source('val x<-1;'+OUT('calculate(x,{x<-9;2})'),'func (+)(ref a:i64 in O,b:i64)->i64 where {owner O}{return intrinsic (+)(a,b);}'),0,'11\n','')
runner.CASES['declared_builtin_failure']=(source('calculate(1,2);','func (+)(a:i64,b:i64)->i64 throws NullPointerException;'),2,'','requires a covering catch')
runner.CASES['declared_builtin_caught']=(source('try{'+OUT('calculate(1,2)')+'}catch(e:const NullPointerException){};','func (+)(a:i64,b:i64)->i64 throws NullPointerException;'),0,'3\n','')
runner.CASES['declaration_result_preserved']=(source('calculate(1,2);','func (+)(a:i64,b:i64)->bool;'),2,'','result')
runner.CASES['reexport_extension'] = ({'ops.cod':ADD+'export (+);',
 'middle.cod':'import "./ops.cod" (+) as combine;func combine(a:i64,b:i64)->i64{return intrinsic (+)(a,b)*2;}export combine;',
 'main.cod':IO+'using "./middle.cod" combine as calculate;'+OUT('calculate(1,2)')+OUT('calculate(1u64,2u64)')+OUT('calculate(false,true)')},0,'6\n3\ntrue\n','')
for order in ('builtin_first','method_first'):
    calls='run(1,2);run(false,true);' if order=='builtin_first' else 'run(false,true);run(1,2);'
    body='proc run(a:T,b:T)->void where T{ {{defer calculate(a,b);}};return;}'+calls
    method='proc (+)(a:bool,b:bool)->void{'+OUT('a')+OUT('b')+'return;}'
    runner.CASES['deferred_context_'+order]=(source(body,method),0,'false\ntrue\n','')
runner.CASES['type_query_lexical_shadow']=(source('proc run(calculate:func(i64,i64)->bool)->void{val result<-make local typeof(calculate(1,2)){true};'+OUT('result')+'return;}func other(a:i64,b:i64)->bool{return true;}run(other);'),0,'true\n','[-Wshadow]')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
