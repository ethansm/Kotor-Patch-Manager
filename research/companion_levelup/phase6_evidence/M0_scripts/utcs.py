import sys
from pykotor.resource.formats.erf import read_erf
from pykotor.resource.formats.gff import read_gff
def fn(c):
    f=c.acquire('FirstName',None)
    try: return str(f.get(0)) if hasattr(f,'get') else str(f)
    except: return repr(f)
def summ(c):
    cls=c.acquire('ClassList',None) or []
    return dict(name=fn(c),tag=c.acquire('Tag',''),xp=c.acquire('Experience',None),cls=[(x.acquire('Class',None),x.acquire('ClassLevel',None)) for x in cls],
      hp=(c.acquire('CurrentHitPoints',None),c.acquire('MaxHitPoints',None),c.acquire('HitPoints',None)),fp=(c.acquire('CurrentForce',None),c.acquire('ForcePoints',None)),
      attrs=[c.acquire(k,None) for k in('Str','Dex','Con','Int','Wis','Cha')],
      nfeat=len(c.acquire('FeatList',None) or []), npow=sum(len(x.acquire('KnownList0',None) or []) for x in cls),
      skills=[x.acquire('Rank',None) for x in (c.acquire('SkillList',None) or [])],
      lvlstat=len(c.acquire('LvlStatList',None) or []))
if __name__=='__main__':
    outer=read_erf(sys.argv[1])
    for r in outer:
        if r.restype.extension=='utc':
            print(r.resref, summ(read_gff(r.data).root))
