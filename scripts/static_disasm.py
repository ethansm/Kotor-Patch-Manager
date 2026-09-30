import struct,os,sys,capstone
d=open(os.environ['GAME']+'/swkotor2.exe','rb').read()
base=0x400000
def off(a):
    for lo,hi,rp in [(0x401000,0x986000,0x1000)]:
        pass
    pe=struct.unpack_from('<I',d,0x3c)[0]; n=struct.unpack_from('<H',d,pe+6)[0]; opt=struct.unpack_from('<H',d,pe+20)[0]
    for i in range(n):
        o=pe+24+opt+40*i; vs,v,rs,rp=struct.unpack_from('<IIII',d,o+8)
        if base+v<=a<base+v+max(rs,vs): return rp+a-base-v
def cstr(a):
    o=off(a)
    if o is None: return None
    e=d.index(b'\0',o); s=d[o:e]
    return s.decode('latin1') if 3<=len(s)<60 and all(32<=c<127 for c in s) else None
md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
def dis(a,b,annot=True):
    o=off(a)
    for i in md.disasm(d[o:o+(b-a)],a):
        t=f'{i.address:08x} {i.mnemonic} {i.op_str}'
        if annot and i.mnemonic in('push','mov') and i.op_str.startswith(('0x9','0x8')) or (i.mnemonic=='push' and i.op_str.startswith('0x9')):
            try:
                s=cstr(int(i.op_str.split()[-1],16)); t+= f'   ; "{s}"' if s else ''
            except: pass
        print(t)
if __name__=='__main__':
    dis(int(sys.argv[1],16),int(sys.argv[2],16))
