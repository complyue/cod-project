#!/usr/bin/env python3
"""Both observed reference outcomes preserve snapshots and consistent write proofs."""
from itertools import product
from pathlib import Path
import foundation_tests as runner

IO='using symbol "std:io" @print;using "std:exceptions" NullPointerException;'
LOCAL='val a<-make local u64,b<-make local u64,c<-make local u64,result<-make local u64;ptr p=if(flag){a}else{b};ptr q=if(other){a}else{b};'
CALLS='run(false,false);run(false,true);run(true,false);run(true,true);'
SAME='func same(ptr x:const u64 in O,ptr y:const u64 in O)->bool where {owner O}{return x is y;}'
FIRST='if(p is q){result<-7u64;{}}else{};'
SECOND='if(p is not q){result<-8u64;{}}else{};'
READ='behave @print(result);'
runner.CASES={}


def case(name,body=FIRST+SECOND+READ,setup='',definitions='',reject=False,output='7\n8\n8\n7\n',local=LOCAL,diagnostic='initialization'):
    source=IO+definitions+'proc run(flag:bool,other:bool)->void{'+local+'try{ '+setup+body+'}catch(e:const NullPointerException){behave @print(99u64);};return;}'+CALLS
    runner.CASES[name]=(source,2 if reject else 0,'' if reject else output,diagnostic if reject else '')


case('complementary')
case('reverse',SECOND+FIRST+READ)
case('negated_equal',FIRST+SECOND.replace('p is not q','!(p is q)')+READ)
case('negated_difference',FIRST.replace('p is q','!(p is not q)')+SECOND+READ)
case('explicit_equal',FIRST.replace('p is q','(is)(p,q)')+SECOND+READ)
case('intrinsic_equal',FIRST.replace('p is q','intrinsic (is)(p,q)')+SECOND+READ)
case('saved_observations',FIRST.replace('p is q','equal')+SECOND.replace('p is not q','different')+READ,setup='val equal<-p is q,different<-p is not q;')
case('copied_guard',FIRST.replace('p is q','equal')+'equal<-false;'+SECOND.replace('p is not q','!saved')+READ,setup='val equal<-p is q,saved<-equal;')
case('changed_guard',FIRST.replace('p is q','equal')+'equal<-true;'+SECOND.replace('p is not q','!equal')+READ,setup='val equal<-p is q;',reject=True)
case('changed_selection',FIRST+'q=p;'+SECOND+READ,reject=True)
case('saved_selection',FIRST+'q=c;'+SECOND.replace('p is not q','p is not saved')+READ,setup='ptr saved=q;')
case('changed_saved_selection',FIRST+'saved=p;'+SECOND.replace('p is not q','p is not saved')+READ,setup='ptr saved=q;',reject=True)
case('weak_slot_change',FIRST+'slot=p;'+SECOND+READ,setup='var slot->if(flag){p}else{q};',reject=True)
case('read_before_write',READ+FIRST+SECOND,reject=True)
case('missing_first',SECOND+READ,reject=True)
case('missing_second',FIRST+READ,reject=True)
case('same_twice',FIRST+FIRST+READ,reject=True)
case('nullable_inputs',local=LOCAL.replace('ptr p=if(flag){a}else{b};ptr q=if(other){a}else{b};','ptr p=if(flag){a}else{null in here};ptr q=if(other){a}else{null in here};'))
case('one_action','{{defer{'+FIRST+SECOND+'};}};'+READ)
case('separate_actions','{{defer{'+SECOND+'};defer{'+FIRST+'};}};'+READ)
case('changed_action','{{defer{'+SECOND+'};defer{'+FIRST+'q=p;};}};'+READ,reject=True)
case('nested_action','{{defer{defer{'+FIRST+SECOND+'};};}};'+READ)
case('finally','try{'+FIRST+'}finally{'+SECOND+'};'+READ)
case('loop_once','for(i in 0u64..1u64){'+FIRST+SECOND+'};'+READ)
case('loop_zero','for(i in 0u64..0u64){'+FIRST+SECOND+'};'+READ,reject=True)
case('failed_registration','try{ {{defer{'+FIRST+SECOND+'};}}; }catch(failure){};'+READ,reject=True)
FAIL='func fail()->u64 throws NullPointerException{ptr absent:u64=null in here;return absent;}'
case('failed_first','try{'+FIRST.replace('7u64','fail()')+'}catch(failure:const NullPointerException){};'+SECOND+READ,definitions=FAIL,reject=True)
case('readonly_target',FIRST+SECOND+READ,local=LOCAL.replace('result<-make local u64','result:const u64<-make local u64'),reject=True,diagnostic='readonly')

for name,setup,guard in (
    ('self','','p is not p'),
    ('direct','','p is not q&&p is q'),
    ('reverse','','p is q&&q is not p'),
    ('saved','val different<-p is not q;','different&&p is q'),
    ('transitive','ptr third=if(flag){a}else{b};','p is not q&&p is third&&third is q'),
    ('transitive_saved','ptr third=if(flag){a}else{b};val different<-p is not q;val equal<-p is third;','different&&equal&&third is q'),
    ('changed','val different<-p is not q;q=p;','different&&p is q'),
    ('unrelated','','p is not q'),
):
    case('contradiction_'+name,'if('+guard+'){'+READ+'}else{behave @print(0u64);};',setup=setup,reject=name in ('changed','unrelated'),output='0\n'*4)

for name,definition,first,second,setup,reject in (
    ('helper',SAME,'same(p,q)','!same(p,q)','',False),
    ('generic',SAME.replace('u64','T').replace('where {owner O}','where {T,owner O}'),'same(p,q)','!same(p,q)','',False),
    ('recursive',SAME.replace(')->bool',',n:u64)->bool').replace('return x is y;','if(n==0u64){return x is y;}else{return same(x,y,n-1u64);};'),'same(p,q,8u64)','!same(p,q,8u64)','',False),
    ('callable',SAME,'compare(p,q)','!compare(p,q)','func compare=same;',False),
    ('captured',SAME,'compare(p,q)','!compare(p,q)','func compare[same](ptr x:const u64 in O,ptr y:const u64 in O)->bool where {owner O}{return same(x,y);};',False),
    ('unknown',SAME+'func unknown_compare(ptr x:const u64 in O,ptr y:const u64 in O)->bool where {owner O}{return true;}','same(p,q)','!unknown_compare(p,q)','',True),
    ('rebound',SAME.replace('return x is y;','x=y;return x is y;'),'p is q','!same(p,q)','',True),
    ('finally_saved',SAME.replace('return x is y;','try{return x is y;}finally{x=y;{}};'),'p is q','!same(p,q)','',False),
    ('finally_replaced',SAME.replace('return x is y;','try{return x is y;}finally{return true;}'),'p is q','!same(p,q)','',True),
    ('deferred',SAME.replace('return x is y;','defer{return x is y;};'),'p is q','!same(p,q)','',False),
):
    case(name,FIRST.replace('p is q',first)+SECOND.replace('p is not q',second)+READ,setup=setup,definitions=definition,reject=reject)

SET='proc set_equal(ptr output:u64 in O,ptr left:const u64 in O,ptr right:const u64 in O)->void throws NullPointerException where {owner O}{if(left is right){output<-7u64;{}}else{};return;}'
DIFFERENT=SET.replace('set_equal','set_different').replace('left is right','left is not right').replace('7u64','8u64')
case('borrowed_output','fill(result,p,q);'+READ,definitions=SET.replace('set_equal','fill').replace('return;}','if(left is not right){output<-8u64;{}}else{};return;}'))
case('complementary_calls','set_equal(result,p,q);set_different(result,p,q);'+READ,definitions=SET+DIFFERENT)
case('changed_call_selection','set_equal(result,p,q);q=p;set_different(result,p,q);'+READ,definitions=SET+DIFFERENT,reject=True)
case('prepared_calls','{{defer set_equal(result,p,q);defer set_different(result,p,q);}};'+READ,definitions=SET+DIFFERENT)
case('prepared_saved_selections','{{defer set_equal(result,p,q);defer set_different(result,p,q);q=p;}};'+READ,definitions=SET+DIFFERENT)
case('ref_entry_promise','skip(result);',definitions='proc skip(ref required:u64 in O)->void where {owner O}{if(required is not required){behave @print(required);}else{};}',reject=True)

# Enumerate reference values independently of the checker's condition relation.
# Every mapping to three physical cells is executed by each accepted program.
guards={
 'eq':('p is q',lambda p,q,r:p==q),
 'ne':('p is not q',lambda p,q,r:p!=q),
 'chain':('p is q&&q is r',lambda p,q,r:p==q and q==r),
 'not_chain':('p is not q||q is not r',lambda p,q,r:p!=q or q!=r),
 'other':('q is not r||p is r',lambda p,q,r:q!=r or p==r),
 'different':('p is not q&&q is r',lambda p,q,r:p!=q and q==r),
 'not_different':('p is q||q is not r',lambda p,q,r:p==q or q!=r),
 'axiom':('p is q||p is not r||q is not r',lambda p,q,r:p==q or p!=r or q!=r),
 'false':('false',lambda p,q,r:False),
}
for left,right in [('eq','ne'),('chain','not_chain'),('ne','other'),('different','not_different'),('axiom','false'),('eq','eq'),('chain','ne'),('different','eq')]:
    first,fn=guards[left];second,gn=guards[right]
    values=[8 if gn(*args) else 7 if fn(*args) else None for args in product(range(3),repeat=3)]
    local='val a<-make local u64,b<-make local u64,c<-make local u64,result<-make local u64;'
    local+=''.join('ptr '+name+'=if('+index+'==0u64){a}else{if('+index+'==1u64){b}else{c}};' for name,index in [('p','first'),('q','second'),('r','third')])
    source=IO+'proc model(first:u64,second:u64,third:u64)->void{'+local+FIRST.replace('p is q',first)+SECOND.replace('p is not q',second)+READ+'return;}'
    source+=''.join('model('+','.join(str(n)+'u64' for n in args)+');' for args in product(range(3),repeat=3))
    reject=None in values
    runner.CASES['model_'+left+'_'+right]=(source,2 if reject else 0,'' if reject else ''.join(str(v)+'\n' for v in values),'initialization' if reject else '')

runner.CASE_ARGUMENTS={}
runner.CASE_SHADOWS={name:[] for name in runner.CASES}
runner.EXTRA_SOURCES=[Path(__file__)]

from language_test_support import expect_source_rejections
expect_source_rejections(runner, (
    'nested_action',
), 'currently draining scope')

if __name__=='__main__':
    raise SystemExit(runner.main())
