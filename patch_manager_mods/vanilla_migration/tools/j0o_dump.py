#!/usr/bin/env python3
"""Dump each J0-o changed region (vanilla vs J0-o text) in full, with VA, length, trailing bytes."""
from j0o_diff import *
import difflib
diffs=[i for i in range(0x400,len(v)) if v[i]!=j[i]]
# merge into clusters (gap<=300) then extend to program start
cl=[]
for i in diffs:
    if cl and i-cl[-1][1]<=300: cl[-1][1]=i
    else: cl.append([i,i])
out=[]
for a,b in cl:
    s=v.rfind(b'!!ARB',0,a+1)
    if s==-1 or a-s>4000: s=a
    # end: extend until both have NUL after b
    e=b+1
    while v[e:e+1]!=b'\0' or j[e:e+1]!=b'\0': e+=1
    out.append((s,e))
for s,e in out:
    print('='*100); print('VA %#x..%#x  file %#x  sec %s  vanilla/J0-o region len %d'%(f2va(s),f2va(e),s,secname(f2va(s)),e-s))
    vt=v[s:e].decode('latin1'); jt=j[s:e].decode('latin1')
    print('--- vanilla\n'+vt.replace('\0','<NUL>')); print('--- J0-o\n'+jt.replace('\0','<NUL>'))
    print('--- diff (line)')
    for l in difflib.unified_diff(vt.splitlines(),jt.splitlines(),'vanilla','j0o',lineterm='',n=0): print(l)
