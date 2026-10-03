"""Compare a KPM stage dir's kpatch DLLs with the installed patches/<id>.dll of the live game.

Usage: python3 scripts/kpm_stage_check.py <stageDir> "<gameDir>"
Prints OK/DIFF per installed id and the DIFF total. Pick the stage with 0 DIFFS as the base for a new stage.
A `DIFF ... kpatch=None` line means the stage has no kpatch for that installed id.
Then prints pre-install warnings (exit stays 0): `WARN orphan dll <id>.dll` (in patches/ but not in
InstalledPatches: the installer deletes it) and `WARN config id <id>` (in patch_config.toml, no kpatch in the stage:
the installer drops it).

Post-install safety: python3 scripts/kpm_stage_check.py --post <backupDir> "<gameDir>" [--allow <id> ...]
<backupDir> mirrors the game dir (patches/, patch_config.toml), taken BEFORE the install. Compares every file under
patches/ (DIFF md5 / MISSING / NEW) and patch_config.toml per patch id (CONFIG <id> added/removed/changed; entry =
dll + hooks, order ignored). Differences whose id is in --allow print as ALLOWED and are not counted.
Ends with `POST DIFFS n (allowed m)`; exit 1 on any non-allowed diff or if the backup has no patch_config.toml.
"""
import sys,os,json,zipfile,hashlib,re,tomllib
md5=lambda p:hashlib.md5(open(p,'rb').read()).hexdigest()

def config_entries(path):
    """patch_config.toml -> {id: (dll, sorted canonical hook strings)}; ordering ignored."""
    c=tomllib.load(open(path,'rb')); out={}
    for p in c.get('patches',[]):
        out[p['id']]=(p.get('dll'),sorted(json.dumps(h,sort_keys=True) for h in p.get('hooks',[])))
    return out

def describe(a,b):
    ha,hb=set(a[1]),set(b[1]); r=[]
    if a[0]!=b[0]: r.append(f'dll {a[0]}->{b[0]}')
    if ha!=hb: r.append(f'hooks -{len(ha-hb)} +{len(hb-ha)} ({len(a[1])}->{len(b[1])})')
    return ', '.join(r)

def files_under(d):
    return {f for f in os.listdir(d)} if os.path.isdir(d) else set()

def post(bak,game,allow):
    n=ok=0; out=[]
    def rec(pid,line):
        nonlocal n,ok
        a=any(pid==x or pid.startswith(x+'.') for x in allow)
        out.append(('ALLOWED ' if a else '')+line); ok+=a; n+=not a
    bp,gp=os.path.join(bak,'patches'),os.path.join(game,'patches')
    for f in sorted(files_under(bp)|files_under(gp)):
        a,b=os.path.join(bp,f),os.path.join(gp,f)
        if not os.path.exists(b): rec(f,f'MISSING patches/{f}')
        elif not os.path.exists(a): rec(f,f'NEW patches/{f}')
        elif md5(a)!=md5(b): rec(f,f'DIFF patches/{f} md5 {md5(a)}->{md5(b)}')
    bc,gc=os.path.join(bak,'patch_config.toml'),os.path.join(game,'patch_config.toml')
    if not os.path.exists(bc):
        out.append('WARN backup has no patch_config.toml'); n+=1
    elif not os.path.exists(gc):
        out.append('WARN game has no patch_config.toml'); n+=1
    else:
        a,b=config_entries(bc),config_entries(gc)
        for i in sorted(set(a)|set(b)):
            if i not in b: rec(i,f'CONFIG {i} removed')
            elif i not in a: rec(i,f'CONFIG {i} added')
            elif a[i]!=b[i]: rec(i,f'CONFIG {i} changed: {describe(a[i],b[i])}')
    print('\n'.join(out)); print(f'POST DIFFS {n} (allowed {ok})')
    return 1 if n else 0

def pre_warnings(game,ids,km):
    for f in sorted(files_under(os.path.join(game,'patches'))):
        if f.endswith('.dll') and f[:-4] not in ids:
            print(f'WARN orphan dll {f}: not in InstalledPatches, the installer will delete it')
    gc=os.path.join(game,'patch_config.toml')
    if os.path.exists(gc):
        for i in config_entries(gc):
            if i not in km: print(f'WARN config id {i} has no kpatch in the stage: the installer will drop it')

def main():
    a=sys.argv[1:]
    if a and a[0]=='--post':
        allow=[a[i+1] for i in range(3,len(a)-1) if a[i]=='--allow']
        return post(a[1],a[2],allow)
    stage,game=a[0],a[1]
    st=json.load(open(os.path.join(game,'kpm_install_state.json')))
    ids=[p if isinstance(p,str) else (p.get('Id') or p.get('PatchId')) for p in st.get('InstalledPatches',[])]
    print('installed ids:',len(ids)); km={}
    for f in os.listdir(stage):
        if not f.endswith('.kpatch'): continue
        z=zipfile.ZipFile(os.path.join(stage,f)); m=z.read('manifest.toml').decode()
        pid=re.search(r'^\s*id\s*=\s*"([^"]+)"',m,re.M).group(1)
        dll=hashlib.md5(z.read('binaries/windows_x86.dll')).hexdigest() if 'binaries/windows_x86.dll' in z.namelist() else None
        km[pid]=(f,dll)
    bad=0
    for i in ids:
        p=os.path.join(game,'patches',i+'.dll'); inst=hashlib.md5(open(p,'rb').read()).hexdigest() if os.path.exists(p) else None
        k=km.get(i); s='OK' if k and k[1]==inst else 'DIFF'
        if s!='OK': bad+=1
        print(f'{s:4} {i:32} kpatch={k[1][:8] if k and k[1] else k} installed={inst[:8] if inst else None}')
    print('extra kpatches not installed:',[k for k in km if k not in ids]); print('DIFFS',bad)
    pre_warnings(game,ids,km)
    return 0


if __name__=='__main__': sys.exit(main())
