#!/usr/bin/env python3
"""Reference-join proof transport preserves exact native resource release."""
from pathlib import Path
from native_test_support import run_native
import returned_view_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('return x;', 'return if(x is y){x}else{x};', 2))
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func choose(ptr left:const u64 in O,ptr right:const u64 in O,ref fallback:const u64 in O)->ptr const u64 in O where {owner O}{
 return if(left is right){null in O}else{fallback};
}
proc verify_choice(first:bool,second:bool)->void{
 val first_target<-make local u64,second_target<-make local u64,fallback<-9u64;
 ptr left=if(first){first_target}else{second_target};
 ptr right=if(second){first_target}else{second_target};
 ptr selected=choose(left,right,fallback);
 try{if(selected is null){left<-85u64;observe(right);}else{observe(86u64);};}
 catch(failure:const NullPointerException){observe(99u64);};
 return;
}
verify_choice(false,false);verify_choice(false,true);
verify_choice(true,false);verify_choice(true,true);
''')
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['85', '86', '86', '85'])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
