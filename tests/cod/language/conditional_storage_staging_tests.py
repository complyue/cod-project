#!/usr/bin/env python3
"""Mixed scalar receiving preserves shared recursive code and spelling equivalence."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE = runner.SOURCE.replace(' if(n==0u64){', '''
 val local<-next;
 ref received=if(n%2u64==0u64){local}else{next+next-next};
 if(n==0u64){''')
runner.SPELLINGS = {name:(write.replace('next','received'),rebind)
                   for name,(write,rebind) in runner.SPELLINGS.items()}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
