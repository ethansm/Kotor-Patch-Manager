"""M0 round-trip stability test on COPIES (arg1 = copied save folder, arg2 = scratch out dir). Read-only on the originals."""
import sys,os,hashlib
from pykotor.resource.formats.erf import read_erf, write_erf, bytes_erf
from pykotor.resource.formats.gff import read_gff, write_gff, bytes_gff
from pykotor.resource.type import ResourceType
src,out=sys.argv[1],sys.argv[2]; os.makedirs(out,exist_ok=True)
H=lambda b:hashlib.md5(b).hexdigest()[:8]
raw=open(src+'/SAVEGAME.sav','rb').read()
e=read_erf(raw)
print('outer erf type',e.erf_type, 'n',len(e))
rt=bytes_erf(e,ResourceType.SAV) if True else None
print('outer: orig',len(raw),H(raw),'rewrite',len(rt),H(rt),'IDENTICAL' if rt==raw else 'DIFF')
e2=read_erf(rt)
diffs=[r.resref for r in e if {(x.resref,x.restype):x.data for x in e2}.get((r.resref,r.restype))!=r.data]
print('per-resource diffs after reread:',diffs)
if rt!=raw:
    # where do they first differ
    i=next((k for k in range(min(len(raw),len(rt))) if raw[k]!=rt[k]),None); print('first diff byte',i)
# nested module
for r in e:
    if r.restype.extension=='sav':
        inner=read_erf(r.data); b=bytes_erf(inner,ResourceType.SAV)
        i2=read_erf(b)
        ok=all({(x.resref,x.restype):x.data for x in i2}[(y.resref,y.restype)]==y.data for y in inner)
        print('nested',r.resref,'file',len(r.data),'->',len(b),'IDENTICAL' if b==r.data else 'DIFF','resources equal' if ok else 'RES DIFF')
# GFFs
def gtest(name,data):
    g=read_gff(data); b=bytes_gff(g,ResourceType.GFF if False else None) if False else bytes_gff(g)
    g2=read_gff(b); b2=bytes_gff(g2)
    print('gff',name,len(data),'->',len(b),'IDENTICAL' if b==data else 'DIFF','; 2nd pass stable' if b2==b else '; 2nd pass UNSTABLE')
for r in e:
    if r.restype.extension in('utc','res','fac') : gtest(str(r.resref)+'.'+r.restype.extension,r.data)
for f in('GLOBALVARS.res','PARTYTABLE.res','savenfo.res'): gtest(f,open(src+'/'+f,'rb').read())
