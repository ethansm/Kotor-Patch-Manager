"""Compare a KPM stage dir's kpatch DLLs with the installed patches/<id>.dll of the live game.

Usage: python3 scripts/kpm_stage_check.py <stageDir> "<gameDir>"
Prints OK/DIFF per installed id and the DIFF total. Pick the stage with 0 DIFFS as the base for a new stage.
A `DIFF ... kpatch=None` line means the stage has no kpatch for that installed id.
"""
import sys,os,json,zipfile,hashlib,re
stage,game=sys.argv[1],sys.argv[2]
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
