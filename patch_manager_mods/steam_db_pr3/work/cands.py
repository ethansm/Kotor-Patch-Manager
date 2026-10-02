import sys; sys.path.insert(0,'/home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/steam_db_pr3/work')
from lib import *
import csv,re,json
sf=steam_funcs(); gf=gog_funcs(); mp=mapper()
def stackbytes(notes):
    if not notes or notes=='void': return 0
    t=0
    for p in notes.split(','):
        p=p.strip()
        if not p: continue
        if re.search(r'\bthis\b',p) and '*' in p and 'param' not in p: continue
        if re.match(r'(undefined8|double|longlong|ulonglong|__int64)',p) : t+=8
        else: t+=4
    return t
g2s={g:(s,tag) for s,(g,n,tag,rt) in mp.items()}
gi={r[2]:i for i,r in enumerate(gf)}
def bracket(g):
    i=gi[g]; prev=next_=None
    for j in range(i-1,max(0,i-40),-1):
        if gf[j][2] in g2s: prev=(gf[j][2],)+g2s[gf[j][2]];break
    for j in range(i+1,min(len(gf),i+40)):
        if gf[j][2] in g2s: next_=(gf[j][2],)+g2s[gf[j][2]];break
    return prev,next_
if __name__=='__main__':
    R=list(csv.DictReader(open('metrics.tsv'),delimiter='\t'))
    out={}
    for r in R:
        if r['steam_cand']: continue
        g=hx(r['gog']); row=gf[gi[g]]
        exp=stackbytes(row[3])
        prev,nx=bracket(g)
        dp=prev[1]-prev[0] if prev else None; dn=nx[1]-nx[0] if nx else None
        st='none'; cand=None
        if dp is not None and dp==dn:
            s=g+dp
            if s in sf: cand=s; st='bracket_eq_delta'
        print(r['cls'],r['name'],hex(g),'exp',exp,'dp',hex(dp) if dp is not None else None,'dn',hex(dn) if dn is not None else None,'cand',hex(cand) if cand else None,'retn',sf[cand]['retn'] if cand else None,'OK' if cand and sf[cand]['retn']==exp else 'MISMATCH')
