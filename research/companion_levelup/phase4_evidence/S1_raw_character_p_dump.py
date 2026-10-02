import sys,os
from pykotor.resource.formats.gff import read_gff
p=os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/override/character_p.gui")
g=read_gff(p)
r=g.root
print("ROOT keys:", [k for k,_,_ in r])
def val(s,k):
    try: return s.get_value(k) if hasattr(s,'get_value') else s.acquire(k,None)
    except Exception: return None
print("root tag",r.acquire("Obj_Tag" if False else "ID",None), "extent", r.acquire("EXTENT",None))
ctrls=r.acquire("CONTROLS",None)
def ext(s):
    e=s.acquire("EXTENT",None)
    return None if e is None else (e.acquire("LEFT",0),e.acquire("TOP",0),e.acquire("WIDTH",0),e.acquire("HEIGHT",0))
def walk(lst,depth=0):
    for s in lst:
        t=s.acquire("TAG","")
        ty=s.acquire("CONTROLTYPE",-1)
        i=s.acquire("ID",-1)
        text=s.acquire("TEXT",None)
        font=text.acquire("FONT",None) if text is not None else None
        strref=text.acquire("STRREF",None) if text is not None else None
        tx=text.acquire("TEXT","") if text is not None else None
        al=text.acquire("ALIGNMENT",None) if text is not None else None
        col=text.acquire("COLOR",None) if text is not None else None
        b=s.acquire("BORDER",None); bf=b.acquire("FILL",None) if b is not None else None
        fill=b.acquire("FILL",None) if b is not None else None
        print(f"{'  '*depth}id={i:<4} type={ty:<3} tag={t:<26} ext={ext(s)} font={font} strref={strref} text={tx!r} align={al} color={col} border_fill={'-' if b is None else (b.acquire('FILL',None) or '-')}")
        sub=s.acquire("CONTROLS",None)
        if sub: walk(sub,depth+1)
walk(ctrls)
