#!/usr/bin/env python3
"""Focused native observations for the unspecialized micro-dispatch baseline."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
from dataclasses import replace
from compatibility import observations as compatibility_observations
from model import Universe, Type, ModelError, METHODS, coverage
from generate import generate
from context import observations as context_observations

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent


def command(args):
    r = subprocess.run(list(map(str,args)),capture_output=True,text=True)
    if r.returncode:
        raise RuntimeError(f'{args}\n{r.stdout}\n{r.stderr}')
    return r.stdout


def digest(p):
    return hashlib.sha256(Path(p).read_bytes()).hexdigest()


# name, plan, x-type/value, y-type/value, destination/value, mode/failure/cleanup
CASES = [
    ('zero-explicit-return', 'Example','IntegerLiteral',-32,'LocalSlot',4,'LocalSlot',2,0,0,0),
    ('integer-local', 'Example','IntegerLiteral',6,'LocalSlot',4,'LocalSlot',2,0,0,0),
    ('runtime-bool-regional', 'Example','BooleanLiteral',1,'RegionalSlot',9,'RegionalSlot',3,0,0,0),
    ('other-shape-current-view', 'Other','IntegerLiteral',8,'RegionalSlot',5,'RegionalSlot',3,1,0,0),
    ('captured-complete-value', 'Example','IntegerLiteral',6,'RegionalSlot',9,'LocalSlot',2,2,0,0),
    ('shared-owner-distinct-view', 'Other','BooleanLiteral',0,'LocalSlot',4,'LocalSlot',2,3,0,0),
    ('alias-changes-field', 'Example','IntegerLiteral',6,'RegionalSlot',9,'LocalSlot',2,4,0,0),
    ('failed-field-update-preserves-pair', 'Other','IntegerLiteral',6,'RegionalSlot',9,'LocalSlot',2,5,0,0),
    ('acquisition-failure', 'Example','IntegerLiteral',6,'LocalSlot',4,'LocalSlot',2,0,2,0),
    ('actual-verifier-failure', 'Example','IntegerLiteral',6,'LocalSlot',4,'LocalSlot',2,0,4,0),
    ('defer-failure-keeps-return', 'Example','IntegerLiteral',6,'LocalSlot',4,'LocalSlot',2,0,0,1),
    ('catch-replaces-return', 'Example','IntegerLiteral',6,'LocalSlot',4,'LocalSlot',2,0,0,2),
    ('finally-replaces-return', 'Example','IntegerLiteral',6,'LocalSlot',4,'LocalSlot',2,0,0,3),
]


def expected(case, u):
    _, plan, xt, xv, yt, yv, dt, dv, mode, failure, cleanup = case
    if mode == 4: xt, xv = 'BooleanLiteral',1
    selected = 3 if plan == 'Example' and dt == 'RegionalSlot' else 1 if plan == 'Example' else 2 if dt == 'RegionalSlot' else 0
    view,bias,increment = ((0,5,2) if mode == 2 else (1,9,1) if mode == 3 else (1,9,3) if mode == 1 else (0,5,1))
    encoded = (255 if xv else 0) + yv if xt == 'BooleanLiteral' and yt == 'RegionalSlot' else xv + yv
    value = encoded + dv + (10,20,30,40)[selected] + view*100 + bias + increment
    trace = [80,81,200,0] + ([200,1] if mode in (1,2,3) else [])
    result,error,summary = 0,0,0
    has_return=False
    trace += [501]
    if failure == 1: error = 904
    else:
        trace += [502]
        if failure == 2: error = 904
        else:
            trace += [503]
            if failure == 3: error = 904; trace += [602]
            else:
                trace += [1000+view*10+selected,1100,xv,1101,yv,220,bias,increment,1200,value,700,value,711 if failure == 4 else 710,85]
                summary = u.type_ids['AddressSummary' if dt == 'RegionalSlot' else 'ValueSummary']
                if failure == 4: error = 901
                else: result = value;has_return=True
                if cleanup: error=902;trace += [86]
                trace += [603,602]
        if error:
            trace += [90,error];error=0
            if cleanup == 2:result=777;has_return=True
        trace += [95]
        if cleanup == 3:result=888;has_return=True
        trace += [601]
    if not error and not has_return:error=903
    trace += [201,0,800,result,error]
    return (result,error,31 if xt == 'IntegerLiteral' else 32,summary),trace


def compile_native(directory, reverse=False, profile="base"):
    info=generate(directory,reverse=reverse,simplify_callsites=reverse,profile=profile)
    args=[ROOT/'build/bin/clang++','-std=c++20','-O0','-g','-UNDEBUG','-fno-inline',
          '-I'+str(directory),'-I'+str(ROOT/'include'),'-I'+str(ROOT/'llvm-project/llvm/include'),
          '-I'+str(ROOT/'build/include'),HERE/'main.cc',ROOT/'build/lib/libLLVM.so.23.1',
          '-Wl,-rpath,'+str(ROOT/'build/lib'),'-o',directory/'native']
    command(args)
    symbols=command([ROOT/'build/bin/llvm-nm','--defined-only','--demangle',directory/'native'])
    for body in info['method_bodies']:
        assert sum((' '+body+'(') in line for line in symbols.splitlines()) == 1, body
    return info,list(map(str,args))


def write_input(path, case, u, override=None):
    _, p,x,xv,y,yv,d,dv,*_=case
    path.write_text(f'{p} {x} {xv} {y} {yv} {d} {dv} {u.type_ids[x] if override is None else override}\n')


def parse(text):
    lines=text.splitlines()
    assert lines[2].startswith('REGIONS')
    return tuple(map(int,lines[0].split()[1:])),list(map(int,lines[1].split()[1:]))


def run(directory):
    u=Universe();info,args=compile_native(directory)
    observations=[]
    for case in CASES:
        name,*_=case; path=directory/(name+'.input');write_input(path,case,u)
        output=command([directory/'native',path,directory/(name+'.dbmr'),*case[-3:],directory/(name+'.ll')])
        actual=parse(output)
        assert actual == expected(case,u),(name,actual,expected(case,u))
        observations.append({'case':name,'result':actual[0],'trace':actual[1]})
        print('PASS',name,flush=True)
    reverse=directory/'reverse';compile_native(reverse,reverse=True)
    case=next(c for c in CASES if c[0]=='runtime-bool-regional');path=directory/(case[0]+'.input')
    actual=parse(command([reverse/'native',path,reverse/'input.dbmr',*case[-3:],reverse/'out.ll']))
    assert actual == expected(case,u)
    rejections={}
    checks=[('missing-intersection',lambda:coverage(u,('Plan','Destination'),METHODS[:-1])),
            ('coverage-work-limit',lambda:coverage(u,('Plan','Destination'),METHODS,budget=1)),
            ('ambiguous-discriminator-map',lambda:Universe(codes={name:17 for name in u.types})),
            ('unbound-generic',lambda:Universe([Type('GenericRoot',fields=(('value',0,'reference',('T',)),))])),
            ('missing-entry-installation',lambda:generate(directory/'rejected',install_entry=False))]
    for name,check in checks:
        try:check()
        except (ModelError,ValueError) as e:rejections[name]=str(e)
        else:raise AssertionError(name)
    alternate = Universe([replace(t,parents=('Scalar',)) if t.name == 'IntegerLiteral' else t for t in u.types.values()])
    assert alternate.type_ids['IntegerLiteral'] == u.type_ids['IntegerLiteral']
    assert u.subtype('IntegerLiteral','SourceLocated') and not alternate.subtype('IntegerLiteral','SourceLocated')
    assert u.layout('IntegerLiteral') == u.layout('BooleanLiteral') == u.layout('Meters') == u.layout('Seconds')
    assert len({u.type_ids[n] for n in ('IntegerLiteral','BooleanLiteral','Meters','Seconds')}) == 4
    path=directory/'mismatch.input';write_input(path,CASES[0],u,u.type_ids['BooleanLiteral'])
    r=subprocess.run(list(map(str,[directory/'native',path,directory/'bad.dbmr',0,0,0,directory/'bad.ll'])),capture_output=True,text=True)
    assert r.returncode==2 and r.stderr.strip()=='REJECT 921';rejections['tag-payload-mismatch']=r.stderr.strip()
    path.write_text('Unknown IntegerLiteral 6 LocalSlot 4 LocalSlot 2 1\n')
    r=subprocess.run(list(map(str,[directory/'native',path,directory/'unknown.dbmr',0,0,0,directory/'unknown.ll'])),capture_output=True,text=True)
    assert r.returncode==2 and r.stderr.strip()=='REJECT 911';rejections['unknown-type']=r.stderr.strip()
    path.write_text(f'Example Scalar 6 LocalSlot 4 LocalSlot 2 {u.type_ids["Scalar"]}\n')
    r=subprocess.run(list(map(str,[directory/'native',path,directory/'abstract.dbmr',0,0,0,directory/'abstract.ll'])),capture_output=True,text=True)
    assert r.returncode==2 and r.stderr.strip()=='REJECT 923';rejections['abstract-value-evidence']=r.stderr.strip()
    path.write_text(f'Example IntegerLiteral 1000001 LocalSlot 4 LocalSlot 2 {u.type_ids["IntegerLiteral"]}\n')
    r=subprocess.run(list(map(str,[directory/'native',path,directory/'range.dbmr',0,0,0,directory/'range.ll'])),capture_output=True,text=True)
    assert r.returncode==2 and r.stderr.strip()=='REJECT 924';rejections['fixture-numeric-range']=r.stderr.strip()
    # Independently prepared newer readers do not rewrite or reinterpret old tags.
    old_case=next(c for c in CASES if c[0]=='runtime-bool-regional');old_input=directory/(old_case[0]+'.input');saved=directory/'older-writer.dbmr'
    command([directory/'native',old_input,saved,0,0,0,directory/'unused.ll','write'])
    before=digest(saved)
    newer=directory/'newer-reader';new_info,new_command=compile_native(newer,profile='extended')
    assert all(new_info['type_ids'][name]==identity for name,identity in info['type_ids'].items())
    older_result=parse(command([directory/'native',old_input,saved,0,0,0,directory/'older-read.ll','read']))
    newer_result=parse(command([newer/'native',old_input,saved,0,0,0,newer/'read.ll','read']))
    assert newer_result==older_result==expected(old_case,u)
    assert digest(saved)==before
    wrong=directory/'wrong-reader';wrong_info,wrong_command=compile_native(wrong,profile='wrong')
    bad_read=subprocess.run(list(map(str,[wrong/'native',old_input,saved,0,0,0,wrong/'bad.ll','read'])),capture_output=True,text=True)
    assert bad_read.returncode==2 and bad_read.stderr.strip()=='REJECT 938'
    assert digest(saved)==before
    persistence={'older_writer_binary':digest(directory/'native'),'newer_reader_binary':digest(newer/'native'),
                 'writer_and_reader_differ':digest(directory/'native')!=digest(newer/'native'),
                 'reader_change':'reordered type declarations plus new unrelated concrete type',
                 'stored_sha256_before_and_after':before,'result':newer_result[0],
                 'interpretation_rejection':bad_read.stderr.strip(),
                 'newer_command':new_command,'wrong_reader_command':wrong_command,
                 'scope':'same supported root layouts; stable definition IDs and one root catalog, no tag/graph rewriting'}
    # Execute an actually emitted module independently of the dispatch host.
    harness=directory/'driver.cc';harness.write_text('extern "C" long long answer(); int main(){return answer()==313?0:1;}\n')
    # runtime-bool-regional = 255+9+3+40+5+1 = 313
    emission=[ROOT/'build/bin/clang++','-std=c++20',harness,directory/'runtime-bool-regional.ll','-o',directory/'emitted']
    command(emission);command([directory/'emitted'])
    info.pop('canonical_descriptors')
    return {'work_directory':str(directory),'commands':args,'generation':info,'observations':observations,
            'negative_controls':rejections,'method_order':'reversed declarations plus proven call-site selection elision preserve full trace and body count',
            'emitted_execution':313,'root_compatibility_planning':compatibility_observations(),'cross_world_persistence':persistence,
            'selected_conformance_control':'same concrete identity with changed externally selected conformance',
            'context':context_observations(),'generated_sha256':digest(directory/'generated.inc'),
            'sources':{p.name:digest(p) for p in HERE.iterdir() if p.suffix in ('.py','.cc','.hh')}}


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--output',type=Path,required=True);parser.add_argument('--work',type=Path)
    args=parser.parse_args();directory=args.work or Path(tempfile.mkdtemp(prefix='cod-micro-dispatch-'))
    directory.mkdir(parents=True,exist_ok=True)
    report=run(directory);args.output.write_text(json.dumps(report,indent=2)+'\n');print('REPORT',args.output)


if __name__=='__main__':main()
