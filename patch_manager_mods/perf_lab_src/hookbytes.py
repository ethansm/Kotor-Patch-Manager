#!/usr/bin/env python3
# Verify hook sites against the static dump: python3 hookbytes.py ADDR[=EXPECTEDHEX] ...
# Prints the >=5 stolen bytes from the instruction boundary, OK/MISMATCH vs expected, and flags relative branches.
import bisect,glob,os,sys
R=os.path.expanduser('~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/')
fns=sorted(int(l.split('\t')[0],16) for l in open(R+'inputs/steam_functions.tsv').readlines()[1:])
def insns(a):
    f=fns[bisect.bisect_right(fns,a)-1]; p=R+'decomp/%02x/%08x.asm'%((f>>16)&0xff,f)
    out=[]
    for l in open(p).readlines()[1:]:
        s=l.split(None,2); out.append((int(s[0],16),s[1],s[2].strip() if len(s)>2 else ''))
    return out
for arg in sys.argv[1:]:
    a,exp=arg.split('=') if '=' in arg else (arg,'')
    a=int(a,16); ins=insns(a); i=[k for k,x in enumerate(ins) if x[0]==a]
    if not i: print('%08x NOT an instruction boundary'%a); continue
    got='';txt=[];k=i[0]
    need=2*max(5,len(exp.replace(' ',''))//2)
    while len(got)<need and k<len(ins):
        got+=ins[k][1]; txt.append(ins[k][2]); k+=1
    ok = '' if not exp else ('OK' if got.upper().startswith(exp.replace(' ','').upper()) else 'MISMATCH exp '+exp)
    rel=any((t.split()[0] in ('CALL','JMP') and '0x' in t and '[' not in t) or (t.startswith('J') and '[' not in t) for t in txt)
    short=' FN-END-SHORT' if len(got)<need else ''
    print('%08x %-26s %s | %s %s%s'%(a,got,ok,' ; '.join(txt),' REL-BRANCH-IN-STOLEN' if rel else '',short))
