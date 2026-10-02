import tomllib,glob,os,sys,subprocess
roots=['.', os.path.expanduser('~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches')]
files=set()
for r in roots:
    for dp,dn,fn in os.walk(r):
        if '.venv' in dp or '.git' in dp or 'node_modules' in dp: continue
        for f in fn:
            if f.startswith('kotor2-steam-aspyr') and f.endswith('.hooks.toml'): files.add(os.path.join(dp,f))
hooks=[]  # addr,len,fn,owner(file)
seen=set()
for f in sorted(files):
    try: d=tomllib.load(open(f,'rb'))
    except Exception as e: print('PARSEERR',f,e); continue
    for h in d.get('hooks',[]):
        a=h['address']; n=len(h.get('original_bytes',[])) or 5
        key=(a,n,h.get('function'),os.path.basename(os.path.dirname(f)))
        if key in seen: continue
        seen.add(key); hooks.append((a,n,h.get('function'),f))
print('toml files',len(files),'unique hooks',len(hooks))
sites=[(int(s.split(':')[0],16),int(s.split(':')[1])) for s in sys.argv[1:]]
for a,n in sites:
    lo,hi=a-16,a+n+16
    ov=[h for h in hooks if h[0]<hi and h[0]+h[1]>lo]
    near=min(hooks,key=lambda h:abs(h[0]-a))
    print(f'0x{a:X} len{n}: overlap(+-16)={[(hex(h[0]),h[2],os.path.relpath(h[3]) if not h[3].startswith("/home/ethansm/Games") else h[3].split("Patches/")[1]) for h in ov]}  nearest=0x{near[0]:X} ({near[2]}, d={near[0]-a:+d}) in {near[3].split("Patches/")[-1]}')
