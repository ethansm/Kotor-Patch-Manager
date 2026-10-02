#!/usr/bin/env python3
"""Diff vanilla vs J0-o 3C-FD swkotor2.exe; group diffs by NUL-delimited string / shader program;
compare with KPM k2_aspyr_shader_fixes hooks. Read-only."""
import struct, sys, tomllib, json
V='/home/ethansm/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/swkotor2.exe'
J='/home/ethansm/Games/KOTOR2_Mod_Workspace/backups_vanilla_migration_1002_1202/swkotor2.exe'
KPM='/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/K2AspyrShaderFixes/kotor2-steam-aspyr.hooks.toml'
v=open(V,'rb').read(); j=open(J,'rb').read()
assert len(v)==len(j)
pe=struct.unpack_from('<I',v,0x3c)[0]
nsec=struct.unpack_from('<H',v,pe+6)[0]; opt=struct.unpack_from('<H',v,pe+20)[0]
secs=[]
o=pe+24+opt
for i in range(nsec):
    n=v[o:o+8].rstrip(b'\0').decode(); vs,va,rs,ro=struct.unpack_from('<IIII',v,o+8)
    secs.append((n,va,vs,ro,rs)); o+=40
def f2va(f):
    for n,va,vs,ro,rs in secs:
        if ro<=f<ro+rs: return 0x400000+va+f-ro
def va2f(a):
    for n,va,vs,ro,rs in secs:
        r=a-0x400000
        if va<=r<va+max(vs,rs): return ro+r-va
def secname(a):
    for n,va,vs,ro,rs in secs:
        r=a-0x400000
        if va<=r<va+max(vs,rs): return n
if __name__=='__main__':
    for s in secs: print(s[0],hex(0x400000+s[1]),hex(s[2]),hex(s[3]))
    diffs=[i for i in range(len(v)) if v[i]!=j[i]]
    print(len(diffs),'diff bytes')
    # group into strings: find string start (after preceding NUL, in vanilla) for each diff byte
    strs={}
    for i in diffs:
        if i<0x400: print('hdr diff',hex(i),v[i],j[i]); continue
        a=f2va(i)
        s=v.rfind(b'\0',0,i)+1
        # shader programs begin with '!!ARB'
        p=v.rfind(b'!!ARB',0,i+1)
        if p!=-1 and v.find(b'\0',p)>=i: s=p
        e=v.find(b'\0',i)
        strs.setdefault((s,e),[]).append(i)
    print(len(strs),'distinct strings/regions')
    for (s,e),d in sorted(strs.items()):
        print(hex(f2va(s)),secname(f2va(s)),'len',e-s,'ndiff',len(d), 'jlen', j.find(b'\0',s)-s)
