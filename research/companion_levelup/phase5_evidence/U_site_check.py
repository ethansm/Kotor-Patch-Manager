# usage (repo root): .venv/bin/python research/companion_levelup/phase5_evidence/U_site_check.py 0x84D84B:6 ...
# For each site: vanilla bytes, live bytes, equality, and any hook (repo tomls, $T/Patches tomls, live patch_config.toml) within +-16 bytes.
import tomllib,os,sys,struct
G=os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/")
T=os.path.expanduser('~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager')
hooks=[]
def add(a,n,fn,src): hooks.append((a,n,fn,src))
seen=set()
for r in ['.',T+'/Patches']:
    for dp,dn,fn in os.walk(r):
        if any(x in dp for x in ('.venv','.git','node_modules')): continue
        for f in fn:
            if f.startswith('kotor2-steam-aspyr') and f.endswith('.hooks.toml'):
                p=os.path.join(dp,f)
                try: d=tomllib.load(open(p,'rb'))
                except Exception: continue
                for h in d.get('hooks',[]):
                    k=(h['address'],len(h.get('original_bytes',[])) or 5,h.get('function'),os.path.basename(os.path.dirname(p)))
                    if k in seen: continue
                    seen.add(k); add(k[0],k[1],k[2],p)
d=tomllib.load(open(G+'patch_config.toml','rb'))
for p in d['patches']:
    for h in p.get('hooks',[]): add(h['address'],len(h.get('original_bytes',[])) or 5,h.get('function'),'LIVE:'+p['id'])
def load(p):
    b=open(p,'rb').read(); pe=struct.unpack_from('<I',b,0x3c)[0]
    n=struct.unpack_from('<H',b,pe+6)[0]; opt=struct.unpack_from('<H',b,pe+20)[0]; base=struct.unpack_from('<I',b,pe+24+28)[0]
    o=pe+24+opt; secs=[]
    for i in range(n):
        vs,va,rs,rp=struct.unpack_from('<IIII',b,o+8); secs.append((va,max(vs,rs),rp)); o+=40
    return b,base,secs
def rd(t,va,n):
    b,base,secs=t
    for sva,sz,rp in secs:
        if sva<=va-base<sva+sz: return b[rp+va-base-sva:rp+va-base-sva+n]
V=load(G+'swkotor2.exe.unpatched_vanilla'); L=load(G+'swkotor2.exe')
for s in sys.argv[1:]:
    a,n=s.split(':'); a=int(a,16); n=int(n)
    v=rd(V,a,n); l=rd(L,a,n)
    ov=[(hex(h[0]),h[1],h[2],h[3].split('Patches/')[-1] if '/Patches/' in h[3] else h[3]) for h in hooks if h[0]<a+n+16 and h[0]+h[1]>a-16]
    print(f'0x{a:X}:{n} vanilla={v.hex(" ")} live={l.hex(" ")} {"SAME" if v==l else "DIFF"} overlap(+-16)={ov or "none"}')
