"""R10 static helper (read-only): callers / data refs / disasm over the Steam swkotor2.exe (vanilla copy by default).
usage (repo root): .venv/bin/python research/companion_levelup/phase6_evidence/R10_scripts/R10_pe.py calls 0x8480B0 [...]
                   ... dataref 0x987xxx ; ... dis 0x8F8070 [maxbytes] ; ... fnstart 0x90B500 ; ... vtab 0x9xxxxx N
"""
import struct,sys,os,bisect
D=os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/")
EXE=os.environ.get('R10_EXE','swkotor2.exe.unpatched_vanilla')
b=open(D+EXE,'rb').read(); pe=struct.unpack_from('<I',b,0x3c)[0]
n=struct.unpack_from('<H',b,pe+6)[0]; opt=struct.unpack_from('<H',b,pe+20)[0]; base=struct.unpack_from('<I',b,pe+24+28)[0]
secs=[]; o=pe+24+opt
for i in range(n):
    nm=b[o:o+8].rstrip(b'\0'); vs,va,rs,rp=struct.unpack_from('<IIII',b,o+8); secs.append((nm,va+base,max(vs,rs),rp)); o+=40
def off(va):
    for nm,sva,sz,rp in secs:
        if sva<=va<sva+sz: return rp+va-sva
def rd(va,k): o=off(va); return b[o:o+k]
text=[s for s in secs if s[0]==b'.text'][0]
def calls(t):
    out=[]; _,sva,sz,rp=text
    for i in range(sz-5):
        if b[rp+i] in (0xE8,0xE9):
            rel=struct.unpack_from('<i',b,rp+i+1)[0]
            if (sva+i+5+rel)&0xffffffff==t: out.append((sva+i,'call' if b[rp+i]==0xE8 else 'jmp'))
    return out
def datarefs(t):
    out=[]
    pat=struct.pack('<I',t)
    for nm,sva,sz,rp in secs:
        i=b.find(pat,rp,rp+sz)
        while i!=-1:
            out.append((sva+i-rp,nm.decode())); i=b.find(pat,i+1,rp+sz)
    return out
def starts():
    # candidate function starts: 16-aligned after int3/ret padding
    _,sva,sz,rp=text; s=[]
    for i in range(0,sz,16):
        if i==0 or b[rp+i-1] in (0xCC,0xC3,0xC2,0x90) or True:
            if b[rp+i:rp+i+3] in (b'\x55\x8b\xec',b'\x56\x8b\xf1',b'\x6a\xff\x68') or b[rp+i] in (0x53,0x83,0x81,0x8b,0x51,0x55,0x56,0x57,0x6a,0xa1,0x64,0xe9,0x33): s.append(sva+i)
    return s
ST=None
def fn_of(va):
    global ST
    if ST is None: ST=starts()
    k=bisect.bisect_right(ST,va)-1
    return ST[k] if k>=0 else None
if __name__=='__main__':
    c=sys.argv[1]
    if c=='calls':
        for a in sys.argv[2:]:
            t=int(a,16); print('== callers of',a)
            for va,k in calls(t): print('  0x%X %s in ~fn 0x%X'%(va,k,fn_of(va) or 0))
    elif c=='dataref':
        for a in sys.argv[2:]:
            t=int(a,16); print('== data refs to',a)
            for va,nm in datarefs(t): print('  0x%X (%s)'%(va,nm))
    elif c=='vtab':
        t=int(sys.argv[2],16); k=int(sys.argv[3])
        for i in range(k): print('  slot %d +0x%X = 0x%X'%(i,i*4,struct.unpack_from('<I',b,off(t+i*4))[0]))
    elif c=='dis':
        import capstone
        md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
        t=int(sys.argv[2],16); k=int(sys.argv[3],0) if len(sys.argv)>3 else 0x400
        for ins in md.disasm(rd(t,k),t): print('%08x  %-20s %s %s'%(ins.address,ins.bytes.hex(' '),ins.mnemonic,ins.op_str))
# extra command: fnlist lo hi  -> 16-aligned addresses preceded by int3 padding (likely function starts)
if __name__=='__main__' and sys.argv[1]=='fnlist':
    lo=int(sys.argv[2],16); hi=int(sys.argv[3],16)
    for va in range(lo,hi):
        if va%16==0 and rd(va-1,1)==b'\xcc' and rd(va,1)!=b'\xcc': print('  0x%X  %s'%(va,rd(va,4).hex(' ')))
if __name__=='__main__' and sys.argv[1]=='dataref2':
    pass
