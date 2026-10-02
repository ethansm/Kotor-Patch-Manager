#!/usr/bin/env python3
"""For every KPM steam-aspyr hook (all patches) overlapping a J0-o diff byte, report whether hook replacement == J0-o bytes."""
import glob,os,tomllib
from j0o_diff import *
root='/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches'
diffs=set(i for i in range(0x400,len(v)) if v[i]!=j[i])
for f in sorted(glob.glob(root+'/*/kotor2-steam-aspyr.hooks.toml')):
    try:h=tomllib.load(open(f,'rb'))
    except Exception as e: continue
    pid=f.split('/')[-2]
    for k in h.get('hooks',[]):
        a=k.get('address'); 
        if not isinstance(a,int) or 'replacement_bytes' not in k: continue
        rb=bytes(k['replacement_bytes']); ob=bytes(k.get('original_bytes',b'')); fo=va2f(a)
        if fo is None: continue
        ov=[i for i in range(fo,fo+max(len(rb),len(ob))) if i in diffs]
        if ov:
            same = j[fo:fo+len(rb)]==rb
            print(pid,hex(a),'len',len(rb),'overlaps',len(ov),'diff bytes; replacement == J0-o bytes:',same, ' orig==vanilla:',v[fo:fo+len(ob)]==ob)
            if not same:
                nd=sum(1 for x,y in zip(rb,j[fo:fo+len(rb)]) if x!=y); print('    differing bytes vs J0-o:',nd)
