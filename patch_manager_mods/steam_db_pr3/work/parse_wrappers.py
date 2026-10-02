#!/usr/bin/env python3
"""Parse GameAPI wrappers (read-only checkout) -> wrapper_lookups.tsv
one row per GameVersion lookup, in source order, grouped by (file, init function)."""
import re, os, sys, glob
G='/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-contributing/Patches/Common/GameAPI'
out=[]
tdre=re.compile(r'typedef\s+([\w\s\*:<>]+?)\s*\(\s*(__\w+)\s*\*\s*(\w+)\s*\)\s*\(([^)]*)\)\s*;')
def argbytes(a):
    a=a.strip()
    if not a or a=='void': return []
    parts=[p.strip() for p in a.split(',')]
    res=[]
    for p in parts:
        if re.search(r'\b(double|__int64|long long|LONGLONG|ULONGLONG|uint64_t|int64_t)\b',p) and '*' not in p: res.append(8)
        else: res.append(4)
    return res
for cpp in sorted(glob.glob(G+'/*.cpp')):
    base=os.path.basename(cpp)[:-4]
    h=cpp[:-4]+'.h'
    td={}
    for f in (h,cpp):
        if os.path.exists(f):
            for m in tdre.finditer(open(f,encoding='utf-8',errors='replace').read()):
                td[m.group(3)]=(m.group(1).strip(),m.group(2),m.group(4))
    src=open(cpp,encoding='utf-8',errors='replace').read()
    # split into init functions
    for m in re.finditer(r'void\s+(\w+)::(InitializeFunctions|InitializeOffsets)\s*\(\s*\)\s*\{',src):
        cls,fn=m.group(1),m.group(2)
        i=m.end(); depth=1
        while depth and i<len(src):
            if src[i]=='{':depth+=1
            elif src[i]=='}':depth-=1
            i+=1
        body=src[m.end():i]
        # base-class init calls
        bases=re.findall(r'\b(\w+)::(Initialize(?:Functions|Offsets))\s*\(\s*\)\s*;',body)
        pos=0
        for lm in re.finditer(r'GameVersion::(GetFunctionAddress|GetOffset|GetClassSize|GetClassVtable|GetGlobalPointer)\s*\(\s*"([^"]+)"(?:\s*,\s*"([^"]+)")?\s*\)',body):
            kind={'GetFunctionAddress':'F','GetOffset':'O','GetClassSize':'S','GetClassVtable':'V','GetGlobalPointer':'G'}[lm.group(1)]
            if kind=='G': cname,name='',lm.group(2)
            elif kind in 'SV': cname,name=lm.group(2),'<classes-table>'
            else: cname,name=lm.group(2),lm.group(3)
            tdn=ret=cc=args=nb=''
            if kind=='F':
                # find the cast type preceding this call
                pre=body[max(0,lm.start()-200):lm.start()]
                cm=re.findall(r'reinterpret_cast<\s*(\w+)\s*>\s*\(\s*$',pre)
                if cm:
                    tdn=cm[-1]
                    if tdn in td:
                        ret,cc,args=td[tdn]
                        ab=argbytes(args)
                        if cc=='__thiscall': ab=ab[1:]
                        nb=sum(ab)
                        args=' '.join(args.split())
            pos+=1
            out.append((base,cls,fn,pos,kind,cname,name,tdn,cc,ret,nb,args,';'.join(f'{a}::{b}' for a,b in bases)))
with open('wrapper_lookups.tsv','w') as f:
    f.write('\t'.join('file class initfn pos kind lookup_class name typedef cc ret param_bytes args base_inits'.split())+'\n')
    for r in out: f.write('\t'.join(map(str,r))+'\n')
print(len(out))
