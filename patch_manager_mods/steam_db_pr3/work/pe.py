import struct
EXE='/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/swkotor2.exe.unpatched_vanilla'
class PE:
    def __init__(s,path=EXE):
        d=open(path,'rb').read(); s.d=d
        pe=struct.unpack_from('<I',d,0x3c)[0]
        n=struct.unpack_from('<H',d,pe+6)[0]; opt=struct.unpack_from('<H',d,pe+20)[0]
        s.base=struct.unpack_from('<I',d,pe+24+28)[0]
        so=pe+24+opt; s.secs=[]
        for i in range(n):
            name,vs,va,rs,rp=struct.unpack_from('<8sIIII',d,so+i*40)
            s.secs.append((name.rstrip(b'\0').decode(),va+s.base,vs,rp,rs))
    def off(s,va):
        for n,a,vs,rp,rs in s.secs:
            if a<=va<a+max(vs,rs): return rp+va-a
        return None
    def u32(s,va):
        o=s.off(va); return struct.unpack_from('<I',s.d,o)[0] if o is not None else None
    def bytes(s,va,n):
        o=s.off(va); return s.d[o:o+n]
