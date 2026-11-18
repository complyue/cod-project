#!/usr/bin/env python3
"""Builtin operands propagate completion without inventing a payload type."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
OBS = IO+'proc observe(x:i64)->void{behave @print(x);}'
EX = 'using "std:exceptions" MissingReturnError;func missing()->i64{}func error()->const MissingReturnError{try{missing();}catch(e:const MissingReturnError){return e;};}'
NPE = 'using "std:exceptions" NullPointerException;'
runner.CASES = {}
for spelling in ('operator', 'call', 'intrinsic'):
    def expression(op, *args):
        if spelling == 'operator':
            return '('+op+args[0]+')' if len(args)==1 else '('+args[0]+op+args[1]+')'
        return ('intrinsic ' if spelling == 'intrinsic' else '')+'('+op+')('+','.join(args)+')'
    def run(body, result='i64'):
        return OBS+'proc run()->'+result+'{defer observe(1);'+body+';return 0;}behave @print(run());'
    for op, label in [('+','add'),('-','subtract'),('*','multiply'),('/','divide'),('%','remainder'),('==','equal'),('!=','unequal'),('<','less'),('>','greater'),('<=','less_equal'),('>=','greater_equal')]:
        runner.CASES[spelling+'_'+label+'_first'] = (run(expression(op,'{return 7;}','{observe(99);2}')),0,'1\n7\n','')
        runner.CASES[spelling+'_'+label+'_second'] = (run(expression(op,'{observe(2);1}','{return 7;}')),0,'2\n1\n7\n','')
    for op, label in [('+','positive'),('-','negative'),('!','not')]:
        if spelling == 'operator' and op == '+': continue
        runner.CASES[spelling+'_'+label] = (run(expression(op,'{return 7;}')),0,'1\n7\n','')
    cases = {
        'both_return': (run(expression('+','{return 7;}','{return 9;}')),0,'1\n7\n',''),
        'bool_equality': (run(expression('==','false','{return 7;}')),0,'1\n7\n',''),
        'float_operand': (run(expression('*','1.5f32','{return 7;}')),0,'1\n7\n',''),
        'finally_replacement': (run(expression('+','1','try{return 7;}finally{return 9;}')),0,'1\n9\n',''),
        'nested_completion': (run(expression('*','1',expression('+','2','{return 7;}'))),0,'1\n7\n',''),
        'normal_branch_remains': (IO+'func run(flag:bool)->i64{return '+expression('+','1','if(flag){return 7;}else{8}')+';}behave @print(run(true));behave @print(run(false));',0,'7\n9\n',''),
        'inferred_return': (IO+'func run(){'+expression('+','1','{return 7;}')+';}behave @print(run());',0,'7\n',''),
        'cleanup_value_snapshot': (IO+'func run()->i64{val x<-7;'+expression('+','1','{defer{x<-9;};return x;}')+';return 0;}behave @print(run());',0,'7\n',''),
        'throws_before_operation': (OBS+EX+'proc run()->void{defer observe(1);'+expression('/','1','{throw error();}')+';observe(99);}try{run();}catch(e:const MissingReturnError){observe(7);};',0,'1\n7\n','[-Wmissing-return]'),
        'void_return': (OBS+'proc run()->void{defer observe(1);'+expression('+','1','{return;}')+';observe(99);}run();',0,'1\n',''),
        'early_null_check': (OBS+NPE+'proc run()->i64 throws NullPointerException{val a<-1;ptr p=a;p=null;'+expression('+','p','{observe(99);return 7;}')+';return 0;}try{run();{}}catch(e:const NullPointerException){observe(8);};',0,'8\n',''),
        'later_null_not_dereferenced': (OBS+NPE+'proc run()->i64{val a<-1;ptr p=a;p=null;'+expression('+','{return 7;}','p')+';return 0;}behave @print(run());',0,'7\n',''),
        'early_read_required': ('func run()->i64{val x<-make local i64;'+expression('+','x','{return 7;}')+';return 0;}run();',2,'','before definite initialization'),
        'later_name_checked': ('func run()->i64{'+expression('+','{return 7;}','absent')+';return 0;}run();',2,'','unknown scalar value'),
        'known_bool_rejected': ('func run()->i64{'+expression('+','{return 7;}','false')+';return 0;}run();',2,'','no applicable numeric operation'),
        'known_void_rejected': ('func run()->i64{'+expression('+','{return 7;}','{}')+';return 0;}run();',2,'','non-value'),
        'float_remainder_rejected': ('func run()->i64{'+expression('%','1f32','{return 7;}')+';return 0;}run();',2,'','remainder requires integer operands'),
    }
    for name, case in cases.items(): runner.CASES[spelling+'_'+name] = case
runner.CASES.update({
    'deferred_preparation_returns': (OBS+'proc run()->i64{defer observe(1);defer intrinsic (+)({observe(2);1},{return 7;});return 0;}behave @print(run());',0,'2\n1\n7\n',''),
    'deferred_block_returns': (OBS+'proc run()->i64{defer observe(1);defer{1+{return 7;};};return 0;}behave @print(run());',0,'1\n7\n',''),
    'intrinsic_bypasses_overloads': (IO+'func (+)(x:T,y:T)->const T where T{return x;}func run()->i64{intrinsic (+)({return 7;},{return 9;});return 0;}behave @print(run());',0,'7\n',''),
    'unknown_generic_selection': ('func (+)(x:T,y:T)->const T where T{return x;}func run()->i64{({return 7;})+({return 9;});return 0;}run();',2,'','parameter type'),
    'wrong_arity': ('func run()->i64{intrinsic (+)({return 7;},1,2);return 0;}run();',2,'','argument count'),
    'wrong_not_arity': ('func run()->i64{intrinsic (!)({return 7;},false);return 0;}run();',2,'','argument count'),
})
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
