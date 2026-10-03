"""R10: per-function call-target summary (E8 targets, vtable-indirect call count, strings pushed) for [lo,hi) using int3-padding function starts.
usage: R10_callmap.py LO HI"""
import sys,os,struct,re
sys.path.insert(0,os.path.dirname(__file__))
import R10_pe as P
lo=int(sys.argv[1],16); hi=int(sys.argv[2],16)
st=[va for va in range(lo,hi) if va%16==0 and P.rd(va-1,1)==b'\xcc' and P.rd(va,1)!=b'\xcc']+[hi]
strs={}
for nm,sva,sz,rp in P.secs:
    if nm in (b'.rdata',b'.data'):
        for m in re.finditer(rb'[\x20-\x7e]{3,}\x00',P.b[rp:rp+sz]): strs[sva+m.start()]=m.group()[:-1].decode()
for a,b in zip(st,st[1:]):
    code=P.rd(a,b-a); calls=[]; ss=[]
    for i in range(len(code)-5):
        if code[i]==0xE8:
            t=(a+i+5+struct.unpack_from('<i',code,i+1)[0])&0xffffffff
            if 0x400000<=t<0x8F0000+0x400000: calls.append('%X'%t)
        if code[i] in (0x68,) :
            v=struct.unpack_from('<I',code,i+1)[0]
            if v in strs: ss.append(strs[v])
    print('0x%X len=0x%X calls=%s strs=%s'%(a,b-a,' '.join(calls),ss[:6]))
