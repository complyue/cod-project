#!/usr/bin/env python3
"""Assignment spellings preserve one storage, preparation and completion contract."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
OUT = lambda value: 'behave @print('+value+');'
runner.CASES = {}
for mode in ('infix', 'call', 'intrinsic'):
    def operation(op, left, right):
        if mode == 'infix':
            return '('+left+op+right+')'
        return ('intrinsic ' if mode == 'intrinsic' else '')+'('+op+')('+left+','+right+')'
    w = lambda left, right: operation('<-', left, right)
    r = lambda left, right: operation('=', left, right)
    cases = {
        'transfer': (IO+'val x<-1;'+w('x','7')+';'+OUT('x'),0,'7\n',''),
        'widening': (IO+'val x<-1u64;'+w('x','7u8')+';'+OUT('x'),0,'7\n',''),
        'initialization': (IO+'val x<-make local i64;'+w('x','7')+';'+OUT('x'),0,'7\n',''),
        'retained_transfer_result': (IO+'val x<-1;ref result='+w('x','7')+';'+w('x','9')+';'+OUT('result'),0,'9\n',''),
        'nested_transfer': (IO+'val x<-1u64,y<-2u32;'+w('x',w('y','7u8'))+';'+OUT('x')+OUT('y'),0,'7\n7\n',''),
        'rebind': (IO+'val x<-1,y<-7;ptr p=x;'+r('p','y')+';'+OUT('p')+w('y','9')+';'+OUT('p'),0,'7\n9\n',''),
        'borrowed_callable_result': (IO+'func identity(ref input:i64 in O)->ref i64 in O where {owner O}{return input;}val a<-1,b<-7;ptr p=a;func get=identity;'+r('p','get(b)')+';'+OUT('p')+w('p','9')+';'+OUT('b'),0,'7\n9\n',''),
        'borrowed_callable_holder': (IO+'func base(x:i64)->const i64{return x;}func other(x:i64)->const i64{return x+7;}func identity(ref input:func(i64)->const i64 in O)->ref (func(i64)->const i64) in O where {owner O}{return input;}func a=base;func b=other;func get=identity;ptr p=a;'+r('p','get(b)')+';'+OUT('p(1)')+w('p','a')+';'+OUT('b(1)'),0,'8\n1\n',''),
        'retained_rebind_result': (IO+'val x<-1,y<-7;ptr p=x;ref result='+r('p','y')+';'+r('p','x')+';'+w('result','9')+';'+OUT('y')+OUT('p'),0,'9\n1\n',''),
        'nested_rebind': (IO+'val x<-1,y<-7;ptr p=x,q=x;'+r('p',r('q','y'))+';'+OUT('p')+OUT('q'),0,'7\n7\n',''),
        'clear_and_restore': (IO+'val x<-7;ptr p=x;'+r('p','null')+';'+OUT('(p is null)')+r('p','x')+';'+OUT('p'),0,'true\n7\n',''),
        'uninitialized_borrow': (IO+'val x<-make local i64;ptr p:i64=null in here;'+r('p','x')+';'+w('p','7')+';'+OUT('x'),0,'7\n',''),
        'saved_destination': (IO+'val a<-1,b<-2;ptr p=a;'+w('p','{'+r('p','b')+';7}')+';'+OUT('a')+OUT('b')+OUT('p'),0,'7\n2\n2\n',''),
        'once_in_order': (IO+'val a<-1,b<-2;ptr p=a;'+r('{'+OUT('1')+'p}','{'+OUT('2')+'b}')+';'+w('{'+OUT('3')+'p}','{'+OUT('4')+'7}')+';'+OUT('b'),0,'1\n2\n3\n4\n7\n',''),
        'selected_binding': (IO+'val a<-1,b<-2,c<-7;ptr p=a,q=b;'+r('(if(true){p}else{q})','c')+';'+OUT('p')+OUT('q'),0,'7\n2\n',''),
        'slot_alias': (IO+'val a<-1,b<-7;ptr p=a;ptr var alias->p;'+r('alias','b')+';'+OUT('p'),0,'7\n',''),
        'delayed_block': (IO+'val a<-1,b<-7;ptr p=a;{{defer{'+r('p','b')+';'+w('p','9')+';};}};'+OUT('p')+OUT('b'),0,'9\n9\n',''),
        'deferred_value_preparation': (IO+'val a<-1;{{defer behave @print('+w('a','7')+');'+w('a','9')+';}};'+OUT('a'),0,'7\n9\n',''),
        'deferred_binding_preparation': (IO+'val a<-1,b<-7;ptr p=a;{{defer behave @print('+r('p','b')+');'+r('p','a')+';'+w('b','9')+';}};'+OUT('p'),0,'7\n1\n',''),
        'recursive_generic': (IO+'func update(ref x:T in O,value:T,n:u64)->const T where {T,owner O}{if(n==0u64){'+w('x','value')+';}else{update(x,value,n-1u64);};return x;}val a<-1,b<-2u64;'+OUT('update(a,7,20u64)')+OUT('update(b,8u64,30u64)'),0,'7\n8\n',''),
        'nested_definition': (IO+'func run()->i64{val a<-1;func update(ref x:i64 in O)->i64 where {owner O}{return '+w('x','7')+';};return update(a);}'+OUT('run()'),0,'7\n',''),
        'anonymous_definition': (IO+'val a<-1;func update=func(ref x:i64 in O)->i64 where {owner O}{return '+w('x','7')+';};'+OUT('update(a)'),0,'7\n',''),
        'aborted_source': (IO+'proc run()->i64{val a<-1;defer{'+OUT('a')+'};'+w('a','{{return 7;}}')+';return 0;}'+OUT('run()'),0,'1\n7\n',''),
        'aborted_destination': (IO+'proc run()->i64{'+w('({{return 7;}})','{'+OUT('99')+'2}')+';return 0;}'+OUT('run()'),0,'7\n',''),
        'rhs_failure_keeps_destination': (IO+NPE+'val a<-1,count<-0;ptr p:i64=null in here;try{'+w('a','{'+w('count','7')+';p}')+';{}}catch(e:const NullPointerException){'+OUT('a')+OUT('count')+'};',0,'1\n7\n',''),
        'cross_owner_bits': (IO+'val a<-1;{{val b<-7;'+w('a','b')+';}};'+OUT('a'),0,'7\n',''),
        'readonly_transfer': ('const val a<-1;'+w('a','7')+';',2,'','readonly destination'),
        'transfer_result_readonly': ('val a<-1;'+w(w('a','7'),'9')+';',2,'','readonly destination'),
        'invalid_transfer_type': ('val a<-1;'+w('a','true')+';',2,'','source does not satisfy destination type'),
        'uninitialized_source': ('val a<-1,b<-make local i64;'+w('a','b')+';',2,'','before definite initialization'),
        'fresh_destination': (w('1','7')+';',2,'','assignment requires a storage destination'),
        'fixed_binding': ('val a<-1,b<-2;const ptr p=a;'+r('p','b')+';',2,'','const reference binding'),
        'value_rebind': ('val a<-1,b<-2;'+r('a','b')+';',2,'','val storage has no reference binding'),
        'fresh_rebind_source': ('val a<-1;ptr p=a;'+r('p','7')+';',2,'','fresh value without an existing target'),
        'owner_change': ('val a<-1;ptr p=a;{{val b<-2;'+r('p','b')+';}};',2,'','initializer-fixed owner'),
        'readonly_source': ('val a<-1;val b:const i64<-2;ptr p=a;'+r('p','b')+';',2,'','readonly source cannot rebind'),
        'reference_type_exact': ('val a<-1,b<-2u64;ptr p=a;'+r('p','b')+';',2,'','exact target type'),
        'query_boundary': ('val a<-1;val result<-make local typeof('+w('a','7')+');',2,'','typeof for this expression form'),
    }
    for name, case in cases.items(): runner.CASES[mode+'_'+name] = case
for prefix in ('', 'intrinsic '):
    for op in ('=', '<-'):
        for arguments in ('', 'a', 'a,a,a'):
            runner.CASES[prefix+op+'_arity_'+arguments] = ('val a<-1;'+prefix+'('+op+')('+arguments+');',2,'','assignment requires a destination and a source')
        runner.CASES[prefix+op+'_deferred'] = ('val a<-1,b<-2;ptr p=a;defer '+prefix+'('+op+')(p,b);',0,'','')
runner.CASES['imported_body'] = ({'helper.cod':'func update(ref x:i64 in O)->i64 where {owner O}{return intrinsic (<-)(x,7);}export update;', 'main.cod':IO+'using "helper.cod" update;val a<-1;'+OUT('update(a)')},0,'7\n','')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
