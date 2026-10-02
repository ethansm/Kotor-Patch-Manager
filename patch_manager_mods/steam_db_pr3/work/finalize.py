import sys; sys.path.insert(0,'/home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/steam_db_pr3/work')
from lib import *
from cands import gf,gi,sf,stackbytes
from overrides import OV
import csv,glob,re,collections
inv_all=inv()
M={(r['cls'],r['name']):r for r in csv.DictReader(open('metrics.tsv'),delimiter='\t')}
R={}
for f in glob.glob('review/*.tsv'):
    for r in csv.DictReader(open(f),delimiter='\t'): R[(r['class'],r['name'])]=r
wl={}
for r in csv.DictReader(open('wrapper_lookups.tsv'),delimiter='\t'):
    if r['kind']=='F': wl[(r['lookup_class'],r['name'])]=r
from digest import final_cand

def reads_ecx_first(a):
    L=asm(a)
    if not L: return None
    written=False
    for l in L[1:30]:
        parts=l.split(None,2)
        if len(parts)<3: continue
        ins=parts[2].upper(); mn=ins.split()[0]; ops=ins[len(mn):].strip()
        dst=ops.split(',')[0].strip(); src=','.join(ops.split(',')[1:])
        if mn=='CALL': 
            return 'passthrough' if not written else 'nothis'
        if mn in('RET',): break
        uses_ecx='ECX' in src or ('ECX' in dst and mn in('PUSH',)) or (mn in ('ADD','SUB','CMP','TEST','AND','OR','INC','DEC','NEG') and 'ECX' in dst) or ('[ECX' in ops)
        if dst=='ECX' and mn in ('MOV','LEA','POP') and 'ECX' not in src: written=True; continue
        if mn=='XOR' and dst=='ECX' and src.strip()=='ECX': written=True; continue
        if uses_ecx and not written: return 'thiscall'
    return 'nothis'
def cc2(a):
    r=reads_ecx_first(a)
    if r=='thiscall': return 'thiscall'
    if r=='passthrough':
        # follow the first direct call target
        for l in asm(a)[1:30]:
            m=re.search(r'CALL 0x([0-9a-f]{8})',l)
            if m:
                t=int(m.group(1),16)
                if t in sf:
                    r2=reads_ecx_first(t)
                    if r2=='thiscall': return 'thiscall'
                break
        return 'stdcall' if sf[a]['retn']>0 else 'cdecl'
    return 'stdcall' if sf[a]['retn']>0 else 'cdecl'

rows=[]
for r in inv_all:
    if r['kind']!='F' or not r['gog']: continue
    k=(r['class'],r['name']); m=M[k]; g=hx(r['gog']); gp=gf[gi[g]][5]
    pos,src=final_cand(m)
    ov=OV.get(k); rv=R.get(k)
    d=dict(kind='F',cls=k[0],name=k[1],gog=r['gog'],gog_param=gp,cand_orig=f'0x{pos:08x}' if pos else '')
    d['wrapper_issue']=''
    if ov:
        d.update(final=ov['addr'],verdict=ov['verdict'],content=ov['content'],status=ov['status'],evidence=ov['note'])
    elif rv:
        a=int(rv['steam_addr'],16)
        d.update(final=a,content=rv['content'],status='confirmed' if rv['content']!='C0' else 'rejected',evidence=rv['reason'])
        if rv['content']=='C3': d['verdict']='verified'
        elif rv['content'] in ('C2','C1'): d['verdict']='likely'   # promoted below
        else: d['verdict']='rejected'; d['final']=None
    else:
        d.update(final=None,verdict='unresolved',content='',status='none',evidence='no candidate and no aligned bracket (bracketing failed: GOG/Steam run structure differs)')
    rows.append(d)
by={ (d['cls'],d['name']):d for d in rows}
# chain promotion for C2
gaddr2row={hx(d['gog']):d for d in rows}
for d in rows:
    if d['verdict']=='likely' and d['content']=='C2' and d['final'] and d['status']=='confirmed':
        g=hx(d['gog']); i=gi[g]; ok=0
        for j in (i-1,i+1):
            nb=gaddr2row.get(gf[j][2])
            if nb and nb['verdict']=='verified' and nb['final'] and (nb['final']-d['final'])==(gf[j][2]-g): ok+=1
        d['chain_ok']=ok
        if ok==2:
            d['verdict']='verified'; d['evidence']+=' | promoted: exact GOG gap on both sides to verified neighbours'
UPG={('CGuiInGame','GetCanClick'):'calls verified CWorldTimer::GetWorldTime + SubtractWorldTimes: click-delay gate on world-time difference vs [this+0xfd4]',
('CGuiInGame','ShowStatusSummary'):'calls verified CSWGuiManager::AddPanel + CClientExoApp::SetInputClass(2) on the status-summary panel [this+0xb8]',
('CGuiInGame','UpdateDialog'):'calls verified ClientToServerObjectId/GetGameObject/AppendToDialogBuffer in the per-frame dialog state machine (+0xcc,+0xe0,+0xe8)',
('CGuiInGame','ResetDialogAnimations'):'verified callees GetGameObject + CSWCObject::GetCurrentAnimation; loops object-id array [this+0x1c8] (freed in ShutDown) resetting anim to 10000',
('CGuiInGame','ResetTalkAnimations'):'verified callees GetGameObject + GetCurrentAnimation; resets exactly the two speaker/listener ids [this+0x198]/[this+0x19c] to anim 10000'}
for k,note in UPG.items():
    d=by.get(k)
    if d and d['verdict']=='likely' and d['status']=='confirmed':
        d['verdict']='verified'; d['evidence']+=' | upgraded by Opus review: '+note
for d in rows:
    a=d['final']
    d['steam_addr']=f'0x{a:08x}' if a else ''
    if a:
        d['steam_size']=sf[a]['size']; d['steam_retn']=sf[a]['retn']
        d['cc']=cc2(a)
        t=wl.get((d['cls'],d['name']))
        if t:
            d['wcc']=t['cc'].replace('__','')
            pb=int(t['param_bytes']) if t['param_bytes']!='' else None
            iss=[]
            harmless={'CreatureAcquireItem','GetCameraAnimationName','GetObjectScale','IsGeomFadeEnabled','SetGeomFadeProperties'}
            if d['wcc']!=d['cc']:
                iss.append(f"WRAPPER_CC(declares {d['wcc']}, Steam fn is a thiscall method"+(", this unused: harmless)" if d['name'] in harmless else ", reads this: real mismatch)"))
            byval=re.search(r'\b(Vector|Quaternion|double|__int64|ULONGLONG|DWORD64|uint64_t|LONGLONG)\b\s+\w+',t['args']) is not None
            if d['cc']!='cdecl' and pb is not None and pb!=sf[a]['retn'] and not byval:
                iss.append(f"WRAPPER_ARGS(typedef {pb}B vs retn {sf[a]['retn']})")
            d['wrapper_issue']=';'.join(iss)
        d['param']=sf[a]['retn'] if d['cc']!='cdecl' else d['gog_param']
    else:
        d['cc']=d['param']=''; d['steam_size']=d['steam_retn']=''
rows.append(dict(kind='F',cls='CSWGuiBorderParams',name='operator=',gog='0x9128c0',gog_param=4,cand_orig='0x004144f0',wrapper_issue='',final=0x4144f0,verdict='verified',content='C3',status='confirmed',steam_addr='0x004144f0',steam_size=390,steam_retn=4,cc='thiscall',param=4,
 evidence='EXTRA (missing from inventory: name has no identifier chars). Copy-assign: reads src resrefs +0x24/+0x34/+0x44 (FUN_00710ca0 x3), copies flags +0x20, +0xc, +0x10, +0x14..+0x1c, +4/+8, then SetBorder(+0x54) re-applies images; retn 4 (this,src); delta -0x4fe3d0 = neighbours (ctor 0x4143d0, SetCornerImage 0x414680); size 390 vs GOG gap 400'))
import json
json.dump(rows,open('final_rows.json','w'),default=str)
c=collections.Counter(d['verdict'] for d in rows); print(c)
print(collections.Counter((d['verdict'],d['status']) for d in rows))
print('cc',collections.Counter(d['cc'] for d in rows))
print('wrapper issues',sum(1 for d in rows if d['wrapper_issue']))
