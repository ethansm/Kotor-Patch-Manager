import sys,glob,os
from pykotor.resource.formats.gff import read_gff
S=os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/cloudsaves/62703561")
def names(g): return [x.acquire('Name','') for x in g.root.get_list('CatBoolean')]
def decode(path,order):
    g=read_gff(open(path,'rb').read()); n=names(g); vb=g.root.acquire('ValBoolean',b'')
    def bit(i):
        b=vb[i//8]; return (b>>(i%8))&1 if order=='lsb' else (b>>(7-i%8))&1
    idx={nm:i for i,nm in enumerate(n)}
    luo=[idx.get('LUO_G%02d'%k) for k in range(32)]
    if any(i is None for i in luo): return None,len(n),len(vb),luo[:3]
    v=0
    for k,i in enumerate(luo):
        if bit(i): v|=1<<k
    return v,len(n),len(vb),luo
for d in sorted(glob.glob(S+'/0*')):
    r=[decode(d+'/GLOBALVARS.res',o) for o in('lsb','msb')]
    print(os.path.basename(d), 'ncat',r[0][1],'bytes',r[0][2],'LUO positions',r[0][3][:4] if r[0][0] is not None else 'absent', 'lsb', ('g%08x'%r[0][0]) if r[0][0] is not None else '-', 'msb', ('g%08x'%r[1][0]) if r[1][0] is not None else '-')
