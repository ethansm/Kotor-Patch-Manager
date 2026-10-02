import sys; sys.path.insert(0,'/home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/steam_db_pr3/work')
from lib import *
from cands import stackbytes,bracket,g2s,gi,gf,sf,mp
import csv,re,os,collections,json
M={ (r['cls'],r['name']):r for r in csv.DictReader(open('metrics.tsv'),delimiter='\t')}
wl={}
for r in csv.DictReader(open('wrapper_lookups.tsv'),delimiter='\t'):
    if r['kind']=='F': wl[(r['lookup_class'],r['name'])]=r
nm=name_map()
# strings per function
strs=collections.defaultdict(list)
for r in csv.DictReader(open(DUMP+'strings.tsv'),delimiter='\t'):
    for fn in r['functions'].split(';'):
        if ':' in fn:
            a=fn.split(':')[0]
            try: strs[hx(a)].append(r['string'][:60])
            except: pass
def cname(a):
    if a in nm: 
        return nm[a][0][0][:50]
    if a in mp: return 'gog:'+mp[a][1]
    return ''
decl=re.compile(r'^\s+(?:unsigned\s+)?(?:int|uint|undefined\d?|char|float|short|ushort|byte|bool|code|longlong|ulonglong|double|undefined8)\s*\**\s*\**\w+(\[\d+\])?;\s*$')
def trimbody(a,maxl=45):
    b=body(a)
    if not b: return '(no decompile)'
    L=b.splitlines()
    # drop header comment
    L=[l for l in L[1:] if l.strip()!='' ]
    out=[]; 
    for l in L:
        if decl.match(l): continue
        out.append(l.rstrip())
    if len(out)>maxl: out=out[:maxl]+[f'... ({len(out)-maxl} more lines)']
    return '\n'.join(out)
def wrapper_decl(cls,name):
    t=wl.get((cls,name))
    if not t: return '(no wrapper typedef)'
    return f"{t['ret']} ({t['cc']}) ({t['args']}) [param_bytes={t['param_bytes']}]"
def final_cand(r):
    if r['steam_cand']: return hx(r['steam_cand']),'mapper' if 'mapper' in r['cand_src'] else r['cand_src']
    g=hx(r['gog']); row=gf[gi[g]]; exp=stackbytes(row[3])
    prev,nx=bracket(g)
    if prev and nx and prev[1]-prev[0]==nx[1]-nx[0]:
        s=g+prev[1]-prev[0]
        if s in sf and sf[s]['retn']==exp: return s,'bracket'
    return None,'none'
def digest(r):
    cls,name=r['cls'],r['name']
    g=hx(r['gog']); row=gf[gi[g]]
    s,src=final_cand(r)
    hd=f"### {cls}::{name}  gog={r['gog']} (param_bytes={row[5]}, notes='{row[3][:70]}')"
    if s is None:
        return hd+'\nNO CANDIDATE (bracketing failed) -- see manual list\n'
    h=hdr(s) or {}
    prev,nx=bracket(g)
    pn=f"prev-mapped={gf[gi[prev[0]]][1]} nxt-mapped={gf[gi[nx[0]]][1]}" if prev and nx else ''
    flags=[f"cand={s:#010x}({src})",f"delta={s-g:#x}",f"align={r['align_hit']}/{r['align_tot']}",f"size={sf[s]['size']}",f"retn={sf[s]['retn']}",f"gapG/S={r['gog_gap']}/{r['steam_gap']}",f"cc_guess={r['cc_guess']}"]
    lines=[hd,'steam: '+' '.join(flags),'wrapper: '+wrapper_decl(cls,name)]
    nms=nm.get(s)
    if nms: lines.append('name_map: '+' || '.join(f"{n} [{c}] {e[:120]}" for n,so,c,e in nms[:2]))
    cs=h.get('callees',[])
    lines.append(f"callers={len(h.get('callers',[]))} callees="+', '.join(f"{c:08x}"+(f"={cname(c)}" if cname(c) else '') for c in cs[:14])+(' ...' if len(cs)>14 else ''))
    if strs.get(s): lines.append('strings: '+' | '.join(strs[s][:6]))
    lines.append('```'); lines.append(trimbody(s)); lines.append('```')
    return '\n'.join(lines)+'\n'
if __name__=='__main__':
    os.makedirs('digests',exist_ok=True)
    groups={
     'A_CClientExoApp':['CClientExoApp','CServerExoApp'],
     'B_CGuiInGame':['CGuiInGame'],
     'C_CResGFF':['CResGFF'],
     'H_CRes_Timer':['CRes','CWorldTimer'],
     'D_Gob':['Gob'],
     'E_CSWCObject':['CSWCObject','CSWCDoor','CSWCPlaceable'],
     'F_Scene_gui':['Scene','CSWGuiBorder','CSWGuiBorderParams','CSWGuiControl','CSWGuiManager','CSWGuiPanel'],
     'G_CSWS':['CSWSObject','CSWSDoor','CSWSPlaceable','CSWSTrigger'],
    }
    for k,cl in groups.items():
        with open(f'digests/{k}.md','w') as f:
            f.write(f'# Digest {k}\n\n')
            rows=[r for r in M.values() if r['cls'] in cl]
            rows.sort(key=lambda r:(r['cls'],hx(r['gog'])))
            for r in rows: f.write(digest(r)+'\n')
        print(k,len(rows),os.path.getsize(f'digests/{k}.md'))
