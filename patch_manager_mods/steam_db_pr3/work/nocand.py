import sys; sys.path.insert(0,'.')
from lib import *
import csv,re
sf=steam_funcs(); gf=gog_funcs(); mp=mapper()
saddrs=sorted(a for a,v in sf.items() if not v['name'].startswith('Unwind'))
def stackbytes(notes):
    if not notes or notes=='void': return 0
    t=0
    for p in notes.split(','):
        p=p.strip()
        if not p: continue
        if re.match(r'(undefined8|double|longlong|ulonglong|__int64)',p) : t+=8
        else: t+=4
    return t
g2s={}
for s,(g,n,tag,rt) in mp.items(): g2s[g]=(s,tag,n)
gi={r[2]:i for i,r in enumerate(gf)}
R=list(csv.DictReader(open('metrics.tsv'),delimiter='\t'))
res=[]
for r in R:
    if r['steam_cand']: continue
    g=hx(r['gog']); i=gi[g]; row=gf[i]
    exp=stackbytes(row[3]); 
    # nearest mapped neighbours
    prev=next_=None
    for j in range(i-1,max(0,i-40),-1):
        if gf[j][2] in g2s: prev=(gf[j],g2s[gf[j][2]]);break
    for j in range(i+1,min(len(gf),i+40)):
        if gf[j][2] in g2s: next_=(gf[j],g2s[gf[j][2]]);break
    out=[r['cls'],r['name'],r['gog'],'notes=%s'%row[3][:60],'exp_ret=%d'%exp]
    for tag,nb in (('prev',prev),('next',next_)):
        if nb: out.append(f"{tag}:{nb[0][1]} d={nb[1][0]-nb[0][2]:#x} {nb[1][1]}")
    d=[nb[1][0]-nb[0][2] for nb in (prev,next_) if nb]
    cand=None
    if d:
        for dd in set(d):
            s=g+dd
            if s in sf:
                out.append(f"cand@{dd:#x}={s:#x} retn={sf[s]['retn']} size={sf[s]['size']}")
    print(' | '.join(out))
