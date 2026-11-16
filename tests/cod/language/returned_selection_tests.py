#!/usr/bin/env python3
"""Selected-reference identity carries completed writes through calls and copies."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;using "std:exceptions" NullPointerException;'
RELAY = 'func relay(ptr p:u64 in O)->ptr u64 in O where {owner O}{return p;}'
PICK = 'func pick(ptr p:u64 in O,ptr q:u64 in O,choose:bool)->ptr u64 in O where {owner O}{if(choose){return p;}else{return q;}}'
SETTER = 'proc set(ptr p:u64 in O,value:u64)->void throws NullPointerException where {owner O}{p<-value;}'
LOCAL = 'val a<-make local u64;val b<-make local u64;val c<-make local u64;ptr selected=if(flag){a}else{b};'
CALLS = 'run(false,false);run(false,true);run(true,false);run(true,true);'
runner.CASES = {}


def case(name, setup='', write='relay(selected)<-7u64;', after='', read='selected', declaration=RELAY,
         reject=False, diagnostic='initialization', local=LOCAL):
    source=IO+declaration+'proc run(flag:bool,other:bool)->void{'+local+'try{ '+setup+write+after+'behave @print('+read+');}catch(e:const NullPointerException){};}'+CALLS
    runner.CASES[name]=(source,2 if reject else 0,'' if reject else '7\n'*4,diagnostic if reject else '')


case('direct', write='selected<-7u64;')
case('returned_write')
case('saved_result', setup='ptr saved=relay(selected);', write='saved<-7u64;')
case('copy_chain', setup='ptr first=relay(selected);ptr saved=first;', write='saved<-7u64;')
case('ordinary_copy', setup='ptr saved=selected;', write='saved<-7u64;')
case('generic', declaration=RELAY.replace('u64','T').replace('where {owner O}','where {T,owner O}'))
case('recursive', declaration=RELAY.replace('in O)', 'in O,n:u64)').replace('return p;', 'if(n>0u64){return relay(p,n-1u64);}else{return p;}'), write='relay(selected,8u64)<-7u64;')
case('mutual', declaration=RELAY.replace('in O)', 'in O,n:u64)').replace('return p;', 'if(n>0u64){return next(p,n-1u64);}else{return p;}')+RELAY.replace('relay','next').replace('in O)', 'in O,n:u64)').replace('return p;', 'return relay(p,n);'), write='relay(selected,8u64)<-7u64;')
case('callable', setup='func forward=relay;', write='forward(selected)<-7u64;')
case('captured_callable', setup='func forward[relay](ptr p:u64 in O)->ptr u64 in O where {owner O}{return relay(p);};func called=forward;', write='called(selected)<-7u64;')
case('same_actual_arguments', declaration=PICK, write='pick(selected,selected,other)<-7u64;')
case('ordered_arguments_keep_snapshot', declaration=PICK.replace('if(choose){return p;}else{return q;}', 'return p;'), setup='ptr old=selected;', write='pick(selected,(selected=c),other)<-7u64;', read='old')
case('ordered_arguments_do_not_write_rebound_source', declaration=PICK.replace('if(choose){return p;}else{return q;}', 'return p;'), write='pick(selected,(selected=c),other)<-7u64;', reject=True)
case('different_actual_arguments', declaration=PICK, write='pick(selected,c,other)<-7u64;', reject=True)
case('independent_same_bounds', declaration=PICK, setup='ptr separate=if(other){a}else{b};', write='pick(selected,separate,other)<-7u64;', reject=True)
case('all_candidates_not_initialized', read='b', reject=True)
case('read_before_write', setup='behave @print(selected);', reject=True)
case('source_rebound_saved_target', setup='ptr old=selected;ptr saved=relay(selected);selected=c;', write='saved<-7u64;', read='old')
case('source_rebound_new_target', setup='ptr saved=relay(selected);selected=c;', write='saved<-7u64;', reject=True)
case('result_rebound', setup='ptr saved=relay(selected);saved=c;', write='saved<-7u64;', reject=True)
case('result_slot_rebound', setup='ptr saved=relay(selected);var writer->saved;writer=c;', write='saved<-7u64;', reject=True)
case('branch_copies', setup='ptr saved=c;if(other){saved=relay(selected);{}}else{saved=relay(selected);{}};', write='saved<-7u64;')
case('branch_differs', setup='ptr saved=c;if(other){saved=relay(selected);{}}else{saved=c;{}};', write='saved<-7u64;', reject=True)
case('joined_aliases', local='val a<-make local u64;val b<-make local u64;val c<-make local u64;ptr selected=c;ptr saved=c;if(flag){selected=a;saved=selected;{}}else{selected=b;saved=selected;{}};', write='relay(saved)<-7u64;')
case('joined_independent', local='val a<-make local u64;val b<-make local u64;val c<-make local u64;ptr selected=c;ptr saved=c;if(flag){selected=a;saved=b;{}}else{selected=b;saved=a;{}};', write='relay(saved)<-7u64;', reject=True)
case('weak_slot_update', setup='ptr saved=c;ptr alternate=selected;var writer->if(other){saved}else{alternate};writer=relay(selected);', write='saved<-7u64;', reject=True)
case('callee_copy', declaration=RELAY.replace('return p;', 'ptr saved=p;p=null;return saved;'))
case('cleanup_preserves', declaration=RELAY.replace('return p;', 'try{return p;}finally{p=null;{}};'))
case('cleanup_replaces', declaration=PICK.replace('if(choose){return p;}else{return q;}', 'try{return p;}finally{return q;}'), write='pick(selected,c,other)<-7u64;', reject=True)
case('deferred_return', declaration=RELAY.replace('return p;', 'defer{return p;};'))
case('deferred_return_observes_rebound_input', declaration=PICK.replace('if(choose){return p;}else{return q;}', 'defer{return p;};defer{p=q;};return p;'), write='pick(selected,c,other)<-7u64;', reject=True)
case('deferred_write', declaration=RELAY+SETTER, write='{{defer set(relay(selected),7u64);}};')
case('deferred_block_write', write='{{defer{relay(selected)<-7u64;};}};')
case('deferred_alias_write', setup='ptr saved=relay(selected);', write='{{defer{saved<-7u64;};}};')
case('deferred_rebind_keeps_old_selection', setup='ptr saved=relay(selected);', write='{{defer{saved<-7u64;};defer{selected=c;};}};', read='saved')
case('deferred_rebind_does_not_write_new_selection', setup='ptr saved=relay(selected);', write='{{defer{saved<-7u64;};defer{selected=c;};}};', reject=True)
case('failed_registration_cannot_supply_write', declaration=RELAY+'using "std:exceptions" OutOfMemoryError;', write='try{ {{defer{relay(selected)<-7u64;};}};}catch(e:const OutOfMemoryError){};', reject=True)
case('failed_operand_cannot_supply_write', declaration=RELAY+'func fail()->u64 throws NullPointerException{ptr p:u64=null in here;return p;}', write='try{relay(selected)<-fail();{}}catch(e:const NullPointerException){};', reject=True)
case('scope_result', write='{{relay(selected)}}<-7u64;')
case('conditional_result', write='(if(other){relay(selected)}else{selected})<-7u64;')
case('finalized_result', write='(try{relay(selected)}finally{{}})<-7u64;')
case('loop_keeps_selection', setup='ptr saved=relay(selected);', write='for(i in 0u64..2u64){saved<-7u64;};')
case('loop_changes_selection', setup='ptr saved=relay(selected);', write='for(i in 0u64..2u64){saved=c;saved<-7u64;};', reject=True)
case('readonly_result', declaration=RELAY.replace('p:u64','p:const u64').replace('->ptr u64','->ptr const u64'), reject=True, diagnostic='readonly')
case('ref_entry_still_requires_initialization', setup='ref saved=relay(selected);', reject=True)
case('result_has_no_slot', write='relay(selected)=c;', reject=True, diagnostic='binding slot')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
