import sys
from pykotor.resource.formats.erf import read_erf
from pykotor.resource.formats.gff import read_gff, GFFList, GFFStruct
def flat(s,p=''):
    o={}
    for l,t,v in s:
        k=p+'/'+l
        if isinstance(v,GFFList):
            o[k+'#len']=len(v)
            for i,x in enumerate(v): o.update(flat(x,f'{k}[{i}]'))
        elif isinstance(v,GFFStruct): o.update(flat(v,k))
        else: o[k]=repr(v)
    return o
outer=read_erf(sys.argv[1]); name=sys.argv[2]; tag=sys.argv[3]; av=sys.argv[4]
for r in outer:
    if r.resref==av and r.restype.extension=='utc': a=read_gff(r.data).root
    if str(r.resref).lower()==name.lower() and r.restype.extension=='sav': d=r.data
for r in read_erf(d):
    if r.restype.extension=='git':
        for c in read_gff(r.data).root.get_list('Creature List'):
            if c.acquire('Tag','')==tag: g=c
fa,fg=flat(a),flat(g)
print('fields avail',len(fa),'git',len(fg))
for k in sorted(set(fa)|set(fg)):
    if fa.get(k)!=fg.get(k): print(k,'AVAIL=',fa.get(k,'<absent>')[:70],'GIT=',fg.get(k,'<absent>')[:70])
