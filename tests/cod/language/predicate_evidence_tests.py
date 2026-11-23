#!/usr/bin/env python3
"""Boolean expression evidence preserves null proofs and mutation boundaries."""
from pathlib import Path
import foundation_tests as runner

PRE = 'using symbol "std:io" @print;using "std:exceptions" NullPointerException;'
NPE = 'requires a covering catch'
OUT = lambda value: 'behave @print(' + value + ');'
CALLS = 'val x<-7,y<-11;ptr absent:i64=null in here;' + ''.join(
    OUT('choose(' + args + ')') for args in ('x,y', 'absent,y', 'x,absent', 'absent,absent'))


def source(condition, yes='p+q', no='0', prefix='', setup=''):
    return PRE + prefix + 'func choose(ptr p:i64 in O,ptr q:i64 in O)->i64 where {owner O}{' + setup + \
        'if(' + condition + '){return ' + yes + ';}else{return ' + no + ';};}' + CALLS


runner.CASES = {
    'plain_block': (source('{p is not null}', 'p'), 0, '7\n0\n7\n0\n', ''),
    'full_scope': (source('{{p is not null}}', 'p'), 0, '7\n0\n7\n0\n', ''),
    'negated_disjunction': (source('!((p is null)||(q is null))'), 0, '18\n0\n0\n0\n', ''),
    'negated_conjunction_false_path': (source('!((p is not null)&&(q is not null))', '0', 'p+q'), 0, '18\n0\n0\n0\n', ''),
    'double_negation': (source('! !((p is not null)&&(q is not null))'), 0, '18\n0\n0\n0\n', ''),
    'nested_blocks': (source('!{{{(p is null)||{{q is null}}}}}'), 0, '18\n0\n0\n0\n', ''),
    'short_circuit_guarded_read': (source('!((p is null)||(q is null)) && {p+q>0}'), 0, '18\n0\n0\n0\n', ''),
    'same_predicate_alternatives': (source('if(q is null){p is not null}else{!(p is null)}', 'p'), 0, '7\n0\n7\n0\n', ''),
    'different_predicate_alternatives': (source('if(q is null){p is not null}else{q is not null}', 'p'), 2, '', NPE),
    'unknown_alternative': (source('if(q is null){p is not null}else{true}', 'p'), 2, '', NPE),
    'abrupt_alternative': (source('if(q is null){return 0;}else{p is not null}', 'p'), 0, '7\n0\n0\n0\n', ''),
    'right_operand_returns': (source('!((p is null)||{return 0;})', '0', '0'), 0, '0\n0\n0\n0\n', ''),
    'try_finally': (source('try{!((p is null)||(q is null))}finally{}'), 0, '18\n0\n0\n0\n', ''),
    'caught_boolean_alternatives': (source('try{q;p is not null}catch(e:const NullPointerException){!(p is null)}', 'p'), 0, '7\n0\n7\n0\n', ''),
    'caught_unknown_alternative': (source('try{q;p is not null}catch(e:const NullPointerException){true}', 'p'), 2, '', NPE),
    'guard_does_not_prove_both': (source('!((p is null)&&(q is null))'), 2, '', NPE),
    'rebind_before_test': (source('{p=q;!(p is null)}', 'p'), 0, '11\n11\n0\n0\n', ''),
    'rebind_after_test': (source('!((p is null)||{p=null;false})', 'p'), 2, '', NPE),
    'alias_rebind_after_test': (source('!((p is null)||{alias=null;false})', 'p', setup='var alias->p;'), 2, '', NPE),
    'swap_after_test': (source('!((p is null)||{p<=>q;false})', 'p'), 2, '', NPE),
    'caller_slot_method_after_test': (source('!((p is null)||{p<=>q;false})', 'p', prefix='func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void where {owner O}{a=null:void;}'), 2, '', NPE),
    'payload_write_preserves_test': (source('{p is not null} && {p<-9;true}', 'p'), 0, '9\n0\n9\n0\n', ''),
    'full_scope_cleanup_rebind': (source('{{defer{p=null;{}};p is not null}}', 'p'), 2, '', NPE),
    'finally_rebind': (source('try{p is not null}finally{p=null;{}}', 'p'), 2, '', NPE),
    'cleanup_preserves_unmutated_test': (source('{{defer{q=null;{}};p is not null}}', 'p'), 0, '7\n0\n7\n0\n', ''),
    'finally_preserves_unmutated_test': (source('try{p is not null}finally{q=null;{}}', 'p'), 0, '7\n0\n7\n0\n', ''),
    'unreachable_catch_preserves_test': (source('try{p is not null}catch(e:const NullPointerException){p=null;true}', 'p'), 0, '7\n0\n7\n0\n', ''),
    'caught_rebind_invalidates_test': (source('try{q;p is not null}catch(e:const NullPointerException){p=null;true}', 'p'), 2, '', NPE),
    'uninitialized_target': (PRE + 'val x<-make local i64;ptr p=x;if({p is not null}){' + OUT('p') + '}else{};', 2, '', 'before definite initialization'),
    'expired_owner': (PRE + 'ptr p={{val x<-7;x}};if({p is not null}){' + OUT('p') + '}else{};', 2, '', 'owner has ended'),
    'stored_boolean': (source('saved', 'p', setup='val saved<-p is not null;'), 0, '7\n0\n7\n0\n', ''),
    'callee_boolean_preserves_relation': (source('test(p)', 'p', prefix='func test(ptr a:i64 in O)->bool where {owner O}{return a is not null;}'), 0, '7\n0\n7\n0\n', ''),
    'selected_method_has_no_builtin_proof': (source('!((p is null)||(q is null))', prefix='func (!)(a:bool)->bool{return a;}'), 2, '', NPE),
    'deferred_block_guard': (PRE + 'proc choose(ptr p:i64 in O,ptr q:i64 in O)->void where {owner O}{defer{if(!((p is null)||(q is null))){' + OUT('p+q') + '}else{' + OUT('0') + '};};return;}val x<-7,y<-11;ptr absent:i64=null in here;choose(x,y);choose(absent,y);', 0, '18\n0\n', ''),
    'loop_guard': (PRE + 'proc choose(ptr p:i64 in O,ptr q:i64 in O)->void where {owner O}{for(i in 0..2){if(!((p is null)||(q is null))){' + OUT('p+q') + '}else{' + OUT('0') + '};p=null;};return;}val x<-7,y<-11;choose(x,y);', 0, '18\n0\n', ''),
    'readonly_target_stays_readonly': (PRE + 'func choose(ptr p:const i64 in O)->void where {owner O}{if({p is not null}){p<-9;}else{};}val x<-7;choose(x);', 2, '', 'readonly destination'),
}

for name, alias in (('named_builtin_alias', 'negate'), ('operator_builtin_alias', '(+)')):
    runner.CASES[name] = ({
        'ops.cod': 'func (!)(a:i64)->bool{return a==0;}export (!);',
        'main.cod': 'using "./ops.cod" (!) as ' + alias + ';' + source(alias + '((p is null)||(q is null))', 'p'),
    }, 0, '7\n0\n0\n0\n', '')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
