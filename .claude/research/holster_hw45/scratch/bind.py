import sys
sys.path.insert(0,'.')
import reach_angles as R
from collections import defaultdict
rows = R.load(sys.argv[1])
d = defaultdict(lambda:[0,0,0,0,0])
for r in rows:
    c = R.CATS.get(r['cat'], str(r['cat'])); x = d[c]
    x[0]+=1
    tb = abs(r['tw'])>=79.0; sb = abs(r['sw'])>=59.0
    x[1]+= tb; x[2]+= sb; x[3]+= (tb and sb); x[4]+= (r['g']>=170)
print("cat n twist_at_cap swing_at_cap both_at_cap g>=170")
for c,x in sorted(d.items()): print(c,*x)
