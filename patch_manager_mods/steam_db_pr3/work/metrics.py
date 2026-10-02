import sys; sys.path.insert(0,'/home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/steam_db_pr3/work')
from lib import *
import csv
inv_rows=[r for r in inv() if r['kind']=='F' and r['gog']]
sf=steam_funcs(); gf=gog_funcs(); mp=mapper(); nm=name_map()
gaddrs=[r[2] for r in gf]
gidx={r[2]:i for i,r in enumerate(gf)}
gby={(r[0],r[1]):r for r in gf}
saddrs=sorted(a for a,v in sf.items() if not v['name'].startswith('Unwind'))
sset=set(saddrs)
def snext(a):
    i=bisect.bisect_right(saddrs,a)
    return saddrs[i] if i<len(saddrs) else None
wl={}
for r in csv.DictReader(open(W+'wrapper_lookups.tsv'),delimiter='\t'):
    if r['kind']=='F': wl[(r['lookup_class'],r['name'])]=r
def align(g_i, delta, win=3):
    """count GOG neighbours (DB order) i-win..i+win whose g+delta is a Steam fn start with retn==param_size"""
    hit=tot=0
    for j in range(max(0,g_i-win),min(len(gf),g_i+win+1)):
        if j==g_i: continue
        c,n,a,notes,cc,ps=gf[j]
        if ps is None: continue
        tot+=1
        s=a+delta
        if s in sset and sf[s]['retn']==ps: hit+=1
    return hit,tot
def cc_guess(a):
    """thiscall if ECX is read before written in first ~12 insns"""
    L=asm(a)
    if not L: return '?'
    ecx_written=False
    for l in L[1:25]:
        parts=l.split(None,2)
        if len(parts)<3: continue
        ins=parts[2].upper()
        mnem=ins.split()[0]
        if 'ECX' in ins or 'CL' in ins.replace(',',' ').split() :
            ops=ins[len(mnem):].strip()
            dst=ops.split(',')[0]
            src=','.join(ops.split(',')[1:])
            if mnem in ('MOV','LEA','XOR','POP') and dst.strip() in ('ECX',) and 'ECX' not in src:
                ecx_written=True
                if mnem=='XOR' : pass
                continue
            if mnem=='XOR' and dst.strip()=='ECX' and src.strip()=='ECX': ecx_written=True; continue
            if not ecx_written and ('ECX' in src or (mnem in('PUSH',) and 'ECX' in dst) or 'ECX' in dst and mnem not in('MOV',)):
                return 'thiscall'
            if not ecx_written and 'ECX' in dst and mnem=='MOV' and 'ECX' not in src: ecx_written=True
        if mnem in ('RET','CALL') and not ecx_written: pass
    return 'nothis'
out=[]
hdrs=['kind','class','name','gog','gog_param','gog_cc','gog_gap','steam_cand','cand_src','conf_cands','steam_size','steam_retn','steam_gap','delta','ret_eq_gogparam','td_cc','td_param','ret_eq_td','cc_guess','cc_ok','align_hit','align_tot','mapper_tag','nm_conf','nm_src']
for r in inv_rows:
    cls,name=r['class'],r['name']
    g=gby[(cls,name)]
    gi=gidx[g[2]]
    gnext=gaddrs[gi+1] if gi+1<len(gaddrs) else None
    gap_g=gnext-g[2] if gnext else ''
    cands={}
    if r['steam_cand']: cands['name_map']=hx(r['steam_cand'])
    if r['mapper_cand']: cands['mapper']=hx(r['mapper_cand'])
    s=None; src=''
    if r['steam_cand'] and r['mapper_cand'] and r['steam_cand']!=r['mapper_cand']: src='CONFLICT'
    s=hx(r['steam_cand']) if r['steam_cand'] else (hx(r['mapper_cand']) if r['mapper_cand'] else None)
    row=dict(kind='F',cls=cls,name=name,gog=r['gog'],gog_param=g[5],gog_cc=g[4],gog_gap=gap_g,steam_cand=f'0x{s:08x}' if s else '',cand_src=src or ('+'.join(cands) if cands else 'none'),conf_cands=r['steam_cand']+'|'+r['mapper_cand'])
    t=wl.get((cls,name))
    row['td_cc']=t['cc'] if t else ''; row['td_param']=t['param_bytes'] if t else ''
    if s and s in sf:
        v=sf[s]; row['steam_size']=v['size']; row['steam_retn']=v['retn']
        n=snext(s); row['steam_gap']=n-s if n else ''
        row['delta']=hex(s-g[2])
        row['ret_eq_gogparam']= (v['retn']==g[5]) if g[5] is not None else ''
        if t and t['param_bytes']!='': row['ret_eq_td']= (v['retn']==int(t['param_bytes'])) if t['cc']!='__cdecl' else 'cdecl'
        else: row['ret_eq_td']=''
        cg=cc_guess(s); row['cc_guess']=cg
        row['cc_ok']= '' if not t else ('Y' if ((t['cc']=='__thiscall')==(cg=='thiscall')) else 'N')
        h,tt=align(gi,s-g[2]); row['align_hit']=h; row['align_tot']=tt
        row['mapper_tag']=r['mapper_tag']
        nmr=nm.get(s,[]); row['nm_conf']=';'.join(x[2] for x in nmr); row['nm_src']=';'.join(x[1][:25] for x in nmr)
    elif s:
        row['steam_size']='NOTFN'
    out.append(row)
cols=['kind','cls','name','gog','gog_param','gog_cc','gog_gap','steam_cand','cand_src','steam_size','steam_retn','steam_gap','delta','ret_eq_gogparam','td_cc','td_param','ret_eq_td','cc_guess','cc_ok','align_hit','align_tot','mapper_tag','nm_conf','nm_src','conf_cands']
with open('metrics.tsv','w') as f:
    f.write('\t'.join(cols)+'\n')
    for r in out: f.write('\t'.join(str(r.get(c,'')) for c in cols)+'\n')
print(len(out))
