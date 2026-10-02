#!/usr/bin/env python3
"""Statement-level diff of vanilla vs J0-o vertex programs (names normalised) and fragment program statement lists."""
import re,difflib
from j0o_diff import *
def prog(buf,a):
    f=va2f(a); e=buf.find(b'\0',f); return buf[f:e].decode('latin1')
def stmts(t):
    t=t.replace('!!ARBvp1.0','').replace('!!ARBfp1.0','')
    t=t.replace('mvMatrix','mvM').replace('mvpMatrix','mvpM')
    out=[]
    for s in re.split(r'[;\n]',t):
        s=re.sub(r'\s+',' ',s.replace('\t',' ')).strip()
        s=re.sub(r' ?= ?','=',s) if s.startswith(('PARAM','ATTRIB','ALIAS')) else s
        s=re.sub(r' ?, ?',',',s)
        if s and s!='END': out.append(s)
    return out
for a in (0x9f7b80,0x9f7fc8,0x9f8580):
    vv=stmts(prog(v,a)); jj=stmts(prog(j,a))
    print('='*80); print('vertex program VA %#x: vanilla %d stmts, J0-o %d stmts'%(a,len(vv),len(jj)))
    for l in difflib.unified_diff(vv,jj,'vanilla','j0o',lineterm='',n=0):
        if not l.startswith(('---','+++','@@')): print(l)
