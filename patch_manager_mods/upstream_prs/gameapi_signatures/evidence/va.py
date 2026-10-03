import struct,sys
data=open('/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/64189a71-021c-4f66-8a9d-eebc5aa38044/scratchpad/swkotor2_steam_vanilla.exe','rb').read()
pe=struct.unpack_from('<I',data,0x3c)[0]
n=struct.unpack_from('<H',data,pe+6)[0]; opt=struct.unpack_from('<H',data,pe+20)[0]
secs=[]
for i in range(n):
    o=pe+24+opt+40*i
    vs,va,rs,ra=struct.unpack_from('<IIII',data,o+8)
    secs.append((va+0x400000,vs,ra,rs))
def rd(va,n):
    for s,vs,ra,rs in secs:
        if s<=va<s+max(vs,rs): return data[ra+va-s:ra+va-s+n]
def cstr(va):
    b=rd(va,64); return b.split(b'\0')[0]
if __name__=='__main__':
    for a in sys.argv[1:]: print(a,cstr(int(a,16)))
