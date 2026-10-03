import tomllib,os,glob,itertools,sys
KPM='/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches'
LIVE='/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/patch_config.toml'
VER='6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF'
OURS={'ActivationFix','BuffDurationHUD','CharEffectsPanel','CompanionAI','EquipCharPreview','EquipIconFix','ForcePowerHotbar','Inventory3DViewport','InventorySortFilter','K2DialogueLetterboxFix','K2WrapMeasureFix','LevelUpOverride','LoadHangProbe','MouseProbe','PerfLab','RangedAI','RobustnessFixes','TextureMemoryRelease'}
live=tomllib.load(open(LIVE,'rb'))
liveids={p['id'] for p in live['patches']}
def manifest_id(d):
    return tomllib.load(open(f'{KPM}/{d}/manifest.toml','rb'))['patch']['id']
rng=[]  # (patch, addr, end, kind, fn)
skipped=[]
for d in sorted(os.listdir(KPM)):
    mp=f'{KPM}/{d}/manifest.toml'
    if not os.path.exists(mp): continue
    pid=manifest_id(d)
    f=f'{KPM}/{d}/kotor2-steam-aspyr.hooks.toml'
    if not os.path.exists(f): continue
    t=tomllib.load(open(f,'rb'))
    tv=t.get('metadata',{}).get('target_versions')
    if tv is not None and VER not in [x.upper() for x in tv]:
        skipped.append((d,'target_versions excludes vanilla 6A52...',tv)); continue
    for h in t.get('hooks',[]):
        a=h['address']; 
        if isinstance(a,str): a=int(a,0)
        n=len(h.get('original_bytes',[]))
        if n==0: skipped.append((d,'hook without original_bytes',hex(a)))
        rng.append((d,pid,a,a+max(n,1),h.get('type'),h.get('function','')))
print('hooks loaded:',len(rng),'from',len({r[0] for r in rng}),'patch dirs; skipped:',skipped)
def report(title,sel):
    rs=[r for r in rng if sel(r)]
    print(f'\n## {title}: {len(rs)} hooks')
    hits=[]
    for a,b in itertools.combinations(rs,2):
        if a[0]==b[0]: continue
        if a[2]<b[3] and b[2]<a[3]: hits.append((a,b,'OVERLAP'))
        else:
            gap=max(a[2]-b[3],b[2]-a[3])
            if gap<=8: hits.append((a,b,f'adjacent gap={gap}'))
    for a,b,k in hits:
        print(f'{k}: {a[0]} {a[2]:#x}-{a[3]:#x} ({a[4]} {a[5]})  <->  {b[0]} {b[2]:#x}-{b[3]:#x} ({b[4]} {b[5]}) live={a[1] in liveids and b[1] in liveids}')
    if not hits: print('none')
    return hits
report('ours vs ours, all sources',lambda r:r[0] in OURS)
report('ours+all installed (live ids) upstream',lambda r:r[1] in liveids)
report('ours vs ALL KPM patches (installed or not; steam toml)',lambda r:True)
# intra-patch overlaps
print('\n## intra-patch overlaps (same patch)')
byp={}
for r in rng: byp.setdefault(r[0],[]).append(r)
for p,rs in byp.items():
    for a,b in itertools.combinations(rs,2):
        if a[2]<b[3] and b[2]<a[3]: print('INTRA',p,hex(a[2]),hex(a[3]),a[5],'<->',hex(b[2]),hex(b[3]),b[5])
# live-config check
print('\n## LIVE patch_config.toml')
lr=[]
for p in live['patches']:
    for h in p.get('hooks',[]):
        lr.append((p['id'],h['address'],h['address']+max(len(h.get('original_bytes',[])),1),h.get('function','')))
print(len(lr),'hooks in',len(live['patches']),'patches')
n=0
for a,b in itertools.combinations(lr,2):
    if a[0]==b[0]: continue
    if a[1]<b[2] and b[1]<a[2]: print('LIVE OVERLAP',a,b); n+=1
    elif max(a[1]-b[2],b[1]-a[2])<=8: print('LIVE adjacent<=8',a[0],hex(a[1]),hex(a[2]),a[3],'|',b[0],hex(b[1]),hex(b[2]),b[3]); n+=1
print('live clashes:',n)
# source-vs-live drift for our mods
print('\n## our source hooks vs live config (same id)')
src={}
for r in rng:
    if r[0] in OURS: src.setdefault(r[1],set()).add((r[2],r[3]))
lv={}
for l in lr: lv.setdefault(l[0],set()).add((l[1],l[2]))
for pid in sorted(src):
    if pid in lv:
        print(pid,'src',len(src[pid]),'live',len(lv[pid]),'same' if src[pid]==lv[pid] else f'DIFF src-only={sorted(map(lambda x:hex(x[0]),src[pid]-lv[pid]))[:5]} live-only={sorted(map(lambda x:hex(x[0]),lv[pid]-src[pid]))[:5]}')
    else: print(pid,'src',len(src[pid]),'NOT live')
