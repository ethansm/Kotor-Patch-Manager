"""R10: find ASCII strings matching a regex in the exe and the code locations that push/mov their address.
usage: R10_strxref.py REGEX [lo hi]   (hex range restricts referencing code)"""
import sys,re,os
sys.argv=[sys.argv[0]]+sys.argv[1:]
sys.path.insert(0,os.path.dirname(__file__))
import R10_pe as P
rx=re.compile(sys.argv[1].encode()); lo=int(sys.argv[2],16) if len(sys.argv)>2 else 0; hi=int(sys.argv[3],16) if len(sys.argv)>3 else 1<<32
for nm,sva,sz,rp in P.secs:
    if nm not in (b'.rdata',b'.data'): continue
    for m in re.finditer(rb'[\x20-\x7e]{3,}\x00',P.b[rp:rp+sz]):
        s=m.group()[:-1]
        if rx.search(s):
            va=sva+m.start()
            refs=[(a,n) for a,n in P.datarefs(va)]
            refs=[r for r in refs if lo<=r[0]<hi]
            print('0x%X %r'%(va,s.decode()), ' '.join('0x%X'%a for a,_ in refs))
