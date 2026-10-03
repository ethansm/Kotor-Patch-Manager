"""M0 nested-capsule edit survival test on a COPY. arg1 = copied save folder, arg2 = scratch dir."""
import sys,os,shutil
from pykotor.resource.formats.erf import read_erf, bytes_erf
from pykotor.resource.formats.gff import read_gff, bytes_gff
from pykotor.resource.type import ResourceType
src,out=sys.argv[1],sys.argv[2]; os.makedirs(out,exist_ok=True)
raw=open(src+'/SAVEGAME.sav','rb').read()
outer=read_erf(raw)
def get(e,n,ext): return next(r for r in e if str(r.resref).lower()==n.lower() and r.restype.extension==ext)
mod=read_erf(get(outer,'203tel','sav').data)
git=read_gff(get(mod,'203tel','git').data)
atton=next(c for c in git.root.get_list('Creature List') if c.acquire('Tag','')=='atton')
print('before: XP',atton.acquire('Experience',0)); atton.set_uint32('Experience',99999)
mod.set_data('203tel',ResourceType.GIT,bytes_gff(git))
outer.set_data('203tel',ResourceType.SAV,bytes_erf(mod,ResourceType.SAV))
# also edit AVAILNPC0
a=read_gff(get(outer,'availnpc0','utc').data); a.root.set_uint32('Experience',99999)
outer.set_data('availnpc0',ResourceType.UTC,bytes_gff(a))
p=out+'/SAVEGAME_edit.sav'; open(p,'wb').write(bytes_erf(outer,ResourceType.SAV))
# re-read from disk
o2=read_erf(open(p,'rb').read()); m2=read_erf(get(o2,'203tel','sav').data); g2=read_gff(get(m2,'203tel','git').data)
x=next(c for c in g2.root.get_list('Creature List') if c.acquire('Tag','')=='atton').acquire('Experience',0)
y=read_gff(get(o2,'availnpc0','utc').data).root.acquire('Experience',0)
print('after reread: git Atton XP',x,'availnpc0 XP',y)
# untouched resources identical?
same=all(get(o2,str(r.resref),r.restype.extension).data==r.data for r in outer if str(r.resref) not in('203tel','availnpc0'))
print('other resources unchanged:',same,'| other nested resources unchanged:',all(get(m2,str(r.resref),r.restype.extension).data==r.data for r in mod if r.restype.extension!='git'))
# LazyCapsule.add path (patch #6 check): replace an existing nested resource in an on-disk copy
from pykotor.extract.capsule import LazyCapsule
shutil.copy(src+'/SAVEGAME.sav',out+'/lazy.sav')
cap=LazyCapsule(out+'/lazy.sav',create_nested_capsules=False) if 'create_nested_capsules' in LazyCapsule.__init__.__code__.co_varnames else LazyCapsule(out+'/lazy.sav')
cap.add('availnpc0',ResourceType.UTC,bytes_gff(a))
o3=read_erf(open(out+'/lazy.sav','rb').read())
print('LazyCapsule.add replace survives:',read_gff(get(o3,'availnpc0','utc').data).root.acquire('Experience',0)==99999,'n resources',len(o3))
