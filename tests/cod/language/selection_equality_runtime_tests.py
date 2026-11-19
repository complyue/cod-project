#!/usr/bin/env python3
"""Identity guards and aliased calls preserve exact native resource release."""
from pathlib import Path
from native_test_support import run_native
import returned_view_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func same(ptr left:const u64 in O,ptr right:const u64 in O)->bool where {owner O}{return left is right;}
proc fill(ptr destination:u64 in O,ptr observed:const u64 in O)->void throws NullPointerException where {owner O}{
 destination<-85u64;observe(observed);
 return;
}
proc verify_choice(first:bool,second:bool)->void{
 val first_target<-make local u64,second_target<-make local u64;
 ptr left=if(first){first_target}else{second_target};
 ptr right=if(second){first_target}else{second_target};
 try{if(same(left,right)){fill(left,right);}else{observe(86u64);};}
 catch(failure:const NullPointerException){observe(99u64);};
 return;
}
verify_choice(false,false);verify_choice(false,true);
verify_choice(true,false);verify_choice(true,true);
''')
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['85', '86', '86', '85'])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
