import sys; sys.path.insert(0,'.')
from lib import *
from cands import gf,gi,g2s,sf,mp
saddrs=sorted(a for a,v in sf.items() if not v['name'].startswith('Unwind'))
def near(g,k=6):
    i=gi[g]
    for j in range(max(0,i-k),min(len(gf),i+k+1)):
        r=gf[j]; s=g2s.get(r[2])
        mark='<<<' if j==i else ''
        print(f"  GOG {r[2]:#x} {r[0]}::{r[1]} p={r[5]} gap={gf[j+1][2]-r[2] if j+1<len(gf) else ''} -> "+(f"S {s[0]:#x} d={s[0]-r[2]:#x} ret={sf[s[0]]['retn']} sz={sf[s[0]]['size']}" if s else '-')+mark)
def ssteam(a,b):
    for s in saddrs:
        if a<=s<=b: print(f"  S {s:#x} sz={sf[s]['size']} ret={sf[s]['retn']}")
if __name__=='__main__':
    near(int(sys.argv[1],16),int(sys.argv[2]) if len(sys.argv)>2 else 6)
    if len(sys.argv)>4: ssteam(int(sys.argv[3],16),int(sys.argv[4],16))
