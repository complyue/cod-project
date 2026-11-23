#!/usr/bin/env python3
"""Reference identity observes selected owners and targets without reading payloads."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
OUT = lambda x: 'behave @print('+x+');'
PAIR = 'func same(ptr x:T in A,ptr y:U in B)->bool where {T,U,owner A,owner B}{return x is y;}'
BASE = 'val x<-1,y<-1;ptr p=x,q=x,r=y;'
runner.CASES = {
 'targets_not_payloads': (IO+BASE+OUT('p is q')+OUT('p is r')+OUT('x is p')+OUT('x is y')+OUT('p is not r'),0,'true\nfalse\ntrue\nfalse\ntrue\n',''),
 'null_forms': (IO+'ptr p:i64=null in here;'+OUT('p is null')+OUT('null is p')+OUT('p is not null')+OUT('null in here is null')+OUT('null is null in here'),0,'true\ntrue\nfalse\ntrue\ntrue\n',''),
 'null_owner_distinction': (IO+'ptr p:i64=null in here;{{ptr q:i64=null in here;'+OUT('p is q')+OUT('p is null')+OUT('p is null in here')+OUT('q is null in here')+'}};',0,'false\ntrue\nfalse\ntrue\n',''),
 'uninitialized_target': (IO+'val x<-make local i64,y<-make local i64;ptr p=x;'+OUT('p is x')+OUT('x is y')+OUT('p is null'),0,'true\nfalse\nfalse\n',''),
 'readonly_target': (IO+BASE+'const ptr s:const i64=p;'+OUT('s is p')+OUT('s is not q'),0,'true\nfalse\n',''),
 'different_types': (IO+'val x<-1,y<-true;'+OUT('x is y'),0,'false\n',''),
 'call_same_and_distinct_owner': (IO+PAIR+'ptr a:i64=null in here,b:bool=null in here;'+OUT('same(a,b)')+'{{ptr c:i64=null in here;'+OUT('same(a,c)')+OUT('same(c,c)')+'}};',0,'true\nfalse\ntrue\n',''),
 'call_selected_targets': (IO+PAIR+BASE+OUT('same(p,q)')+OUT('same(p,r)'),0,'true\nfalse\n',''),
 'forwarded_owners': (IO+PAIR+'func forward(ptr x:T in O,ptr y:T in P)->bool where {T,owner O,owner P}{return same(x,y);}ptr a:i64=null in here;'+OUT('forward(a,a)')+'{{ptr b:i64=null in here;'+OUT('forward(a,b)')+'}};',0,'true\nfalse\n',''),
 'recursive_owner_activation': (IO+'func recurse(ptr parent:i64 in O,n:i64)->bool where {owner O}{ptr local:i64=null in here;if(n==0){return parent is local;}else{return recurse(local,n-1);};}ptr p:i64=null in here;'+OUT('recurse(p,3)'),0,'false\n',''),
 'recursive_forwarding': (IO+'func recurse(ptr x:i64 in O,ptr y:i64 in P,n:i64)->bool where {owner O,owner P}{if(n==0){return x is y;}else{return recurse(x,y,n-1);};}ptr p:i64=null in here;'+OUT('recurse(p,p,3)')+'{{ptr q:i64=null in here;'+OUT('recurse(p,q,3)')+'}};',0,'true\nfalse\n',''),
 'callable_owner_transport': (IO+'func same(ptr x:i64 in A,ptr y:i64 in B)->bool where {owner A,owner B}{return x is y;}func f=same;ptr p:i64=null in here;'+OUT('f(p,p)')+'{{ptr q:i64=null in here;'+OUT('f(p,q)')+'}};',0,'true\nfalse\n',''),
 'borrowed_result': (IO+'func pick(ptr x:i64 in O)->ptr i64 in O where {owner O}{return x;}'+BASE+OUT('pick(p) is q')+'p=null;'+OUT('pick(p) is null')+OUT('pick(p) is null in here'),0,'true\ntrue\ntrue\n',''),
 'left_selection_retained': (IO+BASE+OUT('p is {p=r;q}')+OUT('p is {p=q;r}')+OUT('p is {p=null;q}'),0,'true\ntrue\ntrue\n',''),
 'ordered_once': (IO+BASE+OUT('{'+OUT('3')+'p} is {'+OUT('4')+'q}'),0,'3\n4\ntrue\n',''),
 'different_owner_effects': (IO+'ptr p:i64=null in here;{{ptr q:i64=null in here;'+OUT('{'+OUT('3')+'p} is {'+OUT('4')+'q}')+'}};',0,'3\n4\nfalse\n',''),
 'return_operand': (IO+'func f()->i64{(is)({return 7;},null);return 0;}'+OUT('f()'),0,'7\n',''),
 'right_return_operand': (IO+'func f()->i64{val x<-make local i64;x is {return 8;};return 0;}'+OUT('f()'),0,'8\n',''),
 'explicit_intrinsic_forms': (IO+BASE+OUT('(is)(p,q)')+OUT('(is not)(p,r)')+OUT('intrinsic (is)(p,q)')+OUT('intrinsic (is not)(p,r)'),0,'true\ntrue\ntrue\ntrue\n',''),
 'prepared_identity': (PAIR+'ptr a:i64=null in here;defer (is)(a,null);defer intrinsic (is not)(a,null);defer same(a,a);',0,'',''),
 'prepared_call_owner_transport': (IO+'proc same(ptr x:i64 in O,ptr y:i64 in P)->void where {owner O,owner P}{'+OUT('x is y')+'return;}ptr a:i64=null in here;{{ptr b:i64=null in here;defer same(a,b);defer same(a,a);}};',0,'true\nfalse\n',''),
 'deferred_block': (IO+BASE+'{{defer{'+OUT('p is q')+'};p=r;}};',0,'false\n',''),
 'null_refinement': (IO+'func f(ptr p:i64 in O)->i64 where {owner O}{if(p is not null){return p;}else{return 0;};}val x<-9;ptr p=x;'+OUT('f(p)')+'p=null;'+OUT('f(p)'),0,'9\n0\n',''),
 'null_refinement_symmetric': (IO+'func f(ptr p:i64 in O)->i64 where {owner O}{if(null is not p){return p;}else{return 0;};}val x<-9;'+OUT('f(x)'),0,'9\n',''),
 'unary_null_refinement': (IO+'func f(ptr p:i64 in O)->i64 where {owner O}{if(not (p is null)){return p;}else{return 0;};}val x<-9;'+OUT('f(x)'),0,'9\n',''),
 'mutation_invalidates_refinement': (IO+BASE+'if(p is not null){p=null;'+OUT('p')+'}else{};',2,'','requires a covering catch or throws'),
 'rhs_mutation_invalidates_refinement': (IO+BASE+'if(p is not {p=null;null in here}){'+OUT('p')+'}else{};',2,'','requires a covering catch or throws'),
 'unrelated_null_does_not_refine': (IO+'ptr p:i64=null in here;{{if(p is not null in here){'+OUT('p')+'}else{};}};',2,'','requires a covering catch or throws'),
 'typeof_no_evaluation': (IO+BASE+'val b<-make local typeof({'+OUT('99')+'p} is null){true};'+OUT('b'),0,'true\n',''),
 'typeof_uninitialized': (IO+'val x<-make local i64;val b<-make local typeof(x is not null){false};'+OUT('b'),0,'false\n',''),
 'two_bare_nulls': ('null is null;',2,'','owner context'),
 'typeof_two_bare_nulls': ('val b<-make local typeof(null is null){false};',2,'','owner context'),
 'independent_value': ('1 is null;',2,'','existing target views'),
 'arithmetic_value': (BASE+'(x+0) is p;',2,'','existing target views'),
 'returned_value': ('func f()->i64{return 1;}val x<-1;f() is x;',2,'','existing target views'),
 'chained_identity': (BASE+'p is q is r;',2,'','existing target views'),
 'expired_owner': ('{{val x<-1;x}} is null;',2,'','lexical owner has ended'),
 'untyped_call_arity': ('(is)();',2,'','two reference views'),
 'fixed_intrinsic': ('func (is)(x:i64,y:i64)->bool{return true;}',2,'','cannot have user methods'),
 'old_intrinsic_removed': ('val x<-1;isnull(x);',2,'','unknown callable'),
 'bool_precedence': (IO+BASE+OUT('p is q && p is not r')+OUT('false || p is q')+OUT('not false && true')+OUT('not not true'),0,'true\ntrue\ntrue\ntrue\n',''),
 'unary_call_forms': (IO+OUT('(not)(false)')+OUT('intrinsic (not)(false)')+OUT('(!)(false)'),0,'true\ntrue\ntrue\n',''),
 'unary_method_identity': (IO+'func (not)(x:i64)->i64{return x+1;}'+OUT('not 4')+OUT('!4')+OUT('(not)(4)')+OUT('(!)(4)'),0,'5\n5\n5\n5\n',''),
 'unary_no_pointer_truthiness': (BASE+'not p;',2,'','bool'),
}
runner.CASES.update({
 'contextual_callable_owner_inputs': (IO+'func choose(ptr value:i64 in O)->ptr i64 in O where {owner O}{return value;}func choose(value:bool)->bool{return value;}func borrowed(ptr value:i64 in O)->bool where {owner O}{return value is null in O;}func copied(value:bool)->bool{return value;}func apply(fn:F,ref input:T in P)->bool where {F,T,owner P}{return fn(choose(input));}val a<-1,b<-true;'+OUT('apply(borrowed,a)')+OUT('apply(copied,b)'),0,'false\ntrue\n',''),
 'contextual_call_owner_inputs': (IO+'func choose(ptr value:i64 in O)->ptr i64 in O where {owner O}{return value;}func choose(value:bool)->bool{return value;}func observe(ptr value:i64 in O)->bool where {owner O}{return value is null in O;}func observe(value:bool)->bool{return value;}func run(ref input:T in P)->bool where {T,owner P}{return observe(choose(input));}val a<-1,b<-true;'+OUT('run(a)')+OUT('run(b)'),0,'false\ntrue\n',''),
 'contextual_borrow_or_materialized_value': (IO+'func choose(ptr value:i64 in O)->ptr i64 in O where {owner O}{return value;}func choose(value:bool)->bool{return value;}func run(ref input:T in P)->bool where {T,owner P}{ptr selected=choose(input);return selected is input;}val a<-1,b<-true;'+OUT('run(a)')+OUT('run(b)'),0,'true\nfalse\n',''),
 'local_explicit_identity': (IO+'func test(ptr input:i64 in O)->bool where {owner O}{func nested(ptr value:i64 in P)->bool where {owner P}{return (is)(value,null);};return nested(input);}ptr p:i64=null in here;'+OUT('test(p)'),0,'true\n',''),
 'captured_holder_identity': (IO+'func base()->i64{return 1;}func f=func[base]()->i64{return base();};func g=f;ptr p=f;'+OUT('p is f')+OUT('f is g'),0,'true\nfalse\n',''),
 'unary_duplicate_spelling': ('func (!)(input:i64)->i64{return input;}func (not)(input:i64)->i64{return input;}(!)(1);',2,'','duplicate method signature'),
 'imported_identity_rejected': ({'ops.cod':'func same(lhs:i64,rhs:i64)->bool{return true;}export same;','main.cod':'using "./ops.cod" same as (is);val x<-1;x is x;'},2,'','cannot be an imported user operation'),
 'quoted_operator_rejected': ('func ("!")(input:bool)->bool{return input;}',2,'','operator or intrinsic name'),
 'unary_import_alias': ({'ops.cod':'func (not)(input:i64)->i64{return input+1;}export (not);','main.cod':IO+'using "./ops.cod" (!) as (not);'+OUT('not 3')+OUT('!3')},0,'4\n4\n',''),
})
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {'call_selected_targets': ['x','y'], 'borrowed_result': ['x'], 'null_refinement': ['p']}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
