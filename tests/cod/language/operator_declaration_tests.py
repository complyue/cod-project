#!/usr/bin/env python3
"""Operator declarations constrain selected methods and established builtin signatures."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
ADD = 'func (+)(x:i64,y:i64)->i64;'
SWAP = 'func (<->)(x:i64 in A,y:i64 in B)->void where {owner A,owner B};'
runner.CASES = {
 'declared_recursive_operator': (IO+ADD+'func (+)(x:i64,y:i64){if(y==0){return x;}else{return (+)(intrinsic (+)(x,1),y-1);};}behave @print(2+3);',0,'5\n',''),
 'declaration_after_method': (IO+'func (+)(x:i64,y:i64){return intrinsic (+)(x,y);}'+ADD+'behave @print((+)(2,3));',0,'5\n',''),
 'generic_declaration': (IO+'func (+)(x:T,y:T)->const T where T;func (+)(x:U,y:U) where U{return y;}behave @print(1+2);behave @print(false+true);',0,'2\ntrue\n',''),
 'generic_distinct_results': (IO+'func (+)(x:u32,y:u32)->u64;func (+)(x:bool,y:bool)->bool;func (+)(x:T,y:T) where T{return x;}func tag(x:u64)->u64{return x;}behave @print(tag(1u32+2u32));behave @print(false+true);',0,'1\nfalse\n',''),
 'return_conversion_only': (IO+'func (+)(x:u32,y:u32)->u64;func (+)(x:u32,y:u32){return intrinsic (+)(x,y);}behave @print(1u32+2u32);',0,'3\n',''),
 'intermediate_overflow': ('func (+)(x:u32,y:u32)->u64;func (+)(x:u32,y:u32){return intrinsic (+)(x,y);}4294967295u32+1u32;',70,'','u32 addition overflow'),
 'procedure_declaration': (IO+'proc (+)(x:bool,y:bool)->void;proc (+)(x:bool,y:bool){behave @print(x);behave @print(y);}false+true;',0,'false\ntrue\n',''),
 'borrowed_declared_result': (IO+'func (+)(ref x:i64 in O,ref y:i64 in P)->ref i64 in O where {owner O,owner P};func (+)(ref x:i64 in A,ref y:i64 in B) where {owner A,owner B}{return x;}val x<-1,y<-2;(x+y)<-7;behave @print(x);',0,'7\n',''),
 'explicit_result_conflict': ('func (+)(x:bool,y:bool)->bool;func (+)(x:bool,y:bool)->u64{return 1u64;}',2,'','does not satisfy callable declaration'),
 'body_result_conflict': ('func (+)(x:bool,y:bool)->bool;func (+)(x:bool,y:bool){return 1u64;}',2,'','return type'),
 'kind_conflict': ('func (+)(x:bool,y:bool)->bool;proc (+)(x:bool,y:bool){return true;}',2,'','implementation kind'),
 'declaration_not_dispatch_tiebreaker': ('func (+)(x:u64,y:u64)->u64;func (+)(x:T,y:u64)->u64 where T{return y;}func (+)(x:u64,y:T)->u64 where T{return x;}1u64+2u64;',2,'','ambiguous methods'),
 'builtin_satisfies_contract': (IO+ADD+'behave @print(2+3);',0,'5\n',''),
 'builtin_keeps_result': (IO+'func (+)(x:u32,y:u32)->u64;func tag(x:u32)->u64{return 32u64;}behave @print(tag(1u32+2u32));',0,'32\n',''),
 'builtin_result_conflict': ('func (+)(x:u64,y:u64)->bool;1u64+2u64;',2,'','does not satisfy callable declaration'),
 'builtin_kind_conflict': ('proc (+)(x:u64,y:u64)->u64;1u64+2u64;',2,'','implementation kind'),
 'builtin_all_declarations': ('func (+)(x:u64,y:u64)->u64;func (+)(x:u64,y:u64)->bool;1u64+2u64;',2,'','does not satisfy callable declaration'),
 'unused_declaration': ('func (+)(x:bool,y:bool)->u64;',0,'',''),
 'unmatched_declaration': (IO+'func (+)(x:bool,y:bool)->bool;behave @print(2+3);',0,'5\n',''),
 'intrinsic_bypasses_declaration': (IO+'func (+)(x:u64,y:u64)->bool;behave @print(intrinsic (+)(1u64,2u64));',0,'3\n',''),
 'query_uses_builtin_result': (IO+'func (+)(x:u32,y:u32)->u64;val x<-make local typeof(1u32+2u32){7u32};func tag(x:u32)->u64{return 32u64;}behave @print(tag(x));',0,'32\n',''),
 'query_checks_builtin_contract': ('func (+)(x:u64,y:u64)->bool;val x<-make local typeof(1u64+2u64);',2,'','does not satisfy callable declaration'),
 'query_intrinsic_bypasses_declaration': (IO+'func (+)(x:u64,y:u64)->bool;val x<-make local typeof(intrinsic (+)(1u64,2u64)){7u64};behave @print(x);',0,'7\n',''),
 'query_checks_method_contract': (IO+ADD+'func (+)(x:i64,y:i64){return x;}val x<-make local typeof(2+3){7};behave @print(x);',0,'7\n',''),
 'query_unimplemented_declaration': (IO+'func (+)(x:bool,y:bool)->u64;val x<-make local typeof(false+true){7u64};behave @print(x);',0,'7\n',''),
 'unimplemented_declaration_is_not_body': ('func (+)(x:bool,y:bool)->u64;false+true;',2,'','no applicable numeric operation'),
 'checked_builtin_obligation': (NPE+'func (+)(x:u64,y:u64)->u64 throws NullPointerException;1u64+2u64;',2,'','requires a covering catch or throws'),
 'checked_builtin_caught': (IO+NPE+'func (+)(x:u64,y:u64)->u64 throws NullPointerException;try{behave @print(1u64+2u64);}catch(e:const NullPointerException){};',0,'3\n',''),
 'checked_query_no_invocation': (IO+NPE+'func (+)(x:u64,y:u64)->u64 throws NullPointerException;val x<-make local typeof(1u64+2u64){7u64};behave @print(x);',0,'7\n',''),
 'aborted_builtin_signature_unproved': (IO+NPE+'func (+)(x:u64,y:u64)->u64 throws NullPointerException;func f()->u64{(+)({return 7u64;1u64},2u64);return 0u64;}behave @print(f());',2,'','cannot establish a parameter type for an abruptly completing operand'),
 'aborted_method_no_invocation': (IO+NPE+'func (+)(x:u64,y:u64)->u64 throws NullPointerException;func (+)(x:u64,y:u64){return x;}func f()->u64{(+)({return 7u64;1u64},2u64);return 0u64;}behave @print(f());',0,'7\n',''),
 'deferred_builtin_checked_obligation': (NPE+'func (+)(x:u64,y:u64)->u64 throws NullPointerException;defer (+)(1u64,2u64);',2,'','requires a covering catch or throws'),
 'deferred_builtin_caught': (IO+NPE+'func (+)(x:u64,y:u64)->u64 throws NullPointerException;try{true;{{defer (+)(1u64,2u64);behave @print(7u64);}};}catch(e:const NullPointerException){};',0,'7\n',''),
 'checked_method_obligation': (NPE+'func (+)(x:bool,y:bool)->bool throws NullPointerException;func (+)(x:bool,y:bool){return true;}false+true;',2,'','requires a covering catch or throws'),
 'method_keeps_own_throws': (NPE+'func (+)(x:u64,y:u64)->u64;func (+)(x:u64,y:u64)->u64 throws NullPointerException{return x;}',2,'','checked exception outside its callable declaration'),
 'signed_minimum': (IO+'func (-)(x:i64)->i64;behave @print(-9223372036854775808);',0,'-9223372036854775808\n',''),
 'signed_minimum_contract': ('func (-)(x:i64)->bool;-9223372036854775808;',2,'','does not satisfy callable declaration'),
 'signed_minimum_query_contract': ('func (-)(x:i64)->bool;val x<-make local typeof(-9223372036854775808);',2,'','does not satisfy callable declaration'),
 'signed_minimum_throws': (NPE+'func (-)(x:i64)->i64 throws NullPointerException;-9223372036854775808;',2,'','requires a covering catch or throws'),
 'swap_method': (IO+SWAP+'func (<->)(x:i64 in P,y:i64 in Q) where {owner P,owner Q}{intrinsic (<->)(x,y);}val x<-1,y<-2;x<->y;behave @print(x);behave @print(y);',0,'2\n1\n',''),
 'swap_builtin': (IO+SWAP+'val x<-1,y<-2;x<->y;behave @print(x);behave @print(y);',0,'2\n1\n',''),
 'deferred_swap_builtin': (IO+SWAP+'val x<-1,y<-2;{{defer (<->)(x,y);x<-3;}};behave @print(x);behave @print(y);',0,'2\n3\n',''),
 'swap_checked_obligation': (NPE+SWAP.replace(' where',' throws NullPointerException where')+'val x<-1,y<-2;x<->y;',2,'','requires a covering catch or throws'),
 'deferred_swap_checked_obligation': (NPE+SWAP.replace(' where',' throws NullPointerException where')+'val x<-1,y<-2;defer (<->)(x,y);',2,'','requires a covering catch or throws'),
 'swap_intrinsic_bypasses_declaration': (IO+SWAP.replace('func','proc')+'val x<-1,y<-2;intrinsic (<->)(x,y);behave @print(x);',0,'2\n',''),
}
for op, value, answer in (('-', '2', '-2'), ('!', 'false', 'true'), ('+', '3', '3')):
    kind = 'bool' if op == '!' else 'i64'
    runner.CASES['unary_'+op] = (IO+f'func ({op})(x:{kind})->{kind};behave @print(({op})({value}));',0,answer+'\n','')
for index, (op, answer) in enumerate((('-', '3'), ('*', '10'), ('/', '2'), ('%', '1'), ('==', 'false'), ('!=', 'true'), ('<', 'false'), ('>', 'true'), ('<=', 'false'), ('>=', 'true'))):
    kind = 'bool' if op in ('==','!=','<','>','<=','>=') else 'i64'
    runner.CASES['binary_'+str(index)] = (IO+f'func ({op})(x:i64,y:i64)->{kind};behave @print(5{op}2);',0,answer+'\n','')
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
