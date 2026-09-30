import struct,sys,subprocess
D="/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/"
def load(p):
    b=open(p,'rb').read(); pe=struct.unpack_from('<I',b,0x3c)[0]
    n=struct.unpack_from('<H',b,pe+6)[0]; opt=struct.unpack_from('<H',b,pe+20)[0]; base=struct.unpack_from('<I',b,pe+24+28)[0]
    secs=[]; o=pe+24+opt
    for i in range(n):
        name=b[o:o+8].rstrip(b'\0'); vs,va,rs,rp=struct.unpack_from('<IIII',b,o+8); secs.append((name,va,max(vs,rs),rp)); o+=40
    return b,base,secs
def rd(b,base,secs,va,n):
    rva=va-base
    for name,sva,sz,rp in secs:
        if sva<=rva<sva+sz: return b[rp+rva-sva:rp+rva-sva+n],rp+rva-sva
b,base,secs=load(D+""+(sys.argv.pop(1) if len(sys.argv)>1 and sys.argv[1].endswith("exe") else "swkotor2.exe.unpatched_vanilla")+"")
print(hex(base),[(s[0],hex(s[1])) for s in secs][:4])
for a in sys.argv[1:]:
    r,off=rd(b,base,secs,int(a,16),24); print(a,'file_off',hex(off),r.hex(' '))
