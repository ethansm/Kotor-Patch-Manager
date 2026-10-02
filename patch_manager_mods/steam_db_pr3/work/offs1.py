import sys,re; sys.path.insert(0,'.')
from lib import *
mp=mapper(); sf=steam_funcs()
byclass={}
for s,(g,n,tag,rt) in mp.items():
    c,m=n.split('::',1); byclass.setdefault(c,[]).append((s,m,tag))
rows=[r for r in inv() if r['kind']=='O' and r['gog']]
for r in rows:
    c,m,o=r['class'],r['name'],hx(r['gog'])
    pat=re.compile(r'\+ (?:0x%x|%d)\)'%(o,o) if o else r'XXXX')
    hits=[]
    for s,mm,tag in byclass.get(c,[]):
        b=body(s)
        if b and pat.search(b): hits.append(mm)
    print(f"{c}.{m} +{o:#x}: classfuncs={len(byclass.get(c,[]))} hits={hits[:14]}")
